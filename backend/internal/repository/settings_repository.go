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
	ErrSettingsNotFound = errors.New("player settings not found")
)

type SettingsRepository interface {
	CreateDefaultSettings(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error
	GetSettingsByUserID(ctx context.Context, userID uuid.UUID) (*PlayerSettings, error)
	UpdateSettings(ctx context.Context, settings *PlayerSettings) error
}

type settingsRepo struct {
	pool *pgxpool.Pool
}

func NewSettingsRepository(pool *pgxpool.Pool) SettingsRepository {
	return &settingsRepo{pool: pool}
}

func (r *settingsRepo) CreateDefaultSettings(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error {
	query := `
		INSERT INTO player_settings (
			user_id, language, region, preferred_game_mode,
			privacy_profile_visibility, privacy_match_history, allow_friend_requests,
			created_at, updated_at
		) VALUES ($1, 'en', 'us-east', 'solo', 'public', 'public', true, NOW(), NOW())
		ON CONFLICT (user_id) DO NOTHING
	`
	if tx != nil {
		_, err := tx.Exec(ctx, query, userID)
		return err
	}
	_, err := r.pool.Exec(ctx, query, userID)
	return err
}

func (r *settingsRepo) GetSettingsByUserID(ctx context.Context, userID uuid.UUID) (*PlayerSettings, error) {
	query := `
		SELECT user_id, language, region, preferred_game_mode,
		       privacy_profile_visibility, privacy_match_history, allow_friend_requests,
		       created_at, updated_at
		FROM player_settings
		WHERE user_id = $1
	`
	var s PlayerSettings
	err := r.pool.QueryRow(ctx, query, userID).Scan(
		&s.UserID, &s.Language, &s.Region, &s.PreferredGameMode,
		&s.PrivacyProfileVisibility, &s.PrivacyMatchHistory, &s.AllowFriendRequests,
		&s.CreatedAt, &s.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			// Auto-create default settings if not yet present
			_ = r.CreateDefaultSettings(ctx, nil, userID)
			return &PlayerSettings{
				UserID:                   userID,
				Language:                 "en",
				Region:                   "us-east",
				PreferredGameMode:        "solo",
				PrivacyProfileVisibility: "public",
				PrivacyMatchHistory:      "public",
				AllowFriendRequests:      true,
			}, nil
		}
		return nil, fmt.Errorf("failed to query player settings: %w", err)
	}
	return &s, nil
}

func (r *settingsRepo) UpdateSettings(ctx context.Context, settings *PlayerSettings) error {
	query := `
		INSERT INTO player_settings (
			user_id, language, region, preferred_game_mode,
			privacy_profile_visibility, privacy_match_history, allow_friend_requests,
			updated_at
		) VALUES ($1, $2, $3, $4, $5, $6, $7, NOW())
		ON CONFLICT (user_id) DO UPDATE SET
			language = EXCLUDED.language,
			region = EXCLUDED.region,
			preferred_game_mode = EXCLUDED.preferred_game_mode,
			privacy_profile_visibility = EXCLUDED.privacy_profile_visibility,
			privacy_match_history = EXCLUDED.privacy_match_history,
			allow_friend_requests = EXCLUDED.allow_friend_requests,
			updated_at = NOW()
		RETURNING updated_at
	`
	err := r.pool.QueryRow(ctx, query,
		settings.UserID, settings.Language, settings.Region, settings.PreferredGameMode,
		settings.PrivacyProfileVisibility, settings.PrivacyMatchHistory, settings.AllowFriendRequests,
	).Scan(&settings.UpdatedAt)
	if err != nil {
		return fmt.Errorf("failed to update player settings: %w", err)
	}
	return nil
}

