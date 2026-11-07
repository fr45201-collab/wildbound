package tests

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/cosmetics"
)

// Helper to create and authenticate a test user
func createCosmeticTestUser(t *testing.T, env *TestEnv) (string, string) {
	username := fmt.Sprintf("cosm_user_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)

	regPayload := auth.RegisterRequest{
		Username:         username,
		Email:            email,
		Password:         "Wildbound2026!Sec",
		DisplayName:      "Cosmetic Tester",
		DeviceIdentifier: "test_device_cosm",
		DeviceName:       "Cosmetic Rig",
	}
	body, _ := json.Marshal(regPayload)
	req := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created from register, got %d: %s", w.Code, w.Body.String())
	}

	var regResp struct {
		Data struct {
			User struct {
				ID string `json:"id"`
			} `json:"user"`
			Tokens struct {
				AccessToken string `json:"access_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	if err := json.Unmarshal(w.Body.Bytes(), &regResp); err != nil {
		t.Fatalf("failed to unmarshal register response: %v", err)
	}

	return regResp.Data.User.ID, regResp.Data.Tokens.AccessToken
}

// 1. Test Catalogue Retrieval and Filtering
func TestCosmeticCatalogueRetrievalAndFilters(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1.1 Unfiltered catalogue
	req := httptest.NewRequest(http.MethodGet, "/api/v1/cosmetics?limit=50", nil)
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from catalogue, got %d: %s", w.Code, w.Body.String())
	}

	var catResp struct {
		Data cosmetics.CatalogueResponse `json:"data"`
	}
	if err := json.Unmarshal(w.Body.Bytes(), &catResp); err != nil {
		t.Fatalf("failed to parse catalogue response: %v", err)
	}

	if catResp.Data.Total < 50 {
		t.Fatalf("expected at least 50 cosmetics seeded, got %d", catResp.Data.Total)
	}

	// 1.2 Category filter: clothing
	reqArmor := httptest.NewRequest(http.MethodGet, "/api/v1/cosmetics?category=clothing", nil)
	wArmor := httptest.NewRecorder()
	env.handler.ServeHTTP(wArmor, reqArmor)
	if wArmor.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from clothing filter, got %d", wArmor.Code)
	}

	var armorResp struct {
		Data cosmetics.CatalogueResponse `json:"data"`
	}
	_ = json.Unmarshal(wArmor.Body.Bytes(), &armorResp)
	if armorResp.Data.Total == 0 {
		t.Fatalf("expected non-zero clothing cosmetics")
	}
	for _, it := range armorResp.Data.Items {
		if !strings.EqualFold(it.Category, "clothing") {
			t.Fatalf("expected clothing category, got %s", it.Category)
		}
	}

	// 1.3 Slot filter: chest
	reqChest := httptest.NewRequest(http.MethodGet, "/api/v1/cosmetics?slot=chest", nil)
	wChest := httptest.NewRecorder()
	env.handler.ServeHTTP(wChest, reqChest)
	if wChest.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from slot filter, got %d", wChest.Code)
	}

	var chestResp struct {
		Data cosmetics.CatalogueResponse `json:"data"`
	}
	_ = json.Unmarshal(wChest.Body.Bytes(), &chestResp)
	if chestResp.Data.Total == 0 {
		t.Fatalf("expected non-zero chest cosmetics")
	}
	for _, it := range chestResp.Data.Items {
		if !strings.EqualFold(it.EquipmentSlot, "chest") {
			t.Fatalf("expected chest slot, got %s", it.EquipmentSlot)
		}
	}

	// 1.4 Search filter
	reqSearch := httptest.NewRequest(http.MethodGet, "/api/v1/cosmetics?search=traveler", nil)
	wSearch := httptest.NewRecorder()
	env.handler.ServeHTTP(wSearch, reqSearch)
	if wSearch.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from search filter, got %d", wSearch.Code)
	}

	var searchResp struct {
		Data cosmetics.CatalogueResponse `json:"data"`
	}
	_ = json.Unmarshal(wSearch.Body.Bytes(), &searchResp)
	if searchResp.Data.Total == 0 {
		t.Fatalf("expected search 'traveler' to find items, got 0")
	}
}

// 2. Test Cosmetic Ownership and Server Grant
func TestCosmeticOwnershipAndGrant(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	userID, token := createCosmeticTestUser(t, env)
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// 2.1 Fetch initial cosmetics (starter cosmetics granted on registration)
	reqOwn := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/cosmetics", nil)
	reqOwn.Header.Set("Authorization", "Bearer "+token)
	wOwn := httptest.NewRecorder()
	env.handler.ServeHTTP(wOwn, reqOwn)

	if wOwn.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me/cosmetics, got %d: %s", wOwn.Code, wOwn.Body.String())
	}

	var ownResp struct {
		Data []cosmetics.PlayerCosmeticItemDTO `json:"data"`
	}
	if err := json.Unmarshal(wOwn.Body.Bytes(), &ownResp); err != nil {
		t.Fatalf("failed to parse ownership response: %v", err)
	}
	initCount := len(ownResp.Data)

	// 2.2 Grant a new cosmetic via internal API
	grantPayload := map[string]string{
		"user_id":     userID,
		"cosmetic_id": "clo_royal_outfit_001",
		"source":      "test_reward",
	}
	grantBody, _ := json.Marshal(grantPayload)
	reqGrant := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grantBody))
	reqGrant.Header.Set("Content-Type", "application/json")
	reqGrant.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wGrant := httptest.NewRecorder()
	env.handler.ServeHTTP(wGrant, reqGrant)

	if wGrant.Code != http.StatusOK && wGrant.Code != http.StatusCreated {
		t.Fatalf("expected 200/201 from internal grant, got %d: %s", wGrant.Code, wGrant.Body.String())
	}

	// 2.3 Verify ownership now includes the granted cosmetic
	reqOwn2 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/cosmetics", nil)
	reqOwn2.Header.Set("Authorization", "Bearer "+token)
	wOwn2 := httptest.NewRecorder()
	env.handler.ServeHTTP(wOwn2, reqOwn2)

	if wOwn2.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me/cosmetics, got %d", wOwn2.Code)
	}
	_ = json.Unmarshal(wOwn2.Body.Bytes(), &ownResp)
	if len(ownResp.Data) != initCount+1 {
		t.Fatalf("expected %d owned items after grant, got %d", initCount+1, len(ownResp.Data))
	}
	found := false
	for _, it := range ownResp.Data {
		if it.CosmeticID == "clo_royal_outfit_001" {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("expected clo_royal_outfit_001 in owned cosmetics")
	}
}

// 3. Test Loadout Equip and Unequip with Versioning
func TestLoadoutEquipAndUnequip(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	userID, token := createCosmeticTestUser(t, env)
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// Grant chest and legs
	grant1, _ := json.Marshal(map[string]string{"user_id": userID, "cosmetic_id": "clo_basic_traveler_001", "source": "grant"})
	reqG1 := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grant1))
	reqG1.Header.Set("Content-Type", "application/json")
	reqG1.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wG1 := httptest.NewRecorder()
	env.handler.ServeHTTP(wG1, reqG1)
	if wG1.Code != http.StatusOK && wG1.Code != http.StatusCreated {
		t.Fatalf("failed grant 1: %d", wG1.Code)
	}

	grant2, _ := json.Marshal(map[string]string{"user_id": userID, "cosmetic_id": "clo_p_traveler_pants_001", "source": "grant"})
	reqG2 := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grant2))
	reqG2.Header.Set("Content-Type", "application/json")
	reqG2.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wG2 := httptest.NewRecorder()
	env.handler.ServeHTTP(wG2, reqG2)
	if wG2.Code != http.StatusOK && wG2.Code != http.StatusCreated {
		t.Fatalf("failed grant 2: %d", wG2.Code)
	}

	// 3.1 Check initial loadout
	reqL := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/loadout", nil)
	reqL.Header.Set("Authorization", "Bearer "+token)
	wL := httptest.NewRecorder()
	env.handler.ServeHTTP(wL, reqL)

	if wL.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from get loadout, got %d: %s", wL.Code, wL.Body.String())
	}

	var loadoutResp struct {
		Data cosmetics.PlayerLoadoutResponse `json:"data"`
	}
	if err := json.Unmarshal(wL.Body.Bytes(), &loadoutResp); err != nil {
		t.Fatalf("failed to parse loadout response: %v", err)
	}
	initVersion := loadoutResp.Data.LoadoutVersion
	if len(loadoutResp.Data.Slots) != 0 {
		t.Fatalf("expected 0 equipped items initially, got %d", len(loadoutResp.Data.Slots))
	}

	// 3.2 Equip chest item
	equip1, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: initVersion,
		CosmeticID:      "clo_basic_traveler_001",
	})
	reqEq1 := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equip1))
	reqEq1.Header.Set("Content-Type", "application/json")
	reqEq1.Header.Set("Authorization", "Bearer "+token)
	wEq1 := httptest.NewRecorder()
	env.handler.ServeHTTP(wEq1, reqEq1)

	if wEq1.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from equip 1, got %d: %s", wEq1.Code, wEq1.Body.String())
	}
	_ = json.Unmarshal(wEq1.Body.Bytes(), &loadoutResp)
	if loadoutResp.Data.LoadoutVersion != initVersion+1 {
		t.Fatalf("expected version %d, got %d", initVersion+1, loadoutResp.Data.LoadoutVersion)
	}
	if len(loadoutResp.Data.Slots) != 1 {
		t.Fatalf("expected 1 item equipped, got %d", len(loadoutResp.Data.Slots))
	}
	if loadoutResp.Data.Slots["chest"].CosmeticID != "clo_basic_traveler_001" {
		t.Fatalf("expected chest slot item clo_basic_traveler_001, got %s", loadoutResp.Data.Slots["chest"].CosmeticID)
	}

	// 3.3 Equip legs item
	equip2, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: loadoutResp.Data.LoadoutVersion,
		CosmeticID:      "clo_p_traveler_pants_001",
	})
	reqEq2 := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equip2))
	reqEq2.Header.Set("Content-Type", "application/json")
	reqEq2.Header.Set("Authorization", "Bearer "+token)
	wEq2 := httptest.NewRecorder()
	env.handler.ServeHTTP(wEq2, reqEq2)

	if wEq2.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from equip 2, got %d: %s", wEq2.Code, wEq2.Body.String())
	}
	_ = json.Unmarshal(wEq2.Body.Bytes(), &loadoutResp)
	if loadoutResp.Data.LoadoutVersion != initVersion+2 {
		t.Fatalf("expected version %d, got %d", initVersion+2, loadoutResp.Data.LoadoutVersion)
	}
	if len(loadoutResp.Data.Slots) != 2 {
		t.Fatalf("expected 2 items equipped, got %d", len(loadoutResp.Data.Slots))
	}

	// 3.4 Unequip chest item
	unequip, _ := json.Marshal(cosmetics.UnequipSlotRequest{
		ExpectedVersion: loadoutResp.Data.LoadoutVersion,
		Slot:            "chest",
	})
	reqUneq := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/unequip", bytes.NewReader(unequip))
	reqUneq.Header.Set("Content-Type", "application/json")
	reqUneq.Header.Set("Authorization", "Bearer "+token)
	wUneq := httptest.NewRecorder()
	env.handler.ServeHTTP(wUneq, reqUneq)

	if wUneq.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from unequip, got %d: %s", wUneq.Code, wUneq.Body.String())
	}
	loadoutResp = struct {
		Data cosmetics.PlayerLoadoutResponse `json:"data"`
	}{}
	_ = json.Unmarshal(wUneq.Body.Bytes(), &loadoutResp)
	if loadoutResp.Data.LoadoutVersion != initVersion+3 {
		t.Fatalf("expected version %d, got %d", initVersion+3, loadoutResp.Data.LoadoutVersion)
	}
	if len(loadoutResp.Data.Slots) != 1 {
		t.Fatalf("expected 1 item equipped after chest unequipped, got %d", len(loadoutResp.Data.Slots))
	}
	if _, hasLegs := loadoutResp.Data.Slots["legs"]; !hasLegs {
		t.Fatalf("expected remaining item to be legs")
	}
}

// 4. Test Optimistic Concurrency Conflict
func TestLoadoutOptimisticConcurrencyConflict(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	userID, token := createCosmeticTestUser(t, env)
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// Grant item
	grant, _ := json.Marshal(map[string]string{"user_id": userID, "cosmetic_id": "clo_basic_traveler_001", "source": "grant"})
	reqG := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grant))
	reqG.Header.Set("Content-Type", "application/json")
	reqG.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wG := httptest.NewRecorder()
	env.handler.ServeHTTP(wG, reqG)

	// Send equip with wrong expected version (e.g. 999)
	equipConflict, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: 999,
		CosmeticID:      "clo_basic_traveler_001",
	})
	reqC := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equipConflict))
	reqC.Header.Set("Content-Type", "application/json")
	reqC.Header.Set("Authorization", "Bearer "+token)
	wC := httptest.NewRecorder()
	env.handler.ServeHTTP(wC, reqC)

	if wC.Code != http.StatusConflict {
		t.Fatalf("expected 409 Conflict for stale expected_version, got %d: %s", wC.Code, wC.Body.String())
	}
}

// 5. Test Equip Unowned and Non-Existent Cosmetic Rejection
func TestEquipUnownedCosmeticRejection(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	userID, token := createCosmeticTestUser(t, env)
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// 5.1 Attempt to equip an existing cosmetic that the user does not own -> 403 Forbidden
	equipUnowned, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: 1,
		CosmeticID:      "clo_royal_outfit_001", // In catalogue, but not owned
	})
	reqUnowned := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equipUnowned))
	reqUnowned.Header.Set("Content-Type", "application/json")
	reqUnowned.Header.Set("Authorization", "Bearer "+token)
	wUnowned := httptest.NewRecorder()
	env.handler.ServeHTTP(wUnowned, reqUnowned)

	if wUnowned.Code != http.StatusForbidden {
		t.Fatalf("expected 403 Forbidden for unowned cosmetic, got %d: %s", wUnowned.Code, wUnowned.Body.String())
	}

	// 5.2 Attempt to equip a non-existent cosmetic -> 404 Not Found
	equipNonExistent, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: 1,
		CosmeticID:      "nonexistent_cosmetic_xyz",
	})
	reqNonExistent := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equipNonExistent))
	reqNonExistent.Header.Set("Content-Type", "application/json")
	reqNonExistent.Header.Set("Authorization", "Bearer "+token)
	wNonExistent := httptest.NewRecorder()
	env.handler.ServeHTTP(wNonExistent, reqNonExistent)

	if wNonExistent.Code != http.StatusNotFound {
		t.Fatalf("expected 404 Not Found for non-existent cosmetic, got %d: %s", wNonExistent.Code, wNonExistent.Body.String())
	}
}

// 6. Test Server Verified Loadout Endpoint
func TestServerVerifiedLoadoutEndpoint(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	userID, token := createCosmeticTestUser(t, env)
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// Grant and equip chest
	grant, _ := json.Marshal(map[string]string{"user_id": userID, "cosmetic_id": "clo_basic_traveler_001", "source": "grant"})
	reqG := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grant))
	reqG.Header.Set("Content-Type", "application/json")
	reqG.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wG := httptest.NewRecorder()
	env.handler.ServeHTTP(wG, reqG)

	equip, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: 1,
		CosmeticID:      "clo_basic_traveler_001",
	})
	reqEq := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equip))
	reqEq.Header.Set("Content-Type", "application/json")
	reqEq.Header.Set("Authorization", "Bearer "+token)
	wEq := httptest.NewRecorder()
	env.handler.ServeHTTP(wEq, reqEq)

	// 6.1 Unauthorized call without server token
	reqUnauth := httptest.NewRequest(http.MethodGet, fmt.Sprintf("/api/v1/internal/players/%s/loadout", userID), nil)
	wUnauth := httptest.NewRecorder()
	env.handler.ServeHTTP(wUnauth, reqUnauth)
	if wUnauth.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized without X-Server-Token, got %d", wUnauth.Code)
	}

	// 6.2 Authorized call with server token
	reqAuth := httptest.NewRequest(http.MethodGet, fmt.Sprintf("/api/v1/internal/players/%s/loadout", userID), nil)
	reqAuth.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wAuth := httptest.NewRecorder()
	env.handler.ServeHTTP(wAuth, reqAuth)

	if wAuth.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from server loadout, got %d: %s", wAuth.Code, wAuth.Body.String())
	}

	var srvResp struct {
		Data cosmetics.ServerLoadoutResponse `json:"data"`
	}
	if err := json.Unmarshal(wAuth.Body.Bytes(), &srvResp); err != nil {
		t.Fatalf("failed to parse server loadout response: %v", err)
	}

	if srvResp.Data.UserID.String() != userID {
		t.Fatalf("expected user ID %s, got %s", userID, srvResp.Data.UserID.String())
	}
	if srvResp.Data.LoadoutVersion != 2 {
		t.Fatalf("expected loadout version 2, got %d", srvResp.Data.LoadoutVersion)
	}
	cosmID, exists := srvResp.Data.Slots["chest"]
	if !exists {
		t.Fatalf("expected chest slot in server verified loadout")
	}
	if cosmID != "clo_basic_traveler_001" {
		t.Fatalf("expected clo_basic_traveler_001, got %s", cosmID)
	}
	assetID, hasAsset := srvResp.Data.AssetSlots["chest"]
	if !hasAsset || assetID == "" {
		t.Fatalf("expected non-empty asset ID for server verified item")
	}
}

// 7. Test Loadout Persistence Across Sessions
func TestLoadoutPersistenceAcrossSessions(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("persist_user_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)
	password := "Wildbound2026!Sec"

	// Register
	regPayload := auth.RegisterRequest{
		Username:         username,
		Email:            email,
		Password:         password,
		DisplayName:      "Persist Tester",
		DeviceIdentifier: "device_persist_1",
		DeviceName:       "Rig 1",
	}
	body, _ := json.Marshal(regPayload)
	reqReg := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	reqReg.Header.Set("Content-Type", "application/json")
	wReg := httptest.NewRecorder()
	env.handler.ServeHTTP(wReg, reqReg)

	var regResp struct {
		Data struct {
			User struct {
				ID string `json:"id"`
			} `json:"user"`
			Tokens struct {
				AccessToken string `json:"access_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wReg.Body.Bytes(), &regResp)
	userID := regResp.Data.User.ID
	token1 := regResp.Data.Tokens.AccessToken
	defer env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)

	// Grant and equip
	grant, _ := json.Marshal(map[string]string{"user_id": userID, "cosmetic_id": "clo_p_traveler_pants_001", "source": "grant"})
	reqG := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grant))
	reqG.Header.Set("Content-Type", "application/json")
	reqG.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	wG := httptest.NewRecorder()
	env.handler.ServeHTTP(wG, reqG)

	equip, _ := json.Marshal(cosmetics.EquipSlotRequest{
		ExpectedVersion: 1,
		CosmeticID:      "clo_p_traveler_pants_001",
	})
	reqEq := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/loadout/equip", bytes.NewReader(equip))
	reqEq.Header.Set("Content-Type", "application/json")
	reqEq.Header.Set("Authorization", "Bearer "+token1)
	wEq := httptest.NewRecorder()
	env.handler.ServeHTTP(wEq, reqEq)
	if wEq.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from equip, got %d", wEq.Code)
	}

	// Login again to simulate a new session
	loginPayload := auth.LoginRequest{
		EmailOrUsername:  username,
		Password:         password,
		DeviceIdentifier: "device_persist_2",
		DeviceName:       "Rig 2",
	}
	loginBody, _ := json.Marshal(loginPayload)
	reqLog := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(loginBody))
	reqLog.Header.Set("Content-Type", "application/json")
	wLog := httptest.NewRecorder()
	env.handler.ServeHTTP(wLog, reqLog)

	if wLog.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from login, got %d", wLog.Code)
	}
	var logResp struct {
		Data struct {
			Tokens struct {
				AccessToken string `json:"access_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wLog.Body.Bytes(), &logResp)
	token2 := logResp.Data.Tokens.AccessToken

	// Fetch loadout with session 2 token
	reqL2 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/loadout", nil)
	reqL2.Header.Set("Authorization", "Bearer "+token2)
	wL2 := httptest.NewRecorder()
	env.handler.ServeHTTP(wL2, reqL2)

	if wL2.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from loadout fetch with token 2, got %d", wL2.Code)
	}

	var loadout2 struct {
		Data cosmetics.PlayerLoadoutResponse `json:"data"`
	}
	_ = json.Unmarshal(wL2.Body.Bytes(), &loadout2)

	if loadout2.Data.LoadoutVersion != 2 {
		t.Fatalf("expected loadout version 2 to persist, got %d", loadout2.Data.LoadoutVersion)
	}
	if len(loadout2.Data.Slots) != 1 || loadout2.Data.Slots["legs"].CosmeticID != "clo_p_traveler_pants_001" {
		t.Fatalf("expected clo_p_traveler_pants_001 to remain equipped across sessions")
	}
}
