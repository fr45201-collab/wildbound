package database

import (
	"context"
	"fmt"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

// WithTransaction executes the provided function within an isolated database transaction.
// If fn returns an error or panics, the transaction is automatically rolled back.
// If fn returns nil, the transaction is committed.
func WithTransaction(ctx context.Context, pool *pgxpool.Pool, fn func(tx pgx.Tx) error) error {
	if pool == nil {
		return fmt.Errorf("connection pool is nil")
	}

	tx, err := pool.Begin(ctx)
	if err != nil {
		return fmt.Errorf("failed to begin transaction: %w", err)
	}

	var panicked bool = true
	defer func() {
		if panicked {
			_ = tx.Rollback(ctx)
		}
	}()

	if err := fn(tx); err != nil {
		panicked = false
		_ = tx.Rollback(ctx)
		return err
	}

	panicked = false
	if err := tx.Commit(ctx); err != nil {
		return fmt.Errorf("failed to commit transaction: %w", err)
	}

	return nil
}

// WithTransactionSerializable executes the function within a SERIALIZABLE isolation transaction.
func WithTransactionSerializable(ctx context.Context, pool *pgxpool.Pool, fn func(tx pgx.Tx) error) error {
	if pool == nil {
		return fmt.Errorf("connection pool is nil")
	}

	txOpts := pgx.TxOptions{
		IsoLevel: pgx.Serializable,
	}

	tx, err := pool.BeginTx(ctx, txOpts)
	if err != nil {
		return fmt.Errorf("failed to begin serializable transaction: %w", err)
	}

	var panicked bool = true
	defer func() {
		if panicked {
			_ = tx.Rollback(ctx)
		}
	}()

	if err := fn(tx); err != nil {
		panicked = false
		_ = tx.Rollback(ctx)
		return err
	}

	panicked = false
	if err := tx.Commit(ctx); err != nil {
		return fmt.Errorf("failed to commit serializable transaction: %w", err)
	}

	return nil
}

// WithTransaction executes a transaction using the DB wrapper.
func (db *DB) WithTransaction(ctx context.Context, fn func(tx pgx.Tx) error) error {
	return WithTransaction(ctx, db.Pool, fn)
}

