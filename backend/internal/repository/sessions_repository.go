package repository

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

var (
	ErrSessionNotFound = errors.New("user session not found")
)

type SessionsRepository interface {
	CreateSession(ctx context.Context, tx pgx.Tx, session *UserSession) error
	GetSessionByHash(ctx context.Context, tx pgx.Tx, tokenHash string) (*UserSession, error)
	GetSessionByID(ctx context.Context, sessionID, userID uuid.UUID) (*UserSession, error)
	ListUserSessions(ctx context.Context, userID uuid.UUID) ([]UserSession, error)
	RevokeSession(ctx context.Context, sessionID, userID uuid.UUID, reason string) error
	RevokeSessionByHash(ctx context.Context, tx pgx.Tx, tokenHash string, reason string) error
	RevokeAllUserSessions(ctx context.Context, userID uuid.UUID, reason string) error
	UpdateLastUsed(ctx context.Context, tokenHash string) error
}

type sessionsRepo struct {
	pool *pgxpool.Pool
}

func NewSessionsRepository(pool *pgxpool.Pool) SessionsRepository {
	return &sessionsRepo{pool: pool}
}

func (r *sessionsRepo) CreateSession(ctx context.Context, tx pgx.Tx, session *UserSession) error {
	query := `
		INSERT INTO user_sessions (
			user_id, refresh_token_hash, device_identifier, device_name,
			ip_address, expires_at, created_at, last_used_at
		) VALUES ($1, $2, $3, $4, $5, $6, NOW(), NOW())
		RETURNING id, created_at, last_used_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	err := runner.QueryRow(ctx, query,
		session.UserID, session.RefreshTokenHash, session.DeviceIdentifier,
		session.DeviceName, session.IPAddress, session.ExpiresAt,
	).Scan(&session.ID, &session.CreatedAt, &session.LastUsedAt)
	if err != nil {
		return fmt.Errorf("failed to create user session: %w", err)
	}
	session.IsActive = true
	return nil
}

func (r *sessionsRepo) GetSessionByHash(ctx context.Context, tx pgx.Tx, tokenHash string) (*UserSession, error) {
	query := `
		SELECT id, user_id, refresh_token_hash, device_identifier, device_name,
		       ip_address, created_at, last_used_at, expires_at, revoked_at, revocation_reason
		FROM user_sessions
		WHERE refresh_token_hash = $1
	`
	if tx != nil {
		query += " FOR UPDATE"
	}

	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	var s UserSession
	err := runner.QueryRow(ctx, query, tokenHash).Scan(
		&s.ID, &s.UserID, &s.RefreshTokenHash, &s.DeviceIdentifier, &s.DeviceName,
		&s.IPAddress, &s.CreatedAt, &s.LastUsedAt, &s.ExpiresAt, &s.RevokedAt, &s.RevocationReason,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrSessionNotFound
		}
		return nil, fmt.Errorf("failed to query user session by hash: %w", err)
	}

	s.IsActive = (s.RevokedAt == nil) && time.Now().UTC().Before(s.ExpiresAt)
	return &s, nil
}

func (r *sessionsRepo) GetSessionByID(ctx context.Context, sessionID, userID uuid.UUID) (*UserSession, error) {
	query := `
		SELECT id, user_id, refresh_token_hash, device_identifier, device_name,
		       ip_address, created_at, last_used_at, expires_at, revoked_at, revocation_reason
		FROM user_sessions
		WHERE id = $1 AND user_id = $2
	`
	var s UserSession
	err := r.pool.QueryRow(ctx, query, sessionID, userID).Scan(
		&s.ID, &s.UserID, &s.RefreshTokenHash, &s.DeviceIdentifier, &s.DeviceName,
		&s.IPAddress, &s.CreatedAt, &s.LastUsedAt, &s.ExpiresAt, &s.RevokedAt, &s.RevocationReason,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrSessionNotFound
		}
		return nil, fmt.Errorf("failed to query user session by id: %w", err)
	}

	s.IsActive = (s.RevokedAt == nil) && time.Now().UTC().Before(s.ExpiresAt)
	return &s, nil
}

func (r *sessionsRepo) ListUserSessions(ctx context.Context, userID uuid.UUID) ([]UserSession, error) {
	query := `
		SELECT id, user_id, device_identifier, device_name, ip_address,
		       created_at, last_used_at, expires_at, revoked_at, revocation_reason
		FROM user_sessions
		WHERE user_id = $1
		ORDER BY created_at DESC
	`
	rows, err := r.pool.Query(ctx, query, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to query user sessions: %w", err)
	}
	defer rows.Close()

	var sessions []UserSession
	now := time.Now().UTC()
	for rows.Next() {
		var s UserSession
		if err := rows.Scan(
			&s.ID, &s.UserID, &s.DeviceIdentifier, &s.DeviceName, &s.IPAddress,
			&s.CreatedAt, &s.LastUsedAt, &s.ExpiresAt, &s.RevokedAt, &s.RevocationReason,
		); err != nil {
			return nil, fmt.Errorf("failed to scan session: %w", err)
		}
		s.IsActive = (s.RevokedAt == nil) && now.Before(s.ExpiresAt)
		sessions = append(sessions, s)
	}
	return sessions, rows.Err()
}

func (r *sessionsRepo) RevokeSession(ctx context.Context, sessionID, userID uuid.UUID, reason string) error {
	query := `
		UPDATE user_sessions
		SET revoked_at = NOW(), revocation_reason = $1
		WHERE id = $2 AND user_id = $3 AND revoked_at IS NULL
	`
	res, err := r.pool.Exec(ctx, query, reason, sessionID, userID)
	if err != nil {
		return fmt.Errorf("failed to revoke session: %w", err)
	}
	if res.RowsAffected() == 0 {
		return ErrSessionNotFound
	}
	return nil
}

func (r *sessionsRepo) RevokeSessionByHash(ctx context.Context, tx pgx.Tx, tokenHash string, reason string) error {
	query := `
		UPDATE user_sessions
		SET revoked_at = NOW(), revocation_reason = $1
		WHERE refresh_token_hash = $2 AND revoked_at IS NULL
	`
	if tx != nil {
		_, err := tx.Exec(ctx, query, reason, tokenHash)
		return err
	}
	_, err := r.pool.Exec(ctx, query, reason, tokenHash)
	return err
}

func (r *sessionsRepo) RevokeAllUserSessions(ctx context.Context, userID uuid.UUID, reason string) error {
	query := `
		UPDATE user_sessions
		SET revoked_at = NOW(), revocation_reason = $1
		WHERE user_id = $2 AND revoked_at IS NULL
	`
	_, err := r.pool.Exec(ctx, query, reason, userID)
	return err
}

func (r *sessionsRepo) UpdateLastUsed(ctx context.Context, tokenHash string) error {
	query := `UPDATE user_sessions SET last_used_at = NOW() WHERE refresh_token_hash = $1`
	_, err := r.pool.Exec(ctx, query, tokenHash)
	return err
}
