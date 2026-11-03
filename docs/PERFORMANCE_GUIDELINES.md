# Wildbound Performance Guidelines

## 1. Zero Per-Frame Allocations
- **Render Textures**: Allocated strictly during engine initialization or explicit window resizing. Never allocate textures during the render loop.
- **Buffers & Containers**: Reserve vector capacities upfront. Reuse persistent collections across frames.
- **Strings & Logging**: Avoid string allocations in hot update and render loops. High-frequency logging is restricted to `LogLevel::Debug`.

## 2. Timestep & Simulation Stability
- **Fixed Timestep**: 60 Hz (`fixedDeltaTime = 1.0f / 60.0f`). Prevents variable physics instability and floating-point divergence.
- **Spiral-of-Death Protection**: Frame delta times are clamped to `0.25f` maximum to prevent infinite catch-up loops during heavy debugger pauses or OS lag spikes.
- **Target Frame Rate**: Configurable via `engine.lua` (default 60 FPS) with V-Sync support enabled.

## 3. Render State Efficiency
- **Batching**: Group world draw calls within a single `BeginWorld` / `EndWorld` camera pass.
- **Render Target Swaps**: Minimize render texture binds to exactly one per frame (Virtual Canvas -> Window).
- **Pixel-Art Snapping**: Use integer rounding for camera presentation to eliminate floating-point sub-pixel rasterization penalties.

