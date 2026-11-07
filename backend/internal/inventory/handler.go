package inventory

import (
	"encoding/json"
	"errors"
	"net/http"

	"github.com/google/uuid"
	"github.com/wildbound/backend/internal/middleware"
	"github.com/wildbound/backend/internal/response"
)

type ConsumeRequest struct {
	ItemDefinitionID string `json:"item_definition_id"`
	Quantity         int    `json:"quantity"`
}

type GrantRequest struct {
	UserID           string `json:"user_id"`
	ItemDefinitionID string `json:"item_definition_id"`
	Quantity         int    `json:"quantity"`
}

type Handler struct {
	service *Service
}

func NewHandler(service *Service) *Handler {
	return &Handler{service: service}
}

func (h *Handler) GetMyInventory(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	items, err := h.service.GetMyInventory(r.Context(), userID)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", "Failed to retrieve inventory")
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"items": items,
		"count": len(items),
	})
}

func (h *Handler) ConsumeItem(w http.ResponseWriter, r *http.Request) {
	userID := middleware.GetUserID(r.Context())
	if userID == uuid.Nil {
		response.RenderError(w, http.StatusUnauthorized, "UNAUTHORIZED", "Authentication required")
		return
	}

	var req ConsumeRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "BAD_REQUEST", "Invalid JSON payload")
		return
	}

	if req.ItemDefinitionID == "" {
		response.RenderError(w, http.StatusBadRequest, "MISSING_FIELD", "item_definition_id is required")
		return
	}
	if req.Quantity <= 0 {
		response.RenderError(w, http.StatusBadRequest, "INVALID_QUANTITY", "quantity must be greater than zero")
		return
	}

	item, err := h.service.ConsumeItem(r.Context(), userID, req.ItemDefinitionID, req.Quantity)
	if err != nil {
		if errors.Is(err, ErrItemNotFound) {
			response.RenderError(w, http.StatusNotFound, "NOT_FOUND", "Inventory item not found")
			return
		}
		if errors.Is(err, ErrInsufficientQuantity) {
			response.RenderError(w, http.StatusConflict, "INSUFFICIENT_QUANTITY", "Player does not possess enough quantity")
			return
		}
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusOK, map[string]interface{}{
		"item":    item,
		"message": "Item consumed successfully",
	})
}

func (h *Handler) GrantItem(w http.ResponseWriter, r *http.Request) {
	var req GrantRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		response.RenderError(w, http.StatusBadRequest, "BAD_REQUEST", "Invalid JSON payload")
		return
	}

	userID, err := uuid.Parse(req.UserID)
	if err != nil {
		response.RenderError(w, http.StatusBadRequest, "INVALID_USER_ID", "Malformed user identifier")
		return
	}

	if req.ItemDefinitionID == "" {
		response.RenderError(w, http.StatusBadRequest, "MISSING_FIELD", "item_definition_id is required")
		return
	}
	if req.Quantity <= 0 {
		response.RenderError(w, http.StatusBadRequest, "INVALID_QUANTITY", "quantity must be greater than zero")
		return
	}

	item, err := h.service.GrantItem(r.Context(), userID, req.ItemDefinitionID, req.Quantity)
	if err != nil {
		response.RenderError(w, http.StatusInternalServerError, "INTERNAL_ERROR", err.Error())
		return
	}

	response.RenderSuccess(w, http.StatusCreated, map[string]interface{}{
		"item":    item,
		"message": "Item granted successfully",
	})
}
