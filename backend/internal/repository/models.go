package repository

import (
	"time"

	"github.com/google/uuid"
)

type User struct {
	ID            uuid.UUID  `json:"id"`
	Username      string     `json:"username"`
	Email         string     `json:"email"`
	PasswordHash  string     `json:"-"`
	AccountStatus string     `json:"account_status"`
	CreatedAt     time.Time  `json:"created_at"`
	UpdatedAt     time.Time  `json:"updated_at"`
	LastLoginAt   *time.Time `json:"last_login_at,omitempty"`
}

type RefreshToken struct {
	ID        uuid.UUID  `json:"id"`
	UserID    uuid.UUID  `json:"user_id"`
	TokenHash string     `json:"token_hash"`
	ExpiresAt time.Time  `json:"expires_at"`
	RevokedAt *time.Time `json:"revoked_at,omitempty"`
	CreatedAt time.Time  `json:"created_at"`
}

type UserSession struct {
	ID               uuid.UUID  `json:"id"`
	UserID           uuid.UUID  `json:"user_id"`
	RefreshTokenHash string     `json:"-"`
	DeviceIdentifier string     `json:"device_identifier"`
	DeviceName       string     `json:"device_name"`
	IPAddress        string     `json:"ip_address"`
	CreatedAt        time.Time  `json:"created_at"`
	LastUsedAt       time.Time  `json:"last_used_at"`
	ExpiresAt        time.Time  `json:"expires_at"`
	RevokedAt        *time.Time `json:"revoked_at,omitempty"`
	RevocationReason *string    `json:"revocation_reason,omitempty"`
	IsActive         bool       `json:"is_active"`
}

type PlayerProfile struct {
	ID                uuid.UUID `json:"id"`
	UserID            uuid.UUID `json:"user_id"`
	DisplayName       string    `json:"display_name"`
	Biography         string    `json:"biography"`
	AvatarCosmeticID  string    `json:"avatar_cosmetic_id"`
	PreferredGameMode string    `json:"preferred_game_mode"`
	CreatedAt         time.Time `json:"created_at"`
	UpdatedAt         time.Time `json:"updated_at"`
}

type PlayerSettings struct {
	UserID                   uuid.UUID `json:"user_id"`
	Language                 string    `json:"language"`
	Region                   string    `json:"region"`
	PreferredGameMode        string    `json:"preferred_game_mode"`
	PrivacyProfileVisibility string    `json:"privacy_profile_visibility"`
	PrivacyMatchHistory      string    `json:"privacy_match_history"`
	AllowFriendRequests      bool      `json:"allow_friend_requests"`
	CreatedAt                time.Time `json:"created_at"`
	UpdatedAt                time.Time `json:"updated_at"`
}

type PlayerProgression struct {
	UserID                uuid.UUID `json:"user_id"`
	AccountLevel          int       `json:"account_level"`
	TotalXP               int       `json:"total_xp"`
	AvailableRewardPoints int       `json:"available_reward_points"`
	MatchesCompleted      int       `json:"matches_completed"`
	ProgressionVersion    int       `json:"progression_version"`
	UpdatedAt             time.Time `json:"updated_at"`
}

type ProgressionEvent struct {
	ID                 uuid.UUID `json:"id"`
	UserID             uuid.UUID `json:"user_id"`
	SourceType         string    `json:"source_type"`
	SourceID           string    `json:"source_id"`
	XPDelta            int       `json:"xp_delta"`
	RewardPointsDelta  int       `json:"reward_points_delta"`
	Reason             string    `json:"reason"`
	CreatedAt          time.Time `json:"created_at"`
}

type AchievementDefinition struct {
	ID           string    `json:"id"`
	AchievementKey string  `json:"achievement_key"`
	DisplayName  string    `json:"display_name"`
	Description  string    `json:"description"`
	Category     string    `json:"category"`
	TargetValue  int       `json:"target_value"`
	RewardXP     int       `json:"reward_xp"`
	RewardPoints int       `json:"reward_points"`
	IsActive     bool      `json:"is_active"`
	CreatedAt    time.Time `json:"created_at"`
}

type PlayerAchievement struct {
	ID            uuid.UUID  `json:"id"`
	UserID        uuid.UUID  `json:"user_id"`
	AchievementID string     `json:"achievement_id"`
	Progress      int        `json:"progress"`
	Target        int        `json:"target"`
	IsUnlocked    bool       `json:"is_unlocked"`
	UnlockedAt    *time.Time `json:"unlocked_at,omitempty"`
	UpdatedAt     time.Time  `json:"updated_at"`
}

type InventoryItem struct {
	ID               uuid.UUID `json:"id"`
	UserID           uuid.UUID `json:"user_id"`
	ItemDefinitionID string    `json:"item_definition_id"`
	Quantity         int       `json:"quantity"`
	AcquiredAt       time.Time `json:"acquired_at"`
	UpdatedAt        time.Time `json:"updated_at"`
}

type CosmeticDefinition struct {
	ID            string    `json:"id"`
	CosmeticKey   string    `json:"cosmetic_key"`
	DisplayName   string    `json:"display_name"`
	Category      string    `json:"category"`
	EquipmentSlot string    `json:"equipment_slot"`
	Rarity        string    `json:"rarity"`
	Description   string    `json:"description"`
	AssetID       string    `json:"asset_id"`
	IsAvailable   bool      `json:"is_available"`
	CreatedAt     time.Time `json:"created_at"`
}

type PlayerCosmetic struct {
	ID                uuid.UUID `json:"id"`
	UserID            uuid.UUID `json:"user_id"`
	CosmeticID        string    `json:"cosmetic_id"`
	AcquiredAt        time.Time `json:"acquired_at"`
	AcquisitionSource string    `json:"acquisition_source"`
	IsEquipped        bool      `json:"is_equipped"`
}

type PlayerLoadout struct {
	ID             uuid.UUID `json:"id"`
	UserID         uuid.UUID `json:"user_id"`
	LoadoutVersion int       `json:"loadout_version"`
	CreatedAt      time.Time `json:"created_at"`
	UpdatedAt      time.Time `json:"updated_at"`
}

type PlayerLoadoutItem struct {
	ID            uuid.UUID `json:"id"`
	LoadoutID     uuid.UUID `json:"loadout_id"`
	UserID        uuid.UUID `json:"user_id"`
	CosmeticID    string    `json:"cosmetic_id"`
	EquipmentSlot string    `json:"equipment_slot"`
	EquippedAt    time.Time `json:"equipped_at"`
}

type MatchRecord struct {
	ID                 uuid.UUID  `json:"id"`
	MatchID            string     `json:"match_id"`
	GameMode           string     `json:"game_mode"`
	MapID              string     `json:"map_id"`
	StartedAt          time.Time  `json:"started_at"`
	EndedAt            time.Time  `json:"ended_at"`
	DurationSeconds    int        `json:"duration_seconds"`
	ParticipantCount   int        `json:"participant_count"`
	WinnerUserID       *uuid.UUID `json:"winner_user_id,omitempty"`
	WinnerDisplayName  string     `json:"winner_display_name"`
	ResultStatus       string     `json:"result_status"`
	CreatedAt          time.Time  `json:"created_at"`
}

type MatchParticipant struct {
	ID                     uuid.UUID  `json:"id"`
	MatchRecordID          uuid.UUID  `json:"match_record_id"`
	UserID                 *uuid.UUID `json:"user_id,omitempty"`
	ParticipantDisplayName string     `json:"participant_display_name"`
	TeamID                 int        `json:"team_id"`
	Placement              int        `json:"placement"`
	Eliminations           int        `json:"eliminations"`
	Deaths                 int        `json:"deaths"`
	Assists                int        `json:"assists"`
	DamageDealt            float32    `json:"damage_dealt"`
	DamageReceived         float32    `json:"damage_received"`
	SurvivalSeconds        float32    `json:"survival_seconds"`
	ResultStatus           string     `json:"result_status"`
}

type MatchSummary struct {
	MatchID         string    `json:"match_id"`
	GameMode        string    `json:"game_mode"`
	MapID           string    `json:"map_id"`
	StartedAt       time.Time `json:"started_at"`
	DurationSeconds int       `json:"duration_seconds"`
	Placement       int       `json:"placement"`
	Eliminations    int       `json:"eliminations"`
	DamageDealt     float32   `json:"damage_dealt"`
	ResultStatus    string    `json:"result_status"`
}

type PlayerStatistics struct {
	ID                   uuid.UUID `json:"id"`
	UserID               uuid.UUID `json:"user_id"`
	MatchesPlayed        int       `json:"matches_played"`
	MatchesWon           int       `json:"matches_won"`
	Eliminations         int       `json:"eliminations"`
	Deaths               int       `json:"deaths"`
	Assists              int       `json:"assists"`
	TotalDamage          float32   `json:"total_damage"`
	TotalDamageReceived  float32   `json:"total_damage_received"`
	TotalSurvivalSeconds float32   `json:"total_survival_seconds"`
	HighestPlacement     int       `json:"highest_placement"`
	UpdatedAt            time.Time `json:"updated_at"`
}
