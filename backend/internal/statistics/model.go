package statistics

import (
	"time"

	"github.com/google/uuid"
)

type PlayerStatistics struct {
	UserID                 uuid.UUID `json:"user_id"`
	MatchesPlayed          int       `json:"matches_played"`
	MatchesWon             int       `json:"matches_won"`
	Eliminations           int       `json:"eliminations"`
	Deaths                 int       `json:"deaths"`
	Assists                int       `json:"assists"`
	TotalDamage            float64   `json:"total_damage"`
	TotalDamageReceived    float64   `json:"total_damage_received"`
	TotalSurvivalSeconds   float64   `json:"total_survival_seconds"`
	HighestPlacement       int       `json:"highest_placement"`
	WinRate                float64   `json:"win_rate"`
	KillDeathRatio         float64   `json:"kill_death_ratio"`
	AverageDamage          float64   `json:"average_damage"`
	AverageSurvivalSeconds float64   `json:"average_survival_seconds"`
	UpdatedAt              time.Time `json:"updated_at"`
}
