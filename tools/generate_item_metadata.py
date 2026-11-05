#!/usr/bin/env python3
"""
Wildbound Phase 16 — Item Metadata & AMS Registration Tool
Generates:
  - Individual item JSON definitions in data/items/<category>/<id>.json
  - Master catalog in data/items/item_catalog.json
  - Updates assets/manifests/asset_manifest.json with all 80 item textures
"""

import os
import json

ITEMS_DATA = [
    # -------------------------------------------------------------
    # 1. Collectibles (8 items)
    # -------------------------------------------------------------
    {
        "id": "col_gold_coin_001",
        "display_name": "Gold Coin",
        "category": "collectible",
        "subcategory": "currency",
        "description": "A minted royal gold coin stamped with an imperial dragon emblem. The standard currency of the realm.",
        "icon_file": "items/icons/col_gold_coin_001_icon.png",
        "world_file": "items/world/col_gold_coin_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "col_silver_coin_001",
        "display_name": "Silver Coin",
        "category": "collectible",
        "subcategory": "currency",
        "description": "A polished silver coin stamped with a radiant star. Widely circulated among merchants and settlements.",
        "icon_file": "items/icons/col_silver_coin_001_icon.png",
        "world_file": "items/world/col_silver_coin_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "col_ruby_001",
        "display_name": "Ruby",
        "category": "collectible",
        "subcategory": "gemstone",
        "description": "A brilliant-cut scarlet gemstone of exceptional clarity. Radiates warmth and intense fiery glints.",
        "icon_file": "items/icons/col_ruby_001_icon.png",
        "world_file": "items/world/col_ruby_001_world.png",
        "biome_tags": ["mountain", "desert"],
        "rarity_tier": "rare"
    },
    {
        "id": "col_sapphire_001",
        "display_name": "Sapphire",
        "category": "collectible",
        "subcategory": "gemstone",
        "description": "A deep radiant-cut cobalt sapphire reflecting deep ocean tones. Highly prized by lapidaries and sorcerers.",
        "icon_file": "items/icons/col_sapphire_001_icon.png",
        "world_file": "items/world/col_sapphire_001_world.png",
        "biome_tags": ["snow", "mountain"],
        "rarity_tier": "rare"
    },
    {
        "id": "col_emerald_001",
        "display_name": "Emerald",
        "category": "collectible",
        "subcategory": "gemstone",
        "description": "A flawless step-cut emerald of lush forest-green hue. Coveted across the civilized lands.",
        "icon_file": "items/icons/col_emerald_001_icon.png",
        "world_file": "items/world/col_emerald_001_world.png",
        "biome_tags": ["forest", "swamp"],
        "rarity_tier": "rare"
    },
    {
        "id": "col_ancient_crystal_001",
        "display_name": "Ancient Crystal",
        "category": "collectible",
        "subcategory": "crystal",
        "description": "A cluster of prismatic quartz shards shimmering with alternating violet and cyan phosphorescence.",
        "icon_file": "items/icons/col_ancient_crystal_001_icon.png",
        "world_file": "items/world/col_ancient_crystal_001_world.png",
        "biome_tags": ["mountain", "snow", "desert"],
        "rarity_tier": "epic"
    },
    {
        "id": "col_ancient_relic_001",
        "display_name": "Ancient Relic",
        "category": "collectible",
        "subcategory": "relic",
        "description": "An ornate bronze ceremonial chalice engraved with forgotten pre-collapse heraldry and set with a central ruby.",
        "icon_file": "items/icons/col_ancient_relic_001_icon.png",
        "world_file": "items/world/col_ancient_relic_001_world.png",
        "biome_tags": ["forest", "desert", "swamp"],
        "rarity_tier": "epic"
    },
    {
        "id": "col_golden_emblem_001",
        "display_name": "Golden Emblem",
        "category": "collectible",
        "subcategory": "emblem",
        "description": "A solid gold heraldic insignia depicting an eight-pointed sun. Awarded to high-ranking knights and champions.",
        "icon_file": "items/icons/col_golden_emblem_001_icon.png",
        "world_file": "items/world/col_golden_emblem_001_world.png",
        "biome_tags": ["forest", "desert", "mountain"],
        "rarity_tier": "legendary"
    },

    # -------------------------------------------------------------
    # 2. Consumables (10 items)
    # -------------------------------------------------------------
    {
        "id": "con_health_potion_small_001",
        "display_name": "Small Health Potion",
        "category": "consumable",
        "subcategory": "potion",
        "description": "A spherical glass flask stoppered with cork, containing a modest dose of restorative crimson elixir.",
        "icon_file": "items/icons/con_health_potion_small_001_icon.png",
        "world_file": "items/world/con_health_potion_small_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "con_health_potion_large_001",
        "display_name": "Large Health Potion",
        "category": "consumable",
        "subcategory": "potion",
        "description": "A heavy faceted glass decanter reinforced with silver filigree bands, filled with dense, glowing healing tonic.",
        "icon_file": "items/icons/con_health_potion_large_001_icon.png",
        "world_file": "items/world/con_health_potion_large_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "rare"
    },
    {
        "id": "con_mana_potion_001",
        "display_name": "Mana Potion",
        "category": "consumable",
        "subcategory": "potion",
        "description": "A slender alchemical phial glowing with luminous cobalt fluid that recharges mystical reserves.",
        "icon_file": "items/icons/con_mana_potion_001_icon.png",
        "world_file": "items/world/con_mana_potion_001_world.png",
        "biome_tags": ["forest", "snow", "mountain"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "con_stamina_potion_001",
        "display_name": "Stamina Potion",
        "category": "consumable",
        "subcategory": "potion",
        "description": "A triangular flask bubbling with effervescent emerald tonic that revitalizes tired muscles and lungs.",
        "icon_file": "items/icons/con_stamina_potion_001_icon.png",
        "world_file": "items/world/con_stamina_potion_001_world.png",
        "biome_tags": ["forest", "desert", "mountain"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "con_antidote_001",
        "display_name": "Antidote",
        "category": "consumable",
        "subcategory": "potion",
        "description": "A square apothecary vial of teal herbal tincture capable of neutralizing toxins and swamp venoms.",
        "icon_file": "items/icons/con_antidote_001_icon.png",
        "world_file": "items/world/con_antidote_001_world.png",
        "biome_tags": ["swamp", "forest"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "con_healing_herb_001",
        "display_name": "Healing Herb",
        "category": "consumable",
        "subcategory": "herb",
        "description": "A bundle of fragrant silverleaf sprigs tied with natural hemp cord, known for its soothing poultice properties.",
        "icon_file": "items/icons/con_healing_herb_001_icon.png",
        "world_file": "items/world/con_healing_herb_001_world.png",
        "biome_tags": ["forest", "swamp"],
        "rarity_tier": "common"
    },
    {
        "id": "con_bread_001",
        "display_name": "Bread",
        "category": "consumable",
        "subcategory": "food",
        "description": "A crusty, freshly baked round loaf of hearth-fired sourdough scored with bakery hash marks.",
        "icon_file": "items/icons/con_bread_001_icon.png",
        "world_file": "items/world/con_bread_001_world.png",
        "biome_tags": ["forest"],
        "rarity_tier": "common"
    },
    {
        "id": "con_apple_001",
        "display_name": "Apple",
        "category": "consumable",
        "subcategory": "food",
        "description": "A sweet, crisp red orchard apple with a fresh stem and leaf, picked from highland orchards.",
        "icon_file": "items/icons/con_apple_001_icon.png",
        "world_file": "items/world/con_apple_001_world.png",
        "biome_tags": ["forest", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "con_cooked_meat_001",
        "display_name": "Cooked Meat",
        "category": "consumable",
        "subcategory": "food",
        "description": "A savory fire-roasted shank of game meat cooked over open hardwood coals, seasoned with wild herbs.",
        "icon_file": "items/icons/con_cooked_meat_001_icon.png",
        "world_file": "items/world/con_cooked_meat_001_world.png",
        "biome_tags": ["forest", "snow", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "con_water_flask_001",
        "display_name": "Water Flask",
        "category": "consumable",
        "subcategory": "drink",
        "description": "A curved leather and brass canteen filled with cold, refreshing glacial spring water.",
        "icon_file": "items/icons/con_water_flask_001_icon.png",
        "world_file": "items/world/con_water_flask_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "common"
    },

    # -------------------------------------------------------------
    # 3. Crafting Materials (8 items)
    # -------------------------------------------------------------
    {
        "id": "mat_wood_001",
        "display_name": "Wood",
        "category": "material",
        "subcategory": "raw_material",
        "description": "A neatly stacked bundle of rough-hewn oak timber logs suitable for construction and carpentry.",
        "icon_file": "items/icons/mat_wood_001_icon.png",
        "world_file": "items/world/mat_wood_001_world.png",
        "biome_tags": ["forest", "swamp"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_stone_001",
        "display_name": "Stone",
        "category": "material",
        "subcategory": "raw_material",
        "description": "A square-chiseled block of hard granite quarry stone ready for masonry or fortifications.",
        "icon_file": "items/icons/mat_stone_001_icon.png",
        "world_file": "items/world/mat_stone_001_world.png",
        "biome_tags": ["mountain", "desert"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_iron_ore_001",
        "display_name": "Iron Ore",
        "category": "material",
        "subcategory": "raw_material",
        "description": "Dense, heavy rocky ore studded with visible metallic veins of smeltable raw iron.",
        "icon_file": "items/icons/mat_iron_ore_001_icon.png",
        "world_file": "items/world/mat_iron_ore_001_world.png",
        "biome_tags": ["mountain", "snow"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_gold_ore_001",
        "display_name": "Gold Ore",
        "category": "material",
        "subcategory": "raw_material",
        "description": "A glittering rock chunk interveined with veins and specks of raw yellow gold.",
        "icon_file": "items/icons/mat_gold_ore_001_icon.png",
        "world_file": "items/world/mat_gold_ore_001_world.png",
        "biome_tags": ["mountain", "desert"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "mat_leather_001",
        "display_name": "Leather",
        "category": "material",
        "subcategory": "processed_material",
        "description": "A rolled and tied pelt of tanned beast hide treated with tanning oils for armor and grip bindings.",
        "icon_file": "items/icons/mat_leather_001_icon.png",
        "world_file": "items/world/mat_leather_001_world.png",
        "biome_tags": ["forest", "mountain", "snow"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_cloth_001",
        "display_name": "Cloth",
        "category": "material",
        "subcategory": "processed_material",
        "description": "A tightly woven bolt of bleached flax linen cloth used for tailored clothing, sails, and bandages.",
        "icon_file": "items/icons/mat_cloth_001_icon.png",
        "world_file": "items/world/mat_cloth_001_world.png",
        "biome_tags": ["forest"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_iron_ingot_001",
        "display_name": "Iron Ingot",
        "category": "material",
        "subcategory": "processed_material",
        "description": "A refined, smelted bar of forged iron with beveled edges, ready for the blacksmith's anvil.",
        "icon_file": "items/icons/mat_iron_ingot_001_icon.png",
        "world_file": "items/world/mat_iron_ingot_001_world.png",
        "biome_tags": ["forest", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "mat_gold_ingot_001",
        "display_name": "Gold Ingot",
        "category": "material",
        "subcategory": "processed_material",
        "description": "A heavy bar of pure refined gold bullion stamped with official mint marks.",
        "icon_file": "items/icons/mat_gold_ingot_001_icon.png",
        "world_file": "items/world/mat_gold_ingot_001_world.png",
        "biome_tags": ["forest", "desert"],
        "rarity_tier": "rare"
    },

    # -------------------------------------------------------------
    # 4. Natural Resources (7 items)
    # -------------------------------------------------------------
    {
        "id": "res_red_mushroom_001",
        "display_name": "Red Mushroom",
        "category": "resource",
        "subcategory": "fungus",
        "description": "A wild forest toadstool with a vibrant crimson cap speckled with white spores.",
        "icon_file": "items/icons/res_red_mushroom_001_icon.png",
        "world_file": "items/world/res_red_mushroom_001_world.png",
        "biome_tags": ["forest", "swamp"],
        "rarity_tier": "common"
    },
    {
        "id": "res_blue_mushroom_001",
        "display_name": "Blue Mushroom",
        "category": "resource",
        "subcategory": "fungus",
        "description": "A bioluminescent cavern fungus radiating a cool azure glow from its lower gills.",
        "icon_file": "items/icons/res_blue_mushroom_001_icon.png",
        "world_file": "items/world/res_blue_mushroom_001_world.png",
        "biome_tags": ["swamp", "mountain"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "res_healing_flower_001",
        "display_name": "Healing Flower",
        "category": "resource",
        "subcategory": "flora",
        "description": "A delicate wild meadow blossom with pure white petals and a golden pollen core.",
        "icon_file": "items/icons/res_healing_flower_001_icon.png",
        "world_file": "items/world/res_healing_flower_001_world.png",
        "biome_tags": ["forest"],
        "rarity_tier": "common"
    },
    {
        "id": "res_forest_herb_001",
        "display_name": "Forest Herb",
        "category": "resource",
        "subcategory": "flora",
        "description": "A bundle of fresh woodland herbs with serrated leaves, carrying a sharp peppermint fragrance.",
        "icon_file": "items/icons/res_forest_herb_001_icon.png",
        "world_file": "items/world/res_forest_herb_001_world.png",
        "biome_tags": ["forest"],
        "rarity_tier": "common"
    },
    {
        "id": "res_wild_berries_001",
        "display_name": "Wild Berries",
        "category": "resource",
        "subcategory": "flora_berry",
        "description": "A cluster of sweet, juicy dark purple berries gathered from bramble thickets along forest trails.",
        "icon_file": "items/icons/res_wild_berries_001_icon.png",
        "world_file": "items/world/res_wild_berries_001_world.png",
        "biome_tags": ["forest", "mountain"],
        "rarity_tier": "common"
    },
    {
        "id": "res_desert_cactus_fruit_001",
        "display_name": "Desert Cactus Fruit",
        "category": "resource",
        "subcategory": "flora_desert",
        "description": "A deep magenta prickly pear harvested from arid desert saguaros, rich in natural moisture.",
        "icon_file": "items/icons/res_desert_cactus_fruit_001_icon.png",
        "world_file": "items/world/res_desert_cactus_fruit_001_world.png",
        "biome_tags": ["desert"],
        "rarity_tier": "uncommon"
    },
    {
        "id": "res_frozen_herb_001",
        "display_name": "Frozen Herb",
        "category": "resource",
        "subcategory": "flora_arctic",
        "description": "A frost-rimed arctic alpine herb capable of surviving subzero blizzards under thick snow blankets.",
        "icon_file": "items/icons/res_frozen_herb_001_icon.png",
        "world_file": "items/world/res_frozen_herb_001_world.png",
        "biome_tags": ["snow"],
        "rarity_tier": "uncommon"
    },

    # -------------------------------------------------------------
    # 5. Special Items (7 items)
    # -------------------------------------------------------------
    {
        "id": "spc_ancient_key_001",
        "display_name": "Ancient Key",
        "category": "special",
        "subcategory": "key",
        "description": "A heavy golden key with an intricate labyrinthine bit, crafted to unlock forgotten sanctum gates.",
        "icon_file": "items/icons/spc_ancient_key_001_icon.png",
        "world_file": "items/world/spc_ancient_key_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "epic"
    },
    {
        "id": "spc_rusty_key_001",
        "display_name": "Rusty Key",
        "category": "special",
        "subcategory": "key",
        "description": "A corroded iron skeleton key that feels brittle to the touch. May still turn in an old padlock.",
        "icon_file": "items/icons/spc_rusty_key_001_icon.png",
        "world_file": "items/world/spc_rusty_key_001_world.png",
        "biome_tags": ["forest", "swamp"],
        "rarity_tier": "common"
    },
    {
        "id": "spc_ancient_scroll_001",
        "display_name": "Ancient Scroll",
        "category": "special",
        "subcategory": "document",
        "description": "A rolled parchment of weathered vellum tied with a crimson ribbon and stamped with a wax seal.",
        "icon_file": "items/icons/spc_ancient_scroll_001_icon.png",
        "world_file": "items/world/spc_ancient_scroll_001_world.png",
        "biome_tags": ["forest", "desert", "mountain"],
        "rarity_tier": "rare"
    },
    {
        "id": "spc_treasure_map_001",
        "display_name": "Treasure Map",
        "category": "special",
        "subcategory": "document",
        "description": "A tattered nautical map showing terrain contours, a compass rose, and a faded red X marking hidden cache coordinates.",
        "icon_file": "items/icons/spc_treasure_map_001_icon.png",
        "world_file": "items/world/spc_treasure_map_001_world.png",
        "biome_tags": ["forest", "desert", "swamp", "mountain"],
        "rarity_tier": "rare"
    },
    {
        "id": "spc_rune_stone_001",
        "display_name": "Rune Stone",
        "category": "special",
        "subcategory": "arcane_stone",
        "description": "A smooth dark river stone carved with an arcane sigil that hums and pulses with cyan elemental energy.",
        "icon_file": "items/icons/spc_rune_stone_001_icon.png",
        "world_file": "items/world/spc_rune_stone_001_world.png",
        "biome_tags": ["forest", "snow", "mountain"],
        "rarity_tier": "epic"
    },
    {
        "id": "spc_ancient_amulet_001",
        "display_name": "Ancient Amulet",
        "category": "special",
        "subcategory": "jewelry",
        "description": "A heavy golden talisman set with an azure sapphire eye, suspended from an interlocking chain.",
        "icon_file": "items/icons/spc_ancient_amulet_001_icon.png",
        "world_file": "items/world/spc_ancient_amulet_001_world.png",
        "biome_tags": ["forest", "desert", "mountain"],
        "rarity_tier": "epic"
    },
    {
        "id": "spc_mysterious_artifact_001",
        "display_name": "Mysterious Artifact",
        "category": "special",
        "subcategory": "artifact",
        "description": "A bizarre geometric dodecahedron of dark obsidian metal, its interlocking plates glowing with violet cosmic luminescence.",
        "icon_file": "items/icons/spc_mysterious_artifact_001_icon.png",
        "world_file": "items/world/spc_mysterious_artifact_001_world.png",
        "biome_tags": ["forest", "desert", "snow", "swamp", "mountain"],
        "rarity_tier": "legendary"
    }
]

def main():
    print("Generating Item Metadata and Master Catalog...")
    for cat in ["collectibles", "consumables", "materials", "resources", "special"]:
        os.makedirs(f"data/items/{cat}", exist_ok=True)

    # 1. Output Individual JSON files
    for item in ITEMS_DATA:
        cat_dir = item["category"]
        if cat_dir == "collectible": cat_dir = "collectibles"
        elif cat_dir == "consumable": cat_dir = "consumables"
        elif cat_dir == "material": cat_dir = "materials"
        elif cat_dir == "resource": cat_dir = "resources"
        elif cat_dir == "special": cat_dir = "special"

        out_path = f"data/items/{cat_dir}/{item['id']}.json"
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(item, f, indent=2)
        print(f"  + Wrote {out_path}")

    # 2. Master Catalog
    catalog = {
        "version": "1.0",
        "item_count": len(ITEMS_DATA),
        "categories": ["collectible", "consumable", "material", "resource", "special"],
        "items": ITEMS_DATA
    }
    with open("data/items/item_catalog.json", "w", encoding="utf-8") as f:
        json.dump(catalog, f, indent=2)
    print("  + Wrote data/items/item_catalog.json")

    # 3. Register in assets/manifests/asset_manifest.json
    manifest_path = "assets/manifests/asset_manifest.json"
    with open(manifest_path, "r", encoding="utf-8") as f:
        manifest = json.load(f)

    existing_ids = {a["id"] for a in manifest["assets"]}
    new_assets = []

    for item in ITEMS_DATA:
        iid = item["id"]

        # Icon texture
        icon_id = f"tex_{iid}_icon"
        if icon_id not in existing_ids:
            new_assets.append({
                "id": icon_id,
                "file": item["icon_file"],
                "type": "texture",
                "category": "items/icons",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 64,
                "height": 64,
                "status": "Approved"
            })
            existing_ids.add(icon_id)

        # World texture
        world_id = f"tex_{iid}_world"
        if world_id not in existing_ids:
            new_assets.append({
                "id": world_id,
                "file": item["world_file"],
                "type": "texture",
                "category": "items/world",
                "source": "Wildbound Development Studio",
                "author": "Wildbound Art Team",
                "license": "CC0 1.0 Universal",
                "commercialUse": True,
                "attributionRequired": False,
                "width": 32,
                "height": 32,
                "status": "Approved"
            })
            existing_ids.add(world_id)

    manifest["assets"].extend(new_assets)
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2)

    print(f"AMS Registration: Added {len(new_assets)} new item textures. Total registered: {len(manifest['assets'])}")

if __name__ == "__main__":
    main()

