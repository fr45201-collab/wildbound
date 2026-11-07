/**
 * Headless rlImGui stub implementation for Linux Dedicated Server and CI tests.
 */

#include "rlImGui.h"

#ifdef __cplusplus
extern "C" {
#endif

void rlImGuiSetup(bool darkTheme) {
    (void)darkTheme;
}

void rlImGuiBegin(void) {}
void rlImGuiEnd(void) {}
void rlImGuiShutdown(void) {}

void rlImGuiBeginInitImGui(void) {}
void rlImGuiEndInitImGui(void) {}
void rlImGuiBeginDelta(float deltaTime) {
    (void)deltaTime;
}

void rlImGuiImage(const Texture *image) {
    (void)image;
}

void rlImGuiImageSize(const Texture *image, int width, int height) {
    (void)image; (void)width; (void)height;
}

void rlImGuiImageSizeV(const Texture* image, Vector2 size) {
    (void)image; (void)size;
}

void rlImGuiImageRect(const Texture* image, int destWidth, int destHeight, Rectangle sourceRect) {
    (void)image; (void)destWidth; (void)destHeight; (void)sourceRect;
}

void rlImGuiImageRenderTexture(const RenderTexture* image) {
    (void)image;
}

void rlImGuiImageRenderTextureFit(const RenderTexture* image, bool center) {
    (void)image; (void)center;
}

bool rlImGuiImageButton(const char* name, const Texture* image) {
    (void)name; (void)image;
    return false;
}

bool rlImGuiImageButtonSize(const char* name, const Texture* image, Vector2 size) {
    (void)name; (void)image; (void)size;
    return false;
}

#ifdef __cplusplus
}
#endif

