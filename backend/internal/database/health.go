package database

import (
	"context"
	"fmt"
	"time"
)

type DatabaseHealthReport struct {
	Status        string        `json:"status"`
	Latency       time.Duration `json:"latency"`
	LatencyMillis float64       `json:"latency_ms"`
	PoolStats     PoolStats     `json:"pool_stats"`
	Error         string        `json:"error,omitempty"`
}

// CheckHealth performs an end-to-end health check on the database connection pool,
// measuring round-trip latency and retrieving connection statistics.
func (db *DB) CheckHealth(ctx context.Context) DatabaseHealthReport {
	start := time.Now()
	report := DatabaseHealthReport{
		Status: "unhealthy",
	}

	if db.Pool == nil {
		report.Error = "connection pool is uninitialized"
		return report
	}

	// 1. Collect pool stats
	stats := db.Pool.Stat()
	report.PoolStats = PoolStats{
		TotalConns:        stats.TotalConns(),
		AcquiredConns:     stats.AcquiredConns(),
		IdleConns:         stats.IdleConns(),
		MaxConns:          stats.MaxConns(),
		ConstructingConns: stats.ConstructingConns(),
	}

	// 2. Measure ping latency with context timeout
	pingCtx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()

	var dummy int
	err := db.Pool.QueryRow(pingCtx, "SELECT 1").Scan(&dummy)
	elapsed := time.Since(start)
	report.Latency = elapsed
	report.LatencyMillis = float64(elapsed.Microseconds()) / 1000.0

	if err != nil {
		report.Error = fmt.Sprintf("database ping query failed: %v", err)
		return report
	}

	report.Status = "healthy"
	return report
}

