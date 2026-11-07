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
	ErrProfileNotFound = errors.New("profile not found")
)

type ProfilesRepository interface {
	Create(ctx context.Context, tx pgx.Tx, profile *PlayerProfile) error
	GetByUserID(ctx context.Context, userID uuid.UUID) (*PlayerProfile, error)
	Update(ctx context.Context, profile *PlayerProfile) error
}

type profilesRepo struct {
	pool *pgxpool.Pool
}

func NewProfilesRepository(pool *pgxpool.Pool) ProfilesRepository {
	return &profilesRepo{pool: pool}
}

func (r *profilesRepo) Create(ctx context.Context, tx pgx.Tx, profile *PlayerProfile) error {
	query := `
		INSERT INTO player_profiles (user_id, display_name, biography, avatar_cosmetic_id, preferred_game_mode)
		VALUES ($1, $2, $3, $4, $5)
		RETURNING id, created_at, updated_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	err := runner.QueryRow(ctx, query,
		profile.UserID, profile.DisplayName, profile.Biography,
		profile.AvatarCosmeticID, profile.PreferredGameMode,
	).Scan(&profile.ID, &profile.CreatedAt, &profile.UpdatedAt)
	if err != nil {
		return fmt.Errorf("failed to insert profile: %w", err)
	}
	return nil
}

func (r *profilesRepo) GetByUserID(ctx context.Context, userID uuid.UUID) (*PlayerProfile, error) {
	query := `
		SELECT id, user_id, display_name, biography, avatar_cosmetic_id, preferred_game_mode, created_at, updated_at
		FROM player_profiles
		WHERE user_id = $1
	`
	var p PlayerProfile
	err := r.pool.QueryRow(ctx, query, userID).Scan(
		&p.ID, &p.UserID, &p.DisplayName, &p.Biography,
		&p.AvatarCosmeticID, &p.PreferredGameMode,
		&p.CreatedAt, &p.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrProfileNotFound
		}
		return nil, fmt.Errorf("failed to query profile: %w", err)
	}
	return &p, nil
}

func (r *profilesRepo) Update(ctx context.Context, profile *PlayerProfile) error {
	query := `
		UPDATE player_profiles
		SET display_name = $1, biography = $2, avatar_cosmetic_id = $3, preferred_game_mode = $4, updated_at = NOW()
		WHERE user_id = $5
		RETURNING updated_at
	`
	err := r.pool.QueryRow(ctx, query,
		profile.DisplayName, profile.Biography, profile.AvatarCosmeticID,
		profile.PreferredGameMode, profile.UserID,
	).Scan(&profile.UpdatedAt)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return ErrProfileNotFound
		}
		return fmt.Errorf("failed to update profile: %w", err)
	}
	return nil
}

