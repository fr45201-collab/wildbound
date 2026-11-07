package repository

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

var (
	ErrStatisticsNotFound = errors.New("player statistics not found")
)

type StatisticsRepository interface {
	Create(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error
	GetByUserID(ctx context.Context, userID uuid.UUID) (*PlayerStatistics, error)
	UpdateMatchStats(ctx context.Context, tx pgx.Tx, userID uuid.UUID, won bool, elims, deaths, assists int, dmgDealt, dmgReceived, survivalSec float32, placement int) error
}

type statisticsRepo struct {
	pool *pgxpool.Pool
}

func NewStatisticsRepository(pool *pgxpool.Pool) StatisticsRepository {
	return &statisticsRepo{pool: pool}
}

func (r *statisticsRepo) Create(ctx context.Context, tx pgx.Tx, userID uuid.UUID) error {
	query := `
		INSERT INTO player_statistics (user_id)
		VALUES ($1)
		ON CONFLICT (user_id) DO NOTHING
	`
	if tx != nil {
		_, err := tx.Exec(ctx, query, userID)
		return err
	}
	_, err := r.pool.Exec(ctx, query, userID)
	return err
}

func (r *statisticsRepo) GetByUserID(ctx context.Context, userID uuid.UUID) (*PlayerStatistics, error) {
	query := `
		SELECT id, user_id, matches_played, matches_won, eliminations,
		       deaths, assists, total_damage, total_damage_received,
		       total_survival_seconds, highest_placement, updated_at
		FROM player_statistics
		WHERE user_id = $1
	`
	var s PlayerStatistics
	err := r.pool.QueryRow(ctx, query, userID).Scan(
		&s.ID, &s.UserID, &s.MatchesPlayed, &s.MatchesWon, &s.Eliminations,
		&s.Deaths, &s.Assists, &s.TotalDamage, &s.TotalDamageReceived,
		&s.TotalSurvivalSeconds, &s.HighestPlacement, &s.UpdatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrStatisticsNotFound
		}
		return nil, fmt.Errorf("failed to query player statistics: %w", err)
	}
	return &s, nil
}

func (r *statisticsRepo) UpdateMatchStats(ctx context.Context, tx pgx.Tx, userID uuid.UUID, won bool, elims, deaths, assists int, dmgDealt, dmgReceived, survivalSec float32, placement int) error {
	wonInt := 0
	if won {
		wonInt = 1
	}

	query := `
		INSERT INTO player_statistics (
			user_id, matches_played, matches_won, eliminations,
			deaths, assists, total_damage, total_damage_received,
			total_survival_seconds, highest_placement, updated_at
		) VALUES ($1, 1, $2, $3, $4, $5, $6, $7, $8, $9, NOW())
		ON CONFLICT (user_id) DO UPDATE SET
			matches_played = player_statistics.matches_played + 1,
			matches_won = player_statistics.matches_won + EXCLUDED.matches_won,
			eliminations = player_statistics.eliminations + EXCLUDED.eliminations,
			deaths = player_statistics.deaths + EXCLUDED.deaths,
			assists = player_statistics.assists + EXCLUDED.assists,
			total_damage = player_statistics.total_damage + EXCLUDED.total_damage,
			total_damage_received = player_statistics.total_damage_received + EXCLUDED.total_damage_received,
			total_survival_seconds = player_statistics.total_survival_seconds + EXCLUDED.total_survival_seconds,
			highest_placement = CASE
				WHEN player_statistics.highest_placement = 0 OR EXCLUDED.highest_placement < player_statistics.highest_placement
				THEN EXCLUDED.highest_placement
				ELSE player_statistics.highest_placement
			END,
			updated_at = NOW()
	`

	if tx != nil {
		_, err := tx.Exec(ctx, query, userID, wonInt, elims, deaths, assists, dmgDealt, dmgReceived, survivalSec, placement)
		return err
	}
	_, err := r.pool.Exec(ctx, query, userID, wonInt, elims, deaths, assists, dmgDealt, dmgReceived, survivalSec, placement)
	return err
}
