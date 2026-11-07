package settings

import (
	"context"
	"errors"
	"fmt"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/repository"
)

var (
	ErrInvalidLanguage    = errors.New("unsupported language preference")
	ErrInvalidRegion      = errors.New("unsupported region preference")
	ErrInvalidGameMode    = errors.New("unsupported preferred game mode")
	ErrInvalidVisibility  = errors.New("unsupported privacy visibility setting")
)

var (
	supportedLanguages = map[string]bool{"en": true, "es": true, "fr": true, "de": true, "ja": true}
	supportedRegions   = map[string]bool{"us-east": true, "us-west": true, "eu-central": true, "ap-southeast": true}
	supportedModes     = map[string]bool{"solo": true, "duos": true, "squads": true}
	supportedVisibilities = map[string]bool{"public": true, "friends_only": true, "private": true}
	supportedHistoryVis   = map[string]bool{"public": true, "private": true}
)

type UpdateSettingsRequest struct {
	Language                 *string `json:"language,omitempty"`
	Region                   *string `json:"region,omitempty"`
	PreferredGameMode        *string `json:"preferred_game_mode,omitempty"`
	PrivacyProfileVisibility *string `json:"privacy_profile_visibility,omitempty"`
	PrivacyMatchHistory      *string `json:"privacy_match_history,omitempty"`
	AllowFriendRequests      *bool   `json:"allow_friend_requests,omitempty"`
}

type Service struct {
	repo repository.SettingsRepository
}

func NewService(repo repository.SettingsRepository) *Service {
	return &Service{repo: repo}
}

func (s *Service) GetSettings(ctx context.Context, userID uuid.UUID) (*repository.PlayerSettings, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}
	return s.repo.GetSettingsByUserID(ctx, userID)
}

func (s *Service) UpdateSettings(ctx context.Context, userID uuid.UUID, req *UpdateSettingsRequest) (*repository.PlayerSettings, error) {
	if userID == uuid.Nil {
		return nil, errors.New("invalid user id")
	}

	current, err := s.repo.GetSettingsByUserID(ctx, userID)
	if err != nil {
		return nil, err
	}

	if req.Language != nil {
		if !supportedLanguages[*req.Language] {
			return nil, ErrInvalidLanguage
		}
		current.Language = *req.Language
	}

	if req.Region != nil {
		if !supportedRegions[*req.Region] {
			return nil, ErrInvalidRegion
		}
		current.Region = *req.Region
	}

	if req.PreferredGameMode != nil {
		if !supportedModes[*req.PreferredGameMode] {
			return nil, ErrInvalidGameMode
		}
		current.PreferredGameMode = *req.PreferredGameMode
	}

	if req.PrivacyProfileVisibility != nil {
		if !supportedVisibilities[*req.PrivacyProfileVisibility] {
			return nil, ErrInvalidVisibility
		}
		current.PrivacyProfileVisibility = *req.PrivacyProfileVisibility
	}

	if req.PrivacyMatchHistory != nil {
		if !supportedHistoryVis[*req.PrivacyMatchHistory] {
			return nil, fmt.Errorf("invalid privacy_match_history: must be public or private")
		}
		current.PrivacyMatchHistory = *req.PrivacyMatchHistory
	}

	if req.AllowFriendRequests != nil {
		current.AllowFriendRequests = *req.AllowFriendRequests
	}

	if err := s.repo.UpdateSettings(ctx, current); err != nil {
		return nil, err
	}

	return current, nil
}

