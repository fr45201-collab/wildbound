# Customization Debugging Guide

## Overview
This guide provides developers and testers with tools, console flags, and troubleshooting steps for the Character Customization System.

---

## 1. Dear ImGui Customization Debugger (`DevUI`)

The developer overlay contains a dedicated **Character Customization** panel accessible during development builds:
- **Shortcut**: `F1` or `~` to toggle developer tools.
- **Location**: `DevUI::RenderWardrobe()` / `DevUI::RenderCustomizationDebug()`.

### Debugger Features
1. **Active Preview Status**: Displays whether a preview session is currently active, isolated draft state, and dirtiness flag.
2. **Direction Controls**: Radio buttons to immediately force South, North, East, or West rendering for sprite alignment verification.
3. **Animation Mode**: Quick toggle between Idle and Walk cycles.
4. **Active Layer Inspection**: Real-time table displaying all 9 slots, assigned cosmetic IDs, asset sheet keys, and RGB tint vectors.
5. **Loadout Inspector**: Displays registered preset loadouts, with buttons to test serialization, load, and purge operations.

---

## 2. Common Issues and Resolutions

### A. Hair Visible Through Full Helmets
- **Symptom**: Hair strands clipping through closed full plate helmet visors.
- **Cause**: The cosmetic definition for the helmet has `occludesHair: false` or is classified under `CustomizationSlot::Headwear` instead of `CustomizationSlot::Helmet`.
- **Fix**: In the cosmetic definition JSON, set `"occludesHair": true` and ensure `"slot": "helmet"`.

### B. Cosmetic Changes Not Saved After Exiting Wardrobe
- **Symptom**: Customizations revert upon game restart.
- **Cause**: User clicked Cancel (`ESC`) instead of Confirm (`Enter` / green Apply button), or filesystem permissions blocked writing to `user_data/character/`.
- **Fix**: Check `user_data/character/appearance_loadouts.json` write permissions and inspect logs for `[APPEARANCE_SERIALIZER]` warnings.

### C. Missing Texture Log Warnings
- **Symptom**: `[TEXTURE_MGR] Texture file not found for asset 'sheet_xyz'`.
- **Cause**: The asset key specified in `CosmeticDefinition::assetId` has not been loaded by `AssetManager` or is missing from `assets/manifest.json`.
- **Fix**: Verify asset file existence in `assets/` and check spelling against `CosmeticRegistry::LoadFromCatalog()`.

---

## 3. Running Customization Automated Tests

Run the dedicated test suite directly:
```powershell
.\build\tests\Release\test_customization.exe
```
Or run via CTest:
```powershell
ctest --test-dir build -R test_customization -V
```
All 7 test domains must pass:
1. `CosmeticDefinitionSerialization`
2. `CosmeticRegistryRegistration`
3. `CharacterAppearanceDefaults`
4. `AppearancePreviewIsolation`
5. `AppearanceLoadoutSerialization`
6. `CosmeticStatDecoupling`
7. `InputContextWardrobe`

