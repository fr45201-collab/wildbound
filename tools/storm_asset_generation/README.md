# Wildbound — Storm Asset Generation Pipeline

This toolset procedurally generates all visual and auditory assets required for the **Corruption Storm & Safe Zone System** (Phase 31).

## Scripts:
* `generate_storm_textures.py`: Generates seamless atmospheric storm noise, corruption clouds, and energy textures.
* `generate_particle_textures.py`: Generates 5 distinct particle sprites (corruption motes, energy sparks, debris).
* `generate_boundary_effects.py`: Generates circular boundary rings, energy shimmer bands, and perimeter glows.
* `generate_storm_icons.py`: Generates 6 pixel-art HUD warning icons (approaching, shrinking, outside, return, final, safe).
* `generate_storm_audio.py`: Procedurally synthesizes WAV sound effects and ambient rumble loops.
* `generate_storm_assets.py`: Master orchestrator running all generators and registering assets in `assets/manifests/asset_manifest.json`.
* `validate_storm_assets.py`: Validates PNG integrity, dimensions, non-blank alphas, and contact sheet generation.

