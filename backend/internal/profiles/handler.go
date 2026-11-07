package profiles

import (
	"encoding/json"
	"errors"
	"net/http"

	"github.com/go-chi/chi/v5"
	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/middleware"
	"github.com/wildbound/backend/internal/response"
)

type Handler struct {
	service *Service
}

func NewHandler(service *Service) *Handler {
	return &Handler{service: service}
}

func (h *Handler) GetMe(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	profile, err := h.service.GetProfile(r.Context(), userID)
	if err != nil {
		if errors.Is(err, ErrProfileNotFound) {
			response.RenderError(w, http.StatusNotFound, "PROFILE_NOT_FOUND", "Profile not found")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve profile")
		return
	}

	response.RenderSuccess(w, http.StatusOK, profile)
}

func (h *Handler) UpdateMe(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	var req UpdateProfileRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_REQUEST", "Invalid JSON request body")
		return
	}

	updated, err := h.service.UpdateProfile(r.Context(), userID, &req)
	if err != nil {
		if errors.Is(err, ErrCosmeticNotOwned) {
			response.RenderError(w, http.StatusForbidden, "COSMETIC_NOT_OWNED", err.Error())
			return
		}
		if errors.Is(err, ErrInvalidCosmetic) {
			response.RenderError(w, http.StatusBadRequest, "INVALID_COSMETIC", err.Error())
			return
		}
		response.RenderError(w, http.StatusBadRequest, "VALIDATION_ERROR", err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, updated)
}

func (h *Handler) GetPlayer(w http.ResponseWriter, r *http.Request) {
	idStr := chi.URLParam(r, "player_id")
	playerID, err := uuid.Parse(idStr)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_ID", "Invalid player ID format")
		return
	}

	publicProfile, err := h.service.GetPublicProfile(r.Context(), playerID)
	if err != nil {
		if errors.Is(err, ErrProfileNotFound) {
			response.RenderError(w, http.StatusNotFound, "PLAYER_NOT_FOUND", "Player not found")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player profile")
		return
	}

	response.RenderSuccess(w, http.StatusOK, publicProfile)
}
