package progression

import (
	"net/http"
	"strconv"

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

func (h *Handler) GetMyProgression(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	details, err := h.service.GetMyProgression(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve progression")
		return
	}

	response.RenderSuccess(w, http.StatusOK, details)
}

func (h *Handler) GetMyProgressionEvents(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	limit := 20
	if lStr := r.URL.Query().Get("limit"); lStr != "" {
		if l, err := strconv.Atoi(lStr); err == nil && l > 0 && l <= 100 {
			limit = l
		}
	}

	offset := 0
	if oStr := r.URL.Query().Get("offset"); oStr != "" {
		if o, err := strconv.Atoi(oStr); err == nil && o >= 0 {
			offset = o
		}
	}

	events, err := h.service.ListEvents(r.Context(), userID, limit, offset)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to list progression events")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"events": events,
		"count":  len(events),
	})
}

