# WILDBOUND — COSMETIC DEFINITIONS & REGISTRY
## Phase 25 Specification: Metadata Schema and Registry APIs

---

## 1. Cosmetic Definition Schema

All cosmetic assets are described by a static, validated `CosmeticDefinition` record. Assets are referenced through stable identifiers rather than hardcoded disk paths, integrating with the Asset Management System (AMS).

### 1.1 C++ Data Structure
```cpp
struct CosmeticDefinition {
    std::string id;
    std::string displayName;
    std::string description;
    CustomizationSlot slot = CustomizationSlot::Chest;
    std::string category = "clothing";
    std::string spritesheetId;
    std::string textureId;
    std::string iconId;
    std::vector<std::string> compatibleDirections = { "up", "down", "left", "right" };
    std::vector<std::string> compatibleAnimations = { "idle", "walk", "run" };
    std::vector<std::string> occlusionFlags;
    Vector2 anchorOffset{ 0.0f, 0.0f };
    Color tint = WHITE;
    std::string rarity = "common";
    bool isGameplayEquipment = false;
    std::string equipmentItemDefId;

    bool IsValid() const noexcept;
    bool SupportsDirection(Direction dir) const;
    bool SupportsAnimation(const std::string& animName) const;
    entities::CosmeticItem ToCosmeticItem() const;
    nlohmann::json ToJson() const;
    static CosmeticDefinition FromJson(const nlohmann::json& j);
};
```

### 1.2 JSON Schema Example
```json
{
    "id": "clo_forest_tunic_001",
    "display_name": "Forest Tunic",
    "description": "Deep woodland green tunic with reinforced hem and brass buckle.",
    "slot": "Chest",
    "category": "clothing",
    "spritesheet": "sheet_clo_forest_tunic_001",
    "texture": "tex_clo_forest_tunic_001",
    "icon": "ui/icons/clo_forest_tunic_001.png",
    "compatible_directions": ["up", "down", "left", "right"],
    "compatible_animations": ["idle", "walk", "run"],
    "occlusion_flags": [],
    "anchor_offset": [0.0, 0.0],
    "tint": [255, 255, 255, 255],
    "rarity": "uncommon",
    "is_gameplay_equipment": false,
    "equipment_item_id": ""
}
```

---

## 2. CosmeticRegistry Operations

The `CosmeticRegistry` is a fast, in-memory index of all cosmetics:

```cpp
class CosmeticRegistry {
public:
    bool RegisterCosmetic(const CosmeticDefinition& def);
    bool UnregisterCosmetic(const std::string& id);
    void Clear();

    const CosmeticDefinition* GetCosmetic(const std::string& id) const;
    bool HasCosmetic(const std::string& id) const;
    std::vector<const CosmeticDefinition*> GetCosmeticsBySlot(CustomizationSlot slot) const;
    std::vector<const CosmeticDefinition*> GetAllCosmetics() const;
    size_t GetCount() const noexcept;

    bool LoadFromCatalog(const std::string& catalogPath, const resources::AssetRegistry* assetReg = nullptr);
    void RegisterBuiltinCosmetics(const resources::AssetRegistry* assetReg = nullptr);
    bool ValidateCosmetic(const std::string& id, const resources::AssetRegistry& assetReg) const;
};
```

### 2.1 Indexing & Performance
- Lookups by ID run in $O(1)$ via hash map.
- Slot queries are pre-indexed in `m_slotIndex` mapping each slot to a list of cosmetic IDs, avoiding linear searches during UI frame rendering.
- Caching ensures textures and spritesheets are never re-queried or re-loaded from disk during cosmetic preview updates.

