-- ============================================================================
-- WILDBOUND ENGINE CONFIGURATION
-- Phase 0: Foundation & Core Architecture
-- ============================================================================

local config = {
    game = {
        title = "Wildbound",
        version = "0.1.0-phase0",
        author = "Wildbound Development Team"
    },
    display = {
        window_width = 1280,
        window_height = 720,
        internal_width = 640,
        internal_height = 360,
        target_fps = 60,
        vsync = true,
        resizable = false,
        fullscreen = false
    },
    debug = {
        enabled = true,
        show_metrics_overlay = true,
        log_level = "DEBUG" -- Options: DEBUG, INFO, WARNING, ERROR
    }
}

return config

