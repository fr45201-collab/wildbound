# Storm Configuration Reference

## 1. File Location and JSON Schema

The primary configuration resides in:
```
data/storm/storm_config.json
```

```json
{
  "name": "Wildbound Battle Royale Standard Storm",
  "version": "1.0.0",
  "worldSize": 10240.0,
  "initialRadius": 4000.0,
  "initialCenterX": 5120.0,
  "initialCenterY": 5120.0,
  "phases": [
    {
      "phaseIndex": 1,
      "name": "Phase 1 - Gathering Storm",
      "warningDurationSeconds": 60.0,
      "shrinkDurationSeconds": 90.0,
      "targetRadius": 3000.0,
      "damagePerSecond": 1.0,
      "maxCenterDriftRatio": 0.40
    },
    ...
  ]
}
```

---

## 2. Field Definitions

| Field | Type | Description |
|---|---|---|
| `name` | string | Descriptive name of the storm preset. |
| `version` | string | Version tracking identifier. |
| `worldSize` | float | Total dimension of the world tilemap square (e.g. 10240.0 for $320 \times 320$ at 32px). |
| `initialRadius` | float | Starting radius of the safe zone before any shrinkage begins. |
| `initialCenterX` / `Y` | float | Starting anchor center coordinate. |
| `phases` | array | Sequential list of storm phases executed in order. |
| `phaseIndex` | int | 1-based index of the phase. |
| `name` | string | User-facing display title used on HUD notification banners. |
| `warningDurationSeconds` | float | Waiting period duration before contraction commences. |
| `shrinkDurationSeconds` | float | Contraction duration over which radius and center interpolate. |
| `targetRadius` | float | Radius reached at the conclusion of the shrink period. |
| `damagePerSecond` | float | HP damage per second inflicted on entities outside the safe perimeter. |
| `maxCenterDriftRatio` | float | Maximum percentage of $(R_{\text{current}} - R_{\text{next}})$ that the next center may migrate away from the current center. |

---

## 3. Hot Reloading Support

`StormManager::LoadConfig(const std::string& filepath)` can be called at runtime or triggered through DevUI to immediately hot-reload updated timings, damage numbers, or phase profiles without restarting the application.

