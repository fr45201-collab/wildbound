package database

import (
	"context"
	"fmt"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/config"
)

type PoolStats struct {
	TotalConns       int32 `json:"total_conns"`
	AcquiredConns    int32 `json:"acquired_conns"`
	IdleConns        int32 `json:"idle_conns"`
	MaxConns         int32 `json:"max_conns"`
	ConstructingConns int32 `json:"constructing_conns"`
}

// ConfigurePool creates and validates a pgxpool.Config from the database configuration.
func ConfigurePool(cfg *config.DatabaseConfig) (*pgxpool.Config, error) {
	dsn := cfg.DSN()
	poolConfig, err := pgxpool.ParseConfig(dsn)
	if err != nil {
		return nil, fmt.Errorf("unable to parse database DSN/URL: %w", err)
	}

	if cfg.MaxConns > 0 {
		poolConfig.MaxConns = int32(cfg.MaxConns)
	} else {
		poolConfig.MaxConns = 20
	}

	if cfg.MinConns > 0 {
		poolConfig.MinConns = int32(cfg.MinConns)
	} else {
		poolConfig.MinConns = 2
	}

	poolConfig.MaxConnLifetime = cfg.MaxConnLifetime()
	poolConfig.MaxConnIdleTime = cfg.MaxConnIdleTime()
	poolConfig.HealthCheckPeriod = 1 * time.Minute

	return poolConfig, nil
}

// NewPool initializes a new pgxpool.Pool with the provided configuration and validates connectivity.
func NewPool(ctx context.Context, cfg *config.DatabaseConfig) (*pgxpool.Pool, error) {
	poolConfig, err := ConfigurePool(cfg)
	if err != nil {
		return nil, err
	}

	pool, err := pgxpool.NewWithConfig(ctx, poolConfig)
	if err != nil {
		return nil, fmt.Errorf("unable to initialize connection pool: %w", err)
	}

	pingCtx, cancel := context.WithTimeout(ctx, 5*time.Second)
	defer cancel()

	if err := pool.Ping(pingCtx); err != nil {
		pool.Close()
		return nil, fmt.Errorf("failed to ping database during pool initialization: %w", err)
	}

	return pool, nil
}

