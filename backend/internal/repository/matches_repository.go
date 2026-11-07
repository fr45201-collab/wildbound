package repository

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/database"
)

var (
	ErrMatchNotFound = errors.New("match record not found")
)

type MatchesRepository interface {
	CreateMatchRecord(ctx context.Context, match *MatchRecord, participants []MatchParticipant) error
	GetMatchByID(ctx context.Context, matchID string) (*MatchRecord, []MatchParticipant, error)
	ListPlayerMatches(ctx context.Context, userID uuid.UUID, limit, offset int) ([]MatchSummary, error)
	CheckMatchExists(ctx context.Context, matchID string) (bool, error)
}

type matchesRepo struct {
	pool *pgxpool.Pool
}

func NewMatchesRepository(pool *pgxpool.Pool) MatchesRepository {
	return &matchesRepo{pool: pool}
}

func (r *matchesRepo) CheckMatchExists(ctx context.Context, matchID string) (bool, error) {
	query := `SELECT EXISTS(SELECT 1 FROM match_records WHERE match_id = $1)`
	var exists bool
	err := r.pool.QueryRow(ctx, query, matchID).Scan(&exists)
	if err != nil {
		return false, fmt.Errorf("failed to check match existence: %w", err)
	}
	return exists, nil
}

func (r *matchesRepo) CreateMatchRecord(ctx context.Context, match *MatchRecord, participants []MatchParticipant) error {
	return database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		insertMatchQuery := `
			INSERT INTO match_records (
				match_id, game_mode, map_id, started_at, ended_at,
				duration_seconds, participant_count, winner_user_id,
				winner_display_name, result_status
			) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
			RETURNING id, created_at
		`
		err := tx.QueryRow(ctx, insertMatchQuery,
			match.MatchID, match.GameMode, match.MapID, match.StartedAt, match.EndedAt,
			match.DurationSeconds, match.ParticipantCount, match.WinnerUserID,
			match.WinnerDisplayName, match.ResultStatus,
		).Scan(&match.ID, &match.CreatedAt)
		if err != nil {
			return fmt.Errorf("failed to insert match record: %w", err)
		}

		insertParticipantQuery := `
			INSERT INTO match_participants (
				match_record_id, user_id, participant_display_name, team_id,
				placement, eliminations, deaths, assists,
				damage_dealt, damage_received, survival_seconds, result_status
			) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)
			RETURNING id
		`
		for i := range participants {
			p := &participants[i]
			p.MatchRecordID = match.ID
			err := tx.QueryRow(ctx, insertParticipantQuery,
				p.MatchRecordID, p.UserID, p.ParticipantDisplayName, p.TeamID,
				p.Placement, p.Eliminations, p.Deaths, p.Assists,
				p.DamageDealt, p.DamageReceived, p.SurvivalSeconds, p.ResultStatus,
			).Scan(&p.ID)
			if err != nil {
				return fmt.Errorf("failed to insert match participant: %w", err)
			}
		}

		return nil
	})
}

func (r *matchesRepo) GetMatchByID(ctx context.Context, identifier string) (*MatchRecord, []MatchParticipant, error) {
	// Identifier can be either UUID primary key or external match_id string
	matchQuery := `
		SELECT id, match_id, game_mode, map_id, started_at, ended_at,
		       duration_seconds, participant_count, winner_user_id,
		       winner_display_name, result_status, created_at
		FROM match_records
		WHERE match_id = $1
	`
	var m MatchRecord
	var err error

	if parsedUUID, parseErr := uuid.Parse(identifier); parseErr == nil {
		uuidQuery := `
			SELECT id, match_id, game_mode, map_id, started_at, ended_at,
			       duration_seconds, participant_count, winner_user_id,
			       winner_display_name, result_status, created_at
			FROM match_records
			WHERE id = $1 OR match_id = $2
		`
		err = r.pool.QueryRow(ctx, uuidQuery, parsedUUID, identifier).Scan(
			&m.ID, &m.MatchID, &m.GameMode, &m.MapID, &m.StartedAt, &m.EndedAt,
			&m.DurationSeconds, &m.ParticipantCount, &m.WinnerUserID,
			&m.WinnerDisplayName, &m.ResultStatus, &m.CreatedAt,
		)
	} else {
		err = r.pool.QueryRow(ctx, matchQuery, identifier).Scan(
			&m.ID, &m.MatchID, &m.GameMode, &m.MapID, &m.StartedAt, &m.EndedAt,
			&m.DurationSeconds, &m.ParticipantCount, &m.WinnerUserID,
			&m.WinnerDisplayName, &m.ResultStatus, &m.CreatedAt,
		)
	}

	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, nil, ErrMatchNotFound
		}
		return nil, nil, fmt.Errorf("failed to query match record: %w", err)
	}

	participantsQuery := `
		SELECT id, match_record_id, user_id, participant_display_name, team_id,
		       placement, eliminations, deaths, assists,
		       damage_dealt, damage_received, survival_seconds, result_status
		FROM match_participants
		WHERE match_record_id = $1
		ORDER BY placement ASC
	`
	rows, err := r.pool.Query(ctx, participantsQuery, m.ID)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to query participants: %w", err)
	}
	defer rows.Close()

	var participants []MatchParticipant
	for rows.Next() {
		var p MatchParticipant
		if err := rows.Scan(
			&p.ID, &p.MatchRecordID, &p.UserID, &p.ParticipantDisplayName, &p.TeamID,
			&p.Placement, &p.Eliminations, &p.Deaths, &p.Assists,
			&p.DamageDealt, &p.DamageReceived, &p.SurvivalSeconds, &p.ResultStatus,
		); err != nil {
			return nil, nil, fmt.Errorf("failed to scan participant: %w", err)
		}
		participants = append(participants, p)
	}

	return &m, participants, rows.Err()
}

func (r *matchesRepo) ListPlayerMatches(ctx context.Context, userID uuid.UUID, limit, offset int) ([]MatchSummary, error) {
	if limit <= 0 || limit > 100 {
		limit = 20
	}
	if offset < 0 {
		offset = 0
	}

	query := `
		SELECT mr.match_id, mr.game_mode, mr.map_id, mr.started_at, mr.duration_seconds,
		       mp.placement, mp.eliminations, mp.damage_dealt, mp.result_status
		FROM match_participants mp
		JOIN match_records mr ON mr.id = mp.match_record_id
		WHERE mp.user_id = $1
		ORDER BY mr.started_at DESC
		LIMIT $2 OFFSET $3
	`
	rows, err := r.pool.Query(ctx, query, userID, limit, offset)
	if err != nil {
		return nil, fmt.Errorf("failed to query player matches: %w", err)
	}
	defer rows.Close()

	var list []MatchSummary
	for rows.Next() {
		var s MatchSummary
		if err := rows.Scan(
			&s.MatchID, &s.GameMode, &s.MapID, &s.StartedAt, &s.DurationSeconds,
			&s.Placement, &s.Eliminations, &s.DamageDealt, &s.ResultStatus,
		); err != nil {
			return nil, fmt.Errorf("failed to scan match summary: %w", err)
		}
		list = append(list, s)
	}

	return list, rows.Err()
}

