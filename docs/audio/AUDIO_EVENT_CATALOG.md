# Audio Event Catalog

## 1. Overview

Wildbound decouples gameplay logic from physical asset file paths through stable, normalized string **Audio Event IDs**. Gameplay systems trigger audio strictly via `audioSys->DispatchEvent(eventId)` or `audioSys->DispatchEventSpatial(eventId, worldPos)`.

---

## 2. Complete Event Registry

### 2.1 Player Footstep Events
| Event ID | Variations Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `player.footstep.grass` | `sfx_footstep_grass_001..003` | `0.15s` | `SFX` | Footstep over grassy terrain |
| `player.footstep.dirt`  | `sfx_footstep_dirt_001..003`  | `0.15s` | `SFX` | Footstep over packed dirt / trail |
| `player.footstep.stone` | `sfx_footstep_stone_001..003` | `0.15s` | `SFX` | Footstep over stone / rock / cobble |
| `player.footstep.snow`  | `sfx_footstep_snow_001..003`  | `0.15s` | `SFX` | Footstep over deep snow / frost |
| `player.footstep.sand`  | `sfx_footstep_sand_001..003`  | `0.15s` | `SFX` | Footstep over desert sand dunes |
| `player.footstep.mud`   | `sfx_footstep_mud_001..003`   | `0.15s` | `SFX` | Footstep over wet swamp mud |
| `player.footstep.wood`  | `sfx_footstep_wood_001..003`  | `0.15s` | `SFX` | Footstep over wooden bridge / floors |
| `player.footstep.water` | `sfx_footstep_water_001..003` | `0.15s` | `SFX` | Footstep through shallow water stream |

### 2.2 Weapons & Combat Events
| Event ID | Variations Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `weapon.sword.swing` | `sfx_sword_swing_001`, `002` | `0.12s` | `SFX` | Melee sword light/heavy swing whoosh |
| `weapon.sword.impact` | Metal, wood, stone clashes | `0.08s` | `SFX` | Sword colliding with solid obstacles |
| `weapon.sword.sheathe` | `sfx_sword_sheathe` | `0.20s` | `SFX` | Drawing or sheathing sword |
| `weapon.bow.draw` | `sfx_bow_draw` | `0.25s` | `SFX` | Bowstring tension draw |
| `weapon.bow.release` | `sfx_bow_release` | `0.15s` | `SFX` | Bowstring twang arrow release |
| `weapon.bow.equip` | `sfx_bow_equip` | `0.20s` | `SFX` | Bow equipment handling rustle |
| `weapon.arrow.flight` | `sfx_arrow_flight` | `0.10s` | `SFX` | Arrow whistle through air |
| `weapon.arrow.impact` | `sfx_arrow_impact` | `0.08s` | `SFX` | General arrow thud impact |
| `weapon.arrow.impact_flesh` | `sfx_arrow_impact_flesh` | `0.08s` | `SFX` | Arrow penetrating target entity |
| `weapon.arrow.impact_wood` | `sfx_arrow_impact_wood` | `0.08s` | `SFX` | Arrow embedding into timber |
| `weapon.arrow.impact_stone` | `sfx_arrow_impact_stone` | `0.08s` | `SFX` | Arrow ricochet off stone masonry |
| `weapon.shield.block` | `sfx_shield_block` | `0.15s` | `SFX` | Defensive shield block impact |
| `weapon.parry` | `sfx_parry_clink` | `0.15s` | `SFX` | High-pitched metallic parry clink |

### 2.3 Combat Damage & Health Events
| Event ID | Variations Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `combat.player.hurt` | `sfx_player_hurt` | `0.25s` | `SFX` | Player taking damage |
| `combat.player.death` | `sfx_player_death` | `0.50s` | `SFX` | Player incapacitation / death |
| `combat.enemy.hurt` | `sfx_enemy_hurt` | `0.20s` | `Combat` | Generic monster hurt grunt |
| `combat.enemy.death` | `sfx_enemy_death` | `0.30s` | `Combat` | Monster collapse / death |
| `combat.impact` | `sfx_impact_flesh_001`, `002` | `0.10s` | `Combat` | Melee strike flesh impact |
| `combat.critical_hit` | `sfx_combat_critical_hit` | `0.25s` | `Combat` | High-damage critical strike stinger |

### 2.4 Creature Vocalizations & Actions
| Event ID | Sound Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `creature.goblin.idle` | `sfx_creature_goblin_idle` | `2.00s` | `Creatures` | Throaty goblin chuckle |
| `creature.goblin.step` | `sfx_creature_goblin_step` | `0.20s` | `Creatures` | Scuffling goblin footstep |
| `creature.goblin.attack` | `sfx_creature_goblin_attack` | `0.30s` | `Creatures` | Club strike swing |
| `creature.goblin.hurt` | `sfx_creature_goblin_hurt` | `0.25s` | `Creatures` | Squealing goblin yelp |
| `creature.goblin.death` | `sfx_creature_goblin_death` | `0.50s` | `Creatures` | Shrieking defeat fall |
| `creature.skeleton.step` | `sfx_creature_skeleton_step` | `0.20s` | `Creatures` | Bone rattling step |
| `creature.skeleton.attack` | `sfx_creature_skeleton_attack` | `0.30s` | `Creatures` | Bone thrust whoosh |
| `creature.skeleton.hurt` | `sfx_creature_skeleton_hurt` | `0.25s` | `Creatures` | Bone fracture crack |
| `creature.skeleton.death` | `sfx_creature_skeleton_death` | `0.50s` | `Creatures` | Skeletal collapse into bone pile |
| `creature.wolf.idle` | `sfx_creature_wolf_idle` | `2.00s` | `Creatures` | Low throaty snarl |
| `creature.wolf.growl` | `sfx_creature_wolf_growl` | `1.50s` | `Creatures` | Threatening aggressive growl |
| `creature.wolf.attack` | `sfx_creature_wolf_attack` | `0.30s` | `Creatures` | Rapid jaw snap bite |
| `creature.wolf.hurt` | `sfx_creature_wolf_hurt` | `0.25s` | `Creatures` | Sharp pain yelp |
| `creature.wolf.death` | `sfx_creature_wolf_death` | `0.50s` | `Creatures` | Mournful whimper |
| `creature.wild_boar.idle` | `sfx_creature_boar_idle` | `2.00s` | `Creatures` | Deep snorting grunt |
| `creature.wild_boar.step` | `sfx_creature_boar_step` | `0.20s` | `Creatures` | Heavy hoof stamp |
| `creature.wild_boar.attack` | `sfx_creature_boar_attack` | `0.30s` | `Creatures` | Tusk charge impact |
| `creature.wild_boar.hurt` | `sfx_creature_boar_hurt` | `0.25s` | `Creatures` | Piercing squeal |
| `creature.wild_boar.death` | `sfx_creature_boar_death` | `0.50s` | `Creatures` | Heavy collapse squeal |
| `creature.bat.flight` | `sfx_creature_bat_flight` | `0.30s` | `Creatures` | Leathery wing flutter |
| `creature.bat.attack` | `sfx_creature_bat_attack` | `0.30s` | `Creatures` | Ultrasonic dive shriek |
| `creature.bat.hurt` | `sfx_creature_bat_hurt` | `0.25s` | `Creatures` | High screech |
| `creature.bat.death` | `sfx_creature_bat_death` | `0.50s` | `Creatures` | Soft impact fall |
| `wildlife.bird.takeoff` | `sfx_wildlife_bird_takeoff` | `1.00s` | `Environment` | Bird wing flutter takeoff |
| `wildlife.bird.landing` | `sfx_wildlife_bird_landing` | `1.00s` | `Environment` | Bird gentle branch landing |
| `wildlife.butterfly.flutter` | `sfx_wildlife_butterfly_flutter` | `1.50s` | `Environment` | Subtle butterfly wing murmur |

### 2.5 User Interface Events
| Event ID | Sound Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `ui.button.hover` | `sfx_ui_hover` | `0.04s` | `UI` | Button focus highlight |
| `ui.button.click` | `sfx_ui_click` | `0.05s` | `UI` | Button selection click |
| `ui.button.confirm` | `sfx_ui_confirm` | `0.08s` | `UI` | Positive action confirmation |
| `ui.button.cancel` | `sfx_ui_cancel` | `0.08s` | `UI` | Negative action / back cancel |
| `ui.tab.switch` | `sfx_ui_tab` | `0.06s` | `UI` | Category or menu tab toggle |
| `ui.panel.open` | `sfx_ui_panel_open` | `0.10s` | `UI` | Modal window opening slide |
| `ui.panel.close` | `sfx_ui_panel_close` | `0.10s` | `UI` | Modal window dismissing |
| `ui.item.select` | `sfx_ui_item_select` | `0.05s` | `UI` | Grid inventory slot selection |
| `ui.cosmetic.apply` | `sfx_ui_cosmetic_apply` | `0.10s` | `UI` | Wardrobe cosmetic equipped |
| `ui.cosmetic.remove` | `sfx_ui_cosmetic_remove` | `0.10s` | `UI` | Wardrobe cosmetic unequipped |
| `ui.settings.adjust` | `sfx_ui_settings_adjust` | `0.03s` | `UI` | Slider drag tick |
| `ui.error` | `sfx_ui_error` | `0.15s` | `UI` | Action rejected / error alert |
| `ui.success` | `sfx_ui_success` | `0.15s` | `UI` | Action accomplished / unlock |
| `ui.back` | `sfx_ui_back` | `0.08s` | `UI` | Previous menu navigation |
| `ui.modal.confirm` | `sfx_ui_modal_confirm` | `0.10s` | `UI` | Critical confirmation dialog |

### 2.6 Items & World Interaction Events
| Event ID | Sound Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `item.pickup` | `sfx_item_pickup` | `0.05s` | `SFX` | Collecting ground loot item |
| `item.drop` | `sfx_item_drop` | `0.05s` | `SFX` | Dropping item from inventory |
| `item.equip` | `sfx_sword_sheathe` | `0.10s` | `SFX` | Equipping weapon/armor |
| `item.coin` | `sfx_coin_pickup` | `0.05s` | `SFX` | Collecting gold/silver coin |
| `item.gem` | `sfx_gem_pickup` | `0.05s` | `SFX` | Collecting precious gemstone |
| `item.consume` | `sfx_potion_drink` | `0.10s` | `SFX` | Drinking potion / food item |
| `environment.door.open` | `sfx_door_open` | `0.20s` | `SFX` | Opening building door |
| `environment.door.close` | `sfx_door_close` | `0.20s` | `SFX` | Closing building door |
| `environment.chest.open` | `sfx_chest_open` | `0.20s` | `SFX` | Opening treasure chest lid |

### 2.7 Match & Tournament Events
| Event ID | Sound Mapped | Cooldown | Audio Bus | Description |
|---|---|---|---|---|
| `match.start` | `sfx_match_start` | `0.50s` | `SFX` | Match battle horn |
| `match.victory` | `sfx_stinger_victory` | `1.00s` | `SFX` | Victorious match completion fanfare |
| `match.defeat` | `sfx_stinger_defeat` | `1.00s` | `SFX` | Player elimination gloom stinger |
| `match.countdown` | `sfx_match_countdown` | `0.10s` | `SFX` | Storm/match timer countdown pip |
| `match.alert` | `sfx_gameplay_alert` | `0.20s` | `SFX` | High-urgency zone warning |

