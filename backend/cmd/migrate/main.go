package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"text/tabwriter"
	"time"

	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/database"
)

func main() {
	if len(os.Args) < 2 {
		printUsage()
		os.Exit(1)
	}

	command := os.Args[1]

	migrateFlags := flag.NewFlagSet("migrate", flag.ExitOnError)
	configPath := migrateFlags.String("config", "configs/development.json", "Path to config file")
	migrationsDir := migrateFlags.String("migrations", "migrations", "Path to migrations folder")
	dbURL := migrateFlags.String("database-url", "", "Override database connection URL")

	if err := migrateFlags.Parse(os.Args[2:]); err != nil {
		fmt.Fprintf(os.Stderr, "failed to parse arguments: %v\n", err)
		os.Exit(1)
	}

	// 1. Load configuration
	cfg, err := config.Load(*configPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to load configuration: %v\n", err)
		os.Exit(1)
	}

	if *dbURL != "" {
		cfg.Database.URL = *dbURL
	}

	// 2. Connect to Database
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	pool, err := database.NewPool(ctx, &cfg.Database)
	if err != nil {
		fmt.Fprintf(os.Stderr, "database connection error: %v\n", err)
		os.Exit(1)
	}
	defer pool.Close()

	// 3. Initialize Migrator
	migrator := database.NewMigrator(pool, *migrationsDir)

	switch command {
	case "up":
		applied, err := migrator.Up(ctx)
		if err != nil {
			fmt.Fprintf(os.Stderr, "migration failed: %v\n", err)
			os.Exit(1)
		}
		if len(applied) == 0 {
			fmt.Println("Database is up to date. No pending migrations.")
		} else {
			fmt.Printf("Successfully applied %d migration(s):\n", len(applied))
			for _, m := range applied {
				fmt.Printf("  [+] %s\n", m)
			}
		}

	case "down":
		reverted, err := migrator.Down(ctx)
		if err != nil {
			fmt.Fprintf(os.Stderr, "rollback failed: %v\n", err)
			os.Exit(1)
		}
		fmt.Printf("Successfully rolled back migration:\n  [-] %s\n", reverted)

	case "status":
		statuses, err := migrator.Status(ctx)
		if err != nil {
			fmt.Fprintf(os.Stderr, "failed to fetch migration status: %v\n", err)
			os.Exit(1)
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 3, ' ', 0)
		fmt.Fprintln(w, "VERSION\tNAME\tSTATUS\tAPPLIED AT")
		fmt.Fprintln(w, "-------\t----\t------\t----------")
		for _, s := range statuses {
			statusStr := "PENDING"
			appliedAtStr := "-"
			if s.Applied {
				statusStr = "APPLIED"
				if s.AppliedAt != nil {
					appliedAtStr = s.AppliedAt.Format("2006-01-02 15:04:05 UTC")
				}
			}
			fmt.Fprintf(w, "%06d\t%s\t%s\t%s\n", s.Version, s.Name, statusStr, appliedAtStr)
		}
		w.Flush()

	case "version":
		v, err := migrator.CurrentVersion(ctx)
		if err != nil {
			fmt.Fprintf(os.Stderr, "failed to fetch current version: %v\n", err)
			os.Exit(1)
		}
		fmt.Printf("Current schema version: %06d\n", v)

	default:
		fmt.Fprintf(os.Stderr, "unknown command: %s\n", command)
		printUsage()
		os.Exit(1)
	}
}

func printUsage() {
	fmt.Println("Wildbound Migration CLI")
	fmt.Println("Usage: wildbound-migrate <command> [options]")
	fmt.Println("\nCommands:")
	fmt.Println("  up        Apply all pending migrations")
	fmt.Println("  down      Roll back the most recent migration")
	fmt.Println("  status    View migration history and pending status")
	fmt.Println("  version   Print current active schema version")
	fmt.Println("\nOptions:")
	fmt.Println("  -config         Path to configuration file (default: configs/development.json)")
	fmt.Println("  -migrations     Path to migrations directory (default: migrations)")
	fmt.Println("  -database-url   Direct PostgreSQL connection URL override")
}

