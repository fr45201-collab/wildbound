package auth

import (
	"context"
	"encoding/json"
	"errors"
	"net"
	"net/http"
	"strings"

	"github.com/go-chi/chi/v5"
	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/repository"
	"github.com/wildbound/backend/internal/response"
	"github.com/wildbound/backend/internal/validation"
)

type Handler struct {
	service *Service
}

func NewHandler(s *Service) *Handler {
	return &Handler{service: s}
}

func getUserID(ctx context.Context) uuid.UUID {
	val, ok := ctx.Value(ContextKeyUserID).(string)
	if !ok || val == "" {
		return uuid.Nil
	}
	u, err := uuid.Parse(val)
	if err != nil {
		return uuid.Nil
	}
	return u
}

func getClientIP(r *http.Request) string {
	if xff := r.Header.Get("X-Forwarded-For"); xff != "" {
		parts := strings.Split(xff, ",")
		return strings.TrimSpace(parts[0])
	}
	if xri := r.Header.Get("X-Real-IP"); xri != "" {
		return xri
	}
	host, _, err := net.SplitHostPort(r.RemoteAddr)
	if err == nil {
		return host
	}
	return r.RemoteAddr
}

func (h *Handler) Register(w http.ResponseWriter, r *http.Request) {
	var req RegisterRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload in request body")
		return
	}

	ip := getClientIP(r)
	res, err := h.service.Register(r.Context(), &req, ip)
	if err != nil {
		switch {
		case errors.Is(err, validation.ErrInvalidEmail):
			response.RenderError(w, http.StatusBadRequest, "INVALID_EMAIL", err.Error())
		case errors.Is(err, validation.ErrInvalidUsername):
			response.RenderError(w, http.StatusBadRequest, "INVALID_USERNAME", err.Error())
		case errors.Is(err, validation.ErrPasswordTooShort), errors.Is(err, validation.ErrPasswordComplexity):
			response.RenderError(w, http.StatusBadRequest, "WEAK_PASSWORD", err.Error())
		case errors.Is(err, validation.ErrInvalidDisplayName):
			response.RenderError(w, http.StatusBadRequest, "INVALID_DISPLAY_NAME", err.Error())
		case errors.Is(err, ErrUserAlreadyExists):
			response.RenderError(w, http.StatusConflict, "USER_ALREADY_EXISTS", "A user with this username or email already exists.")
		default:
			response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to register user account")
		}
		return
	}

	response.RenderSuccess(w, http.StatusCreated, res)
}

func (h *Handler) Login(w http.ResponseWriter, r *http.Request) {
	var req LoginRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload in request body")
		return
	}

	ip := getClientIP(r)
	res, err := h.service.Login(r.Context(), &req, ip)
	if err != nil {
		if errors.Is(err, ErrInvalidCredentials) {
			response.RenderError(w, http.StatusUnauthorized, "INVALID_CREDENTIALS", "Invalid email/username or password.")
			return
		}
		if errors.Is(err, ErrAccountSuspended) {
			response.RenderError(w, http.StatusForbidden, "ACCOUNT_SUSPENDED", "Account is suspended or inactive.")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "An error occurred during authentication.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}

func (h *Handler) Refresh(w http.ResponseWriter, r *http.Request) {
	var req RefreshRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload in request body")
		return
	}

	ip := getClientIP(r)
	res, err := h.service.RefreshTokens(r.Context(), req.RefreshToken, ip)
	if err != nil {
		if errors.Is(err, ErrTokenTheftSuspected) {
			response.RenderError(w, http.StatusUnauthorized, "TOKEN_THEFT_DETECTED", "Suspicious token reuse detected; all sessions revoked for security.")
			return
		}
		if errors.Is(err, ErrInvalidRefreshToken) {
			response.RenderError(w, http.StatusUnauthorized, "INVALID_REFRESH_TOKEN", "Refresh token is expired, invalid, or revoked.")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to refresh authentication tokens.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, res)
}

func (h *Handler) Logout(w http.ResponseWriter, r *http.Request) {
	var req RefreshRequest
	_ = json.NewDecoder(r.Body).Decode(&req)

	_ = h.service.Logout(r.Context(), req.RefreshToken)
	response.RenderSuccess(w, http.StatusOK, map[string]string{
		"message": "Successfully logged out and revoked refresh token session.",
	})
}

func (h *Handler) LogoutAll(w http.ResponseWriter, r *http.Request) {
	userID := getUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	if err := h.service.LogoutAll(r.Context(), userID); err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to revoke all user sessions.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]string{
		"message": "Successfully revoked all active sessions for this account.",
	})
}

func (h *Handler) ListSessions(w http.ResponseWriter, r *http.Request) {
	userID := getUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	sessions, err := h.service.ListSessions(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to list active sessions.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"sessions": sessions,
		"count":    len(sessions),
	})
}

func (h *Handler) RevokeSession(w http.ResponseWriter, r *http.Request) {
	userID := getUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	sessionIDStr := chi.URLParam(r, "session_id")
	sessionID, err := uuid.Parse(sessionIDStr)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_SESSION_ID", "Malformed session ID format")
		return
	}

	if err := h.service.RevokeSession(r.Context(), sessionID, userID); err != nil {
		if errors.Is(err, repository.ErrSessionNotFound) {
			response.RenderError(w, http.StatusNotFound, "SESSION_NOT_FOUND", "Specified session was not found or already revoked.")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to revoke session.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]string{
		"message": "Session successfully revoked.",
	})
}

func (h *Handler) Me(w http.ResponseWriter, r *http.Request) {
	userID := getUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Missing or invalid authentication context.")
		return
	}

	user, err := h.service.GetMe(r.Context(), userID.String())
	if err != nil {
		if errors.Is(err, ErrUserNotFound) {
			response.RenderError(w, http.StatusNotFound, "USER_NOT_FOUND", "Authenticated user record could not be found.")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to fetch user profile.")
		return
	}

	response.RenderSuccess(w, http.StatusOK, user)
}
