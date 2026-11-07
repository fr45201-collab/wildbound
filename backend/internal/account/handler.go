package account

import (
	"encoding/json"
	"errors"
	"net/http"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/middleware"
	"github.com/wildbound/backend/internal/response"
	"github.com/wildbound/backend/internal/validation"
)

type Handler struct {
	service *Service
}

func NewHandler(service *Service) *Handler {
	return &Handler{service: service}
}

func (h *Handler) ChangePassword(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	var req ChangePasswordRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload")
		return
	}

	if err := h.service.ChangePassword(r.Context(), userID, &req); err != nil {
		switch {
		case errors.Is(err, ErrIncorrectPassword):
			response.RenderError(w, http.StatusUnauthorized, "INCORRECT_PASSWORD", "Current password does not match.")
		case errors.Is(err, validation.ErrPasswordTooShort), errors.Is(err, validation.ErrPasswordComplexity):
			response.RenderError(w, http.StatusBadRequest, "WEAK_PASSWORD", err.Error())
		default:
			response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", err.Error())
		}
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]string{
		"message": "Password changed successfully. All active sessions have been revoked.",
	})
}

func (h *Handler) ChangeEmail(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	var req ChangeEmailRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload")
		return
	}

	if err := h.service.ChangeEmail(r.Context(), userID, &req); err != nil {
		switch {
		case errors.Is(err, ErrIncorrectPassword):
			response.RenderError(w, http.StatusUnauthorized, "INCORRECT_PASSWORD", "Current password does not match.")
		case errors.Is(err, ErrEmailInUse):
			response.RenderError(w, http.StatusConflict, "EMAIL_IN_USE", "Email is already registered.")
		case errors.Is(err, validation.ErrInvalidEmail):
			response.RenderError(w, http.StatusBadRequest, "INVALID_EMAIL", err.Error())
		default:
			response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", err.Error())
		}
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]string{
		"message": "Email updated successfully.",
	})
}

func (h *Handler) GetSecurityStatus(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	status, err := h.service.GetSecurityStatus(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, status)
}

