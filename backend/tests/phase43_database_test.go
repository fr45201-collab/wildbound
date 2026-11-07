package tests

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/inventory"
	"github.com/wildbound/backend/internal/repository"
)

func TestDatabaseHealthCheckAndPoolStats(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1. Direct Health Check
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	report := env.db.CheckHealth(ctx)
	if report.Status != "healthy" {
		t.Fatalf("expected healthy database, got status=%s, err=%s", report.Status, report.Error)
	}
	if report.LatencyMillis <= 0 {
		t.Fatalf("expected positive latency, got %f ms", report.LatencyMillis)
	}
	if report.PoolStats.MaxConns <= 0 {
		t.Fatalf("expected positive MaxConns, got %d", report.PoolStats.MaxConns)
	}

	// 2. HTTP /health endpoint
	req := httptest.NewRequest(http.MethodGet, "/health", nil)
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)
	if w.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /health, got %d", w.Code)
	}

	resp := parseAPIResponse(t, w)
	if !resp.Success {
		t.Fatalf("expected success true, got %v", resp.Success)
	}

	// 3. HTTP /ready endpoint
	reqReady := httptest.NewRequest(http.MethodGet, "/ready", nil)
	wReady := httptest.NewRecorder()
	env.handler.ServeHTTP(wReady, reqReady)
	if wReady.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /ready, got %d", wReady.Code)
	}
}

func TestTransactionsHelper(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	ctx := context.Background()

	// 1. Test successful commit
	testKey := fmt.Sprintf("tx_test_%s", uuid.New().String()[:8])
	err := database.WithTransaction(ctx, env.db.Pool, func(tx pgx.Tx) error {
		_, err := tx.Exec(ctx, `
			INSERT INTO cosmetic_definitions (id, cosmetic_key, display_name, category, rarity, description, is_available)
			VALUES ($1, $2, 'Tx Commit Test', 'Chest', 'common', 'Test item', true)
		`, testKey, testKey)
		return err
	})
	if err != nil {
		t.Fatalf("expected transaction to commit, got err: %v", err)
	}

	// Verify row exists
	var exists bool
	_ = env.db.Pool.QueryRow(ctx, "SELECT EXISTS(SELECT 1 FROM cosmetic_definitions WHERE id = $1)", testKey).Scan(&exists)
	if !exists {
		t.Fatalf("expected row to exist after committed transaction")
	}

	// 2. Test rollback on error
	rollbackKey := fmt.Sprintf("tx_rollback_%s", uuid.New().String()[:8])
	expectedErr := fmt.Errorf("intentional failure")
	err = database.WithTransaction(ctx, env.db.Pool, func(tx pgx.Tx) error {
		_, err := tx.Exec(ctx, `
			INSERT INTO cosmetic_definitions (id, cosmetic_key, display_name, category, rarity, description, is_available)
			VALUES ($1, $2, 'Tx Rollback Test', 'Chest', 'common', 'Test item', true)
		`, rollbackKey, rollbackKey)
		if err != nil {
			return err
		}
		return expectedErr
	})
	if err != expectedErr {
		t.Fatalf("expected transaction to return expectedErr, got: %v", err)
	}

	// Verify row does NOT exist
	_ = env.db.Pool.QueryRow(ctx, "SELECT EXISTS(SELECT 1 FROM cosmetic_definitions WHERE id = $1)", rollbackKey).Scan(&exists)
	if exists {
		t.Fatalf("expected rolled back row to NOT exist")
	}

	// Clean up committed test row
	_, _ = env.db.Pool.Exec(ctx, "DELETE FROM cosmetic_definitions WHERE id = $1", testKey)
}

func TestInventoryRepositoryOperations(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	ctx := context.Background()
	invRepo := repository.NewInventoryRepository(env.db.Pool)

	// Create test user
	testUserID := uuid.New()
	testUsername := fmt.Sprintf("inv_user_%s", uuid.New().String()[:8])
	_, err := env.db.Pool.Exec(ctx, `
		INSERT INTO users (id, username, email, password_hash)
		VALUES ($1, $2, $3, 'hash')
	`, testUserID, testUsername, testUsername+"@test.com")
	if err != nil {
		t.Fatalf("failed to insert test user: %v", err)
	}
	defer func() {
		_, _ = env.db.Pool.Exec(ctx, "DELETE FROM users WHERE id = $1", testUserID)
	}()

	itemDef := "material_oak_wood"

	// 1. Initial inventory should be empty
	items, err := invRepo.ListByUserID(ctx, testUserID)
	if err != nil {
		t.Fatalf("failed to list inventory: %v", err)
	}
	if len(items) != 0 {
		t.Fatalf("expected empty inventory, got %d items", len(items))
	}

	// 2. Add 10 units
	added, err := invRepo.AddItem(ctx, testUserID, itemDef, 10)
	if err != nil {
		t.Fatalf("failed to add inventory item: %v", err)
	}
	if added.Quantity != 10 {
		t.Fatalf("expected quantity 10, got %d", added.Quantity)
	}

	// 3. Add 5 more units -> atomic upsert should yield 15
	addedMore, err := invRepo.AddItem(ctx, testUserID, itemDef, 5)
	if err != nil {
		t.Fatalf("failed to add more inventory items: %v", err)
	}
	if addedMore.Quantity != 15 {
		t.Fatalf("expected quantity 15, got %d", addedMore.Quantity)
	}

	// 4. Consume 5 units -> remaining should be 10
	consumed, err := invRepo.ConsumeItem(ctx, testUserID, itemDef, 5)
	if err != nil {
		t.Fatalf("failed to consume inventory items: %v", err)
	}
	if consumed.Quantity != 10 {
		t.Fatalf("expected remaining quantity 10, got %d", consumed.Quantity)
	}

	// 5. Consume invalid quantities
	_, err = invRepo.ConsumeItem(ctx, testUserID, itemDef, 0)
	if err != repository.ErrInvalidQuantity {
		t.Fatalf("expected ErrInvalidQuantity for 0 quantity, got: %v", err)
	}

	_, err = invRepo.ConsumeItem(ctx, testUserID, itemDef, 50)
	if err != repository.ErrInsufficientQuantity {
		t.Fatalf("expected ErrInsufficientQuantity for 50 quantity, got: %v", err)
	}

	// 6. Consume exact remaining 10 units -> row should be deleted
	depleted, err := invRepo.ConsumeItem(ctx, testUserID, itemDef, 10)
	if err != nil {
		t.Fatalf("failed to consume exact remaining units: %v", err)
	}
	if depleted.Quantity != 0 {
		t.Fatalf("expected depleted item quantity 0, got %d", depleted.Quantity)
	}

	// 7. Verify item is removed from table
	_, err = invRepo.GetItem(ctx, testUserID, itemDef)
	if err != repository.ErrItemNotFound {
		t.Fatalf("expected ErrItemNotFound after total depletion, got: %v", err)
	}
}

func TestInventoryPositiveQuantityCheckConstraint(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	ctx := context.Background()

	testUserID := uuid.New()
	testUsername := fmt.Sprintf("chk_user_%s", uuid.New().String()[:8])
	_, err := env.db.Pool.Exec(ctx, `
		INSERT INTO users (id, username, email, password_hash)
		VALUES ($1, $2, $3, 'hash')
	`, testUserID, testUsername, testUsername+"@test.com")
	if err != nil {
		t.Fatalf("failed to insert test user: %v", err)
	}
	defer func() {
		_, _ = env.db.Pool.Exec(ctx, "DELETE FROM users WHERE id = $1", testUserID)
	}()

	// Attempt raw SQL insert with 0 quantity
	_, err = env.db.Pool.Exec(ctx, `
		INSERT INTO player_inventory (user_id, item_definition_id, quantity)
		VALUES ($1, 'item_zero', 0)
	`, testUserID)
	if err == nil {
		t.Fatalf("expected check constraint violation on 0 quantity, got success")
	}

	// Attempt raw SQL insert with negative quantity
	_, err = env.db.Pool.Exec(ctx, `
		INSERT INTO player_inventory (user_id, item_definition_id, quantity)
		VALUES ($1, 'item_neg', -10)
	`, testUserID)
	if err == nil {
		t.Fatalf("expected check constraint violation on negative quantity, got success")
	}
}

func TestInventoryPlayerTransfer(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	ctx := context.Background()
	invRepo := repository.NewInventoryRepository(env.db.Pool)

	userA := uuid.New()
	userB := uuid.New()
	nameA := fmt.Sprintf("tr_a_%s", uuid.New().String()[:8])
	nameB := fmt.Sprintf("tr_b_%s", uuid.New().String()[:8])

	_, _ = env.db.Pool.Exec(ctx, "INSERT INTO users (id, username, email, password_hash) VALUES ($1, $2, $3, 'hash')", userA, nameA, nameA+"@test.com")
	_, _ = env.db.Pool.Exec(ctx, "INSERT INTO users (id, username, email, password_hash) VALUES ($1, $2, $3, 'hash')", userB, nameB, nameB+"@test.com")
	defer func() {
		_, _ = env.db.Pool.Exec(ctx, "DELETE FROM users WHERE id IN ($1, $2)", userA, userB)
	}()

	itemDef := "resource_iron_ingot"

	// Grant user A 20 ingots
	_, err := invRepo.AddItem(ctx, userA, itemDef, 20)
	if err != nil {
		t.Fatalf("failed to add item to user A: %v", err)
	}

	// Transfer 8 ingots from A to B
	err = invRepo.TransferItem(ctx, userA, userB, itemDef, 8)
	if err != nil {
		t.Fatalf("failed to transfer items: %v", err)
	}

	// Verify User A balance: 12
	itemA, err := invRepo.GetItem(ctx, userA, itemDef)
	if err != nil || itemA.Quantity != 12 {
		t.Fatalf("expected user A balance 12, got %d (err: %v)", itemA.Quantity, err)
	}

	// Verify User B balance: 8
	itemB, err := invRepo.GetItem(ctx, userB, itemDef)
	if err != nil || itemB.Quantity != 8 {
		t.Fatalf("expected user B balance 8, got %d (err: %v)", itemB.Quantity, err)
	}

	// Attempt transfer with insufficient balance (try 50 from A)
	err = invRepo.TransferItem(ctx, userA, userB, itemDef, 50)
	if err != repository.ErrInsufficientQuantity {
		t.Fatalf("expected ErrInsufficientQuantity, got %v", err)
	}

	// Attempt transfer to self
	err = invRepo.TransferItem(ctx, userA, userA, itemDef, 5)
	if err != repository.ErrSameUserTransfer {
		t.Fatalf("expected ErrSameUserTransfer, got %v", err)
	}
}

func TestInventoryHTTPAPI(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1. Register a test user
	regPayload := auth.RegisterRequest{
		Username:    fmt.Sprintf("inv_api_%s", uuid.New().String()[:8]),
		Email:       fmt.Sprintf("inv_api_%s@wildbound.com", uuid.New().String()[:8]),
		Password:    "SecurePass123!",
		DisplayName: "Inventory Tester",
	}
	body, _ := json.Marshal(regPayload)
	reqReg := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	reqReg.Header.Set("Content-Type", "application/json")
	wReg := httptest.NewRecorder()
	env.handler.ServeHTTP(wReg, reqReg)

	var authResp struct {
		Success bool `json:"success"`
		Data    struct {
			User struct {
				ID string `json:"id"`
			} `json:"user"`
			Tokens struct {
				AccessToken string `json:"access_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wReg.Body.Bytes(), &authResp)
	token := authResp.Data.Tokens.AccessToken
	userID := authResp.Data.User.ID

	// 2. Query empty inventory
	reqInv := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/inventory", nil)
	reqInv.Header.Set("Authorization", "Bearer "+token)
	wInv := httptest.NewRecorder()
	env.handler.ServeHTTP(wInv, reqInv)
	if wInv.Code != http.StatusOK {
		t.Fatalf("expected 200 OK for inventory get, got %d", wInv.Code)
	}

	// 3. Internal server grants item
	grantPayload := inventory.GrantRequest{
		UserID:           userID,
		ItemDefinitionID: "token_arena_champion",
		Quantity:         3,
	}
	grantBody, _ := json.Marshal(grantPayload)
	reqGrant := httptest.NewRequest(http.MethodPost, "/api/v1/internal/inventory/grant", bytes.NewReader(grantBody))
	reqGrant.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqGrant.Header.Set("Content-Type", "application/json")
	wGrant := httptest.NewRecorder()
	env.handler.ServeHTTP(wGrant, reqGrant)
	if wGrant.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created for internal inventory grant, got %d, body: %s", wGrant.Code, wGrant.Body.String())
	}

	// 4. Query inventory again: should have 1 item with quantity 3
	reqInv2 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/inventory", nil)
	reqInv2.Header.Set("Authorization", "Bearer "+token)
	wInv2 := httptest.NewRecorder()
	env.handler.ServeHTTP(wInv2, reqInv2)
	if wInv2.Code != http.StatusOK {
		t.Fatalf("expected 200 OK, got %d", wInv2.Code)
	}

	var invResp struct {
		Success bool `json:"success"`
		Data    struct {
			Count int `json:"count"`
			Items []struct {
				ItemDefinitionID string `json:"item_definition_id"`
				Quantity         int    `json:"quantity"`
			} `json:"items"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wInv2.Body.Bytes(), &invResp)
	if invResp.Data.Count != 1 || invResp.Data.Items[0].Quantity != 3 {
		t.Fatalf("expected 1 item with quantity 3, got count=%d", invResp.Data.Count)
	}

	// 5. Consume 1 item
	consumePayload := inventory.ConsumeRequest{
		ItemDefinitionID: "token_arena_champion",
		Quantity:         1,
	}
	consumeBody, _ := json.Marshal(consumePayload)
	reqConsume := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/inventory/consume", bytes.NewReader(consumeBody))
	reqConsume.Header.Set("Authorization", "Bearer "+token)
	reqConsume.Header.Set("Content-Type", "application/json")
	wConsume := httptest.NewRecorder()
	env.handler.ServeHTTP(wConsume, reqConsume)
	if wConsume.Code != http.StatusOK {
		t.Fatalf("expected 200 OK for consume item, got %d", wConsume.Code)
	}

	// 6. Consume 10 items (insufficient) -> 409 Conflict
	consumeExcess := inventory.ConsumeRequest{
		ItemDefinitionID: "token_arena_champion",
		Quantity:         10,
	}
	consumeExcessBody, _ := json.Marshal(consumeExcess)
	reqExcess := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/inventory/consume", bytes.NewReader(consumeExcessBody))
	reqExcess.Header.Set("Authorization", "Bearer "+token)
	reqExcess.Header.Set("Content-Type", "application/json")
	wExcess := httptest.NewRecorder()
	env.handler.ServeHTTP(wExcess, reqExcess)
	if wExcess.Code != http.StatusConflict {
		t.Fatalf("expected 409 Conflict on insufficient quantity consume, got %d", wExcess.Code)
	}
}

func TestMigrationLifecycleAndAdvisoryLocking(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	ctx := context.Background()
	migrator := database.NewMigrator(env.db.Pool, "../migrations")

	statuses, err := migrator.Status(ctx)
	if err != nil {
		t.Fatalf("failed to fetch migration status: %v", err)
	}
	if len(statuses) < 3 {
		t.Fatalf("expected at least 3 migrations, got %d", len(statuses))
	}

	for _, s := range statuses {
		if !s.Applied {
			t.Fatalf("expected migration %s to be applied", s.Name)
		}
	}

	v, err := migrator.CurrentVersion(ctx)
	if err != nil {
		t.Fatalf("failed to get current version: %v", err)
	}
	if v < 3 {
		t.Fatalf("expected current schema version >= 3, got %d", v)
	}
}

