package statistics

import (
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

func (h *Handler) GetMyStatistics(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	stats, err := h.service.GetStatistics(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player statistics")
		return
	}

	response.RenderSuccess(w, http.StatusOK, stats)
}

func (h *Handler) GetPlayerStatistics(w http.ResponseWriter, r *http.Request) {
	idStr := chi.URLParam(r, "player_id")
	playerID, err := uuid.Parse(idStr)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_ID", "Invalid player ID format")
		return
	}

	stats, err := h.service.GetStatistics(r.Context(), playerID)
	if err != nil {
		if errors.Is(err, ErrStatsNotFound) {
			response.RenderError(w, http.StatusNotFound, "PLAYER_NOT_FOUND", "Player not found")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player statistics")
		return
	}

	response.RenderSuccess(w, http.StatusOK, stats)
}
