package cosmetics

import (
	"time"

	"github.com/google/uuid"
)

type CosmeticDefinitionDTO struct {
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

type CatalogueResponse struct {
	Items  []CosmeticDefinitionDTO `json:"items"`
	Total  int                     `json:"total"`
	Limit  int                     `json:"limit"`
	Offset int                     `json:"offset"`
}

type PlayerCosmeticItemDTO struct {
	ID                uuid.UUID `json:"id"`
	CosmeticID        string    `json:"cosmetic_id"`
	CosmeticKey       string    `json:"cosmetic_key"`
	DisplayName       string    `json:"display_name"`
	Category          string    `json:"category"`
	EquipmentSlot     string    `json:"equipment_slot"`
	Rarity            string    `json:"rarity"`
	Description       string    `json:"description"`
	AssetID           string    `json:"asset_id"`
	AcquiredAt        time.Time `json:"acquired_at"`
	AcquisitionSource string    `json:"acquisition_source"`
	IsEquipped        bool      `json:"is_equipped"`
}

type LoadoutItemDTO struct {
	Slot        string `json:"slot"`
	CosmeticID  string `json:"cosmetic_id"`
	DisplayName string `json:"display_name,omitempty"`
	AssetID     string `json:"asset_id,omitempty"`
	Rarity      string `json:"rarity,omitempty"`
}

type PlayerLoadoutResponse struct {
	UserID         uuid.UUID                 `json:"user_id"`
	LoadoutVersion int                       `json:"loadout_version"`
	UpdatedAt      time.Time                 `json:"updated_at"`
	Slots          map[string]LoadoutItemDTO `json:"slots"`
}

type EquipSlotRequest struct {
	CosmeticID      string `json:"cosmetic_id"`
	ExpectedVersion int    `json:"expected_version,omitempty"`
}

type UnequipSlotRequest struct {
	Slot            string `json:"slot"`
	ExpectedVersion int    `json:"expected_version,omitempty"`
}

type UpdateLoadoutRequest struct {
	Cosmetics       []string `json:"cosmetics"`
	ExpectedVersion int      `json:"expected_version,omitempty"`
}

type GrantCosmeticRequest struct {
	UserID     uuid.UUID `json:"user_id"`
	CosmeticID string    `json:"cosmetic_id"`
	Source     string    `json:"source,omitempty"`
}

// ServerLoadoutResponse is returned to dedicated game servers
type ServerLoadoutResponse struct {
	UserID         uuid.UUID         `json:"user_id"`
	LoadoutVersion int               `json:"loadout_version"`
	IsValid        bool              `json:"is_valid"`
	Slots          map[string]string `json:"slots"`           // Slot -> CosmeticID
	AssetSlots     map[string]string `json:"asset_slots"`     // Slot -> AssetID
	ValidatedAt    time.Time         `json:"validated_at"`
}

// Backward compatibility aliases
type PlayerCosmeticItem = PlayerCosmeticItemDTO
type EquipCosmeticRequest = EquipSlotRequest

