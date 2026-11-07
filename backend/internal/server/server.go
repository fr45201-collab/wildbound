package server

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"github.com/go-chi/chi/v5"
	chimiddleware "github.com/go-chi/chi/v5/middleware"
	"github.com/wildbound/backend/internal/account"
	"github.com/wildbound/backend/internal/achievements"
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/config"
	"github.com/wildbound/backend/internal/cosmetics"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/inventory"
	"github.com/wildbound/backend/internal/matches"
	"github.com/wildbound/backend/internal/middleware"
	"github.com/wildbound/backend/internal/observability"
	"github.com/wildbound/backend/internal/profiles"
	"github.com/wildbound/backend/internal/progression"
	"github.com/wildbound/backend/internal/repository"
	"github.com/wildbound/backend/internal/response"
	"github.com/wildbound/backend/internal/settings"
	"github.com/wildbound/backend/internal/statistics"
)

type Server struct {
	httpServer  *http.Server
	db          *database.DB
	logger      *observability.Logger
	cfg         *config.Config
	rateLimiter *middleware.RateLimiter
}

func NewServer(cfg *config.Config, db *database.DB, logger *observability.Logger) *Server {
	r := chi.NewRouter()

	rateLimiter := middleware.NewRateLimiter(
		cfg.RateLimit.RequestsPerSec,
		cfg.RateLimit.Burst,
		cfg.RateLimit.Enabled,
	)

	// Global Middleware
	r.Use(middleware.RequestID)
	r.Use(middleware.RequestLogger(logger.Logger))
	r.Use(middleware.CORS(cfg.CORSAllowedOrigins))
	r.Use(rateLimiter.Middleware())
	r.Use(chimiddleware.Recoverer)

	// Health & Readiness probes
	r.Get("/health", func(w http.ResponseWriter, r *http.Request) {
		report := db.CheckHealth(r.Context())
		response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
			"status":   "healthy",
			"service":  "wildbound-backend",
			"version":  "1.0.0",
			"time":     time.Now().UTC(),
			"database": report,
		})
	})

	r.Get("/ready", func(w http.ResponseWriter, r *http.Request) {
		report := db.CheckHealth(r.Context())
		if report.Status != "healthy" {
			response.RenderError(w, http.StatusServiceUnavailable, "SERVICE_UNAVAILABLE", report.Error)
			return
		}
		response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
			"status":   "ready",
			"database": report,
		})
	})

	// Initialize Repositories
	inventoryRepo := repository.NewInventoryRepository(db.Pool)
	usersRepo := repository.NewUsersRepository(db.Pool)
	sessionsRepo := repository.NewSessionsRepository(db.Pool)
	settingsRepo := repository.NewSettingsRepository(db.Pool)
	progressionRepo := repository.NewProgressionRepository(db.Pool)
	achievementsRepo := repository.NewAchievementsRepository(db.Pool)

	// Initialize Token Manager
	tokenManager := auth.NewTokenManager(cfg)

	// Initialize Services & Handlers
	authService := auth.NewService(db, tokenManager, cfg)
	authHandler := auth.NewHandler(authService)

	accountService := account.NewService(usersRepo, sessionsRepo)
	accountHandler := account.NewHandler(accountService)

	profileService := profiles.NewService(db.Pool)
	profileHandler := profiles.NewHandler(profileService)

	settingsService := settings.NewService(settingsRepo)
	settingsHandler := settings.NewHandler(settingsService)

	progressionService := progression.NewService(db.Pool, progressionRepo)
	progressionHandler := progression.NewHandler(progressionService)

	achievementsService := achievements.NewService(achievementsRepo)
	achievementsHandler := achievements.NewHandler(achievementsService)

	cosmeticsRepo := repository.NewCosmeticsRepository(db.Pool)
	cosmeticService := cosmetics.NewService(cosmeticsRepo)
	cosmeticHandler := cosmetics.NewHandler(cosmeticService)

	matchService := matches.NewService(db.Pool)
	matchService.SetProgression(progressionService, achievementsRepo)
	matchHandler := matches.NewHandler(matchService)

	statsService := statistics.NewService(db.Pool)
	statsHandler := statistics.NewHandler(statsService)

	inventoryService := inventory.NewService(inventoryRepo)
	inventoryHandler := inventory.NewHandler(inventoryService)

	// Auth Middleware
	bearerAuth := middleware.RequireAuth(tokenManager)
	serverAuth := middleware.RequireServerAuth(cfg.ServerSecret)

	// API v1 Routes
	r.Route("/api/v1", func(api chi.Router) {
		// Public Auth
		api.Post("/auth/register", authHandler.Register)
		api.Post("/auth/login", authHandler.Login)
		api.Post("/auth/refresh", authHandler.Refresh)

		// Authenticated Auth & Session Management
		api.With(bearerAuth).Post("/auth/logout", authHandler.Logout)
		api.With(bearerAuth).Post("/auth/logout-all", authHandler.LogoutAll)
		api.With(bearerAuth).Get("/auth/sessions", authHandler.ListSessions)
		api.With(bearerAuth).Delete("/auth/sessions/{session_id}", authHandler.RevokeSession)
		api.With(bearerAuth).Get("/auth/me", authHandler.Me)

		// Authenticated Account Management
		api.Route("/account", func(acc chi.Router) {
			acc.Use(bearerAuth)
			acc.Patch("/password", accountHandler.ChangePassword)
			acc.Patch("/email", accountHandler.ChangeEmail)
			acc.Get("/security", accountHandler.GetSecurityStatus)
		})

		// Public Cosmetics & Achievement Definitions
		api.Get("/cosmetics", cosmeticHandler.ListCatalogue)
		api.Get("/achievements", achievementsHandler.ListDefinitions)

		// Authenticated Player Profiles, Cosmetics, Inventory, Settings, Progression, Matches & Statistics
		api.Route("/players/me", func(me chi.Router) {
			me.Use(bearerAuth)
			me.Get("/", profileHandler.GetMe)
			me.Patch("/", profileHandler.UpdateMe)
			me.Get("/settings", settingsHandler.GetSettings)
			me.Patch("/settings", settingsHandler.UpdateSettings)
			me.Get("/progression", progressionHandler.GetMyProgression)
			me.Get("/progression/events", progressionHandler.GetMyProgressionEvents)
			me.Get("/achievements", achievementsHandler.ListPlayerAchievements)
			me.Get("/cosmetics", cosmeticHandler.ListPlayerCosmetics)
			me.Get("/loadout", cosmeticHandler.GetMyLoadout)
			me.Put("/loadout", cosmeticHandler.UpdateMyLoadout)
			me.Post("/loadout/equip", cosmeticHandler.EquipItem)
			me.Post("/loadout/unequip", cosmeticHandler.UnequipSlot)
			me.Post("/cosmetics/equip", cosmeticHandler.EquipItem)
			me.Get("/inventory", inventoryHandler.GetMyInventory)
			me.Post("/inventory/consume", inventoryHandler.ConsumeItem)
			me.Get("/matches", matchHandler.GetMyMatches)
			me.Get("/statistics", statsHandler.GetMyStatistics)
		})

		// Public Player Profiles, Match History & Statistics
		api.Get("/players/{player_id}", profileHandler.GetPlayer)
		api.Get("/players/{player_id}/matches", matchHandler.GetPlayerMatches)
		api.Get("/players/{player_id}/statistics", statsHandler.GetPlayerStatistics)

		// Public Match Inspection
		api.Get("/matches/{match_id}", matchHandler.GetMatch)

		// Internal Server / Coordinator Endpoints
		api.Route("/internal", func(internal chi.Router) {
			internal.Use(serverAuth)
			internal.Post("/matches/results", matchHandler.SubmitResult)
			internal.Get("/players/{user_id}/loadout", cosmeticHandler.GetServerVerifiedLoadout)
			internal.Post("/cosmetics/grant", cosmeticHandler.GrantCosmetic)
			internal.Post("/inventory/grant", inventoryHandler.GrantItem)
		})
	})

	addr := fmt.Sprintf("%s:%d", cfg.APIHost, cfg.APIPort)
	httpServer := &http.Server{
		Addr:         addr,
		Handler:      r,
		ReadTimeout:  15 * time.Second,
		WriteTimeout: 15 * time.Second,
		IdleTimeout:  120 * time.Second,
	}

	return &Server{
		httpServer:  httpServer,
		db:          db,
		logger:      logger,
		cfg:         cfg,
		rateLimiter: rateLimiter,
	}
}

func (s *Server) Handler() http.Handler {
	return s.httpServer.Handler
}

func (s *Server) Start() error {
	s.logger.Info("starting Wildbound backend server",
		"host", s.cfg.APIHost,
		"port", s.cfg.APIPort,
		"env", s.cfg.AppEnv,
	)
	if err := s.httpServer.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		return fmt.Errorf("http server failed: %w", err)
	}
	return nil
}

func (s *Server) Shutdown(ctx context.Context) error {
	s.logger.Info("shutting down Wildbound backend server")
	s.rateLimiter.Stop()
	return s.httpServer.Shutdown(ctx)
}
