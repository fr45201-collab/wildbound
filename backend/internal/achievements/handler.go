package achievements

import (
	"net/http"

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

func (h *Handler) ListDefinitions(w http.ResponseWriter, r *http.Request) {
	defs, err := h.service.ListDefinitions(r.Context())
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve achievement definitions")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"achievements": defs,
		"count":        len(defs),
	})
}

func (h *Handler) ListPlayerAchievements(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	achievements, err := h.service.ListPlayerAchievements(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve player achievements")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"achievements": achievements,
		"count":        len(achievements),
	})
}

