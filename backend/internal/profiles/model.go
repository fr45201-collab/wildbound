package profiles

import (
	"time"

	"github.com/google/uuid"
)

type Profile struct {
	ID                uuid.UUID `json:"id"`
	UserID            uuid.UUID `json:"user_id"`
	DisplayName       string    `json:"display_name"`
	Biography         string    `json:"biography"`
	AvatarCosmeticID  string    `json:"avatar_cosmetic_id"`
	PreferredGameMode string    `json:"preferred_game_mode"`
	CreatedAt         time.Time `json:"created_at"`
	UpdatedAt         time.Time `json:"updated_at"`
}

type PublicProfile struct {
	PlayerID          uuid.UUID `json:"player_id"`
	DisplayName       string    `json:"display_name"`
	Biography         string    `json:"biography"`
	AvatarCosmeticID  string    `json:"avatar_cosmetic_id"`
	PreferredGameMode string    `json:"preferred_game_mode"`
	CreatedAt         time.Time `json:"created_at"`
}

type UpdateProfileRequest struct {
	DisplayName       *string `json:"display_name,omitempty"`
	Biography         *string `json:"biography,omitempty"`
	AvatarCosmeticID  *string `json:"avatar_cosmetic_id,omitempty"`
	PreferredGameMode *string `json:"preferred_game_mode,omitempty"`
}
