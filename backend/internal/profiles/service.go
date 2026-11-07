package profiles

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/validation"
)

var (
	ErrProfileNotFound  = errors.New("profile not found")
	ErrCosmeticNotOwned = errors.New("player does not own specified avatar cosmetic")
	ErrInvalidCosmetic  = errors.New("invalid avatar cosmetic item")
)

type Service struct {
	db *pgxpool.Pool
}

func NewService(db *pgxpool.Pool) *Service {
	return &Service{db: db}
}

func (s *Service) GetProfile(ctx context.Context, userID uuid.UUID) (*Profile, error) {
	query := `
		SELECT id, user_id, display_name, biography, avatar_cosmetic_id, preferred_game_mode, created_at, updated_at
		FROM player_profiles
		WHERE user_id = $1;
	`
	var p Profile
	err := s.db.QueryRow(ctx, query, userID).Scan(
		&p.ID, &p.UserID, &p.DisplayName, &p.Biography,
		&p.AvatarCosmeticID, &p.PreferredGameMode,
		&p.CreatedAt, &p.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrProfileNotFound
		}
		return nil, fmt.Errorf("failed to get profile: %w", err)
	}
	return &p, nil
}

func (s *Service) GetPublicProfile(ctx context.Context, playerID uuid.UUID) (*PublicProfile, error) {
	p, err := s.GetProfile(ctx, playerID)
	if err != nil {
		return nil, err
	}
	return &PublicProfile{
		PlayerID:          p.UserID,
		DisplayName:       p.DisplayName,
		Biography:         p.Biography,
		AvatarCosmeticID:  p.AvatarCosmeticID,
		PreferredGameMode: p.PreferredGameMode,
		CreatedAt:         p.CreatedAt,
	}, nil
}

func (s *Service) checkAvatarOwnership(ctx context.Context, userID uuid.UUID, cosmeticID string) error {
	var category string
	err := s.db.QueryRow(ctx, `SELECT category FROM cosmetic_definitions WHERE id = $1;`, cosmeticID).Scan(&category)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return ErrInvalidCosmetic
		}
		return err
	}

	var exists bool
	err = s.db.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM player_cosmetics WHERE user_id = $1 AND cosmetic_id = $2);`, userID, cosmeticID).Scan(&exists)
	if err != nil {
		return err
	}
	if !exists {
		return ErrCosmeticNotOwned
	}
	return nil
}

func (s *Service) UpdateProfile(ctx context.Context, userID uuid.UUID, req *UpdateProfileRequest) (*Profile, error) {
	current, err := s.GetProfile(ctx, userID)
	if err != nil {
		return nil, err
	}

	if req.DisplayName != nil {
		validName, err := validation.ValidateDisplayName(*req.DisplayName)
		if err != nil {
			return nil, err
		}
		current.DisplayName = validName
	}

	if req.Biography != nil {
		if len(*req.Biography) > 256 {
			return nil, fmt.Errorf("biography must not exceed 256 characters")
		}
		current.Biography = *req.Biography
	}

	if req.PreferredGameMode != nil {
		validModes := map[string]bool{
			"solo":         true,
			"solo_br":      true,
			"duos_br":      true,
			"squads_br":    true,
			"practice":     true,
			"offline_bots": true,
		}
		if !validModes[*req.PreferredGameMode] {
			return nil, fmt.Errorf("invalid preferred game mode")
		}
		current.PreferredGameMode = *req.PreferredGameMode
	}

	if req.AvatarCosmeticID != nil {
		if err := s.checkAvatarOwnership(ctx, userID, *req.AvatarCosmeticID); err != nil {
			return nil, err
		}
		current.AvatarCosmeticID = *req.AvatarCosmeticID
	}

	current.UpdatedAt = time.Now()

	updateQuery := `
		UPDATE player_profiles
		SET display_name = $1,
		    biography = $2,
		    avatar_cosmetic_id = $3,
		    preferred_game_mode = $4,
		    updated_at = $5
		WHERE user_id = $6;
	`
	_, err = s.db.Exec(ctx, updateQuery,
		current.DisplayName,
		current.Biography,
		current.AvatarCosmeticID,
		current.PreferredGameMode,
		current.UpdatedAt,
		userID,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to update profile: %w", err)
	}

	return current, nil
}
