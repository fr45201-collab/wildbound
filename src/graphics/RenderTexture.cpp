#include "RenderTexture.hpp"
#include "../core/Logger.hpp"
#include <utility>

using wildbound::core::Logger;

namespace wildbound::graphics {

RenderTexture::RenderTexture() = default;

RenderTexture::RenderTexture(int width, int height) {
    Load(width, height);
}

RenderTexture::~RenderTexture() {
    Unload();
}

RenderTexture::RenderTexture(RenderTexture&& other) noexcept
    : m_target(other.m_target)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_isBound(other.m_isBound)
{
    other.m_target = {};
    other.m_width = 0;
    other.m_height = 0;
    other.m_isBound = false;
}

RenderTexture& RenderTexture::operator=(RenderTexture&& other) noexcept {
    if (this != &other) {
        Unload();
        m_target = other.m_target;
        m_width = other.m_width;
        m_height = other.m_height;
        m_isBound = other.m_isBound;

        other.m_target = {};
        other.m_width = 0;
        other.m_height = 0;
        other.m_isBound = false;
    }
    return *this;
}

bool RenderTexture::Load(int width, int height) {
    if (width <= 0 || height <= 0) {
        Logger::Error("GRAPHICS", "Invalid RenderTexture dimensions: " +
                      std::to_string(width) + "x" + std::to_string(height));
        return false;
    }

    if (IsValid()) {
        if (m_width == width && m_height == height) {
            return true;
        }
        Unload();
    }

    m_target = LoadRenderTexture(width, height);
    if (m_target.id == 0) {
        Logger::Error("GRAPHICS", "Failed to allocate raylib RenderTexture2D (" +
                      std::to_string(width) + "x" + std::to_string(height) + ")");
        return false;
    }

    m_width = width;
    m_height = height;

    // Strict nearest-neighbor point filtering for pixel-art clarity
    SetTextureFilter(m_target.texture, TEXTURE_FILTER_POINT);
    return true;
}

void RenderTexture::Unload() {
    if (m_isBound) {
        End();
    }

    if (m_target.id != 0) {
        UnloadRenderTexture(m_target);
        m_target = {};
        m_width = 0;
        m_height = 0;
    }
}

void RenderTexture::Begin() {
    if (!IsValid()) {
        Logger::Error("GRAPHICS", "Attempted to Begin() unallocated RenderTexture.");
        return;
    }
    BeginTextureMode(m_target);
    m_isBound = true;
}

void RenderTexture::End() {
    if (m_isBound) {
        EndTextureMode();
        m_isBound = false;
    }
}

} // namespace wildbound::graphics

