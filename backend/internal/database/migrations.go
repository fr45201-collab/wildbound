package database

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

const (
	// Advisory lock ID to prevent concurrent migration execution
	MigrationAdvisoryLockID = 987654321
)

type MigrationRecord struct {
	Version   int       `json:"version"`
	Name      string    `json:"name"`
	AppliedAt time.Time `json:"applied_at"`
}

type MigrationStatus struct {
	Version   int        `json:"version"`
	Name      string     `json:"name"`
	Applied   bool       `json:"applied"`
	AppliedAt *time.Time `json:"applied_at,omitempty"`
}

type Migrator struct {
	pool          *pgxpool.Pool
	migrationsDir string
}

func NewMigrator(pool *pgxpool.Pool, migrationsDir string) *Migrator {
	return &Migrator{
		pool:          pool,
		migrationsDir: migrationsDir,
	}
}

// EnsureSchemaMigrationsTable initializes the tracking table.
func (m *Migrator) EnsureSchemaMigrationsTable(ctx context.Context) error {
	query := `
		CREATE TABLE IF NOT EXISTS schema_migrations (
			version INT PRIMARY KEY,
			name VARCHAR(255) NOT NULL,
			applied_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
		);
	`
	_, err := m.pool.Exec(ctx, query)
	return err
}

func (m *Migrator) acquireLock(ctx context.Context) error {
	var acquired bool
	err := m.pool.QueryRow(ctx, "SELECT pg_try_advisory_lock($1)", MigrationAdvisoryLockID).Scan(&acquired)
	if err != nil {
		return fmt.Errorf("failed to check advisory lock: %w", err)
	}
	if !acquired {
		return fmt.Errorf("migration lock is already held by another process")
	}
	return nil
}

func (m *Migrator) releaseLock(ctx context.Context) error {
	var released bool
	err := m.pool.QueryRow(ctx, "SELECT pg_advisory_unlock($1)", MigrationAdvisoryLockID).Scan(&released)
	if err != nil {
		return fmt.Errorf("failed to release advisory lock: %w", err)
	}
	return nil
}

type migrationFile struct {
	version   int
	baseName  string
	upPath    string
	downPath  string
}

func (m *Migrator) discoverMigrations() ([]migrationFile, error) {
	entries, err := os.ReadDir(m.migrationsDir)
	if err != nil {
		return nil, fmt.Errorf("failed to read migrations directory %s: %w", m.migrationsDir, err)
	}

	byVersion := make(map[int]*migrationFile)

	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}
		name := entry.Name()
		if !strings.HasSuffix(name, ".sql") {
			continue
		}

		parts := strings.Split(name, "_")
		if len(parts) < 2 {
			continue
		}
		v, err := strconv.Atoi(parts[0])
		if err != nil {
			continue
		}

		if _, exists := byVersion[v]; !exists {
			byVersion[v] = &migrationFile{
				version: v,
			}
		}

		if strings.HasSuffix(name, ".up.sql") {
			byVersion[v].upPath = filepath.Join(m.migrationsDir, name)
			base := strings.TrimSuffix(name, ".up.sql")
			byVersion[v].baseName = base
		} else if strings.HasSuffix(name, ".down.sql") {
			byVersion[v].downPath = filepath.Join(m.migrationsDir, name)
		}
	}

	var list []migrationFile
	for _, mf := range byVersion {
		if mf.upPath != "" {
			list = append(list, *mf)
		}
	}

	sort.Slice(list, func(i, j int) bool {
		return list[i].version < list[j].version
	})

	return list, nil
}

// GetAppliedMigrations returns a map of applied versions.
func (m *Migrator) GetAppliedMigrations(ctx context.Context) (map[int]MigrationRecord, error) {
	if err := m.EnsureSchemaMigrationsTable(ctx); err != nil {
		return nil, err
	}

	rows, err := m.pool.Query(ctx, "SELECT version, name, applied_at FROM schema_migrations ORDER BY version ASC")
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	applied := make(map[int]MigrationRecord)
	for rows.Next() {
		var rec MigrationRecord
		if err := rows.Scan(&rec.Version, &rec.Name, &rec.AppliedAt); err != nil {
			return nil, err
		}
		applied[rec.Version] = rec
	}

	return applied, rows.Err()
}

// Up runs all pending migrations in ascending order.
func (m *Migrator) Up(ctx context.Context) ([]string, error) {
	if err := m.acquireLock(ctx); err != nil {
		return nil, err
	}
	defer func() {
		_ = m.releaseLock(ctx)
	}()

	if err := m.EnsureSchemaMigrationsTable(ctx); err != nil {
		return nil, err
	}

	applied, err := m.GetAppliedMigrations(ctx)
	if err != nil {
		return nil, err
	}

	allMigrations, err := m.discoverMigrations()
	if err != nil {
		return nil, err
	}

	var appliedNames []string
	for _, mig := range allMigrations {
		if _, alreadyApplied := applied[mig.version]; alreadyApplied {
			continue
		}

		content, err := os.ReadFile(mig.upPath)
		if err != nil {
			return appliedNames, fmt.Errorf("failed to read migration file %s: %w", mig.upPath, err)
		}

		err = WithTransaction(ctx, m.pool, func(tx pgx.Tx) error {
			if _, err := tx.Exec(ctx, string(content)); err != nil {
				return fmt.Errorf("execution failed: %w", err)
			}
			_, err := tx.Exec(ctx, "INSERT INTO schema_migrations (version, name, applied_at) VALUES ($1, $2, $3)",
				mig.version, mig.baseName, time.Now().UTC())
			return err
		})

		if err != nil {
			return appliedNames, fmt.Errorf("migration %s failed: %w", mig.baseName, err)
		}

		appliedNames = append(appliedNames, mig.baseName)
	}

	return appliedNames, nil
}

// Down rolls back the most recent applied migration.
func (m *Migrator) Down(ctx context.Context) (string, error) {
	if err := m.acquireLock(ctx); err != nil {
		return "", err
	}
	defer func() {
		_ = m.releaseLock(ctx)
	}()

	if err := m.EnsureSchemaMigrationsTable(ctx); err != nil {
		return "", err
	}

	applied, err := m.GetAppliedMigrations(ctx)
	if err != nil {
		return "", err
	}

	if len(applied) == 0 {
		return "", fmt.Errorf("no applied migrations to roll back")
	}

	// Find the latest applied migration version
	var latestVersion int = -1
	for v := range applied {
		if v > latestVersion {
			latestVersion = v
		}
	}

	allMigrations, err := m.discoverMigrations()
	if err != nil {
		return "", err
	}

	var target *migrationFile
	for i := range allMigrations {
		if allMigrations[i].version == latestVersion {
			target = &allMigrations[i]
			break
		}
	}

	if target == nil || target.downPath == "" {
		return "", fmt.Errorf("down migration script not found for version %d", latestVersion)
	}

	content, err := os.ReadFile(target.downPath)
	if err != nil {
		return "", fmt.Errorf("failed to read down migration %s: %w", target.downPath, err)
	}

	err = WithTransaction(ctx, m.pool, func(tx pgx.Tx) error {
		if _, err := tx.Exec(ctx, string(content)); err != nil {
			return fmt.Errorf("execution of down script failed: %w", err)
		}
		_, err := tx.Exec(ctx, "DELETE FROM schema_migrations WHERE version = $1", latestVersion)
		return err
	})

	if err != nil {
		return "", fmt.Errorf("rollback of migration %s failed: %w", target.baseName, err)
	}

	return target.baseName, nil
}

// Status returns the list of all migrations and their current state.
func (m *Migrator) Status(ctx context.Context) ([]MigrationStatus, error) {
	applied, err := m.GetAppliedMigrations(ctx)
	if err != nil {
		return nil, err
	}

	allMigrations, err := m.discoverMigrations()
	if err != nil {
		return nil, err
	}

	var statuses []MigrationStatus
	for _, mig := range allMigrations {
		rec, isApplied := applied[mig.version]
		st := MigrationStatus{
			Version: mig.version,
			Name:    mig.baseName,
			Applied: isApplied,
		}
		if isApplied {
			appliedAtCopy := rec.AppliedAt
			st.AppliedAt = &appliedAtCopy
		}
		statuses = append(statuses, st)
	}

	return statuses, nil
}

// CurrentVersion returns the highest applied version or 0 if none.
func (m *Migrator) CurrentVersion(ctx context.Context) (int, error) {
	applied, err := m.GetAppliedMigrations(ctx)
	if err != nil {
		return 0, err
	}

	var latestVersion int = 0
	for v := range applied {
		if v > latestVersion {
			latestVersion = v
		}
	}

	return latestVersion, nil
}

