package settings

import (
	"encoding/json"
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

func (h *Handler) GetSettings(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	settings, err := h.service.GetSettings(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve settings")
		return
	}

	response.RenderSuccess(w, http.StatusOK, settings)
}

func (h *Handler) UpdateSettings(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	var req UpdateSettingsRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "MALFORMED_REQUEST", "Invalid JSON payload")
		return
	}

	updated, err := h.service.UpdateSettings(r.Context(), userID, &req)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "VALIDATION_ERROR", err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, updated)
}

