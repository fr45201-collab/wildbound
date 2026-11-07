package auth

import (
	"testing"
	"time"

	"github.com/wildbound/backend/internal/config"
)

func TestArgon2idHashing(t *testing.T) {
	password := "SecretPassw0rd!"

	hash, err := HashPassword(password)
	if err != nil {
		t.Fatalf("failed to hash password: %v", err)
	}

	if len(hash) == 0 {
		t.Fatal("generated hash should not be empty")
	}

	// Verify correct password
	match, err := VerifyPassword(password, hash)
	if err != nil {
		t.Fatalf("verification returned unexpected error: %v", err)
	}
	if !match {
		t.Fatal("expected password to match hash")
	}

	// Verify incorrect password
	matchWrong, err := VerifyPassword("WrongPassword123!", hash)
	if err != nil {
		t.Fatalf("verification returned unexpected error on wrong password: %v", err)
	}
	if matchWrong {
		t.Fatal("expected wrong password to NOT match hash")
	}
}

func TestJWTTokenManager(t *testing.T) {
	cfg := &config.Config{
		JWT: config.JWTConfig{
			Issuer:           "wildbound-test",
			Audience:         "wildbound-client-test",
			AccessTTLMinutes: 5,
			RefreshTTLHours:  24,
			SecretKey:        "super-secret-test-key-32-chars-long!",
		},
	}

	tm := NewTokenManager(cfg)
	userID := "d3b07384-d113-46fb-a0b4-3a9d9e6e4f32"
	username := "test_hunter"

	tokenStr, expiresAt, err := tm.GenerateAccessToken(userID, username)
	if err != nil {
		t.Fatalf("failed to generate access token: %v", err)
	}

	if tokenStr == "" {
		t.Fatal("generated token string should not be empty")
	}
	if expiresAt.Before(time.Now()) {
		t.Fatal("token expiration should be in the future")
	}

	// Validate valid token
	claims, err := tm.ValidateAccessToken(tokenStr)
	if err != nil {
		t.Fatalf("expected token to be valid, got: %v", err)
	}
	if claims.UserID != userID {
		t.Errorf("expected UserID %s, got %s", userID, claims.UserID)
	}
	if claims.Username != username {
		t.Errorf("expected Username %s, got %s", username, claims.Username)
	}

	// Test invalid / tampered token
	tamperedToken := tokenStr + "tampered"
	_, err = tm.ValidateAccessToken(tamperedToken)
	if err == nil {
		t.Fatal("expected tampered token to fail validation")
	}

	// Test token with wrong secret
	otherCfg := &config.Config{
		JWT: config.JWTConfig{
			Issuer:           "wildbound-test",
			Audience:         "wildbound-client-test",
			AccessTTLMinutes: 5,
			RefreshTTLHours:  24,
			SecretKey:        "completely-different-signing-key!!",
		},
	}
	otherTM := NewTokenManager(otherCfg)
	_, err = otherTM.ValidateAccessToken(tokenStr)
	if err == nil {
		t.Fatal("expected token validated with different key to fail")
	}
}

func TestRefreshTokenGenerationAndHashing(t *testing.T) {
	rawToken, hash, err := GenerateRefreshToken()
	if err != nil {
		t.Fatalf("failed to generate refresh token: %v", err)
	}

	if rawToken == "" || hash == "" {
		t.Fatal("rawToken and hash should not be empty")
	}

	recomputedHash := HashRefreshToken(rawToken)
	if hash != recomputedHash {
		t.Fatalf("hash mismatch: expected %s, got %s", hash, recomputedHash)
	}

	// Two generated refresh tokens should be unique
	rawToken2, hash2, _ := GenerateRefreshToken()
	if rawToken == rawToken2 || hash == hash2 {
		t.Fatal("expected consecutive refresh tokens to be unique")
	}
}

