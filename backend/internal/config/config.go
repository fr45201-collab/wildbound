package config

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

type DatabaseConfig struct {
	URL                    string `json:"url"`
	Host                   string `json:"host"`
	Port                   int    `json:"port"`
	Name                   string `json:"name"`
	User                   string `json:"user"`
	Password               string `json:"password"`
	SSLMode                string `json:"ssl_mode"`
	MaxConns               int    `json:"max_conns"`
	MinConns               int    `json:"min_conns"`
	MaxConnLifetimeMinutes int    `json:"max_conn_lifetime_minutes"`
	MaxConnIdleMinutes     int    `json:"max_conn_idle_minutes"`
	MigrationMode          string `json:"migration_mode"`
}

type JWTConfig struct {
	Issuer           string `json:"issuer"`
	Audience         string `json:"audience"`
	AccessTTLMinutes int    `json:"access_ttl_minutes"`
	RefreshTTLHours  int    `json:"refresh_ttl_hours"`
	SecretKey        string `json:"secret_key"`
}

type RateLimitConfig struct {
	RequestsPerSec int  `json:"requests_per_sec"`
	Burst          int  `json:"burst"`
	Enabled        bool `json:"enabled"`
}

type Config struct {
	AppEnv             string          `json:"app_env"`
	APIHost            string          `json:"api_host"`
	APIPort            int             `json:"api_port"`
	Database           DatabaseConfig  `json:"database"`
	JWT                JWTConfig       `json:"jwt"`
	RateLimit          RateLimitConfig `json:"rate_limit"`
	ServerSecret       string          `json:"server_secret"`
	CoordinatorSecret  string          `json:"coordinator_secret"`
	LogLevel           string          `json:"log_level"`
	CORSAllowedOrigins []string        `json:"cors_allowed_origins"`
}

func (c *DatabaseConfig) DSN() string {
	if c.URL != "" {
		return c.URL
	}
	if c.Password != "" {
		return fmt.Sprintf("postgres://%s:%s@%s:%d/%s?sslmode=%s",
			c.User, c.Password, c.Host, c.Port, c.Name, c.SSLMode)
	}
	return fmt.Sprintf("postgres://%s@%s:%d/%s?sslmode=%s",
		c.User, c.Host, c.Port, c.Name, c.SSLMode)
}

func (c *DatabaseConfig) MaxConnLifetime() time.Duration {
	if c.MaxConnLifetimeMinutes <= 0 {
		return 30 * time.Minute
	}
	return time.Duration(c.MaxConnLifetimeMinutes) * time.Minute
}

func (c *DatabaseConfig) MaxConnIdleTime() time.Duration {
	if c.MaxConnIdleMinutes <= 0 {
		return 5 * time.Minute
	}
	return time.Duration(c.MaxConnIdleMinutes) * time.Minute
}

func (c *Config) AccessTokenDuration() time.Duration {
	if c.JWT.AccessTTLMinutes <= 0 {
		return 15 * time.Minute
	}
	return time.Duration(c.JWT.AccessTTLMinutes) * time.Minute
}

func (c *Config) RefreshTokenDuration() time.Duration {
	if c.JWT.RefreshTTLHours <= 0 {
		return 168 * time.Hour
	}
	return time.Duration(c.JWT.RefreshTTLHours) * time.Hour
}

func (c *Config) IsDevelopment() bool {
	return c.AppEnv == "development" || c.AppEnv == "dev"
}

// loadDotEnv parses key-value pairs from a .env file if it exists, without overriding existing env vars.
func loadDotEnv(paths ...string) {
	for _, p := range paths {
		f, err := os.Open(p)
		if err != nil {
			continue
		}
		defer f.Close()

		scanner := bufio.NewScanner(f)
		for scanner.Scan() {
			line := strings.TrimSpace(scanner.Text())
			if line == "" || strings.HasPrefix(line, "#") {
				continue
			}
			parts := strings.SplitN(line, "=", 2)
			if len(parts) != 2 {
				continue
			}
			k := strings.TrimSpace(parts[0])
			v := strings.TrimSpace(parts[1])
			v = strings.Trim(v, `"'`)
			if _, exists := os.LookupEnv(k); !exists {
				os.Setenv(k, v)
			}
		}
		break
	}
}

func Load(configPath string) (*Config, error) {
	// Attempt to load .env from multiple common locations
	loadDotEnv(".env", "../.env", "backend/.env")

	cfg := &Config{
		AppEnv:   "development",
		APIHost:  "127.0.0.1",
		APIPort:  8080,
		LogLevel: "info",
		Database: DatabaseConfig{
			Host:                   "127.0.0.1",
			Port:                   5433,
			Name:                   "wildbound",
			User:                   "wildbound_user",
			SSLMode:                "disable",
			MaxConns:               20,
			MinConns:               2,
			MaxConnLifetimeMinutes: 30,
			MaxConnIdleMinutes:     5,
			MigrationMode:          "manual",
		},
		JWT: JWTConfig{
			Issuer:           "wildbound",
			Audience:         "wildbound-client",
			AccessTTLMinutes: 15,
			RefreshTTLHours:  168,
			SecretKey:        "wildbound-default-development-secret-change-me",
		},
		RateLimit: RateLimitConfig{
			RequestsPerSec: 60,
			Burst:          100,
			Enabled:        true,
		},
		ServerSecret:      "wildbound-server-internal-secret-token",
		CoordinatorSecret: "wildbound-coordinator-internal-secret-token",
		CORSAllowedOrigins: []string{
			"http://localhost:8080",
			"http://127.0.0.1:8080",
		},
	}

	if configPath != "" {
		if file, err := os.ReadFile(configPath); err == nil {
			if err := json.Unmarshal(file, cfg); err != nil {
				return nil, fmt.Errorf("failed to parse config JSON: %w", err)
			}
		}
	}

	// Environment variable overrides
	if val := os.Getenv("APP_ENV"); val != "" {
		cfg.AppEnv = val
	}
	if val := os.Getenv("API_HOST"); val != "" {
		cfg.APIHost = val
	}
	if val := os.Getenv("API_PORT"); val != "" {
		if p, err := strconv.Atoi(val); err == nil {
			cfg.APIPort = p
		}
	}

	// Database URL override takes precedence
	if val := os.Getenv("DATABASE_URL"); val != "" {
		cfg.Database.URL = val
	}

	if val := os.Getenv("DATABASE_HOST"); val != "" {
		cfg.Database.Host = val
	}
	if val := os.Getenv("DATABASE_PORT"); val != "" {
		if p, err := strconv.Atoi(val); err == nil {
			cfg.Database.Port = p
		}
	}
	if val := os.Getenv("DATABASE_NAME"); val != "" {
		cfg.Database.Name = val
	}
	if val := os.Getenv("DATABASE_USER"); val != "" {
		cfg.Database.User = val
	}
	if val := os.Getenv("DATABASE_PASSWORD"); val != "" {
		cfg.Database.Password = val
	}
	if val := os.Getenv("DATABASE_SSL_MODE"); val != "" {
		cfg.Database.SSLMode = val
	}
	if val := os.Getenv("DATABASE_MAX_CONNECTIONS"); val != "" {
		if mc, err := strconv.Atoi(val); err == nil {
			cfg.Database.MaxConns = mc
		}
	}
	if val := os.Getenv("DATABASE_MIN_CONNECTIONS"); val != "" {
		if mc, err := strconv.Atoi(val); err == nil {
			cfg.Database.MinConns = mc
		}
	}
	if val := os.Getenv("DATABASE_MAX_CONNECTION_LIFETIME"); val != "" {
		if d, err := time.ParseDuration(val); err == nil {
			cfg.Database.MaxConnLifetimeMinutes = int(d.Minutes())
		}
	}
	if val := os.Getenv("DATABASE_MAX_CONNECTION_IDLE_TIME"); val != "" {
		if d, err := time.ParseDuration(val); err == nil {
			cfg.Database.MaxConnIdleMinutes = int(d.Minutes())
		}
	}
	if val := os.Getenv("DATABASE_MIGRATION_MODE"); val != "" {
		cfg.Database.MigrationMode = val
	}

	if val := os.Getenv("JWT_SECRET_KEY"); val != "" {
		cfg.JWT.SecretKey = val
	}
	if val := os.Getenv("SERVER_INTERNAL_SECRET"); val != "" {
		cfg.ServerSecret = val
	}
	if val := os.Getenv("COORDINATOR_INTERNAL_SECRET"); val != "" {
		cfg.CoordinatorSecret = val
	}
	if val := os.Getenv("LOG_LEVEL"); val != "" {
		cfg.LogLevel = val
	}

	if err := cfg.Validate(); err != nil {
		return nil, err
	}

	return cfg, nil
}

func (c *Config) Validate() error {
	if c.APIPort < 1 || c.APIPort > 65535 {
		return fmt.Errorf("invalid api_port: %d", c.APIPort)
	}
	if c.Database.URL == "" {
		if c.Database.Port < 1 || c.Database.Port > 65535 {
			return fmt.Errorf("invalid database port: %d", c.Database.Port)
		}
		if c.Database.Name == "" {
			return fmt.Errorf("database name cannot be empty")
		}
		if c.Database.User == "" {
			return fmt.Errorf("database user cannot be empty")
		}
	}
	if c.JWT.SecretKey == "" {
		return fmt.Errorf("jwt secret_key cannot be empty")
	}
	if c.ServerSecret == "" {
		return fmt.Errorf("server_secret cannot be empty")
	}
	return nil
}
