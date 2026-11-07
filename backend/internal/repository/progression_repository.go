package repository

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

var (
	ErrProgressionNotFound     = errors.New("player progression not found")
	ErrDuplicateProgressionEvent = errors.New("progression event already awarded for this source")
)

type ProgressionRepository interface {
	CreateDefaultProgression(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error
	GetProgressionByUserID(ctx context.Context, userID uuid.UUID) (*PlayerProgression, error)
	RecordProgressionEvent(ctx context.Context, tx pgx.Tx, event *ProgressionEvent) error
	HasProgressionEvent(ctx context.Context, userID uuid.UUID, sourceType, sourceID string) (bool, error)
	UpdateProgression(ctx context.Context, tx pgx.Tx, userID uuid.UUID, newLevel, newTotalXP, rewardPointsDelta, matchesCompletedDelta int) (*PlayerProgression, error)
	ListProgressionEvents(ctx context.Context, userID uuid.UUID, limit, offset int) ([]ProgressionEvent, error)
}

type progressionRepo struct {
	pool *pgxpool.Pool
}

func NewProgressionRepository(pool *pgxpool.Pool) ProgressionRepository {
	return &progressionRepo{pool: pool}
}

func (r *progressionRepo) CreateDefaultProgression(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error {
	query := `
		INSERT INTO player_progression (
			user_id, account_level, total_xp, available_reward_points,
			matches_completed, progression_version, updated_at
		) VALUES ($1, 1, 0, 0, 0, 1, NOW())
		ON CONFLICT (user_id) DO NOTHING
	`
	if tx != nil {
		_, err := tx.Exec(ctx, query, userID)
		return err
	}
	_, err := r.pool.Exec(ctx, query, userID)
	return err
}

func (r *progressionRepo) GetProgressionByUserID(ctx context.Context, userID uuid.UUID) (*PlayerProgression, error) {
	query := `
		SELECT user_id, account_level, total_xp, available_reward_points,
		       matches_completed, progression_version, updated_at
		FROM player_progression
		WHERE user_id = $1
	`
	var p PlayerProgression
	err := r.pool.QueryRow(ctx, query, userID).Scan(
		&p.UserID, &p.AccountLevel, &p.TotalXP, &p.AvailableRewardPoints,
		&p.MatchesCompleted, &p.ProgressionVersion, &p.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			_ = r.CreateDefaultProgression(ctx, nil, userID)
			return &PlayerProgression{
				UserID:                userID,
				AccountLevel:          1,
				TotalXP:               0,
				AvailableRewardPoints: 0,
				MatchesCompleted:      0,
				ProgressionVersion:    1,
			}, nil
		}
		return nil, fmt.Errorf("failed to query player progression: %w", err)
	}
	return &p, nil
}

func (r *progressionRepo) HasProgressionEvent(ctx context.Context, userID uuid.UUID, sourceType, sourceID string) (bool, error) {
	query := `SELECT EXISTS(SELECT 1 FROM progression_events WHERE user_id = $1 AND source_type = $2 AND source_id = $3)`
	var exists bool
	err := r.pool.QueryRow(ctx, query, userID, sourceType, sourceID).Scan(&exists)
	if err != nil {
		return false, fmt.Errorf("failed to check existing progression event: %w", err)
	}
	return exists, nil
}

func (r *progressionRepo) RecordProgressionEvent(ctx context.Context, tx pgx.Tx, event *ProgressionEvent) error {
	query := `
		INSERT INTO progression_events (
			user_id, source_type, source_id, xp_delta,
			reward_points_delta, reason, created_at
		) VALUES ($1, $2, $3, $4, $5, $6, NOW())
		RETURNING id, created_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	err := runner.QueryRow(ctx, query,
		event.UserID, event.SourceType, event.SourceID,
		event.XPDelta, event.RewardPointsDelta, event.Reason,
	).Scan(&event.ID, &event.CreatedAt)
	if err != nil {
		return fmt.Errorf("failed to record progression event: %w", err)
	}
	return nil
}

func (r *progressionRepo) UpdateProgression(ctx context.Context, tx pgx.Tx, userID uuid.UUID, newLevel, newTotalXP, rewardPointsDelta, matchesCompletedDelta int) (*PlayerProgression, error) {
	query := `
		INSERT INTO player_progression (
			user_id, account_level, total_xp, available_reward_points,
			matches_completed, progression_version, updated_at
		) VALUES ($1, $2, $3, GREATEST(0, $4), GREATEST(0, $5), 1, NOW())
		ON CONFLICT (user_id) DO UPDATE SET
			account_level = EXCLUDED.account_level,
			total_xp = EXCLUDED.total_xp,
			available_reward_points = player_progression.available_reward_points + $4,
			matches_completed = player_progression.matches_completed + $5,
			updated_at = NOW()
		RETURNING user_id, account_level, total_xp, available_reward_points, matches_completed, progression_version, updated_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	var p PlayerProgression
	err := runner.QueryRow(ctx, query, userID, newLevel, newTotalXP, rewardPointsDelta, matchesCompletedDelta).Scan(
		&p.UserID, &p.AccountLevel, &p.TotalXP, &p.AvailableRewardPoints,
		&p.MatchesCompleted, &p.ProgressionVersion, &p.UpdatedAt,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to update player progression: %w", err)
	}
	return &p, nil
}

func (r *progressionRepo) ListProgressionEvents(ctx context.Context, userID uuid.UUID, limit, offset int) ([]ProgressionEvent, error) {
	if limit <= 0 || limit > 100 {
		limit = 20
	}
	if offset < 0 {
		offset = 0
	}

	query := `
		SELECT id, user_id, source_type, source_id, xp_delta,
		       reward_points_delta, reason, created_at
		FROM progression_events
		WHERE user_id = $1
		ORDER BY created_at DESC
		LIMIT $2 OFFSET $3
	`
	rows, err := r.pool.Query(ctx, query, userID, limit, offset)
	if err != nil {
		return nil, fmt.Errorf("failed to query progression events: %w", err)
	}
	defer rows.Close()

	var events []ProgressionEvent
	for rows.Next() {
		var e ProgressionEvent
		if err := rows.Scan(
			&e.ID, &e.UserID, &e.SourceType, &e.SourceID,
			&e.XPDelta, &e.RewardPointsDelta, &e.Reason, &e.CreatedAt,
		); err != nil {
			return nil, fmt.Errorf("failed to scan progression event: %w", err)
		}
		events = append(events, e)
	}
	return events, rows.Err()
}

