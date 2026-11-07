package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/observability"
	"github.com/wildbound/backend/internal/server"
)

func main() {
	configPath := flag.String("config", "configs/development.json", "Path to configuration file")
	portOverride := flag.Int("port", 0, "Override HTTP server port")
	flag.Parse()

	// 1. Load configuration
	cfg, err := config.Load(*configPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to load config from %s: %v\n", *configPath, err)
		os.Exit(1)
	}

	if *portOverride > 0 {
		cfg.APIPort = *portOverride
	}

	// 2. Initialize structured logger
	logger := observability.NewLogger(cfg.LogLevel, cfg.IsDevelopment())
	logger.Info("initializing Wildbound backend",
		"version", "1.0.0",
		"env", cfg.AppEnv,
	)

	// 3. Connect to PostgreSQL
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	db, err := database.Connect(ctx, &cfg.Database, logger)
	if err != nil {
		logger.Error("failed to connect to postgresql database", "error", err.Error())
		os.Exit(1)
	}
	defer db.Close()

	// 4. Initialize HTTP server
	srv := server.NewServer(cfg, db, logger)

	// 5. Handle graceful shutdown
	serverErrors := make(chan error, 1)
	go func() {
		serverErrors <- srv.Start()
	}()

	shutdownSig := make(chan os.Signal, 1)
	signal.Notify(shutdownSig, os.Interrupt, syscall.SIGTERM)

	select {
	case err := <-serverErrors:
		logger.Error("server encountered fatal error", "error", err.Error())
		os.Exit(1)
	case sig := <-shutdownSig:
		logger.Info("shutdown signal received", "signal", sig.String())

		shutdownCtx, shutdownCancel := context.WithTimeout(context.Background(), 15*time.Second)
		defer shutdownCancel()

		if err := srv.Shutdown(shutdownCtx); err != nil {
			logger.Error("graceful shutdown failed", "error", err.Error())
			_ = srv.Shutdown(context.Background())
		}
		logger.Info("server exited cleanly")
	}
}
