# Wildbound — External Asset Licensing & Provenance Register

## 1. Provenance Register Policy

Every external asset integrated into Wildbound must have a verified legal trail documenting:
- Commercial gameplay usage rights.
- Right to modify, slice, and adapt artwork.
- Proper credit and attribution to the original artist.
- Checksums ensuring data integrity.

---

## 2. Machine-Readable Schema (`assets/manifests/asset_licenses.json`)

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "WildboundAssetLicenses",
  "type": "object",
  "required": ["version", "records"],
  "properties": {
    "version": { "type": "string" },
    "records": {
      "type": "array",
      "items": {
        "type": "object",
        "required": [
          "asset_id",
          "asset_name",
          "creator",
          "source_library",
          "source_url",
          "license_name",
          "license_url",
          "commercial_use",
          "modification_permitted",
          "attribution_required",
          "approval_status"
        ],
        "properties": {
          "asset_id": { "type": "string" },
          "asset_name": { "type": "string" },
          "creator": { "type": "string" },
          "source_library": { "type": "string" },
          "source_url": { "type": "string" },
          "download_url": { "type": "string" },
          "license_name": { "type": "string" },
          "license_url": { "type": "string" },
          "download_date": { "type": "string" },
          "commercial_use": { "type": "boolean" },
          "modification_permitted": { "type": "boolean" },
          "attribution_required": { "type": "boolean" },
          "attribution_text": { "type": "string" },
          "redistribution_restrictions": { "type": "string" },
          "local_raw_path": { "type": "string" },
          "local_processed_path": { "type": "string" },
          "approval_status": { 
            "type": "string",
            "enum": [
              "DISCOVERED",
              "LICENSE_VERIFIED",
              "APPROVED",
              "DOWNLOADED",
              "VALIDATED",
              "PROCESSED",
              "INTEGRATED",
              "REJECTED",
              "LICENSE_REVIEW_REQUIRED"
            ]
          }
        }
      }
    }
  }
}
```

---

## 3. Preliminary Provenance Log

| Asset ID | Candidate Asset Name | Creator | Source | License | Commercial Permitted? | Attribution Text Required | Status |
|---|---|---|---|---|---|---|---|
| `pilot_ts_desert_kenney` | Tiny Town / Roguelike Desert Tiles | Kenney | Kenney.nl / GitHub | CC0 1.0 Universal | **YES** | None (Credit: Kenney) | **LICENSE_VERIFIED** |
| `pilot_creature_slime_sparklin` | Medieval Fantasy Slime / Creature | Sparklin Labs | Superpowers Asset Packs | CC0 1.0 Universal | **YES** | None (Sparklin Labs) | **LICENSE_VERIFIED** |
| `pilot_ui_icons_gameicons` | Fantasy Action & Inventory Icons | Lorc, Delapouite | Game-icons.net | CC BY 3.0 | **YES** | "Icons by Lorc & Delapouite under CC BY 3.0" | **LICENSE_VERIFIED** |

*Note: In Gate 1, no assets are downloaded or integrated yet. These candidates represent our verified pilot recommendations.*

