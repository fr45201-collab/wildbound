package repository

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

var (
	ErrAchievementNotFound = errors.New("achievement definition not found")
)

type AchievementsRepository interface {
	ListDefinitions(ctx context.Context) ([]AchievementDefinition, error)
	GetDefinitionByID(ctx context.Context, id string) (*AchievementDefinition, error)
	ListPlayerAchievements(ctx context.Context, userID uuid.UUID) ([]PlayerAchievement, error)
	IncrementProgress(ctx context.Context, tx pgx.Tx, userID uuid.UUID, achievementID string, progressDelta int) (*PlayerAchievement, bool, error)
}

type achievementsRepo struct {
	pool *pgxpool.Pool
}

func NewAchievementsRepository(pool *pgxpool.Pool) AchievementsRepository {
	return &achievementsRepo{pool: pool}
}

func (r *achievementsRepo) ListDefinitions(ctx context.Context) ([]AchievementDefinition, error) {
	query := `
		SELECT id, achievement_key, display_name, description, category,
		       target_value, reward_xp, reward_points, is_active, created_at
		FROM achievement_definitions
		WHERE is_active = true
		ORDER BY category ASC, display_name ASC
	`
	rows, err := r.pool.Query(ctx, query)
	if err != nil {
		return nil, fmt.Errorf("failed to query achievement definitions: %w", err)
	}
	defer rows.Close()

	var defs []AchievementDefinition
	for rows.Next() {
		var d AchievementDefinition
		if err := rows.Scan(
			&d.ID, &d.AchievementKey, &d.DisplayName, &d.Description, &d.Category,
			&d.TargetValue, &d.RewardXP, &d.RewardPoints, &d.IsActive, &d.CreatedAt,
		); err != nil {
			return nil, fmt.Errorf("failed to scan achievement definition: %w", err)
		}
		defs = append(defs, d)
	}
	return defs, rows.Err()
}

func (r *achievementsRepo) GetDefinitionByID(ctx context.Context, id string) (*AchievementDefinition, error) {
	query := `
		SELECT id, achievement_key, display_name, description, category,
		       target_value, reward_xp, reward_points, is_active, created_at
		FROM achievement_definitions
		WHERE id = $1
	`
	var d AchievementDefinition
	err := r.pool.QueryRow(ctx, query, id).Scan(
		&d.ID, &d.AchievementKey, &d.DisplayName, &d.Description, &d.Category,
		&d.TargetValue, &d.RewardXP, &d.RewardPoints, &d.IsActive, &d.CreatedAt,
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return nil, ErrAchievementNotFound
		}
		return nil, fmt.Errorf("failed to query achievement definition: %w", err)
	}
	return &d, nil
}

func (r *achievementsRepo) ListPlayerAchievements(ctx context.Context, userID uuid.UUID) ([]PlayerAchievement, error) {
	query := `
		SELECT pa.id, pa.user_id, pa.achievement_id, pa.progress, pa.target,
		       pa.is_unlocked, pa.unlocked_at, pa.updated_at
		FROM player_achievements pa
		JOIN achievement_definitions ad ON ad.id = pa.achievement_id
		WHERE pa.user_id = $1
		ORDER BY pa.is_unlocked DESC, ad.category ASC
	`
	rows, err := r.pool.Query(ctx, query, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to query player achievements: %w", err)
	}
	defer rows.Close()

	var list []PlayerAchievement
	for rows.Next() {
		var a PlayerAchievement
		if err := rows.Scan(
			&a.ID, &a.UserID, &a.AchievementID, &a.Progress, &a.Target,
			&a.IsUnlocked, &a.UnlockedAt, &a.UpdatedAt,
		); err != nil {
			return nil, fmt.Errorf("failed to scan player achievement: %w", err)
		}
		list = append(list, a)
	}
	return list, rows.Err()
}

func (r *achievementsRepo) IncrementProgress(ctx context.Context, tx pgx.Tx, userID uuid.UUID, achievementID string, progressDelta int) (*PlayerAchievement, bool, error) {
	def, err := r.GetDefinitionByID(ctx, achievementID)
	if err != nil {
		return nil, false, err
	}

	query := `
		INSERT INTO player_achievements (
			user_id, achievement_id, progress, target, is_unlocked, unlocked_at, updated_at
		) VALUES (
			$1, $2, LEAST($3::int, $4::int), $4::int,
			CASE WHEN $3::int >= $4::int THEN true ELSE false END,
			CASE WHEN $3::int >= $4::int THEN NOW() ELSE NULL END,
			NOW()
		)
		ON CONFLICT (user_id, achievement_id) DO UPDATE SET
			progress = LEAST(player_achievements.progress + $3::int, player_achievements.target),
			is_unlocked = CASE
				WHEN player_achievements.is_unlocked THEN true
				WHEN (player_achievements.progress + $3::int) >= player_achievements.target THEN true
				ELSE false
			END,
			unlocked_at = CASE
				WHEN player_achievements.is_unlocked THEN player_achievements.unlocked_at
				WHEN (player_achievements.progress + $3::int) >= player_achievements.target THEN NOW()
				ELSE NULL
			END,
			updated_at = NOW()
		RETURNING id, user_id, achievement_id, progress, target, is_unlocked, unlocked_at, updated_at
	`

	var runner interface {
		QueryRow(ctx context.Context, sql string, args ...any) pgx.Row
	} = r.pool
	if tx != nil {
		runner = tx
	}

	var a PlayerAchievement
	err = runner.QueryRow(ctx, query, userID, achievementID, progressDelta, def.TargetValue).Scan(
		&a.ID, &a.UserID, &a.AchievementID, &a.Progress, &a.Target,
		&a.IsUnlocked, &a.UnlockedAt, &a.UpdatedAt,
	)
	if err != nil {
		return nil, false, fmt.Errorf("failed to increment achievement progress: %w", err)
	}

	// Just unlocked if is_unlocked is true and unlocked_at is within the last 5 seconds
	justUnlocked := a.IsUnlocked && a.UnlockedAt != nil && time.Since(*a.UnlockedAt) < 5*time.Second
	return &a, justUnlocked, nil
}
