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
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/cosmetics"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/matches"
	"github.com/wildbound/backend/internal/observability"
	"github.com/wildbound/backend/internal/response"
	"github.com/wildbound/backend/internal/server"
	"github.com/wildbound/backend/internal/statistics"
)

type TestEnv struct {
	cfg     *config.Config
	db      *database.DB
	logger  *observability.Logger
	server  *server.Server
	handler http.Handler
}

func setupTestEnv(t *testing.T) *TestEnv {
	cfg, err := config.Load("../configs/development.json")
	if err != nil {
		t.Fatalf("failed to load test config: %v", err)
	}

	logger := observability.NewLogger("debug", true)

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	db, err := database.Connect(ctx, &cfg.Database, logger)
	if err != nil {
		t.Fatalf("failed to connect to test postgres database: %v", err)
	}

	srv := server.NewServer(cfg, db, logger)

	return &TestEnv{
		cfg:     cfg,
		db:      db,
		logger:  logger,
		server:  srv,
		handler: srv.Handler(),
	}
}

func (e *TestEnv) close() {
	if e.db != nil {
		e.db.Close()
	}
}

func parseAPIResponse(t *testing.T, w *httptest.ResponseRecorder) response.APIResponse {
	var resp response.APIResponse
	if err := json.Unmarshal(w.Body.Bytes(), &resp); err != nil {
		t.Fatalf("failed to parse API response: %v, body: %s", err, w.Body.String())
	}
	return resp
}

func TestHealthAndReadyEndpoints(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1. Health check
	req := httptest.NewRequest(http.MethodGet, "/health", nil)
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /health, got %d", w.Code)
	}
	resp := parseAPIResponse(t, w)
	if !resp.Success {
		t.Fatalf("expected success true from /health")
	}

	// 2. Readiness check
	reqReady := httptest.NewRequest(http.MethodGet, "/ready", nil)
	wReady := httptest.NewRecorder()
	env.handler.ServeHTTP(wReady, reqReady)

	if wReady.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /ready, got %d", wReady.Code)
	}
	respReady := parseAPIResponse(t, wReady)
	if !respReady.Success {
		t.Fatalf("expected success true from /ready")
	}
}

func TestEndToEndPlayerLifecycle(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	timestamp := time.Now().UnixNano()
	testUsername := fmt.Sprintf("test_user_%d", timestamp%100000)
	testEmail := fmt.Sprintf("test_%d@wildbound.test", timestamp)
	testPassword := "SecureWildbound2026!"
	testDisplayName := fmt.Sprintf("Wild_%d", timestamp%10000)

	// Step 1: Register Player
	regPayload := auth.RegisterRequest{
		Email:       testEmail,
		Username:    testUsername,
		Password:    testPassword,
		DisplayName: testDisplayName,
	}
	body, _ := json.Marshal(regPayload)
	req := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created on register, got %d. Body: %s", w.Code, w.Body.String())
	}

	var regResp response.APIResponse
	_ = json.Unmarshal(w.Body.Bytes(), &regResp)
	regDataBytes, _ := json.Marshal(regResp.Data)
	var authData auth.AuthResponse
	_ = json.Unmarshal(regDataBytes, &authData)

	if authData.Tokens.AccessToken == "" || authData.Tokens.RefreshToken == "" {
		t.Fatal("expected non-empty access and refresh tokens on registration")
	}
	playerID := authData.User.ID
	playerUUID := uuid.MustParse(playerID)
	accessToken := authData.Tokens.AccessToken
	refreshToken := authData.Tokens.RefreshToken

	// Step 2: Verify Duplicate Registration Rejection (Conflict 409)
	reqDup := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	reqDup.Header.Set("Content-Type", "application/json")
	wDup := httptest.NewRecorder()
	env.handler.ServeHTTP(wDup, reqDup)
	if wDup.Code != http.StatusConflict {
		t.Fatalf("expected 409 Conflict on duplicate registration, got %d", wDup.Code)
	}

	// Step 3: Login
	loginPayload := auth.LoginRequest{
		EmailOrUsername: testUsername,
		Password:        testPassword,
	}
	loginBody, _ := json.Marshal(loginPayload)
	reqLogin := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(loginBody))
	reqLogin.Header.Set("Content-Type", "application/json")
	wLogin := httptest.NewRecorder()
	env.handler.ServeHTTP(wLogin, reqLogin)

	if wLogin.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on login, got %d", wLogin.Code)
	}

	// Step 4: Login with Wrong Password -> 401
	badLoginPayload := auth.LoginRequest{
		EmailOrUsername: testUsername,
		Password:        "WrongPassword999!",
	}
	badBody, _ := json.Marshal(badLoginPayload)
	reqBadLogin := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(badBody))
	reqBadLogin.Header.Set("Content-Type", "application/json")
	wBadLogin := httptest.NewRecorder()
	env.handler.ServeHTTP(wBadLogin, reqBadLogin)
	if wBadLogin.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized on bad login, got %d", wBadLogin.Code)
	}

	// Step 5: Verify Auth Me (/api/v1/auth/me)
	reqMe := httptest.NewRequest(http.MethodGet, "/api/v1/auth/me", nil)
	reqMe.Header.Set("Authorization", "Bearer "+accessToken)
	wMe := httptest.NewRecorder()
	env.handler.ServeHTTP(wMe, reqMe)
	if wMe.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /auth/me, got %d", wMe.Code)
	}

	// Step 6: Verify Player Profile (/api/v1/players/me)
	reqProf := httptest.NewRequest(http.MethodGet, "/api/v1/players/me", nil)
	reqProf.Header.Set("Authorization", "Bearer "+accessToken)
	wProf := httptest.NewRecorder()
	env.handler.ServeHTTP(wProf, reqProf)
	if wProf.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me, got %d. Body: %s", wProf.Code, wProf.Body.String())
	}

	// Step 7: Check Starter Cosmetics Inventory (/api/v1/players/me/cosmetics)
	reqCosm := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/cosmetics", nil)
	reqCosm.Header.Set("Authorization", "Bearer "+accessToken)
	wCosm := httptest.NewRecorder()
	env.handler.ServeHTTP(wCosm, reqCosm)
	if wCosm.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me/cosmetics, got %d", wCosm.Code)
	}

	var cosmResp response.APIResponse
	_ = json.Unmarshal(wCosm.Body.Bytes(), &cosmResp)
	cosmDataBytes, _ := json.Marshal(cosmResp.Data)
	var ownedItems []cosmetics.PlayerCosmeticItem
	_ = json.Unmarshal(cosmDataBytes, &ownedItems)

	if len(ownedItems) == 0 {
		t.Fatal("expected starter cosmetics to be automatically granted on registration")
	}

	// Step 8: Equipping an Unowned Cosmetic should be Forbidden (403)
	unownedPayload := cosmetics.EquipCosmeticRequest{
		CosmeticID: "cosmetic_hat_adventurer", // Rare Explorer Fedora, not starter
	}
	unownedBody, _ := json.Marshal(unownedPayload)
	reqUnowned := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/cosmetics/equip", bytes.NewReader(unownedBody))
	reqUnowned.Header.Set("Authorization", "Bearer "+accessToken)
	reqUnowned.Header.Set("Content-Type", "application/json")
	wUnowned := httptest.NewRecorder()
	env.handler.ServeHTTP(wUnowned, reqUnowned)
	if wUnowned.Code != http.StatusForbidden {
		t.Fatalf("expected 403 Forbidden when equipping unowned cosmetic, got %d", wUnowned.Code)
	}

	// Step 9: Grant Cosmetic via Internal Endpoint with Server Token
	grantPayload := cosmetics.GrantCosmeticRequest{
		UserID:     playerUUID,
		CosmeticID: "cosmetic_hat_adventurer",
	}
	grantBody, _ := json.Marshal(grantPayload)
	reqGrant := httptest.NewRequest(http.MethodPost, "/api/v1/internal/cosmetics/grant", bytes.NewReader(grantBody))
	reqGrant.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqGrant.Header.Set("Content-Type", "application/json")
	wGrant := httptest.NewRecorder()
	env.handler.ServeHTTP(wGrant, reqGrant)
	if wGrant.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on internal grant, got %d", wGrant.Code)
	}

	// Step 10: Player can now Equip newly granted cosmetic
	reqEquipNow := httptest.NewRequest(http.MethodPost, "/api/v1/players/me/cosmetics/equip", bytes.NewReader(unownedBody))
	reqEquipNow.Header.Set("Authorization", "Bearer "+accessToken)
	reqEquipNow.Header.Set("Content-Type", "application/json")
	wEquipNow := httptest.NewRecorder()
	env.handler.ServeHTTP(wEquipNow, reqEquipNow)
	if wEquipNow.Code != http.StatusOK {
		t.Fatalf("expected 200 OK after granting cosmetic, got %d", wEquipNow.Code)
	}

	// Step 11: Match Result Submission (Server Ingestion)
	matchIDStr := fmt.Sprintf("match_%d", timestamp)
	startTime := time.Now().Add(-15 * time.Minute)
	endTime := time.Now()
	matchPayload := matches.SubmitMatchResultRequest{
		MatchID:           matchIDStr,
		GameMode:          "solo",
		MapID:             "map_whispering_pines",
		StartedAt:         startTime,
		EndedAt:           endTime,
		DurationSeconds:   900,
		WinnerUserID:      &playerUUID,
		WinnerDisplayName: testDisplayName,
		ResultStatus:      "completed",
		Participants: []matches.MatchParticipantPayload{
			{
				UserID:                 &playerUUID,
				ParticipantDisplayName: testDisplayName,
				TeamID:                 1,
				Placement:              1,
				Eliminations:           6,
				Deaths:                 0,
				Assists:                2,
				DamageDealt:            1450.5,
				DamageReceived:         320.0,
				SurvivalSeconds:        900.0,
				ResultStatus:           "winner",
			},
		},
	}

	// 11a: Missing server token -> 401
	mBody, _ := json.Marshal(matchPayload)
	reqMNoAuth := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader(mBody))
	reqMNoAuth.Header.Set("Content-Type", "application/json")
	wMNoAuth := httptest.NewRecorder()
	env.handler.ServeHTTP(wMNoAuth, reqMNoAuth)
	if wMNoAuth.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 without server token, got %d", wMNoAuth.Code)
	}

	// 11b: Valid server token -> 201 Created
	reqMValid := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader(mBody))
	reqMValid.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqMValid.Header.Set("Content-Type", "application/json")
	wMValid := httptest.NewRecorder()
	env.handler.ServeHTTP(wMValid, reqMValid)
	if wMValid.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created on valid match result submission, got %d. Body: %s", wMValid.Code, wMValid.Body.String())
	}

	// 11c: IDEMPOTENCY: Re-submitting identical match result returns 200 OK and does NOT duplicate stats
	reqMReplay := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader(mBody))
	reqMReplay.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqMReplay.Header.Set("Content-Type", "application/json")
	wMReplay := httptest.NewRecorder()
	env.handler.ServeHTTP(wMReplay, reqMReplay)
	if wMReplay.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on idempotent match replay, got %d", wMReplay.Code)
	}

	// Step 12: Verify Updated Player Statistics
	reqStats := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/statistics", nil)
	reqStats.Header.Set("Authorization", "Bearer "+accessToken)
	wStats := httptest.NewRecorder()
	env.handler.ServeHTTP(wStats, reqStats)
	if wStats.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me/statistics, got %d", wStats.Code)
	}

	var statsResp response.APIResponse
	_ = json.Unmarshal(wStats.Body.Bytes(), &statsResp)
	statsBytes, _ := json.Marshal(statsResp.Data)
	var playerStats statistics.PlayerStatistics
	_ = json.Unmarshal(statsBytes, &playerStats)

	if playerStats.MatchesPlayed != 1 {
		t.Fatalf("expected matches_played = 1 (idempotency check), got %d", playerStats.MatchesPlayed)
	}
	if playerStats.MatchesWon != 1 {
		t.Fatalf("expected matches_won = 1, got %d", playerStats.MatchesWon)
	}
	if playerStats.Eliminations != 6 {
		t.Fatalf("expected eliminations = 6, got %d", playerStats.Eliminations)
	}
	if playerStats.WinRate != 100.0 {
		t.Fatalf("expected 100%% win rate, got %f", playerStats.WinRate)
	}

	// Step 13: Verify Player Match History
	reqHistory := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/matches", nil)
	reqHistory.Header.Set("Authorization", "Bearer "+accessToken)
	wHistory := httptest.NewRecorder()
	env.handler.ServeHTTP(wHistory, reqHistory)
	if wHistory.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from /players/me/matches, got %d", wHistory.Code)
	}

	// Step 14: Refresh Token Rotation
	refreshPayload := auth.RefreshRequest{
		RefreshToken: refreshToken,
	}
	refBody, _ := json.Marshal(refreshPayload)
	reqRefresh := httptest.NewRequest(http.MethodPost, "/api/v1/auth/refresh", bytes.NewReader(refBody))
	reqRefresh.Header.Set("Content-Type", "application/json")
	wRefresh := httptest.NewRecorder()
	env.handler.ServeHTTP(wRefresh, reqRefresh)
	if wRefresh.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on refresh token, got %d. Body: %s", wRefresh.Code, wRefresh.Body.String())
	}

	var refResp response.APIResponse
	_ = json.Unmarshal(wRefresh.Body.Bytes(), &refResp)
	refDataBytes, _ := json.Marshal(refResp.Data)
	var newTokens auth.TokenPairResponse
	_ = json.Unmarshal(refDataBytes, &newTokens)

	if newTokens.AccessToken == "" || newTokens.RefreshToken == "" {
		t.Fatal("expected new access and refresh tokens from rotation")
	}

	// Reusing previous (revoked) refresh token must fail with 401
	reqReplayRef := httptest.NewRequest(http.MethodPost, "/api/v1/auth/refresh", bytes.NewReader(refBody))
	reqReplayRef.Header.Set("Content-Type", "application/json")
	wReplayRef := httptest.NewRecorder()
	env.handler.ServeHTTP(wReplayRef, reqReplayRef)
	if wReplayRef.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized when replaying rotated refresh token, got %d", wReplayRef.Code)
	}

	// Step 15: Logout Revocation
	logoutPayload := auth.RefreshRequest{
		RefreshToken: newTokens.RefreshToken,
	}
	logoutBody, _ := json.Marshal(logoutPayload)
	reqLogout := httptest.NewRequest(http.MethodPost, "/api/v1/auth/logout", bytes.NewReader(logoutBody))
	reqLogout.Header.Set("Authorization", "Bearer "+newTokens.AccessToken)
	reqLogout.Header.Set("Content-Type", "application/json")
	wLogout := httptest.NewRecorder()
	env.handler.ServeHTTP(wLogout, reqLogout)
	if wLogout.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on logout, got %d", wLogout.Code)
	}
}


func TestPublicProfileAndStatistics(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	timestamp := time.Now().UnixNano()
	testUsername := fmt.Sprintf("pub_user_%d", timestamp%100000)
	testEmail := fmt.Sprintf("pub_%d@wildbound.test", timestamp)

	regPayload := auth.RegisterRequest{
		Email:       testEmail,
		Username:    testUsername,
		Password:    "SecureWildbound2026!",
		DisplayName: "PublicHero",
	}
	body, _ := json.Marshal(regPayload)
	req := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created on register, got %d", w.Code)
	}

	var regResp response.APIResponse
	_ = json.Unmarshal(w.Body.Bytes(), &regResp)
	regDataBytes, _ := json.Marshal(regResp.Data)
	var authData auth.AuthResponse
	_ = json.Unmarshal(regDataBytes, &authData)
	playerID := authData.User.ID

	// 1. Get Public Profile (unauthenticated)
	reqPub := httptest.NewRequest(http.MethodGet, "/api/v1/players/"+playerID, nil)
	wPub := httptest.NewRecorder()
	env.handler.ServeHTTP(wPub, reqPub)
	if wPub.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from public profile, got %d", wPub.Code)
	}

	// 2. Get Public Statistics (unauthenticated)
	reqStats := httptest.NewRequest(http.MethodGet, "/api/v1/players/"+playerID+"/statistics", nil)
	wStats := httptest.NewRecorder()
	env.handler.ServeHTTP(wStats, reqStats)
	if wStats.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from public player statistics, got %d", wStats.Code)
	}

	// 3. Query nonexistent player returns 404
	randomID := uuid.New().String()
	reqNon := httptest.NewRequest(http.MethodGet, "/api/v1/players/"+randomID, nil)
	wNon := httptest.NewRecorder()
	env.handler.ServeHTTP(wNon, reqNon)
	if wNon.Code != http.StatusNotFound {
		t.Fatalf("expected 404 for nonexistent player profile, got %d", wNon.Code)
	}
}

func TestSecurityAndValidationBoundaries(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1. Protected endpoint without token -> 401
	req := httptest.NewRequest(http.MethodGet, "/api/v1/players/me", nil)
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)
	if w.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized, got %d", w.Code)
	}

	// 2. Protected endpoint with malformed bearer -> 401
	req2 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me", nil)
	req2.Header.Set("Authorization", "InvalidFormat123")
	w2 := httptest.NewRecorder()
	env.handler.ServeHTTP(w2, req2)
	if w2.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 for malformed Authorization header, got %d", w2.Code)
	}

	// 3. Protected endpoint with invalid JWT signature -> 401
	req3 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me", nil)
	req3.Header.Set("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.invalid.signature")
	w3 := httptest.NewRecorder()
	env.handler.ServeHTTP(w3, req3)
	if w3.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 for invalid JWT signature, got %d", w3.Code)
	}

	// 4. SQL Injection payload rejection in login
	sqlInjPayload := auth.LoginRequest{
		EmailOrUsername: "' OR '1'='1",
		Password:        "' OR '1'='1",
	}
	body, _ := json.Marshal(sqlInjPayload)
	reqSQL := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(body))
	reqSQL.Header.Set("Content-Type", "application/json")
	wSQL := httptest.NewRecorder()
	env.handler.ServeHTTP(wSQL, reqSQL)
	if wSQL.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized on SQL injection attempt, got %d", wSQL.Code)
	}

	// 5. Server endpoint with invalid secret token -> 403
	reqServ := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader([]byte("{}")))
	reqServ.Header.Set("X-Server-Token", "incorrect-token-value")
	reqServ.Header.Set("Content-Type", "application/json")
	wServ := httptest.NewRecorder()
	env.handler.ServeHTTP(wServ, reqServ)
	if wServ.Code != http.StatusForbidden {
		t.Fatalf("expected 403 Forbidden with invalid X-Server-Token, got %d", wServ.Code)
	}
}
