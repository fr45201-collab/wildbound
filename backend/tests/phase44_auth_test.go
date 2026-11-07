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
	"github.com/wildbound/backend/internal/account"
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/matches"
	"github.com/wildbound/backend/internal/settings"
)

func TestRegistrationLifecycleWithSettingsAndProgression(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("prog_user_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)

	regPayload := auth.RegisterRequest{
		Username:         username,
		Email:            email,
		Password:         "Wildbound2026!Sec",
		DisplayName:      "Progress Contender",
		DeviceIdentifier: "desktop_rig_01",
		DeviceName:       "Contender Battlestation",
	}
	body, _ := json.Marshal(regPayload)
	req := httptest.NewRequest(http.MethodPost, "/api/v1/auth/register", bytes.NewReader(body))
	req.Header.Set("Content-Type", "application/json")
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)

	if w.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created from register, got %d, body: %s", w.Code, w.Body.String())
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
	_ = json.Unmarshal(w.Body.Bytes(), &regResp)
	userID := regResp.Data.User.ID
	token := regResp.Data.Tokens.AccessToken

	// 1. Verify default player settings exist
	reqSet := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/settings", nil)
	reqSet.Header.Set("Authorization", "Bearer "+token)
	wSet := httptest.NewRecorder()
	env.handler.ServeHTTP(wSet, reqSet)
	if wSet.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from settings, got %d", wSet.Code)
	}

	var setResp struct {
		Data struct {
			Language string `json:"language"`
			Region   string `json:"region"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wSet.Body.Bytes(), &setResp)
	if setResp.Data.Language != "en" || setResp.Data.Region != "us-east" {
		t.Fatalf("expected default en/us-east settings, got %s/%s", setResp.Data.Language, setResp.Data.Region)
	}

	// 2. Verify initial progression exists (Level 1, 0 XP)
	reqProg := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/progression", nil)
	reqProg.Header.Set("Authorization", "Bearer "+token)
	wProg := httptest.NewRecorder()
	env.handler.ServeHTTP(wProg, reqProg)
	if wProg.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from progression, got %d", wProg.Code)
	}

	var progResp struct {
		Data struct {
			AccountLevel int `json:"account_level"`
			TotalXP      int `json:"total_xp"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wProg.Body.Bytes(), &progResp)
	if progResp.Data.AccountLevel != 1 || progResp.Data.TotalXP != 0 {
		t.Fatalf("expected level 1 and 0 XP, got level=%d, xp=%d", progResp.Data.AccountLevel, progResp.Data.TotalXP)
	}

	// Clean up user
	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()
}

func TestMultiSessionManagementAndRevocation(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("sess_user_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)
	password := "SecurePass2026!"

	// 1. Register user
	regPayload := auth.RegisterRequest{
		Username: username,
		Email:    email,
		Password: password,
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
				AccessToken  string `json:"access_token"`
				RefreshToken string `json:"refresh_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wReg.Body.Bytes(), &regResp)
	userID := regResp.Data.User.ID
	tokenA := regResp.Data.Tokens.AccessToken
	refreshA := regResp.Data.Tokens.RefreshToken
	_ = refreshA

	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()

	// 2. Log in on Device B
	loginPayload := auth.LoginRequest{
		EmailOrUsername:  username,
		Password:         password,
		DeviceIdentifier: "mobile_deck_02",
		DeviceName:       "Handheld Steam Deck",
	}
	loginBody, _ := json.Marshal(loginPayload)
	reqLogin := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(loginBody))
	reqLogin.Header.Set("Content-Type", "application/json")
	wLogin := httptest.NewRecorder()
	env.handler.ServeHTTP(wLogin, reqLogin)

	var loginResp struct {
		Data struct {
			Tokens struct {
				AccessToken  string `json:"access_token"`
				RefreshToken string `json:"refresh_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wLogin.Body.Bytes(), &loginResp)
	tokenB := loginResp.Data.Tokens.AccessToken
	refreshB := loginResp.Data.Tokens.RefreshToken

	// 3. List active sessions: should report at least 2 sessions
	reqSess := httptest.NewRequest(http.MethodGet, "/api/v1/auth/sessions", nil)
	reqSess.Header.Set("Authorization", "Bearer "+tokenB)
	wSess := httptest.NewRecorder()
	env.handler.ServeHTTP(wSess, reqSess)
	if wSess.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from list sessions, got %d", wSess.Code)
	}

	var sessListResp struct {
		Data struct {
			Count    int `json:"count"`
			Sessions []struct {
				ID               string `json:"id"`
				DeviceIdentifier string `json:"device_identifier"`
				IsActive         bool   `json:"is_active"`
			} `json:"sessions"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wSess.Body.Bytes(), &sessListResp)
	if sessListResp.Data.Count < 2 {
		t.Fatalf("expected at least 2 sessions, got %d", sessListResp.Data.Count)
	}

	// 4. Revoke Session A using its ID
	sessionAID := sessListResp.Data.Sessions[1].ID
	reqRev := httptest.NewRequest(http.MethodDelete, "/api/v1/auth/sessions/"+sessionAID, nil)
	reqRev.Header.Set("Authorization", "Bearer "+tokenB)
	wRev := httptest.NewRecorder()
	env.handler.ServeHTTP(wRev, reqRev)
	if wRev.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from revoke session, got %d", wRev.Code)
	}

	// 5. Verify Session B can still refresh
	refPayload := auth.RefreshRequest{RefreshToken: refreshB}
	refBody, _ := json.Marshal(refPayload)
	reqRefB := httptest.NewRequest(http.MethodPost, "/api/v1/auth/refresh", bytes.NewReader(refBody))
	reqRefB.Header.Set("Content-Type", "application/json")
	wRefB := httptest.NewRecorder()
	env.handler.ServeHTTP(wRefB, reqRefB)
	if wRefB.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from active session refresh, got %d", wRefB.Code)
	}

	// 6. Logout-All: revokes all sessions
	reqLogAll := httptest.NewRequest(http.MethodPost, "/api/v1/auth/logout-all", nil)
	reqLogAll.Header.Set("Authorization", "Bearer "+tokenA)
	wLogAll := httptest.NewRecorder()
	env.handler.ServeHTTP(wLogAll, reqLogAll)
	if wLogAll.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from logout-all, got %d", wLogAll.Code)
	}
}

func TestTokenTheftReplayDetection(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("theft_user_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)

	regPayload := auth.RegisterRequest{
		Username: username,
		Email:    email,
		Password: "SecurePassword123!",
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
				RefreshToken string `json:"refresh_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wReg.Body.Bytes(), &regResp)
	userID := regResp.Data.User.ID
	initialRefreshToken := regResp.Data.Tokens.RefreshToken

	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()

	// 1. Legitimate user rotates token
	refPayload := auth.RefreshRequest{RefreshToken: initialRefreshToken}
	refBody, _ := json.Marshal(refPayload)
	reqRef := httptest.NewRequest(http.MethodPost, "/api/v1/auth/refresh", bytes.NewReader(refBody))
	reqRef.Header.Set("Content-Type", "application/json")
	wRef := httptest.NewRecorder()
	env.handler.ServeHTTP(wRef, reqRef)
	if wRef.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on legitimate refresh, got %d", wRef.Code)
	}

	// 2. Attacker replays initialRefreshToken (which was already rotated and marked revoked!)
	reqTheft := httptest.NewRequest(http.MethodPost, "/api/v1/auth/refresh", bytes.NewReader(refBody))
	reqTheft.Header.Set("Content-Type", "application/json")
	wTheft := httptest.NewRecorder()
	env.handler.ServeHTTP(wTheft, reqTheft)

	if wTheft.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized on stolen token replay, got %d", wTheft.Code)
	}

	var theftResp struct {
		Error struct {
			Code string `json:"code"`
		} `json:"error"`
	}
	_ = json.Unmarshal(wTheft.Body.Bytes(), &theftResp)
	if theftResp.Error.Code != "TOKEN_THEFT_DETECTED" {
		t.Fatalf("expected TOKEN_THEFT_DETECTED error code, got %s", theftResp.Error.Code)
	}
}

func TestAccountSettingsLifecycle(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("settings_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)

	regPayload := auth.RegisterRequest{
		Username: username,
		Email:    email,
		Password: "SecurePassword123!",
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
	token := regResp.Data.Tokens.AccessToken
	userID := regResp.Data.User.ID

	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()

	// 1. Update settings: change language to "es", region to "eu-central", privacy to "private"
	newLang := "es"
	newRegion := "eu-central"
	newVis := "private"
	updatePayload := settings.UpdateSettingsRequest{
		Language:                 &newLang,
		Region:                   &newRegion,
		PrivacyProfileVisibility: &newVis,
	}
	updateBody, _ := json.Marshal(updatePayload)
	reqUp := httptest.NewRequest(http.MethodPatch, "/api/v1/players/me/settings", bytes.NewReader(updateBody))
	reqUp.Header.Set("Authorization", "Bearer "+token)
	reqUp.Header.Set("Content-Type", "application/json")
	wUp := httptest.NewRecorder()
	env.handler.ServeHTTP(wUp, reqUp)
	if wUp.Code != http.StatusOK {
		t.Fatalf("expected 200 OK on settings update, got %d, body: %s", wUp.Code, wUp.Body.String())
	}

	// 2. Query settings again to verify persistence
	reqGet := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/settings", nil)
	reqGet.Header.Set("Authorization", "Bearer "+token)
	wGet := httptest.NewRecorder()
	env.handler.ServeHTTP(wGet, reqGet)

	var getResp struct {
		Data struct {
			Language                 string `json:"language"`
			Region                   string `json:"region"`
			PrivacyProfileVisibility string `json:"privacy_profile_visibility"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wGet.Body.Bytes(), &getResp)
	if getResp.Data.Language != "es" || getResp.Data.Region != "eu-central" || getResp.Data.PrivacyProfileVisibility != "private" {
		t.Fatalf("expected persisted updated settings, got %v", getResp.Data)
	}

	// 3. Attempt update with invalid language -> 400 Validation Error
	badLang := "klingon"
	badPayload := settings.UpdateSettingsRequest{Language: &badLang}
	badBody, _ := json.Marshal(badPayload)
	reqBad := httptest.NewRequest(http.MethodPatch, "/api/v1/players/me/settings", bytes.NewReader(badBody))
	reqBad.Header.Set("Authorization", "Bearer "+token)
	reqBad.Header.Set("Content-Type", "application/json")
	wBad := httptest.NewRecorder()
	env.handler.ServeHTTP(wBad, reqBad)
	if wBad.Code != http.StatusBadRequest {
		t.Fatalf("expected 400 Bad Request on invalid language, got %d", wBad.Code)
	}
}

func TestAccountProgressionAndAuditEvents(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	username := fmt.Sprintf("hero_%s", uuid.New().String()[:8])
	email := fmt.Sprintf("%s@wildbound.com", username)

	regPayload := auth.RegisterRequest{
		Username: username,
		Email:    email,
		Password: "SecurePassword123!",
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
	token := regResp.Data.Tokens.AccessToken
	userIDStr := regResp.Data.User.ID
	userID, _ := uuid.Parse(userIDStr)

	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()

	// 1. Submit match result: Contender took 1st place, 4 eliminations, 500 damage, 300s survival
	matchID := fmt.Sprintf("match_prog_%s", uuid.New().String()[:8])
	matchPayload := matches.SubmitMatchResultRequest{
		MatchID:          matchID,
		GameMode:         "solo",
		MapID:            "maps/world/battle_royale_island.json",
		StartedAt:        time.Now().Add(-10 * time.Minute),
		EndedAt:          time.Now(),
		DurationSeconds:  600,
		WinnerUserID:     &userID,
		WinnerDisplayName: username,
		ResultStatus:     "completed",
		Participants: []matches.MatchParticipantPayload{
			{
				UserID:                 &userID,
				ParticipantDisplayName: username,
				TeamID:                 1,
				Placement:              1,
				Eliminations:           4,
				Deaths:                 0,
				DamageDealt:            500.0,
				DamageReceived:         150.0,
				SurvivalSeconds:        600.0,
				ResultStatus:           "winner",
			},
		},
	}
	matchBody, _ := json.Marshal(matchPayload)
	reqMatch := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader(matchBody))
	reqMatch.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqMatch.Header.Set("Content-Type", "application/json")
	wMatch := httptest.NewRecorder()
	env.handler.ServeHTTP(wMatch, reqMatch)
	if wMatch.Code != http.StatusCreated {
		t.Fatalf("expected 201 Created from match submit, got %d, body: %s", wMatch.Code, wMatch.Body.String())
	}

	// 2. Query progression: Level should be advanced and XP awarded!
	reqProg := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/progression", nil)
	reqProg.Header.Set("Authorization", "Bearer "+token)
	wProg := httptest.NewRecorder()
	env.handler.ServeHTTP(wProg, reqProg)
	if wProg.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from progression, got %d", wProg.Code)
	}

	var progResp struct {
		Data struct {
			AccountLevel          int `json:"account_level"`
			TotalXP               int `json:"total_xp"`
			AvailableRewardPoints int `json:"available_reward_points"`
			MatchesCompleted      int `json:"matches_completed"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wProg.Body.Bytes(), &progResp)
	if progResp.Data.TotalXP <= 0 || progResp.Data.AccountLevel < 2 {
		t.Fatalf("expected level advancement and earned XP, got level=%d, total_xp=%d",
			progResp.Data.AccountLevel, progResp.Data.TotalXP)
	}

	// 3. Query progression events audit log
	reqEv := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/progression/events", nil)
	reqEv.Header.Set("Authorization", "Bearer "+token)
	wEv := httptest.NewRecorder()
	env.handler.ServeHTTP(wEv, reqEv)
	if wEv.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from progression events, got %d", wEv.Code)
	}

	var evResp struct {
		Data struct {
			Count  int `json:"count"`
			Events []struct {
				SourceType string `json:"source_type"`
				SourceID   string `json:"source_id"`
				XPDelta    int    `json:"xp_delta"`
			} `json:"events"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wEv.Body.Bytes(), &evResp)
	if evResp.Data.Count != 1 || evResp.Data.Events[0].SourceType != "match" {
		t.Fatalf("expected 1 match progression event, got count=%d", evResp.Data.Count)
	}

	// 4. Duplicate match submission: Replay must NOT grant duplicate XP
	reqReplay := httptest.NewRequest(http.MethodPost, "/api/v1/internal/matches/results", bytes.NewReader(matchBody))
	reqReplay.Header.Set("X-Server-Token", env.cfg.ServerSecret)
	reqReplay.Header.Set("Content-Type", "application/json")
	wReplay := httptest.NewRecorder()
	env.handler.ServeHTTP(wReplay, reqReplay)
	if wReplay.Code != http.StatusOK { // 200 OK on idempotent return
		t.Fatalf("expected 200 OK on match replay, got %d", wReplay.Code)
	}

	// Total events count must still be exactly 1!
	wEv2 := httptest.NewRecorder()
	reqEv2 := httptest.NewRequest(http.MethodGet, "/api/v1/players/me/progression/events", nil)
	reqEv2.Header.Set("Authorization", "Bearer "+token)
	env.handler.ServeHTTP(wEv2, reqEv2)
	_ = json.Unmarshal(wEv2.Body.Bytes(), &evResp)
	if evResp.Data.Count != 1 {
		t.Fatalf("expected event count to remain 1 after replay, got %d", evResp.Data.Count)
	}
}

func TestAchievementsCatalogAndUnlocks(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	// 1. Public catalog of achievements
	req := httptest.NewRequest(http.MethodGet, "/api/v1/achievements", nil)
	w := httptest.NewRecorder()
	env.handler.ServeHTTP(w, req)
	if w.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from achievements catalog, got %d", w.Code)
	}

	var catResp struct {
		Data struct {
			Count        int `json:"count"`
			Achievements []struct {
				ID          string `json:"id"`
				DisplayName string `json:"display_name"`
			} `json:"achievements"`
		} `json:"data"`
	}
	_ = json.Unmarshal(w.Body.Bytes(), &catResp)
	if catResp.Data.Count < 5 {
		t.Fatalf("expected at least 5 foundational achievements, got %d", catResp.Data.Count)
	}
}

func TestPasswordAndEmailChange(t *testing.T) {
	env := setupTestEnv(t)
	defer env.close()

	oldUsername := fmt.Sprintf("acct_%s", uuid.New().String()[:8])
	oldEmail := fmt.Sprintf("%s@wildbound.com", oldUsername)
	oldPassword := "OldPassword123!"
	newPassword := "NewPassword456!Sec"
	newEmail := fmt.Sprintf("new_%s", oldEmail)

	// 1. Register account
	regPayload := auth.RegisterRequest{
		Username: oldUsername,
		Email:    oldEmail,
		Password: oldPassword,
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
	token := regResp.Data.Tokens.AccessToken
	userID := regResp.Data.User.ID

	defer func() {
		_, _ = env.db.Pool.Exec(context.Background(), "DELETE FROM users WHERE id = $1", userID)
	}()

	// 2. Change password
	pwdPayload := account.ChangePasswordRequest{
		CurrentPassword: oldPassword,
		NewPassword:     newPassword,
	}
	pwdBody, _ := json.Marshal(pwdPayload)
	reqPwd := httptest.NewRequest(http.MethodPatch, "/api/v1/account/password", bytes.NewReader(pwdBody))
	reqPwd.Header.Set("Authorization", "Bearer "+token)
	reqPwd.Header.Set("Content-Type", "application/json")
	wPwd := httptest.NewRecorder()
	env.handler.ServeHTTP(wPwd, reqPwd)
	if wPwd.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from password change, got %d, body: %s", wPwd.Code, wPwd.Body.String())
	}

	// 3. Login with old password must fail
	loginOld := auth.LoginRequest{
		EmailOrUsername: oldUsername,
		Password:        oldPassword,
	}
	loginOldBody, _ := json.Marshal(loginOld)
	reqOldLogin := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(loginOldBody))
	reqOldLogin.Header.Set("Content-Type", "application/json")
	wOldLogin := httptest.NewRecorder()
	env.handler.ServeHTTP(wOldLogin, reqOldLogin)
	if wOldLogin.Code != http.StatusUnauthorized {
		t.Fatalf("expected 401 Unauthorized with old password, got %d", wOldLogin.Code)
	}

	// 4. Login with new password must succeed
	loginNew := auth.LoginRequest{
		EmailOrUsername: oldUsername,
		Password:        newPassword,
	}
	loginNewBody, _ := json.Marshal(loginNew)
	reqNewLogin := httptest.NewRequest(http.MethodPost, "/api/v1/auth/login", bytes.NewReader(loginNewBody))
	reqNewLogin.Header.Set("Content-Type", "application/json")
	wNewLogin := httptest.NewRecorder()
	env.handler.ServeHTTP(wNewLogin, reqNewLogin)
	if wNewLogin.Code != http.StatusOK {
		t.Fatalf("expected 200 OK with new password, got %d", wNewLogin.Code)
	}

	var newLoginResp struct {
		Data struct {
			Tokens struct {
				AccessToken string `json:"access_token"`
			} `json:"tokens"`
		} `json:"data"`
	}
	_ = json.Unmarshal(wNewLogin.Body.Bytes(), &newLoginResp)
	newToken := newLoginResp.Data.Tokens.AccessToken

	// 5. Change email
	emailPayload := account.ChangeEmailRequest{
		NewEmail:        newEmail,
		CurrentPassword: newPassword,
	}
	emailBody, _ := json.Marshal(emailPayload)
	reqEmail := httptest.NewRequest(http.MethodPatch, "/api/v1/account/email", bytes.NewReader(emailBody))
	reqEmail.Header.Set("Authorization", "Bearer "+newToken)
	reqEmail.Header.Set("Content-Type", "application/json")
	wEmail := httptest.NewRecorder()
	env.handler.ServeHTTP(wEmail, reqEmail)
	if wEmail.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from email change, got %d, body: %s", wEmail.Code, wEmail.Body.String())
	}

	// 6. Security status check
	reqSec := httptest.NewRequest(http.MethodGet, "/api/v1/account/security", nil)
	reqSec.Header.Set("Authorization", "Bearer "+newToken)
	wSec := httptest.NewRecorder()
	env.handler.ServeHTTP(wSec, reqSec)
	if wSec.Code != http.StatusOK {
		t.Fatalf("expected 200 OK from security status, got %d", wSec.Code)
	}
}
