package progression

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"github.com/wildbound/backend/internal/database"
	"github.com/wildbound/backend/internal/repository"
)

type ProgressionDetailsResponse struct {
	UserID                uuid.UUID `json:"user_id"`
	AccountLevel          int       `json:"account_level"`
	TotalXP               int       `json:"total_xp"`
	CurrentLevelXP        int       `json:"current_level_xp"`
	NextLevelXP           int       `json:"next_level_xp"`
	ProgressPercentage    float32   `json:"progress_percentage"`
	AvailableRewardPoints int       `json:"available_reward_points"`
	MatchesCompleted      int       `json:"matches_completed"`
}

type Service struct {
	pool            *pgxpool.Pool
	progressionRepo repository.ProgressionRepository
}

func NewService(pool *pgxpool.Pool, repo repository.ProgressionRepository) *Service {
	return &Service{
		pool:            pool,
		progressionRepo: repo,
	}
}

func (s *Service) GetMyProgression(ctx context.Context, userID uuid.UUID) (*ProgressionDetailsResponse, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}

	p, err := s.progressionRepo.GetProgressionByUserID(ctx, userID)
	if err != nil {
		return nil, err
	}

	lvl, curXP, nextXP, pct := CalculateLevelFromXP(p.TotalXP)

	return &ProgressionDetailsResponse{
		UserID:                p.UserID,
		AccountLevel:          lvl,
		TotalXP:               p.TotalXP,
		CurrentLevelXP:        curXP,
		NextLevelXP:           nextXP,
		ProgressPercentage:    pct,
		AvailableRewardPoints: p.AvailableRewardPoints,
		MatchesCompleted:      p.MatchesCompleted,
	}, nil
}

// AwardMatchProgression transactionally records match XP and updates player progression.
// Prevents duplicate grants for the same match via unique constraint check.
func (s *Service) AwardMatchProgression(ctx context.Context, tx pgx.Tx, userID uuid.UUID, matchID string, placement, eliminations int, damageDealt, survivalSeconds float32) (*repository.ProgressionEvent, *repository.PlayerProgression, error) {
	if userID == uuid.Nil || matchID == "" {
		return nil, nil, errors.New("invalid user id or match id")
	}

	// 1. Check duplicate grant
	hasEvent, err := s.progressionRepo.HasProgressionEvent(ctx, userID, "match", matchID)
	if err != nil {
		return nil, nil, err
	}
	if hasEvent {
		return nil, nil, repository.ErrDuplicateProgressionEvent
	}

	// 2. Compute XP and reward points
	xpDelta, pointsDelta, reason := CalculateMatchXP(placement, eliminations, damageDealt, survivalSeconds)

	// 3. Current progression
	current, err := s.progressionRepo.GetProgressionByUserID(ctx, userID)
	if err != nil {
		return nil, nil, err
	}

	newTotalXP := current.TotalXP + xpDelta
	newLevel, _, _, _ := CalculateLevelFromXP(newTotalXP)

	// 4. Record progression event
	event := &repository.ProgressionEvent{
		UserID:            userID,
		SourceType:        "match",
		SourceID:          matchID,
		XPDelta:           xpDelta,
		RewardPointsDelta: pointsDelta,
		Reason:            reason,
	}

	if err := s.progressionRepo.RecordProgressionEvent(ctx, tx, event); err != nil {
		return nil, nil, fmt.Errorf("failed to record match progression event: %w", err)
	}

	// 5. Update player progression
	updatedProg, err := s.progressionRepo.UpdateProgression(ctx, tx, userID, newLevel, newTotalXP, pointsDelta, 1)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to update progression: %w", err)
	}

	return event, updatedProg, nil
}

// AwardBonusProgression awards XP / rewards for achievements or bonuses.
func (s *Service) AwardBonusProgression(ctx context.Context, userID uuid.UUID, sourceType, sourceID string, xpDelta, pointsDelta int, reason string) (*repository.ProgressionEvent, *repository.PlayerProgression, error) {
	if userID == uuid.Nil {
		return nil, nil, errors.New("invalid user id")
	}

	var event *repository.ProgressionEvent
	var updated *repository.PlayerProgression

	err := database.WithTransaction(ctx, s.pool, func(tx pgx.Tx) error {
		hasEvent, err := s.progressionRepo.HasProgressionEvent(ctx, userID, sourceType, sourceID)
		if err != nil {
			return err
		}
		if hasEvent {
			return repository.ErrDuplicateProgressionEvent
		}

		current, err := s.progressionRepo.GetProgressionByUserID(ctx, userID)
		if err != nil {
			return err
		}

		newTotalXP := current.TotalXP + xpDelta
		newLevel, _, _, _ := CalculateLevelFromXP(newTotalXP)

		event = &repository.ProgressionEvent{
			UserID:            userID,
			SourceType:        sourceType,
			SourceID:          sourceID,
			XPDelta:           xpDelta,
			RewardPointsDelta: pointsDelta,
			Reason:            reason,
		}

		if err := s.progressionRepo.RecordProgressionEvent(ctx, tx, event); err != nil {
			return err
		}

		updated, err = s.progressionRepo.UpdateProgression(ctx, tx, userID, newLevel, newTotalXP, pointsDelta, 0)
		return err
	})

	if err != nil {
		return nil, nil, err
	}
	return event, updated, nil
}

func (s *Service) ListEvents(ctx context.Context, userID uuid.UUID, limit, offset int) ([]repository.ProgressionEvent, error) {
	return s.progressionRepo.ListProgressionEvents(ctx, userID, limit, offset)
}

