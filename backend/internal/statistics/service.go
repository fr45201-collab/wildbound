package statistics

import (
	"context"
	"errors"
	"fmt"
	"math"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

var ErrStatsNotFound = errors.New("player statistics not found")

type Service struct {
	db *pgxpool.Pool
}

func NewService(db *pgxpool.Pool) *Service {
	return &Service{db: db}
}

func (s *Service) GetStatistics(ctx context.Context, userID uuid.UUID) (*PlayerStatistics, error) {
	query := `
		SELECT user_id, matches_played, matches_won, eliminations, deaths, assists,
		       total_damage, total_damage_received, total_survival_seconds,
		       highest_placement, updated_at
		FROM player_statistics
		WHERE user_id = $1;
	`

	var stat PlayerStatistics
	err := s.db.QueryRow(ctx, query, userID).Scan(
		&stat.UserID, &stat.MatchesPlayed, &stat.MatchesWon, &stat.Eliminations,
		&stat.Deaths, &stat.Assists, &stat.TotalDamage, &stat.TotalDamageReceived,
		&stat.TotalSurvivalSeconds, &stat.HighestPlacement, &stat.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			var exists bool
			_ = s.db.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM users WHERE id = $1);`, userID).Scan(&exists)
			if !exists {
				return nil, ErrStatsNotFound
			}
			return &PlayerStatistics{
				UserID:                 userID,
				MatchesPlayed:          0,
				MatchesWon:             0,
				Eliminations:           0,
				Deaths:                 0,
				Assists:                0,
				TotalDamage:            0,
				TotalDamageReceived:    0,
				TotalSurvivalSeconds:   0,
				HighestPlacement:       0,
				WinRate:                0,
				KillDeathRatio:         0,
				AverageDamage:          0,
				AverageSurvivalSeconds: 0,
				UpdatedAt:              time.Now(),
			}, nil
		}
		return nil, fmt.Errorf("failed to query statistics: %w", err)
	}

	if stat.MatchesPlayed > 0 {
		stat.WinRate = math.Round((float64(stat.MatchesWon)/float64(stat.MatchesPlayed)*100)*100) / 100
		stat.AverageDamage = math.Round((stat.TotalDamage/float64(stat.MatchesPlayed))*100) / 100
		stat.AverageSurvivalSeconds = math.Round((stat.TotalSurvivalSeconds/float64(stat.MatchesPlayed))*100) / 100
	}

	if stat.Deaths > 0 {
		stat.KillDeathRatio = math.Round((float64(stat.Eliminations)/float64(stat.Deaths))*100) / 100
	} else {
		stat.KillDeathRatio = float64(stat.Eliminations)
	}

	return &stat, nil
}
