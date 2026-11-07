package matches

import (
	"context"
	"errors"
	"fmt"
	"math"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/progression"
	"github.com/wildbound/backend/internal/repository"
)

var (
	ErrMatchNotFound    = errors.New("match not found")
	ErrInvalidMatchData = errors.New("invalid match result data")
)

type Service struct {
	db                 *pgxpool.Pool
	progressionService *progression.Service
	achievementsRepo   repository.AchievementsRepository
}

func NewService(db *pgxpool.Pool) *Service {
	return &Service{db: db}
}

func (s *Service) SetProgression(prog *progression.Service, ach repository.AchievementsRepository) {
	s.progressionService = prog
	s.achievementsRepo = ach
}

func (s *Service) SubmitMatchResult(ctx context.Context, req *SubmitMatchResultRequest) (*MatchRecord, bool, error) {
	if req.MatchID == "" || req.GameMode == "" {
		return nil, false, ErrInvalidMatchData
	}

	tx, err := s.db.Begin(ctx)
	if err != nil {
		return nil, false, fmt.Errorf("failed to begin transaction: %w", err)
	}
	defer tx.Rollback(ctx)

	// 1. Idempotency check
	var existingUUID uuid.UUID
	checkQuery := `SELECT id FROM match_records WHERE match_id = $1;`
	err = tx.QueryRow(ctx, checkQuery, req.MatchID).Scan(&existingUUID)
	if err == nil {
		_ = tx.Commit(ctx)
		existingRecord, getErr := s.GetMatch(ctx, req.MatchID)
		if getErr != nil {
			return nil, false, getErr
		}
		return existingRecord, false, nil
	} else if !errors.Is(err, pgx.ErrNoRows) {
		return nil, false, fmt.Errorf("failed to check existing match: %w", err)
	}

	// 2. Insert match record
	participantCount := len(req.Participants)
	createdAt := time.Now()
	resultStatus := req.ResultStatus
	if resultStatus == "" {
		resultStatus = "completed"
	}
	winnerDisplayName := req.WinnerDisplayName
	if winnerDisplayName == "" {
		winnerDisplayName = "No Winner"
	}

	var matchRecordID uuid.UUID
	insertMatchQuery := `
		INSERT INTO match_records (
			match_id, game_mode, map_id, started_at, ended_at,
			duration_seconds, participant_count, winner_user_id, winner_display_name,
			result_status, created_at
		) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
		RETURNING id;
	`
	err = tx.QueryRow(ctx, insertMatchQuery,
		req.MatchID, req.GameMode, req.MapID,
		req.StartedAt, req.EndedAt, req.DurationSeconds,
		participantCount, req.WinnerUserID, winnerDisplayName,
		resultStatus, createdAt,
	).Scan(&matchRecordID)
	if err != nil {
		return nil, false, fmt.Errorf("failed to insert match record: %w", err)
	}

	// 3. Insert participants and update player statistics
	insertParticipantQuery := `
		INSERT INTO match_participants (
			match_record_id, user_id, participant_display_name, team_id,
			placement, eliminations, deaths, assists, damage_dealt,
			damage_received, survival_seconds, result_status
		) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12);
	`

	upsertStatsQuery := `
		INSERT INTO player_statistics (
			user_id, matches_played, matches_won, eliminations, deaths, assists,
			total_damage, total_damage_received, total_survival_seconds,
			highest_placement, updated_at
		) VALUES ($1, 1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
		ON CONFLICT (user_id) DO UPDATE SET
			matches_played = player_statistics.matches_played + 1,
			matches_won = player_statistics.matches_won + EXCLUDED.matches_won,
			eliminations = player_statistics.eliminations + EXCLUDED.eliminations,
			deaths = player_statistics.deaths + EXCLUDED.deaths,
			assists = player_statistics.assists + EXCLUDED.assists,
			total_damage = player_statistics.total_damage + EXCLUDED.total_damage,
			total_damage_received = player_statistics.total_damage_received + EXCLUDED.total_damage_received,
			total_survival_seconds = player_statistics.total_survival_seconds + EXCLUDED.total_survival_seconds,
			highest_placement = CASE
				WHEN player_statistics.highest_placement = 0 OR EXCLUDED.highest_placement < player_statistics.highest_placement
				THEN EXCLUDED.highest_placement
				ELSE player_statistics.highest_placement
			END,
			updated_at = EXCLUDED.updated_at;
	`

	for _, p := range req.Participants {
		partStatus := p.ResultStatus
		if partStatus == "" {
			if p.Placement == 1 {
				partStatus = "winner"
			} else {
				partStatus = "eliminated"
			}
		}

		_, err = tx.Exec(ctx, insertParticipantQuery,
			matchRecordID, p.UserID, p.ParticipantDisplayName, p.TeamID,
			p.Placement, p.Eliminations, p.Deaths, p.Assists,
			p.DamageDealt, p.DamageReceived, p.SurvivalSeconds, partStatus,
		)
		if err != nil {
			return nil, false, fmt.Errorf("failed to insert participant: %w", err)
		}

		if p.UserID != nil && *p.UserID != uuid.Nil {
			wonCount := 0
			if p.Placement == 1 || partStatus == "winner" {
				wonCount = 1
			}

			_, err = tx.Exec(ctx, upsertStatsQuery,
				*p.UserID, wonCount, p.Eliminations, p.Deaths, p.Assists,
				p.DamageDealt, p.DamageReceived, p.SurvivalSeconds,
				p.Placement, createdAt,
			)
			if err != nil {
				return nil, false, fmt.Errorf("failed to update statistics: %w", err)
			}

			// Award persistent account progression (XP, levels, reward points)
			if s.progressionService != nil {
				_, _, errProg := s.progressionService.AwardMatchProgression(
					ctx, tx, *p.UserID, req.MatchID,
					p.Placement, p.Eliminations, float32(p.DamageDealt), float32(p.SurvivalSeconds),
				)
				if errProg != nil && !errors.Is(errProg, repository.ErrDuplicateProgressionEvent) {
					return nil, false, fmt.Errorf("failed to award progression: %w", errProg)
				}
			}

			// Progress relevant achievements
			if s.achievementsRepo != nil {
				if _, _, errAch := s.achievementsRepo.IncrementProgress(ctx, tx, *p.UserID, "ach_veteran_contender", 1); errAch != nil {
					return nil, false, fmt.Errorf("failed veteran contender achievement: %w", errAch)
				}
				if p.Eliminations > 0 {
					if _, _, errAch := s.achievementsRepo.IncrementProgress(ctx, tx, *p.UserID, "ach_first_blood", p.Eliminations); errAch != nil {
						return nil, false, fmt.Errorf("failed first blood achievement: %w", errAch)
					}
				}
				if wonCount == 1 {
					if _, _, errAch := s.achievementsRepo.IncrementProgress(ctx, tx, *p.UserID, "ach_arena_victor", 1); errAch != nil {
						return nil, false, fmt.Errorf("failed arena victor achievement: %w", errAch)
					}
				}
				if p.DamageDealt > 0 {
					if _, _, errAch := s.achievementsRepo.IncrementProgress(ctx, tx, *p.UserID, "ach_sharpshooter", int(p.DamageDealt)); errAch != nil {
						return nil, false, fmt.Errorf("failed sharpshooter achievement: %w", errAch)
					}
				}
				if p.SurvivalSeconds > 0 {
					if _, _, errAch := s.achievementsRepo.IncrementProgress(ctx, tx, *p.UserID, "ach_seasoned_survivor", int(p.SurvivalSeconds)); errAch != nil {
						return nil, false, fmt.Errorf("failed seasoned survivor achievement: %w", errAch)
					}
				}
			}
		}
	}

	if err := tx.Commit(ctx); err != nil {
		return nil, false, fmt.Errorf("failed to commit match result: %w", err)
	}

	record := &MatchRecord{
		ID:                matchRecordID,
		MatchID:           req.MatchID,
		GameMode:          req.GameMode,
		MapID:             req.MapID,
		StartedAt:         req.StartedAt,
		EndedAt:           req.EndedAt,
		DurationSeconds:   req.DurationSeconds,
		ParticipantCount:  participantCount,
		WinnerUserID:      req.WinnerUserID,
		WinnerDisplayName: winnerDisplayName,
		ResultStatus:      resultStatus,
		CreatedAt:         createdAt,
		Participants:      req.Participants,
	}

	return record, true, nil
}

func (s *Service) GetMatch(ctx context.Context, identifier string) (*MatchRecord, error) {
	var m MatchRecord
	var err error

	if parsedUUID, parseErr := uuid.Parse(identifier); parseErr == nil && parsedUUID != uuid.Nil {
		recordQuery := `
			SELECT id, match_id, game_mode, map_id, started_at, ended_at,
			       duration_seconds, participant_count, winner_user_id, winner_display_name,
			       result_status, created_at
			FROM match_records
			WHERE id = $1 OR match_id = $2;
		`
		err = s.db.QueryRow(ctx, recordQuery, parsedUUID, identifier).Scan(
			&m.ID, &m.MatchID, &m.GameMode, &m.MapID, &m.StartedAt, &m.EndedAt,
			&m.DurationSeconds, &m.ParticipantCount, &m.WinnerUserID, &m.WinnerDisplayName,
			&m.ResultStatus, &m.CreatedAt,
		)
	} else {
		recordQuery := `
			SELECT id, match_id, game_mode, map_id, started_at, ended_at,
			       duration_seconds, participant_count, winner_user_id, winner_display_name,
			       result_status, created_at
			FROM match_records
			WHERE match_id = $1;
		`
		err = s.db.QueryRow(ctx, recordQuery, identifier).Scan(
			&m.ID, &m.MatchID, &m.GameMode, &m.MapID, &m.StartedAt, &m.EndedAt,
			&m.DurationSeconds, &m.ParticipantCount, &m.WinnerUserID, &m.WinnerDisplayName,
			&m.ResultStatus, &m.CreatedAt,
		)
	}
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrMatchNotFound
		}
		return nil, fmt.Errorf("failed to get match record: %w", err)
	}

	partsQuery := `
		SELECT user_id, participant_display_name, team_id, placement,
		       eliminations, deaths, assists, damage_dealt, damage_received,
		       survival_seconds, result_status
		FROM match_participants
		WHERE match_record_id = $1
		ORDER BY placement ASC;
	`
	rows, err := s.db.Query(ctx, partsQuery, m.ID)
	if err != nil {
		return nil, fmt.Errorf("failed to query participants: %w", err)
	}
	defer rows.Close()

	m.Participants = make([]MatchParticipantPayload, 0)
	for rows.Next() {
		var p MatchParticipantPayload
		if err := rows.Scan(
			&p.UserID, &p.ParticipantDisplayName, &p.TeamID, &p.Placement,
			&p.Eliminations, &p.Deaths, &p.Assists,
			&p.DamageDealt, &p.DamageReceived, &p.SurvivalSeconds, &p.ResultStatus,
		); err != nil {
			return nil, fmt.Errorf("failed to scan participant: %w", err)
		}
		m.Participants = append(m.Participants, p)
	}

	return &m, nil
}

func (s *Service) ListPlayerMatches(ctx context.Context, userID uuid.UUID, page int, limit int) (*PaginatedMatchesResponse, error) {
	if page < 1 {
		page = 1
	}
	if limit < 1 || limit > 100 {
		limit = 20
	}
	offset := (page - 1) * limit

	var totalCount int
	countQuery := `SELECT COUNT(*) FROM match_participants WHERE user_id = $1;`
	if err := s.db.QueryRow(ctx, countQuery, userID).Scan(&totalCount); err != nil {
		return nil, fmt.Errorf("failed to count player matches: %w", err)
	}

	totalPages := int(math.Ceil(float64(totalCount) / float64(limit)))

	query := `
		SELECT m.id, m.match_id, m.game_mode, m.duration_seconds, m.ended_at,
		       mp.placement, m.participant_count, mp.eliminations, mp.deaths, mp.assists,
		       mp.damage_dealt, mp.survival_seconds, mp.result_status
		FROM match_participants mp
		JOIN match_records m ON mp.match_record_id = m.id
		WHERE mp.user_id = $1
		ORDER BY m.ended_at DESC
		LIMIT $2 OFFSET $3;
	`
	rows, err := s.db.Query(ctx, query, userID, limit, offset)
	if err != nil {
		return nil, fmt.Errorf("failed to query player matches: %w", err)
	}
	defer rows.Close()

	summaries := make([]PlayerMatchSummary, 0)
	for rows.Next() {
		var sm PlayerMatchSummary
		if err := rows.Scan(
			&sm.MatchRecordID, &sm.MatchID, &sm.GameMode, &sm.DurationSeconds, &sm.EndedAt,
			&sm.Placement, &sm.ParticipantCount, &sm.Eliminations, &sm.Deaths, &sm.Assists,
			&sm.DamageDealt, &sm.SurvivalSeconds, &sm.ParticipantResultStatus,
		); err != nil {
			return nil, fmt.Errorf("failed to scan player match summary: %w", err)
		}
		summaries = append(summaries, sm)
	}

	return &PaginatedMatchesResponse{
		Matches:    summaries,
		TotalCount: totalCount,
		Page:       page,
		Limit:      limit,
		TotalPages: totalPages,
	}, nil
}
