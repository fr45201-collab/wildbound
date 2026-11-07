/**
 * Headless raylib stub implementation for Linux Dedicated Server and CI tests.
 * Decouples headless builds completely from X11, Wayland, OpenGL, and audio subsystems.
 */

#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Core Window & Context Stubs
void InitWindow(int width, int height, const char *title) {
    (void)width; (void)height; (void)title;
}

bool IsWindowReady(void) {
    return false;
}

bool IsWindowResized(void) {
    return false;
}

bool IsWindowFocused(void) {
    return false;
}

int GetFPS(void) {
    return 60;
}

void CloseWindow(void) {}

bool WindowShouldClose(void) {
    return false;
}

void SetWindowSize(int width, int height) {
    (void)width; (void)height;
}

void SetWindowTitle(const char *title) {
    (void)title;
}

int GetScreenWidth(void) {
    return 1920;
}

int GetScreenHeight(void) {
    return 1080;
}

void SetTargetFPS(int fps) {
    (void)fps;
}

float GetFrameTime(void) {
    return 1.0f / 60.0f;
}

double GetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void SetConfigFlags(unsigned int flags) {
    (void)flags;
}

void SetTraceLogLevel(int logLevel) {
    (void)logLevel;
}

// Drawing & Framebuffer Stubs (No-Op in Headless)
void BeginDrawing(void) {}
void EndDrawing(void) {}
void ClearBackground(Color color) { (void)color; }
void BeginMode2D(Camera2D camera) { (void)camera; }
void EndMode2D(void) {}

Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera) {
    float matScale = camera.zoom;
    Vector2 matTranslation = { -camera.target.x * camera.zoom + camera.offset.x, -camera.target.y * camera.zoom + camera.offset.y };
    return (Vector2){ position.x * matScale + matTranslation.x, position.y * matScale + matTranslation.y };
}

Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera) {
    if (camera.zoom == 0.0f) return position;
    return (Vector2){ (position.x - camera.offset.x) / camera.zoom + camera.target.x,
                      (position.y - camera.offset.y) / camera.zoom + camera.target.y };
}

void DrawRectangle(int posX, int posY, int width, int height, Color color) {
    (void)posX; (void)posY; (void)width; (void)height; (void)color;
}

void DrawRectangleRec(Rectangle rec, Color color) {
    (void)rec; (void)color;
}

void DrawRectangleLines(int posX, int posY, int width, int height, Color color) {
    (void)posX; (void)posY; (void)width; (void)height; (void)color;
}

void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color) {
    (void)rec; (void)lineThick; (void)color;
}

void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color) {
    (void)rec; (void)origin; (void)rotation; (void)color;
}

void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom) {
    (void)posX; (void)posY; (void)width; (void)height; (void)top; (void)bottom;
}

void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color) {
    (void)rec; (void)roundness; (void)segments; (void)color;
}

void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color) {
    (void)rec; (void)roundness; (void)segments; (void)color;
}

void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color) {
    (void)rec; (void)roundness; (void)segments; (void)lineThick; (void)color;
}

void DrawCircle(int centerX, int centerY, float radius, Color color) {
    (void)centerX; (void)centerY; (void)radius; (void)color;
}

void DrawCircleV(Vector2 center, float radius, Color color) {
    (void)center; (void)radius; (void)color;
}

void DrawCircleLines(int centerX, int centerY, float radius, Color color) {
    (void)centerX; (void)centerY; (void)radius; (void)color;
}

void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color) {
    (void)centerX; (void)centerY; (void)radiusH; (void)radiusV; (void)color;
}

void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color) {
    (void)startPosX; (void)startPosY; (void)endPosX; (void)endPosY; (void)color;
}

void DrawLineV(Vector2 startPos, Vector2 endPos, Color color) {
    (void)startPos; (void)endPos; (void)color;
}

void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color) {
    (void)startPos; (void)endPos; (void)thick; (void)color;
}

void DrawText(const char *text, int posX, int posY, int fontSize, Color color) {
    (void)text; (void)posX; (void)posY; (void)fontSize; (void)color;
}

int MeasureText(const char *text, int fontSize) {
    if (!text) return 0;
    return (int)strlen(text) * (fontSize / 2);
}

const char *TextFormat(const char *text, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);
    return buffer;
}

void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
    (void)v1; (void)v2; (void)v3; (void)color;
}

void DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
    (void)texture; (void)posX; (void)posY; (void)tint;
}

void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint) {
    (void)texture; (void)source; (void)position; (void)tint;
}

void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint) {
    (void)texture; (void)source; (void)dest; (void)origin; (void)rotation; (void)tint;
}

// Texture & Image Stubs
Texture2D LoadTexture(const char *fileName) {
    (void)fileName;
    return (Texture2D){ 0, 0, 0, 1, 0 };
}

Texture2D LoadTextureFromImage(Image image) {
    (void)image;
    return (Texture2D){ 0, 0, 0, 1, 0 };
}

void UnloadTexture(Texture2D texture) {
    (void)texture;
}

Image LoadImage(const char *fileName) {
    (void)fileName;
    return (Image){ NULL, 0, 0, 1, 0 };
}

Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2) {
    (void)width; (void)height; (void)checksX; (void)checksY; (void)col1; (void)col2;
    return (Image){ NULL, 0, 0, 1, 0 };
}

void UnloadImage(Image image) {
    (void)image;
}

RenderTexture2D LoadRenderTexture(int width, int height) {
    (void)width; (void)height;
    RenderTexture2D target;
    memset(&target, 0, sizeof(target));
    return target;
}

void UnloadRenderTexture(RenderTexture2D target) {
    (void)target;
}

void BeginTextureMode(RenderTexture2D target) {
    (void)target;
}

void EndTextureMode(void) {}

void SetTextureFilter(Texture2D texture, int filter) {
    (void)texture; (void)filter;
}

// Font Stubs
Font GetFontDefault(void) {
    Font font;
    memset(&font, 0, sizeof(font));
    return font;
}

Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount) {
    (void)fileName; (void)fontSize; (void)codepoints; (void)codepointCount;
    return GetFontDefault();
}

void UnloadFont(Font font) {
    (void)font;
}

// Audio Stubs
void InitAudioDevice(void) {}
void CloseAudioDevice(void) {}
bool IsAudioDeviceReady(void) { return false; }

Sound LoadSound(const char *fileName) {
    (void)fileName;
    Sound sound;
    memset(&sound, 0, sizeof(sound));
    return sound;
}

void UnloadSound(Sound sound) {
    (void)sound;
}

void PlaySound(Sound sound) {
    (void)sound;
}

void StopSound(Sound sound) {
    (void)sound;
}

void SetSoundVolume(Sound sound, float volume) {
    (void)sound; (void)volume;
}

void SetSoundPitch(Sound sound, float pitch) {
    (void)sound; (void)pitch;
}

void SetSoundPan(Sound sound, float pan) {
    (void)sound; (void)pan;
}

// Music Stubs
Music LoadMusicStream(const char *fileName) {
    (void)fileName;
    Music music;
    memset(&music, 0, sizeof(music));
    return music;
}

void UnloadMusicStream(Music music) {
    (void)music;
}

void PlayMusicStream(Music music) {
    (void)music;
}

bool IsMusicStreamPlaying(Music music) {
    (void)music;
    return false;
}

void UpdateMusicStream(Music music) {
    (void)music;
}

void StopMusicStream(Music music) {
    (void)music;
}

void PauseMusicStream(Music music) {
    (void)music;
}

void ResumeMusicStream(Music music) {
    (void)music;
}

void SetMusicVolume(Music music, float volume) {
    (void)music; (void)volume;
}

// Input Stubs
int GetKeyPressed(void) { return 0; }
bool IsKeyPressed(int key) { (void)key; return false; }
bool IsKeyDown(int key) { (void)key; return false; }
bool IsKeyReleased(int key) { (void)key; return false; }
bool IsMouseButtonPressed(int button) { (void)button; return false; }
bool IsMouseButtonDown(int button) { (void)button; return false; }
bool IsMouseButtonReleased(int button) { (void)button; return false; }
Vector2 GetMousePosition(void) { return (Vector2){ 0.0f, 0.0f }; }
float GetMouseWheelMove(void) { return 0.0f; }

bool IsGamepadAvailable(int gamepad) { (void)gamepad; return false; }
const char *GetGamepadName(int gamepad) { (void)gamepad; return "None"; }
bool IsGamepadButtonPressed(int gamepad, int button) { (void)gamepad; (void)button; return false; }
bool IsGamepadButtonDown(int gamepad, int button) { (void)gamepad; (void)button; return false; }
bool IsGamepadButtonReleased(int gamepad, int button) { (void)gamepad; (void)button; return false; }
float GetGamepadAxisMovement(int gamepad, int axis) { (void)gamepad; (void)axis; return 0.0f; }

// Color Helpers
Color ColorAlpha(Color color, float alpha) {
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    color.a = (unsigned char)(255.0f * alpha);
    return color;
}

Color Fade(Color color, float alpha) {
    return ColorAlpha(color, alpha);
}

// 2D Collision & Math Functions (authoritative calculations)
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2) {
    return ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
            (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y));
}

bool CheckCollisionPointRec(Vector2 point, Rectangle rec) {
    return ((point.x >= rec.x) && (point.x < (rec.x + rec.width)) &&
            (point.y >= rec.y) && (point.y < (rec.y + rec.height)));
}

bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2) {
    float dx = center2.x - center1.x;
    float dy = center2.y - center1.y;
    float distSq = dx * dx + dy * dy;
    float radSum = radius1 + radius2;
    return distSq <= (radSum * radSum);
}

bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
    float recCenterX = rec.x + rec.width / 2.0f;
    float recCenterY = rec.y + rec.height / 2.0f;
    float dx = (center.x > recCenterX) ? (center.x - recCenterX) : (recCenterX - center.x);
    float dy = (center.y > recCenterY) ? (center.y - recCenterY) : (recCenterY - center.y);

    if (dx > (rec.width / 2.0f + radius)) return false;
    if (dy > (rec.height / 2.0f + radius)) return false;

    if (dx <= (rec.width / 2.0f)) return true;
    if (dy <= (rec.height / 2.0f)) return true;

    float cornerDistanceSq = (dx - rec.width / 2.0f) * (dx - rec.width / 2.0f) +
                             (dy - rec.height / 2.0f) * (dy - rec.height / 2.0f);

    return cornerDistanceSq <= (radius * radius);
}

#ifdef __cplusplus
}
#endif

