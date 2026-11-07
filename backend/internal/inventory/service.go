package inventory

import (
	"context"
	"errors"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/repository"
)

var (
	ErrItemNotFound         = repository.ErrItemNotFound
	ErrInsufficientQuantity = repository.ErrInsufficientQuantity
	ErrInvalidQuantity      = repository.ErrInvalidQuantity
)

type Service struct {
	repo repository.InventoryRepository
}

func NewService(repo repository.InventoryRepository) *Service {
	return &Service{repo: repo}
}

func (s *Service) GetMyInventory(ctx context.Context, userID uuid.UUID) ([]repository.InventoryItem, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}
	return s.repo.ListByUserID(ctx, userID)
}

func (s *Service) ConsumeItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*repository.InventoryItem, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}
	if itemDefID == "" {
		return nil, errors.New("item_definition_id cannot be empty")
	}
	if quantity <= 0 {
		return nil, ErrInvalidQuantity
	}
	return s.repo.ConsumeItem(ctx, userID, itemDefID, quantity)
}

func (s *Service) GrantItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*repository.InventoryItem, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}
	if itemDefID == "" {
		return nil, errors.New("item_definition_id cannot be empty")
	}
	if quantity <= 0 {
		return nil, ErrInvalidQuantity
	}
	return s.repo.AddItem(ctx, userID, itemDefID, quantity)
}

