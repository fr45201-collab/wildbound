package account

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/auth"
	"github.com/wildbound/backend/internal/repository"
	"github.com/wildbound/backend/internal/validation"
)

var (
	ErrIncorrectPassword = errors.New("current password is incorrect")
	ErrEmailInUse        = errors.New("email address is already in use by another account")
	ErrUserNotFound      = errors.New("user account not found")
)

type ChangePasswordRequest struct {
	CurrentPassword string `json:"current_password"`
	NewPassword     string `json:"new_password"`
}

type ChangeEmailRequest struct {
	NewEmail        string `json:"new_email"`
	CurrentPassword string `json:"current_password"`
}

type SecurityStatusResponse struct {
	UserID              uuid.UUID  `json:"user_id"`
	AccountStatus       string     `json:"account_status"`
	CreatedAt           time.Time  `json:"created_at"`
	LastLoginAt         *time.Time `json:"last_login_at,omitempty"`
	ActiveSessionsCount int        `json:"active_sessions_count"`
}

type Service struct {
	usersRepo    repository.UsersRepository
	sessionsRepo repository.SessionsRepository
}

func NewService(usersRepo repository.UsersRepository, sessionsRepo repository.SessionsRepository) *Service {
	return &Service{
		usersRepo:    usersRepo,
		sessionsRepo: sessionsRepo,
	}
}

func (s *Service) ChangePassword(ctx context.Context, userID uuid.UUID, req *ChangePasswordRequest) error {
	if req.CurrentPassword == "" || req.NewPassword == "" {
		return errors.New("current_password and new_password are required")
	}

	if err := validation.ValidatePassword(req.NewPassword); err != nil {
		return err
	}

	user, err := s.usersRepo.GetByID(ctx, userID)
	if err != nil {
		return ErrUserNotFound
	}

	valid, err := auth.VerifyPassword(req.CurrentPassword, user.PasswordHash)
	if err != nil || !valid {
		return ErrIncorrectPassword
	}

	newHash, err := auth.HashPassword(req.NewPassword)
	if err != nil {
		return fmt.Errorf("failed to hash password: %w", err)
	}

	if err := s.usersRepo.UpdatePassword(ctx, userID, newHash); err != nil {
		return fmt.Errorf("failed to update password: %w", err)
	}

	// Revoke all sessions on password change for security
	_ = s.sessionsRepo.RevokeAllUserSessions(ctx, userID, "password_changed")

	return nil
}

func (s *Service) ChangeEmail(ctx context.Context, userID uuid.UUID, req *ChangeEmailRequest) error {
	if req.CurrentPassword == "" || req.NewEmail == "" {
		return errors.New("new_email and current_password are required")
	}

	normEmail, err := validation.ValidateEmail(req.NewEmail)
	if err != nil {
		return err
	}

	user, err := s.usersRepo.GetByID(ctx, userID)
	if err != nil {
		return ErrUserNotFound
	}

	valid, err := auth.VerifyPassword(req.CurrentPassword, user.PasswordHash)
	if err != nil || !valid {
		return ErrIncorrectPassword
	}

	exists, err := s.usersRepo.CheckExists(ctx, "", normEmail)
	if err != nil {
		return fmt.Errorf("failed to check email uniqueness: %w", err)
	}
	if exists {
		return ErrEmailInUse
	}

	if err := s.usersRepo.UpdateEmail(ctx, userID, normEmail); err != nil {
		return fmt.Errorf("failed to update email: %w", err)
	}

	return nil
}

func (s *Service) GetSecurityStatus(ctx context.Context, userID uuid.UUID) (*SecurityStatusResponse, error) {
	user, err := s.usersRepo.GetByID(ctx, userID)
	if err != nil {
		return nil, ErrUserNotFound
	}

	sessions, err := s.sessionsRepo.ListUserSessions(ctx, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to list sessions: %w", err)
	}

	activeCount := 0
	for _, sess := range sessions {
		if sess.IsActive {
			activeCount++
		}
	}

	return &SecurityStatusResponse{
		UserID:              user.ID,
		AccountStatus:       user.AccountStatus,
		CreatedAt:           user.CreatedAt,
		LastLoginAt:         user.LastLoginAt,
		ActiveSessionsCount: activeCount,
	}, nil
}

