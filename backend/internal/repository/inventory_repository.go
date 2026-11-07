package repository

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/database"
)

var (
	ErrItemNotFound         = errors.New("inventory item not found")
	ErrInsufficientQuantity = errors.New("insufficient inventory item quantity")
	ErrInvalidQuantity      = errors.New("quantity must be strictly positive")
	ErrSameUserTransfer     = errors.New("cannot transfer inventory to the same player")
)

type InventoryRepository interface {
	ListByUserID(ctx context.Context, userID uuid.UUID) ([]InventoryItem, error)
	GetItem(ctx context.Context, userID uuid.UUID, itemDefID string) (*InventoryItem, error)
	AddItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*InventoryItem, error)
	ConsumeItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*InventoryItem, error)
	TransferItem(ctx context.Context, fromUserID, toUserID uuid.UUID, itemDefID string, quantity int) error
}

type inventoryRepo struct {
	pool *pgxpool.Pool
}

func NewInventoryRepository(pool *pgxpool.Pool) InventoryRepository {
	return &inventoryRepo{pool: pool}
}

func (r *inventoryRepo) ListByUserID(ctx context.Context, userID uuid.UUID) ([]InventoryItem, error) {
	query := `
		SELECT id, user_id, item_definition_id, quantity, acquired_at, updated_at
		FROM player_inventory
		WHERE user_id = $1
		ORDER BY acquired_at ASC
	`
	rows, err := r.pool.Query(ctx, query, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to query inventory items: %w", err)
	}
	defer rows.Close()

	items := make([]InventoryItem, 0)
	for rows.Next() {
		var item InventoryItem
		if err := rows.Scan(
			&item.ID, &item.UserID, &item.ItemDefinitionID,
			&item.Quantity, &item.AcquiredAt, &item.UpdatedAt,
		); err != nil {
			return nil, fmt.Errorf("failed to scan inventory item: %w", err)
		}
		items = append(items, item)
	}
	return items, rows.Err()
}

func (r *inventoryRepo) GetItem(ctx context.Context, userID uuid.UUID, itemDefID string) (*InventoryItem, error) {
	query := `
		SELECT id, user_id, item_definition_id, quantity, acquired_at, updated_at
		FROM player_inventory
		WHERE user_id = $1 AND item_definition_id = $2
	`
	var item InventoryItem
	err := r.pool.QueryRow(ctx, query, userID, itemDefID).Scan(
		&item.ID, &item.UserID, &item.ItemDefinitionID,
		&item.Quantity, &item.AcquiredAt, &item.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrItemNotFound
		}
		return nil, fmt.Errorf("failed to query inventory item: %w", err)
	}
	return &item, nil
}

func (r *inventoryRepo) AddItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*InventoryItem, error) {
	if quantity <= 0 {
		return nil, ErrInvalidQuantity
	}

	query := `
		INSERT INTO player_inventory (user_id, item_definition_id, quantity, updated_at)
		VALUES ($1, $2, $3, NOW())
		ON CONFLICT (user_id, item_definition_id)
		DO UPDATE SET quantity = player_inventory.quantity + EXCLUDED.quantity, updated_at = NOW()
		RETURNING id, user_id, item_definition_id, quantity, acquired_at, updated_at
	`
	var item InventoryItem
	err := r.pool.QueryRow(ctx, query, userID, itemDefID, quantity).Scan(
		&item.ID, &item.UserID, &item.ItemDefinitionID,
		&item.Quantity, &item.AcquiredAt, &item.UpdatedAt,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to add inventory item: %w", err)
	}
	return &item, nil
}

func (r *inventoryRepo) ConsumeItem(ctx context.Context, userID uuid.UUID, itemDefID string, quantity int) (*InventoryItem, error) {
	if quantity <= 0 {
		return nil, ErrInvalidQuantity
	}

	var result InventoryItem
	err := database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		selectQuery := `
			SELECT id, user_id, item_definition_id, quantity, acquired_at, updated_at
			FROM player_inventory
			WHERE user_id = $1 AND item_definition_id = $2
			FOR UPDATE
		`
		var cur InventoryItem
		err := tx.QueryRow(ctx, selectQuery, userID, itemDefID).Scan(
			&cur.ID, &cur.UserID, &cur.ItemDefinitionID,
			&cur.Quantity, &cur.AcquiredAt, &cur.UpdatedAt,
		)
		if err != nil {
			if errors.Is(err, pgx.ErrNoRows) {
				return ErrItemNotFound
			}
			return fmt.Errorf("failed to lock inventory row: %w", err)
		}

		if cur.Quantity < quantity {
			return ErrInsufficientQuantity
		}

		if cur.Quantity == quantity {
			deleteQuery := `DELETE FROM player_inventory WHERE id = $1`
			if _, err := tx.Exec(ctx, deleteQuery, cur.ID); err != nil {
				return fmt.Errorf("failed to delete depleted inventory item: %w", err)
			}
			cur.Quantity = 0
			result = cur
			return nil
		}

		updateQuery := `
			UPDATE player_inventory
			SET quantity = quantity - $1, updated_at = NOW()
			WHERE id = $2
			RETURNING id, user_id, item_definition_id, quantity, acquired_at, updated_at
		`
		err = tx.QueryRow(ctx, updateQuery, quantity, cur.ID).Scan(
			&result.ID, &result.UserID, &result.ItemDefinitionID,
			&result.Quantity, &result.AcquiredAt, &result.UpdatedAt,
		)
		if err != nil {
			return fmt.Errorf("failed to update reduced inventory item: %w", err)
		}
		return nil
	})

	if err != nil {
		return nil, err
	}
	return &result, nil
}

func (r *inventoryRepo) TransferItem(ctx context.Context, fromUserID, toUserID uuid.UUID, itemDefID string, quantity int) error {
	if fromUserID == toUserID {
		return ErrSameUserTransfer
	}
	if quantity <= 0 {
		return ErrInvalidQuantity
	}

	return database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		// 1. Lock and decrement from sender
		selectQuery := `
			SELECT id, quantity
			FROM player_inventory
			WHERE user_id = $1 AND item_definition_id = $2
			FOR UPDATE
		`
		var itemID uuid.UUID
		var currentQty int
		err := tx.QueryRow(ctx, selectQuery, fromUserID, itemDefID).Scan(&itemID, &currentQty)
		if err != nil {
			if errors.Is(err, pgx.ErrNoRows) {
				return ErrItemNotFound
			}
			return fmt.Errorf("failed to lock sender inventory item: %w", err)
		}

		if currentQty < quantity {
			return ErrInsufficientQuantity
		}

		if currentQty == quantity {
			if _, err := tx.Exec(ctx, "DELETE FROM player_inventory WHERE id = $1", itemID); err != nil {
				return fmt.Errorf("failed to remove depleted sender inventory item: %w", err)
			}
		} else {
			if _, err := tx.Exec(ctx, "UPDATE player_inventory SET quantity = quantity - $1, updated_at = NOW() WHERE id = $2", quantity, itemID); err != nil {
				return fmt.Errorf("failed to decrement sender inventory item: %w", err)
			}
		}

		// 2. Upsert to recipient
		upsertQuery := `
			INSERT INTO player_inventory (user_id, item_definition_id, quantity, updated_at)
			VALUES ($1, $2, $3, NOW())
			ON CONFLICT (user_id, item_definition_id)
			DO UPDATE SET quantity = player_inventory.quantity + EXCLUDED.quantity, updated_at = NOW()
		`
		if _, err := tx.Exec(ctx, upsertQuery, toUserID, itemDefID, quantity); err != nil {
			return fmt.Errorf("failed to credit recipient inventory item: %w", err)
		}

		return nil
	})
}

