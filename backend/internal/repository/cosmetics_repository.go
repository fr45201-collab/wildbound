package repository

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/database"
)

var (
	ErrCosmeticNotFound        = errors.New("cosmetic definition not found")
	ErrCosmeticNotOwned        = errors.New("player does not own this cosmetic")
	ErrCosmeticUnavailable     = errors.New("cosmetic is not currently available")
	ErrLoadoutVersionConflict  = errors.New("loadout version conflict: concurrent modification detected")
	ErrInvalidSlot             = errors.New("invalid or incompatible equipment slot")
)

type CosmeticsRepository interface {
	ListDefinitions(ctx context.Context, category, slot, rarity, search string, limit, offset int) ([]CosmeticDefinition, int, error)
	GetDefinitionByID(ctx context.Context, id string) (*CosmeticDefinition, error)
	ListPlayerCosmetics(ctx context.Context, userID uuid.UUID) ([]PlayerCosmetic, error)
	CheckOwnership(ctx context.Context, userID uuid.UUID, cosmeticID string) (bool, error)
	GrantCosmetic(ctx context.Context, tx pgx.Tx, userID uuid.UUID, cosmeticID string, source string, isEquipped bool) (*PlayerCosmetic, error)

	// Loadout operations
	GetPlayerLoadout(ctx context.Context, userID uuid.UUID) (*PlayerLoadout, []PlayerLoadoutItem, error)
	EquipLoadoutItem(ctx context.Context, userID uuid.UUID, cosmeticID string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error)
	UnequipLoadoutSlot(ctx context.Context, userID uuid.UUID, slot string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error)
	SetFullLoadout(ctx context.Context, userID uuid.UUID, cosmeticIDs []string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error)
}

type cosmeticsRepo struct {
	pool *pgxpool.Pool
}

func NewCosmeticsRepository(pool *pgxpool.Pool) CosmeticsRepository {
	return &cosmeticsRepo{pool: pool}
}

func (r *cosmeticsRepo) ListDefinitions(ctx context.Context, category, slot, rarity, search string, limit, offset int) ([]CosmeticDefinition, int, error) {
	if limit <= 0 || limit > 100 {
		limit = 50
	}
	if offset < 0 {
		offset = 0
	}

	var conditions []string
	var args []any
	argIdx := 1

	conditions = append(conditions, "is_available = true")

	if category != "" && category != "all" {
		conditions = append(conditions, fmt.Sprintf("LOWER(category) = LOWER($%d)", argIdx))
		args = append(args, category)
		argIdx++
	}

	if slot != "" && slot != "all" {
		conditions = append(conditions, fmt.Sprintf("LOWER(equipment_slot) = LOWER($%d)", argIdx))
		args = append(args, slot)
		argIdx++
	}

	if rarity != "" && rarity != "all" {
		conditions = append(conditions, fmt.Sprintf("LOWER(rarity) = LOWER($%d)", argIdx))
		args = append(args, rarity)
		argIdx++
	}

	if search != "" {
		conditions = append(conditions, fmt.Sprintf("(LOWER(display_name) LIKE LOWER($%d) OR LOWER(description) LIKE LOWER($%d))", argIdx, argIdx))
		args = append(args, "%"+search+"%")
		argIdx++
	}

	whereClause := ""
	if len(conditions) > 0 {
		whereClause = "WHERE " + strings.Join(conditions, " AND ")
	}

	countQuery := fmt.Sprintf("SELECT COUNT(*) FROM cosmetic_definitions %s", whereClause)
	var total int
	err := r.pool.QueryRow(ctx, countQuery, args...).Scan(&total)
	if err != nil {
		return nil, 0, fmt.Errorf("failed to count cosmetic definitions: %w", err)
	}

	query := fmt.Sprintf(`
		SELECT id, cosmetic_key, display_name, category, equipment_slot, rarity, description, asset_id, is_available, created_at
		FROM cosmetic_definitions
		%s
		ORDER BY category ASC, display_name ASC
		LIMIT $%d OFFSET $%d
	`, whereClause, argIdx, argIdx+1)

	args = append(args, limit, offset)

	rows, err := r.pool.Query(ctx, query, args...)
	if err != nil {
		return nil, 0, fmt.Errorf("failed to query cosmetic definitions: %w", err)
	}
	defer rows.Close()

	defs := make([]CosmeticDefinition, 0)
	for rows.Next() {
		var def CosmeticDefinition
		if err := rows.Scan(
			&def.ID, &def.CosmeticKey, &def.DisplayName,
			&def.Category, &def.EquipmentSlot, &def.Rarity, &def.Description,
			&def.AssetID, &def.IsAvailable, &def.CreatedAt,
		); err != nil {
			return nil, 0, fmt.Errorf("failed to scan cosmetic definition: %w", err)
		}
		defs = append(defs, def)
	}
	return defs, total, rows.Err()
}

func (r *cosmeticsRepo) GetDefinitionByID(ctx context.Context, id string) (*CosmeticDefinition, error) {
	query := `
		SELECT id, cosmetic_key, display_name, category, equipment_slot, rarity, description, asset_id, is_available, created_at
		FROM cosmetic_definitions
		WHERE id = $1
	`
	var def CosmeticDefinition
	err := r.pool.QueryRow(ctx, query, id).Scan(
		&def.ID, &def.CosmeticKey, &def.DisplayName,
		&def.Category, &def.EquipmentSlot, &def.Rarity, &def.Description,
		&def.AssetID, &def.IsAvailable, &def.CreatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrCosmeticNotFound
		}
		return nil, fmt.Errorf("failed to query cosmetic definition: %w", err)
	}
	return &def, nil
}

func (r *cosmeticsRepo) ListPlayerCosmetics(ctx context.Context, userID uuid.UUID) ([]PlayerCosmetic, error) {
	query := `
		SELECT id, user_id, cosmetic_id, acquired_at, acquisition_source, is_equipped
		FROM player_cosmetics
		WHERE user_id = $1
		ORDER BY acquired_at ASC
	`
	rows, err := r.pool.Query(ctx, query, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to query player cosmetics: %w", err)
	}
	defer rows.Close()

	items := make([]PlayerCosmetic, 0)
	for rows.Next() {
		var item PlayerCosmetic
		if err := rows.Scan(
			&item.ID, &item.UserID, &item.CosmeticID,
			&item.AcquiredAt, &item.AcquisitionSource, &item.IsEquipped,
		); err != nil {
			return nil, fmt.Errorf("failed to scan player cosmetic: %w", err)
		}
		items = append(items, item)
	}
	return items, rows.Err()
}

func (r *cosmeticsRepo) CheckOwnership(ctx context.Context, userID uuid.UUID, cosmeticID string) (bool, error) {
	query := `SELECT EXISTS(SELECT 1 FROM player_cosmetics WHERE user_id = $1 AND cosmetic_id = $2)`
	var exists bool
	err := r.pool.QueryRow(ctx, query, userID, cosmeticID).Scan(&exists)
	if err != nil {
		return false, fmt.Errorf("failed to check cosmetic ownership: %w", err)
	}
	return exists, nil
}

func (r *cosmeticsRepo) GrantCosmetic(ctx context.Context, tx pgx.Tx, userID uuid.UUID, cosmeticID string, source string, isEquipped bool) (*PlayerCosmetic, error) {
	query := `
		INSERT INTO player_cosmetics (user_id, cosmetic_id, acquisition_source, is_equipped)
		VALUES ($1, $2, $3, $4)
		ON CONFLICT (user_id, cosmetic_id)
		DO UPDATE SET acquisition_source = EXCLUDED.acquisition_source
		RETURNING id, user_id, cosmetic_id, acquired_at, acquisition_source, is_equipped
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	var item PlayerCosmetic
	err := runner.QueryRow(ctx, query, userID, cosmeticID, source, isEquipped).Scan(
		&item.ID, &item.UserID, &item.CosmeticID,
		&item.AcquiredAt, &item.AcquisitionSource, &item.IsEquipped,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to grant cosmetic: %w", err)
	}
	return &item, nil
}

func (r *cosmeticsRepo) ensureLoadoutRecord(ctx context.Context, tx pgx.Tx, userID uuid.UUID) (*PlayerLoadout, error) {
	query := `
		INSERT INTO player_loadouts (user_id, loadout_version, created_at, updated_at)
		VALUES ($1, 1, NOW(), NOW())
		ON CONFLICT (user_id) DO UPDATE SET updated_at = player_loadouts.updated_at
		RETURNING id, user_id, loadout_version, created_at, updated_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	var l PlayerLoadout
	err := runner.QueryRow(ctx, query, userID).Scan(&l.ID, &l.UserID, &l.LoadoutVersion, &l.CreatedAt, &l.UpdatedAt)
	if err != nil {
		return nil, fmt.Errorf("failed to ensure loadout record: %w", err)
	}
	return &l, nil
}

func (r *cosmeticsRepo) GetPlayerLoadout(ctx context.Context, userID uuid.UUID) (*PlayerLoadout, []PlayerLoadoutItem, error) {
	loadout, err := r.ensureLoadoutRecord(ctx, nil, userID)
	if err != nil {
		return nil, nil, err
	}

	itemsQuery := `
		SELECT id, loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at
		FROM player_loadout_items
		WHERE user_id = $1
		ORDER BY equipment_slot ASC
	`
	rows, err := r.pool.Query(ctx, itemsQuery, userID)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to query loadout items: %w", err)
	}
	defer rows.Close()

	items := make([]PlayerLoadoutItem, 0)
	for rows.Next() {
		var item PlayerLoadoutItem
		if err := rows.Scan(
			&item.ID, &item.LoadoutID, &item.UserID, &item.CosmeticID,
			&item.EquipmentSlot, &item.EquippedAt,
		); err != nil {
			return nil, nil, fmt.Errorf("failed to scan loadout item: %w", err)
		}
		items = append(items, item)
	}

	return loadout, items, rows.Err()
}

func (r *cosmeticsRepo) EquipLoadoutItem(ctx context.Context, userID uuid.UUID, cosmeticID string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error) {
	def, err := r.GetDefinitionByID(ctx, cosmeticID)
	if err != nil {
		return nil, nil, err
	}
	if !def.IsAvailable {
		return nil, nil, ErrCosmeticUnavailable
	}

	owned, err := r.CheckOwnership(ctx, userID, cosmeticID)
	if err != nil {
		return nil, nil, err
	}
	if !owned {
		return nil, nil, ErrCosmeticNotOwned
	}

	var loadout *PlayerLoadout
	var items []PlayerLoadoutItem

	err = database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		current, err := r.ensureLoadoutRecord(ctx, tx, userID)
		if err != nil {
			return err
		}

		if expectedVersion > 0 && current.LoadoutVersion != expectedVersion {
			return ErrLoadoutVersionConflict
		}

		// Increment version and bump updated_at
		updateLoadoutQuery := `
			UPDATE player_loadouts
			SET loadout_version = loadout_version + 1, updated_at = NOW()
			WHERE user_id = $1
			RETURNING id, user_id, loadout_version, created_at, updated_at
		`
		var updated PlayerLoadout
		err = tx.QueryRow(ctx, updateLoadoutQuery, userID).Scan(
			&updated.ID, &updated.UserID, &updated.LoadoutVersion, &updated.CreatedAt, &updated.UpdatedAt,
		)
		if err != nil {
			return fmt.Errorf("failed to increment loadout version: %w", err)
		}
		loadout = &updated

		// Upsert into player_loadout_items for this slot
		upsertItemQuery := `
			INSERT INTO player_loadout_items (
				loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at
			) VALUES ($1, $2, $3, $4, NOW())
			ON CONFLICT (user_id, equipment_slot)
			DO UPDATE SET cosmetic_id = EXCLUDED.cosmetic_id, equipped_at = NOW()
		`
		if _, err := tx.Exec(ctx, upsertItemQuery, updated.ID, userID, cosmeticID, def.EquipmentSlot); err != nil {
			return fmt.Errorf("failed to upsert loadout item: %w", err)
		}

		// Sync is_equipped in player_cosmetics table
		unequipOldQuery := `
			UPDATE player_cosmetics pc
			SET is_equipped = false
			FROM cosmetic_definitions cd
			WHERE pc.cosmetic_id = cd.id
			  AND pc.user_id = $1
			  AND cd.equipment_slot = $2
		`
		if _, err := tx.Exec(ctx, unequipOldQuery, userID, def.EquipmentSlot); err != nil {
			return fmt.Errorf("failed to unequip old cosmetic category: %w", err)
		}

		equipNewQuery := `UPDATE player_cosmetics SET is_equipped = true WHERE user_id = $1 AND cosmetic_id = $2`
		if _, err := tx.Exec(ctx, equipNewQuery, userID, cosmeticID); err != nil {
			return fmt.Errorf("failed to set is_equipped true: %w", err)
		}

		// Read back all items in the loadout
		itemsQuery := `
			SELECT id, loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at
			FROM player_loadout_items
			WHERE user_id = $1
			ORDER BY equipment_slot ASC
		`
		rows, err := tx.Query(ctx, itemsQuery, userID)
		if err != nil {
			return fmt.Errorf("failed to query loadout items: %w", err)
		}
		defer rows.Close()

		items = make([]PlayerLoadoutItem, 0)
		for rows.Next() {
			var it PlayerLoadoutItem
			if err := rows.Scan(
				&it.ID, &it.LoadoutID, &it.UserID, &it.CosmeticID,
				&it.EquipmentSlot, &it.EquippedAt,
			); err != nil {
				return fmt.Errorf("failed to scan loadout item: %w", err)
			}
			items = append(items, it)
		}

		return rows.Err()
	})

	if err != nil {
		return nil, nil, err
	}
	return loadout, items, nil
}

func (r *cosmeticsRepo) UnequipLoadoutSlot(ctx context.Context, userID uuid.UUID, slot string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error) {
	var loadout *PlayerLoadout
	var items []PlayerLoadoutItem

	err := database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		current, err := r.ensureLoadoutRecord(ctx, tx, userID)
		if err != nil {
			return err
		}

		if expectedVersion > 0 && current.LoadoutVersion != expectedVersion {
			return ErrLoadoutVersionConflict
		}

		// Find item currently in this slot to update player_cosmetics
		var oldCosmeticID string
		_ = tx.QueryRow(ctx, "SELECT cosmetic_id FROM player_loadout_items WHERE user_id = $1 AND LOWER(equipment_slot) = LOWER($2)", userID, slot).Scan(&oldCosmeticID)

		// Increment version
		updateLoadoutQuery := `
			UPDATE player_loadouts
			SET loadout_version = loadout_version + 1, updated_at = NOW()
			WHERE user_id = $1
			RETURNING id, user_id, loadout_version, created_at, updated_at
		`
		var updated PlayerLoadout
		err = tx.QueryRow(ctx, updateLoadoutQuery, userID).Scan(
			&updated.ID, &updated.UserID, &updated.LoadoutVersion, &updated.CreatedAt, &updated.UpdatedAt,
		)
		if err != nil {
			return fmt.Errorf("failed to increment loadout version: %w", err)
		}
		loadout = &updated

		// Delete item from slot
		deleteQuery := `DELETE FROM player_loadout_items WHERE user_id = $1 AND LOWER(equipment_slot) = LOWER($2)`
		if _, err := tx.Exec(ctx, deleteQuery, userID, slot); err != nil {
			return fmt.Errorf("failed to delete loadout item: %w", err)
		}

		if oldCosmeticID != "" {
			_, _ = tx.Exec(ctx, "UPDATE player_cosmetics SET is_equipped = false WHERE user_id = $1 AND cosmetic_id = $2", userID, oldCosmeticID)
		}

		// Read back remaining items
		itemsQuery := `
			SELECT id, loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at
			FROM player_loadout_items
			WHERE user_id = $1
			ORDER BY equipment_slot ASC
		`
		rows, err := tx.Query(ctx, itemsQuery, userID)
		if err != nil {
			return fmt.Errorf("failed to query loadout items: %w", err)
		}
		defer rows.Close()

		items = make([]PlayerLoadoutItem, 0)
		for rows.Next() {
			var it PlayerLoadoutItem
			if err := rows.Scan(
				&it.ID, &it.LoadoutID, &it.UserID, &it.CosmeticID,
				&it.EquipmentSlot, &it.EquippedAt,
			); err != nil {
				return fmt.Errorf("failed to scan loadout item: %w", err)
			}
			items = append(items, it)
		}

		return rows.Err()
	})

	if err != nil {
		return nil, nil, err
	}
	return loadout, items, nil
}

func (r *cosmeticsRepo) SetFullLoadout(ctx context.Context, userID uuid.UUID, cosmeticIDs []string, expectedVersion int) (*PlayerLoadout, []PlayerLoadoutItem, error) {
	// Verify definitions and ownership for all items upfront
	slotToDef := make(map[string]*CosmeticDefinition)
	for _, cid := range cosmeticIDs {
		if cid == "" {
			continue
		}
		def, err := r.GetDefinitionByID(ctx, cid)
		if err != nil {
			return nil, nil, err
		}
		if !def.IsAvailable {
			return nil, nil, ErrCosmeticUnavailable
		}
		owned, err := r.CheckOwnership(ctx, userID, cid)
		if err != nil {
			return nil, nil, err
		}
		if !owned {
			return nil, nil, fmt.Errorf("%w: %s", ErrCosmeticNotOwned, cid)
		}
		slotToDef[def.EquipmentSlot] = def
	}

	var loadout *PlayerLoadout
	var items []PlayerLoadoutItem

	err := database.WithTransaction(ctx, r.pool, func(tx pgx.Tx) error {
		current, err := r.ensureLoadoutRecord(ctx, tx, userID)
		if err != nil {
			return err
		}

		if expectedVersion > 0 && current.LoadoutVersion != expectedVersion {
			return ErrLoadoutVersionConflict
		}

		// Increment version
		updateLoadoutQuery := `
			UPDATE player_loadouts
			SET loadout_version = loadout_version + 1, updated_at = NOW()
			WHERE user_id = $1
			RETURNING id, user_id, loadout_version, created_at, updated_at
		`
		var updated PlayerLoadout
		err = tx.QueryRow(ctx, updateLoadoutQuery, userID).Scan(
			&updated.ID, &updated.UserID, &updated.LoadoutVersion, &updated.CreatedAt, &updated.UpdatedAt,
		)
		if err != nil {
			return fmt.Errorf("failed to increment loadout version: %w", err)
		}
		loadout = &updated

		// Clear previous loadout items
		if _, err := tx.Exec(ctx, "DELETE FROM player_loadout_items WHERE user_id = $1", userID); err != nil {
			return fmt.Errorf("failed to clear old loadout items: %w", err)
		}
		if _, err := tx.Exec(ctx, "UPDATE player_cosmetics SET is_equipped = false WHERE user_id = $1", userID); err != nil {
			return fmt.Errorf("failed to reset is_equipped: %w", err)
		}

		// Insert new items
		insertQuery := `
			INSERT INTO player_loadout_items (loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at)
			VALUES ($1, $2, $3, $4, NOW())
		`
		for slot, def := range slotToDef {
			if _, err := tx.Exec(ctx, insertQuery, updated.ID, userID, def.ID, slot); err != nil {
				return fmt.Errorf("failed to insert loadout item (%s): %w", def.ID, err)
			}
			_, _ = tx.Exec(ctx, "UPDATE player_cosmetics SET is_equipped = true WHERE user_id = $1 AND cosmetic_id = $2", userID, def.ID)
		}

		// Read back items
		rows, err := tx.Query(ctx, "SELECT id, loadout_id, user_id, cosmetic_id, equipment_slot, equipped_at FROM player_loadout_items WHERE user_id = $1 ORDER BY equipment_slot ASC", userID)
		if err != nil {
			return fmt.Errorf("failed to query loadout items: %w", err)
		}
		defer rows.Close()

		items = make([]PlayerLoadoutItem, 0)
		for rows.Next() {
			var it PlayerLoadoutItem
			if err := rows.Scan(
				&it.ID, &it.LoadoutID, &it.UserID, &it.CosmeticID,
				&it.EquipmentSlot, &it.EquippedAt,
			); err != nil {
				return fmt.Errorf("failed to scan loadout item: %w", err)
			}
			items = append(items, it)
		}

		return rows.Err()
	})

	if err != nil {
		return nil, nil, err
	}
	return loadout, items, nil
}
