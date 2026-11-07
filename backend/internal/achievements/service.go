package achievements

import (
	"context"
	"errors"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/repository"
)

type Service struct {
	repo repository.AchievementsRepository
}

func NewService(repo repository.AchievementsRepository) *Service {
	return &Service{repo: repo}
}

func (s *Service) ListDefinitions(ctx context.Context) ([]repository.AchievementDefinition, error) {
	return s.repo.ListDefinitions(ctx)
}

func (s *Service) ListPlayerAchievements(ctx context.Context, userID uuid.UUID) ([]repository.PlayerAchievement, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}
	return s.repo.ListPlayerAchievements(ctx, userID)
}

