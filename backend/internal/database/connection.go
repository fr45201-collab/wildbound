package database

import (
	"context"

	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/observability"
)

type DB struct {
	Pool   *pgxpool.Pool
	logger *observability.Logger
}

// Connect creates a database wrapper with a managed pgxpool.Pool.
func Connect(ctx context.Context, cfg *config.DatabaseConfig, log *observability.Logger) (*DB, error) {
	pool, err := NewPool(ctx, cfg)
	if err != nil {
		return nil, err
	}

	stats := pool.Stat()
	log.Info("PostgreSQL connection pool established",
		"dsn_configured", cfg.URL != "",
		"host", cfg.Host,
		"port", cfg.Port,
		"database", cfg.Name,
		"max_conns", stats.MaxConns(),
		"idle_conns", stats.IdleConns(),
	)

	return &DB{
		Pool:   pool,
		logger: log,
	}, nil
}

// Close gracefully closes the connection pool.
func (db *DB) Close() {
	if db.Pool != nil {
		if db.logger != nil {
			db.logger.Info("Closing PostgreSQL connection pool")
		}
		db.Pool.Close()
	}
}

// Ping verifies that a connection to the database is still alive.
func (db *DB) Ping(ctx context.Context) error {
	if db.Pool == nil {
		return context.Canceled
	}
	return db.Pool.Ping(ctx)
}

// Stats returns the current connection pool statistics.
func (db *DB) Stats() PoolStats {
	if db.Pool == nil {
		return PoolStats{}
	}
	stat := db.Pool.Stat()
	return PoolStats{
		TotalConns:        stat.TotalConns(),
		AcquiredConns:     stat.AcquiredConns(),
		IdleConns:         stat.IdleConns(),
		MaxConns:          stat.MaxConns(),
		ConstructingConns: stat.ConstructingConns(),
	}
}

