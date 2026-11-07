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
	ErrUserNotFound         = errors.New("user not found")
	ErrRefreshTokenNotFound = errors.New("refresh token not found")
)

type UsersRepository interface {
	Create(ctx context.Context, tx pgx.Tx, user *User) error
	GetByID(ctx context.Context, id uuid.UUID) (*User, error)
	GetByUsernameOrEmail(ctx context.Context, identifier string) (*User, error)
	CheckExists(ctx context.Context, username, email string) (bool, error)
	UpdateLastLogin(ctx context.Context, id uuid.UUID, lastLogin time.Time) error
	StoreRefreshToken(ctx context.Context, tx pgx.Tx, token *RefreshToken) error
	GetRefreshToken(ctx context.Context, tokenHash string) (*RefreshToken, error)
	RevokeRefreshToken(ctx context.Context, tokenHash string) error
	RevokeAllUserRefreshTokens(ctx context.Context, userID uuid.UUID) error
	UpdatePassword(ctx context.Context, id uuid.UUID, passwordHash string) error
	UpdateEmail(ctx context.Context, id uuid.UUID, email string) error
}

type usersRepo struct {
	pool *pgxpool.Pool
}

func NewUsersRepository(pool *pgxpool.Pool) UsersRepository {
	return &usersRepo{pool: pool}
}

func (r *usersRepo) Create(ctx context.Context, tx pgx.Tx, user *User) error {
	query := `
		INSERT INTO users (username, email, password_hash, account_status)
		VALUES ($1, $2, $3, $4)
		RETURNING id, created_at, updated_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	err := runner.QueryRow(ctx, query, user.Username, user.Email, user.PasswordHash, user.AccountStatus).
		Scan(&user.ID, &user.CreatedAt, &user.UpdatedAt)
	if err != nil {
		return fmt.Errorf("failed to insert user: %w", err)
	}
	return nil
}

func (r *usersRepo) GetByID(ctx context.Context, id uuid.UUID) (*User, error) {
	query := `
		SELECT id, username, email, password_hash, account_status, created_at, updated_at, last_login_at
		FROM users
		WHERE id = $1
	`
	var u User
	err := r.pool.QueryRow(ctx, query, id).Scan(
		&u.ID, &u.Username, &u.Email, &u.PasswordHash,
		&u.AccountStatus, &u.CreatedAt, &u.UpdatedAt, &u.LastLoginAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrUserNotFound
		}
		return nil, fmt.Errorf("failed to query user by id: %w", err)
	}
	return &u, nil
}

func (r *usersRepo) GetByUsernameOrEmail(ctx context.Context, identifier string) (*User, error) {
	query := `
		SELECT id, username, email, password_hash, account_status, created_at, updated_at, last_login_at
		FROM users
		WHERE LOWER(email) = LOWER($1) OR username = $1
	`
	var u User
	err := r.pool.QueryRow(ctx, query, identifier).Scan(
		&u.ID, &u.Username, &u.Email, &u.PasswordHash,
		&u.AccountStatus, &u.CreatedAt, &u.UpdatedAt, &u.LastLoginAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrUserNotFound
		}
		return nil, fmt.Errorf("failed to query user by identifier: %w", err)
	}
	return &u, nil
}

func (r *usersRepo) CheckExists(ctx context.Context, username, email string) (bool, error) {
	query := `SELECT EXISTS(SELECT 1 FROM users WHERE username = $1 OR LOWER(email) = LOWER($2))`
	var exists bool
	err := r.pool.QueryRow(ctx, query, username, email).Scan(&exists)
	if err != nil {
		return false, fmt.Errorf("failed to check existing user: %w", err)
	}
	return exists, nil
}

func (r *usersRepo) UpdateLastLogin(ctx context.Context, id uuid.UUID, lastLogin time.Time) error {
	query := `UPDATE users SET last_login_at = $1, updated_at = NOW() WHERE id = $2`
	_, err := r.pool.Exec(ctx, query, lastLogin, id)
	return err
}

func (r *usersRepo) StoreRefreshToken(ctx context.Context, tx pgx.Tx, token *RefreshToken) error {
	query := `
		INSERT INTO refresh_tokens (user_id, token_hash, expires_at)
		VALUES ($1, $2, $3)
		RETURNING id, created_at
	`
	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	err := runner.QueryRow(ctx, query, token.UserID, token.TokenHash, token.ExpiresAt).
		Scan(&token.ID, &token.CreatedAt)
	if err != nil {
		return fmt.Errorf("failed to store refresh token: %w", err)
	}
	return nil
}

func (r *usersRepo) GetRefreshToken(ctx context.Context, tokenHash string) (*RefreshToken, error) {
	query := `
		SELECT id, user_id, token_hash, expires_at, revoked_at, created_at
		FROM refresh_tokens
		WHERE token_hash = $1
	`
	var t RefreshToken
	err := r.pool.QueryRow(ctx, query, tokenHash).Scan(
		&t.ID, &t.UserID, &t.TokenHash, &t.ExpiresAt, &t.RevokedAt, &t.CreatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrRefreshTokenNotFound
		}
		return nil, fmt.Errorf("failed to query refresh token: %w", err)
	}
	return &t, nil
}

func (r *usersRepo) RevokeRefreshToken(ctx context.Context, tokenHash string) error {
	query := `UPDATE refresh_tokens SET revoked_at = NOW() WHERE token_hash = $1 AND revoked_at IS NULL`
	_, err := r.pool.Exec(ctx, query, tokenHash)
	return err
}

func (r *usersRepo) RevokeAllUserRefreshTokens(ctx context.Context, userID uuid.UUID) error {
	query := `UPDATE refresh_tokens SET revoked_at = NOW() WHERE user_id = $1 AND revoked_at IS NULL`
	_, err := r.pool.Exec(ctx, query, userID)
	return err
}

func (r *usersRepo) UpdatePassword(ctx context.Context, id uuid.UUID, passwordHash string) error {
	query := `UPDATE users SET password_hash = $1, updated_at = NOW() WHERE id = $2`
	_, err := r.pool.Exec(ctx, query, passwordHash, id)
	return err
}

func (r *usersRepo) UpdateEmail(ctx context.Context, id uuid.UUID, email string) error {
	query := `UPDATE users SET email = $1, updated_at = NOW() WHERE id = $2`
	_, err := r.pool.Exec(ctx, query, email, id)
	return err
}


