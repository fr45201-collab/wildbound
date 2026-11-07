package cosmetics

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/repository"
)

var (
	ErrCosmeticNotFound       = repository.ErrCosmeticNotFound
	ErrCosmeticNotOwned       = repository.ErrCosmeticNotOwned
	ErrCosmeticUnavailable    = repository.ErrCosmeticUnavailable
	ErrLoadoutVersionConflict = repository.ErrLoadoutVersionConflict
	ErrInvalidSlot            = repository.ErrInvalidSlot
	ErrInvalidUserID          = errors.New("invalid user id")
)

type Service struct {
	cosmeticsRepo repository.CosmeticsRepository
}

func NewService(repo repository.CosmeticsRepository) *Service {
	return &Service{cosmeticsRepo: repo}
}

func (s *Service) ListCatalogue(ctx context.Context, category, slot, rarity, search string, limit, offset int) (*CatalogueResponse, error) {
	defs, total, err := s.cosmeticsRepo.ListDefinitions(ctx, category, slot, rarity, search, limit, offset)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve cosmetic catalogue: %w", err)
	}

	items := make([]CosmeticDefinitionDTO, 0, len(defs))
	for _, d := range defs {
		items = append(items, CosmeticDefinitionDTO{
			ID:            d.ID,
			CosmeticKey:   d.CosmeticKey,
			DisplayName:   d.DisplayName,
			Category:      d.Category,
			EquipmentSlot: d.EquipmentSlot,
			Rarity:        d.Rarity,
			Description:   d.Description,
			AssetID:       d.AssetID,
			IsAvailable:   d.IsAvailable,
			CreatedAt:     d.CreatedAt,
		})
	}

	return &CatalogueResponse{
		Items:  items,
		Total:  total,
		Limit:  limit,
		Offset: offset,
	}, nil
}

func (s *Service) ListPlayerCosmetics(ctx context.Context, userID uuid.UUID) ([]PlayerCosmeticItemDTO, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}

	cosmetics, err := s.cosmeticsRepo.ListPlayerCosmetics(ctx, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to list player cosmetics: %w", err)
	}

	items := make([]PlayerCosmeticItemDTO, 0, len(cosmetics))
	for _, pc := range cosmetics {
		def, err := s.cosmeticsRepo.GetDefinitionByID(ctx, pc.CosmeticID)
		if err != nil {
			continue // Skip dangling references safely
		}

		items = append(items, PlayerCosmeticItemDTO{
			ID:                pc.ID,
			CosmeticID:        pc.CosmeticID,
			CosmeticKey:       def.CosmeticKey,
			DisplayName:       def.DisplayName,
			Category:          def.Category,
			EquipmentSlot:     def.EquipmentSlot,
			Rarity:            def.Rarity,
			Description:       def.Description,
			AssetID:           def.AssetID,
			AcquiredAt:        pc.AcquiredAt,
			AcquisitionSource: pc.AcquisitionSource,
			IsEquipped:        pc.IsEquipped,
		})
	}

	return items, nil
}

func (s *Service) formatLoadoutResponse(ctx context.Context, loadout *repository.PlayerLoadout, items []repository.PlayerLoadoutItem) *PlayerLoadoutResponse {
	slots := make(map[string]LoadoutItemDTO)
	for _, it := range items {
		slotDTO := LoadoutItemDTO{
			Slot:       strings.ToLower(it.EquipmentSlot),
			CosmeticID: it.CosmeticID,
		}
		if def, err := s.cosmeticsRepo.GetDefinitionByID(ctx, it.CosmeticID); err == nil {
			slotDTO.DisplayName = def.DisplayName
			slotDTO.AssetID = def.AssetID
			slotDTO.Rarity = def.Rarity
		}
		slots[strings.ToLower(it.EquipmentSlot)] = slotDTO
	}

	return &PlayerLoadoutResponse{
		UserID:         loadout.UserID,
		LoadoutVersion: loadout.LoadoutVersion,
		UpdatedAt:      loadout.UpdatedAt,
		Slots:          slots,
	}
}

func (s *Service) GetPlayerLoadout(ctx context.Context, userID uuid.UUID) (*PlayerLoadoutResponse, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}

	loadout, items, err := s.cosmeticsRepo.GetPlayerLoadout(ctx, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to get player loadout: %w", err)
	}

	return s.formatLoadoutResponse(ctx, loadout, items), nil
}

func (s *Service) EquipItem(ctx context.Context, userID uuid.UUID, cosmeticID string, expectedVersion int) (*PlayerLoadoutResponse, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}
	if cosmeticID == "" {
		return nil, errors.New("empty cosmetic id")
	}

	loadout, items, err := s.cosmeticsRepo.EquipLoadoutItem(ctx, userID, cosmeticID, expectedVersion)
	if err != nil {
		return nil, err
	}

	return s.formatLoadoutResponse(ctx, loadout, items), nil
}

func (s *Service) UnequipSlot(ctx context.Context, userID uuid.UUID, slot string, expectedVersion int) (*PlayerLoadoutResponse, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}
	if slot == "" {
		return nil, errors.New("empty equipment slot")
	}

	loadout, items, err := s.cosmeticsRepo.UnequipLoadoutSlot(ctx, userID, slot, expectedVersion)
	if err != nil {
		return nil, err
	}

	return s.formatLoadoutResponse(ctx, loadout, items), nil
}

func (s *Service) UpdateFullLoadout(ctx context.Context, userID uuid.UUID, cosmeticIDs []string, expectedVersion int) (*PlayerLoadoutResponse, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}

	loadout, items, err := s.cosmeticsRepo.SetFullLoadout(ctx, userID, cosmeticIDs, expectedVersion)
	if err != nil {
		return nil, err
	}

	return s.formatLoadoutResponse(ctx, loadout, items), nil
}

func (s *Service) GetServerVerifiedLoadout(ctx context.Context, userID uuid.UUID) (*ServerLoadoutResponse, error) {
	if userID == uuid.Nil {
		return nil, ErrInvalidUserID
	}

	loadout, items, err := s.cosmeticsRepo.GetPlayerLoadout(ctx, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve loadout for server validation: %w", err)
	}

	slots := make(map[string]string)
	assetSlots := make(map[string]string)

	for _, it := range items {
		// Re-verify that user actually owns each equipped item
		owned, err := s.cosmeticsRepo.CheckOwnership(ctx, userID, it.CosmeticID)
		if err != nil || !owned {
			continue // Skip unverified/unowned items to enforce server authority
		}

		def, err := s.cosmeticsRepo.GetDefinitionByID(ctx, it.CosmeticID)
		if err != nil || !def.IsAvailable {
			continue
		}

		slots[strings.ToLower(it.EquipmentSlot)] = it.CosmeticID
		assetSlots[strings.ToLower(it.EquipmentSlot)] = def.AssetID
	}

	return &ServerLoadoutResponse{
		UserID:         userID,
		LoadoutVersion: loadout.LoadoutVersion,
		IsValid:        true,
		Slots:          slots,
		AssetSlots:     assetSlots,
		ValidatedAt:    time.Now().UTC(),
	}, nil
}

func (s *Service) GrantCosmetic(ctx context.Context, userID uuid.UUID, cosmeticID, source string) error {
	if userID == uuid.Nil {
		return ErrInvalidUserID
	}
	if cosmeticID == "" {
		return errors.New("empty cosmetic id")
	}
	if source == "" {
		source = "admin_grant"
	}

	def, err := s.cosmeticsRepo.GetDefinitionByID(ctx, cosmeticID)
	if err != nil {
		return err
	}
	if !def.IsAvailable {
		return ErrCosmeticUnavailable
	}

	_, err = s.cosmeticsRepo.GrantCosmetic(ctx, nil, userID, cosmeticID, source, false)
	return err
}
