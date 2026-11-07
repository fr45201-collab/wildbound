package auth

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/repository"
	"github.com/wildbound/backend/internal/validation"
)

var (
	ErrUserNotFound        = errors.New("user not found")
	ErrInvalidCredentials  = errors.New("invalid email/username or password")
	ErrUserAlreadyExists   = errors.New("user with this username or email already exists")
	ErrAccountSuspended    = errors.New("account is suspended or inactive")
	ErrInvalidRefreshToken = errors.New("refresh token is invalid, expired, or revoked")
	ErrTokenTheftSuspected = errors.New("token reuse detected; all sessions revoked for security")
)

type Service struct {
	db           *database.DB
	tokenManager *TokenManager
	cfg          *config.Config
	sessionsRepo repository.SessionsRepository
}

func NewService(db *database.DB, tm *TokenManager, cfg *config.Config) *Service {
	return &Service{
		db:           db,
		tokenManager: tm,
		cfg:          cfg,
		sessionsRepo: repository.NewSessionsRepository(db.Pool),
	}
}

func (s *Service) Register(ctx context.Context, req *RegisterRequest, ipAddress string) (*AuthResponse, error) {
	normEmail, err := validation.ValidateEmail(req.Email)
	if err != nil {
		return nil, err
	}
	normUsername, err := validation.ValidateUsername(req.Username)
	if err != nil {
		return nil, err
	}
	if err := validation.ValidatePassword(req.Password); err != nil {
		return nil, err
	}

	displayName := req.DisplayName
	if displayName == "" {
		displayName = normUsername
	}
	validDisplayName, err := validation.ValidateDisplayName(displayName)
	if err != nil {
		return nil, err
	}

	pwdHash, err := HashPassword(req.Password)
	if err != nil {
		return nil, fmt.Errorf("failed to hash password: %w", err)
	}

	tx, err := s.db.Pool.Begin(ctx)
	if err != nil {
		return nil, fmt.Errorf("failed to begin transaction: %w", err)
	}
	defer tx.Rollback(ctx)

	// Check existing username or email
	var exists bool
	checkQuery := `SELECT EXISTS(SELECT 1 FROM users WHERE username = $1 OR email = $2)`
	if err := tx.QueryRow(ctx, checkQuery, normUsername, normEmail).Scan(&exists); err != nil {
		return nil, fmt.Errorf("failed to check existing user: %w", err)
	}
	if exists {
		return nil, ErrUserAlreadyExists
	}

	// 1. Insert User
	var userID string
	var createdAt time.Time
	insertUserQuery := `
		INSERT INTO users (username, email, password_hash, account_status)
		VALUES ($1, $2, $3, 'active')
		RETURNING id, created_at`
	if err := tx.QueryRow(ctx, insertUserQuery, normUsername, normEmail, pwdHash).Scan(&userID, &createdAt); err != nil {
		return nil, fmt.Errorf("failed to insert user: %w", err)
	}

	// 2. Insert Profile
	insertProfileQuery := `
		INSERT INTO player_profiles (user_id, display_name, avatar_cosmetic_id, preferred_game_mode)
		VALUES ($1, $2, 'cosmetic_avatar_default', 'solo')`
	if _, err := tx.Exec(ctx, insertProfileQuery, userID, validDisplayName); err != nil {
		return nil, fmt.Errorf("failed to create player profile: %w", err)
	}

	// 3. Insert Initial Statistics
	insertStatsQuery := `
		INSERT INTO player_statistics (user_id)
		VALUES ($1)`
	if _, err := tx.Exec(ctx, insertStatsQuery, userID); err != nil {
		return nil, fmt.Errorf("failed to initialize player statistics: %w", err)
	}

	// 4. Insert Default Player Settings
	insertSettingsQuery := `
		INSERT INTO player_settings (user_id)
		VALUES ($1)
		ON CONFLICT (user_id) DO NOTHING`
	if _, err := tx.Exec(ctx, insertSettingsQuery, userID); err != nil {
		return nil, fmt.Errorf("failed to initialize player settings: %w", err)
	}

	// 5. Insert Initial Progression Record
	insertProgressionQuery := `
		INSERT INTO player_progression (user_id)
		VALUES ($1)
		ON CONFLICT (user_id) DO NOTHING`
	if _, err := tx.Exec(ctx, insertProgressionQuery, userID); err != nil {
		return nil, fmt.Errorf("failed to initialize player progression: %w", err)
	}

	// 6. Grant Starter Cosmetics (equipped)
	starterCosmetics := []string{
		"cosmetic_avatar_default",
		"cosmetic_tunic_adventurer",
		"cosmetic_pants_leather",
		"cosmetic_boots_leather",
		"cosmetic_hair_short",
		"cosmetic_skin_fair",
	}

	for _, cosID := range starterCosmetics {
		grantQuery := `
			INSERT INTO player_cosmetics (user_id, cosmetic_id, acquisition_source, is_equipped)
			VALUES ($1, $2, 'starter', true)
			ON CONFLICT (user_id, cosmetic_id) DO NOTHING`
		if _, err := tx.Exec(ctx, grantQuery, userID, cosID); err != nil {
			return nil, fmt.Errorf("failed to grant starter cosmetic %s: %w", cosID, err)
		}
	}

	// 7. Generate Tokens
	accessToken, accessExpiresAt, err := s.tokenManager.GenerateAccessToken(userID, normUsername)
	if err != nil {
		return nil, err
	}

	rawRefreshToken, refreshHash, err := GenerateRefreshToken()
	if err != nil {
		return nil, err
	}

	refreshExpiresAt := time.Now().UTC().Add(s.cfg.RefreshTokenDuration())

	// Insert into refresh_tokens for backward compatibility
	insertTokenQuery := `
		INSERT INTO refresh_tokens (user_id, token_hash, expires_at)
		VALUES ($1, $2, $3)`
	if _, err := tx.Exec(ctx, insertTokenQuery, userID, refreshHash, refreshExpiresAt); err != nil {
		return nil, fmt.Errorf("failed to store refresh token: %w", err)
	}

	// 8. Insert into user_sessions
	parsedUID, _ := uuid.Parse(userID)
	devID := req.DeviceIdentifier
	if devID == "" {
		devID = "pc_" + uuid.New().String()[:8]
	}
	devName := req.DeviceName
	if devName == "" {
		devName = "PC Contender Client"
	}
	if ipAddress == "" {
		ipAddress = "127.0.0.1"
	}

	insertSessionQuery := `
		INSERT INTO user_sessions (
			user_id, refresh_token_hash, device_identifier, device_name,
			ip_address, expires_at
		) VALUES ($1, $2, $3, $4, $5, $6)`
	if _, err := tx.Exec(ctx, insertSessionQuery, parsedUID, refreshHash, devID, devName, ipAddress, refreshExpiresAt); err != nil {
		return nil, fmt.Errorf("failed to create user session: %w", err)
	}

	if err := tx.Commit(ctx); err != nil {
		return nil, fmt.Errorf("failed to commit registration: %w", err)
	}

	return &AuthResponse{
		User: AuthUserResponse{
			ID:          userID,
			Username:    normUsername,
			Email:       normEmail,
			DisplayName: validDisplayName,
			CreatedAt:   createdAt,
		},
		Tokens: TokenPairResponse{
			AccessToken:      accessToken,
			TokenType:        "Bearer",
			ExpiresAt:        accessExpiresAt,
			RefreshToken:     rawRefreshToken,
			RefreshExpiresAt: refreshExpiresAt,
		},
	}, nil
}

func (s *Service) Login(ctx context.Context, req *LoginRequest, ipAddress string) (*AuthResponse, error) {
	identifier := req.EmailOrUsername
	if identifier == "" || req.Password == "" {
		return nil, ErrInvalidCredentials
	}

	query := `
		SELECT u.id, u.username, u.email, u.password_hash, u.account_status, u.created_at, p.display_name
		FROM users u
		JOIN player_profiles p ON p.user_id = u.id
		WHERE u.email = LOWER($1) OR u.username = $1`

	var (
		userID      string
		username    string
		email       string
		pwdHash     string
		status      string
		createdAt   time.Time
		displayName string
	)

	err := s.db.Pool.QueryRow(ctx, query, identifier).Scan(
		&userID, &username, &email, &pwdHash, &status, &createdAt, &displayName,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrInvalidCredentials
		}
		return nil, fmt.Errorf("query error: %w", err)
	}

	if status != "active" {
		return nil, ErrAccountSuspended
	}

	valid, err := VerifyPassword(req.Password, pwdHash)
	if err != nil || !valid {
		return nil, ErrInvalidCredentials
	}

	// Update last login timestamp asynchronously
	go func() {
		bgCtx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancel()
		_, _ = s.db.Pool.Exec(bgCtx, `UPDATE users SET last_login_at = NOW(), updated_at = NOW() WHERE id = $1`, userID)
	}()

	// Generate Tokens
	accessToken, accessExpiresAt, err := s.tokenManager.GenerateAccessToken(userID, username)
	if err != nil {
		return nil, err
	}

	rawRefreshToken, refreshHash, err := GenerateRefreshToken()
	if err != nil {
		return nil, err
	}

	refreshExpiresAt := time.Now().UTC().Add(s.cfg.RefreshTokenDuration())
	insertTokenQuery := `
		INSERT INTO refresh_tokens (user_id, token_hash, expires_at)
		VALUES ($1, $2, $3)`
	if _, err := s.db.Pool.Exec(ctx, insertTokenQuery, userID, refreshHash, refreshExpiresAt); err != nil {
		return nil, fmt.Errorf("failed to store refresh token: %w", err)
	}

	// Create user_sessions record
	parsedUID, _ := uuid.Parse(userID)
	devID := req.DeviceIdentifier
	if devID == "" {
		devID = "pc_device"
	}
	devName := req.DeviceName
	if devName == "" {
		devName = "PC Contender Client"
	}
	if ipAddress == "" {
		ipAddress = "127.0.0.1"
	}

	insertSessionQuery := `
		INSERT INTO user_sessions (
			user_id, refresh_token_hash, device_identifier, device_name,
			ip_address, expires_at
		) VALUES ($1, $2, $3, $4, $5, $6)`
	_, _ = s.db.Pool.Exec(ctx, insertSessionQuery, parsedUID, refreshHash, devID, devName, ipAddress, refreshExpiresAt)

	return &AuthResponse{
		User: AuthUserResponse{
			ID:          userID,
			Username:    username,
			Email:       email,
			DisplayName: displayName,
			CreatedAt:   createdAt,
		},
		Tokens: TokenPairResponse{
			AccessToken:      accessToken,
			TokenType:        "Bearer",
			ExpiresAt:        accessExpiresAt,
			RefreshToken:     rawRefreshToken,
			RefreshExpiresAt: refreshExpiresAt,
		},
	}, nil
}

func (s *Service) RefreshTokens(ctx context.Context, rawToken string, ipAddress string) (*TokenPairResponse, error) {
	if rawToken == "" {
		return nil, ErrInvalidRefreshToken
	}

	tokenHash := HashRefreshToken(rawToken)

	tx, err := s.db.Pool.Begin(ctx)
	if err != nil {
		return nil, fmt.Errorf("failed to begin transaction: %w", err)
	}
	defer tx.Rollback(ctx)

	// Check user_sessions first for token theft detection
	var (
		sessionID   uuid.UUID
		userID      uuid.UUID
		username    string
		expiresAt   time.Time
		revokedAt   *time.Time
		devID       string
		devName     string
	)

	sessionQuery := `
		SELECT s.id, s.user_id, u.username, s.expires_at, s.revoked_at, s.device_identifier, s.device_name
		FROM user_sessions s
		JOIN users u ON u.id = s.user_id
		WHERE s.refresh_token_hash = $1
		FOR UPDATE OF s`

	err = tx.QueryRow(ctx, sessionQuery, tokenHash).Scan(
		&sessionID, &userID, &username, &expiresAt, &revokedAt, &devID, &devName,
	)

	if err != nil {
		// Fallback check against refresh_tokens table
		if errors.Is(err, pgx.ErrNoRows) {
			rtQuery := `
				SELECT r.id, r.user_id, u.username, r.expires_at, r.revoked_at
				FROM refresh_tokens r
				JOIN users u ON u.id = r.user_id
				WHERE r.token_hash = $1
				FOR UPDATE OF r`
			var rtID string
			rtErr := tx.QueryRow(ctx, rtQuery, tokenHash).Scan(&rtID, &userID, &username, &expiresAt, &revokedAt)
			if rtErr != nil {
				return nil, ErrInvalidRefreshToken
			}
		} else {
			return nil, fmt.Errorf("session query error: %w", err)
		}
	}

	// TOKEN THEFT DETECTION: If token was already revoked, someone is replaying a stolen token!
	if revokedAt != nil {
		// Revoke ALL active sessions for this user immediately!
		_, _ = tx.Exec(ctx, `UPDATE user_sessions SET revoked_at = NOW(), revocation_reason = 'token_theft_detected' WHERE user_id = $1 AND revoked_at IS NULL`, userID)
		_, _ = tx.Exec(ctx, `UPDATE refresh_tokens SET revoked_at = NOW() WHERE user_id = $1 AND revoked_at IS NULL`, userID)
		_ = tx.Commit(ctx)
		return nil, ErrTokenTheftSuspected
	}

	if time.Now().UTC().After(expiresAt) {
		return nil, ErrInvalidRefreshToken
	}

	// Revoke current session (rotation)
	if sessionID != uuid.Nil {
		_, _ = tx.Exec(ctx, `UPDATE user_sessions SET revoked_at = NOW(), revocation_reason = 'rotated' WHERE id = $1`, sessionID)
	}
	_, _ = tx.Exec(ctx, `UPDATE refresh_tokens SET revoked_at = NOW() WHERE token_hash = $1`, tokenHash)

	// Generate New Tokens
	accessToken, accessExpiresAt, err := s.tokenManager.GenerateAccessToken(userID.String(), username)
	if err != nil {
		return nil, err
	}

	newRawRefreshToken, newRefreshHash, err := GenerateRefreshToken()
	if err != nil {
		return nil, err
	}

	newRefreshExpiresAt := time.Now().UTC().Add(s.cfg.RefreshTokenDuration())
	if ipAddress == "" {
		ipAddress = "127.0.0.1"
	}
	if devID == "" {
		devID = "pc_client"
	}
	if devName == "" {
		devName = "PC Contender Client"
	}

	// Insert into user_sessions and refresh_tokens
	_, _ = tx.Exec(ctx, `
		INSERT INTO user_sessions (user_id, refresh_token_hash, device_identifier, device_name, ip_address, expires_at)
		VALUES ($1, $2, $3, $4, $5, $6)`, userID, newRefreshHash, devID, devName, ipAddress, newRefreshExpiresAt)

	_, _ = tx.Exec(ctx, `
		INSERT INTO refresh_tokens (user_id, token_hash, expires_at)
		VALUES ($1, $2, $3)`, userID, newRefreshHash, newRefreshExpiresAt)

	if err := tx.Commit(ctx); err != nil {
		return nil, fmt.Errorf("failed to commit token refresh: %w", err)
	}

	return &TokenPairResponse{
		AccessToken:      accessToken,
		TokenType:        "Bearer",
		ExpiresAt:        accessExpiresAt,
		RefreshToken:     newRawRefreshToken,
		RefreshExpiresAt: newRefreshExpiresAt,
	}, nil
}

func (s *Service) Logout(ctx context.Context, rawToken string) error {
	if rawToken == "" {
		return nil
	}
	tokenHash := HashRefreshToken(rawToken)
	_ = s.sessionsRepo.RevokeSessionByHash(ctx, nil, tokenHash, "user_logout")
	_, err := s.db.Pool.Exec(ctx, `UPDATE refresh_tokens SET revoked_at = NOW() WHERE token_hash = $1 AND revoked_at IS NULL`, tokenHash)
	return err
}

func (s *Service) LogoutAll(ctx context.Context, userID uuid.UUID) error {
	_ = s.sessionsRepo.RevokeAllUserSessions(ctx, userID, "user_logout_all")
	_, err := s.db.Pool.Exec(ctx, `UPDATE refresh_tokens SET revoked_at = NOW() WHERE user_id = $1 AND revoked_at IS NULL`, userID)
	return err
}

func (s *Service) ListSessions(ctx context.Context, userID uuid.UUID) ([]repository.UserSession, error) {
	return s.sessionsRepo.ListUserSessions(ctx, userID)
}

func (s *Service) RevokeSession(ctx context.Context, sessionID, userID uuid.UUID) error {
	return s.sessionsRepo.RevokeSession(ctx, sessionID, userID, "user_revoked")
}

func (s *Service) GetMe(ctx context.Context, userID string) (*AuthUserResponse, error) {
	query := `
		SELECT u.id, u.username, u.email, p.display_name, u.created_at
		FROM users u
		JOIN player_profiles p ON p.user_id = u.id
		WHERE u.id = $1`

	var user AuthUserResponse
	err := s.db.Pool.QueryRow(ctx, query, userID).Scan(
		&user.ID, &user.Username, &user.Email, &user.DisplayName, &user.CreatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrUserNotFound
		}
		return nil, err
	}
	return &user, nil
}
