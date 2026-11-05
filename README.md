# Wildbound

> **Wildbound** is a 2D top-down fantasy battle royale and survival adventure game built with C++20, raylib, Lua 5.4, and sol2.

## Current Status: Phase 24 — Weapons & Equipment System (580 AMS Assets, 26 Test Suites)

### Key Features
- **Authoritative Weapon Architecture (`wildbound::weapons::WeaponManager`, `WeaponController`, `WeaponInstance`)**: Comprehensive weapon simulation system supporting 11 distinct fantasy weapons (6 swords, 5 bows) and 1 dedicated ammunition type (iron arrow), with canonical statistics, runtime instances, and JSON data overrides (`assets/definitions/weapons.json`).
- **6-Tier Rarity Damage & Crit Scaling**: Multi-tier weapon scaling across `Common` (1.00x), `Uncommon` (1.05x), `Rare` (1.10x), `Epic` (1.15x), `Legendary` (1.20x), and `Mythic` (1.25x), dynamically applied to effective damage calculations and crit probabilities with dedicated UI rarity palettes.
- **Deterministic Attack Lifecycle State Machine (`WeaponController`)**: Four-phase discrete attack progression (`Prep` / `Windup` $\to$ `Active` $\to$ `Recovery` $\to$ `Cooldown`) driving weapon readiness, cadence pacing, and animation transitions.
- **Multi-Target Cleave with Single-Hit Deduplication**: Swiping melee arcs detect and strike all combatants in range while strictly enforcing a single damage tick per entity per swing via `m_targetsHitThisSwing` tracking.
- **Dynamic Bow Draw & Ammunition Mechanics**: Bows feature dynamic charge tension scaling from 0.0 to 1.0, requiring holding the attack input. Damage and projectile launch velocity dynamically scale based on charge ratio. Firing consumes physical ammunition stacks (`arrow_iron_001`) from player inventory; dry-firing without arrows is cleanly prevented.
- **8-Slot Modular Equipment System (`wildbound::entities::EquipmentSystem`)**: Full humanoid equipment management covering `MainHand`, `OffHand`, `Head`, `Chest`, `Legs`, `Feet`, `Face`, and `Back`. Supports equipping both Primary and Secondary weapons with instant hot-switching (`SwitchWeapon`, `ToggleWeapon`) and state-guarded validation.
- **Aggregate Stat Modifiers & Diminishing Returns Mitigation**: Equipping gear aggregates total defense ratings, applies diminishing returns mitigation ($\frac{\text{defense}}{\text{defense} + 100}$), grants health bonuses, and applies tactical movement speed modifications (-5% to +10%).
- **Player-Facing raylib HUD & Inventory Integration (`UIManager`)**: Bottom-left Active Weapon & Ammo HUD widget displaying weapon name, rarity color badge, damage readout, arrow counter or weapon slot badge, and real-time cooldown drain bar. Action buttons in the inventory interface allow instantaneous weapon equipping and swapping.
- **Developer Tooling & Weapon Inspector (`DevUI` Tab 10)**: Real-time telemetry inspector for weapon catalog stats, 6-tier rarity scaling matrix, live 8-slot player equipment, active weapon slot toggling, attack state monitor, and quick equipment spawner.
- **Authoritative 24-Slot Inventory Architecture (`wildbound::items::Inventory`)**: Deterministic fixed 24-slot inventory managing empty states (`std::nullopt`), filled/free slot counts, and capacity enforcement (`IsFull()`, `CanFitItem()`, `CalculateAvailableSpace()`).
- **Two-Pass Auto-Stacking & Overflow Safety**: Incoming items execute a deterministic two-pass algorithm (Pass 1: merge into existing non-full stacks up to `maxStackSize`; Pass 2: allocate into earliest free slots). Overflow remainder is returned cleanly without state mutation.
- **Stack Splitting & Merging (`SplitStack`, `MergeStacks`)**: Precise stack management allowing players to split stacks into arbitrary counts or merge matching item slots up to maximum stack capacity.
- **Two-Phase Transactional Item Dropping (`DropItem`, `DropAll`)**: Atomic two-phase commit: in-world `WorldItem` is spawned in front of the player first; only upon successful spawn pointer verification is inventory quantity deducted. Guarantees zero item loss or duplication.
- **Ground Loot Physical Representation (`wildbound::world::WorldItem`)**: In-world item entity featuring a 32x32 pixel representation, continuous vertical sinusoidal hovering (`offsetY = sin(t * 3.5) * 3.0`), grounded contact drop shadow, and 36.0f pickup radius.
- **Dual Pickup Policy Integration**: Unified pickup framework supporting manual interaction pickup (`[E] Collect ItemName`) via Phase 22 `InteractionManager`, or automatic proximity pickup via `World::ProcessPickups`.
- **Authoritative Consumable & Healing Framework (`wildbound::items::ConsumableSystem`)**: Data-driven recovery item usage with explicit `ItemUseResult` result codes (`Success`, `FullHealth`, `FullStamina`, `NotConsumable`, `TargetDead`). Prevents over-healing by disallowing health potion consumption when at maximum HP.
- **Combat Ammunition System & Bow Firing Enforcement**: Stackable iron arrow definition (`arrow_iron_001`, category `ItemCategory::Ammunition`, max stack 30). Enforces ammunition availability in `Player::TryAttack` when wielding a bow; fires fail and warn if out of arrows, and consume exactly 1 arrow upon successful shot.
- **Player-Facing Inventory UI (`UIManager::RenderInventory`)**: Native pixel-art interface rendered on the 640x360 virtual canvas with header capacity readout (`Capacity: X / 24`), 7 filter tabs (`All`, `Coins/Gems`, `Potions/Food`, `Materials`, `Resources`, `Weapons/Gear`, `Ammo`), 24-slot grid with stack count badges, detail card with 48x48 icon preview, and action buttons (`USE`, `EQUIP`, `SPLIT`, `DROP 1`, `DROP ALL`, `CLOSE`).
- **Modal Input Suppression**: Opening the inventory triggers `InputContextType::Inventory` via `InputManager`, suppressing player movement, attacks, and sprints while keeping UI shortcuts active.
- **Transactional Loot Containers (`LootContainer::LootAll`)**: Chests and caches support transactional `LootAll` transfers into player inventory without item loss or duplication.
- **DevUI Diagnostics & Loot Spawner (Tab 16)**: Developer suite tab providing a real-time 24-slot inventory inspection table, test loot spawners (potions, arrows, coins, weapons), and active ground loot registry table.
- **Centralized World Interaction Architecture (`InteractionManager`, `InteractableComponent`)**: Robust, data-driven interaction framework unifying doors, building entrances, loot containers, ground collectibles, mechanisms (levers, switches, pressure plates), and environmental objects into a consistent C++20 architecture.
- **Deterministic Proximity Discovery & Directional Facing Bias**: Euclidean range filtering paired with facing unit-vector dot product ($\vec{d} \cdot \vec{f} > 0.15$), priority tiers, distance sorting, and stable numeric `InteractableId` tie-breakers ensuring deterministic, jitter-free candidate selection in dense spaces.
- **Precondition Validation Pipeline (`InteractionValidator`, `InteractionResult`)**: Comprehensive pre-flight checks (proximity radius, cooldown timers, player health status, concurrency execution locks, and required inventory keys) delivering rich feedback before mutation.
- **Door Subsystem & Solid Collider Synchronization (`Door`)**: Three-state state machine (`Closed`, `Open`, `Locked`) featuring inventory key item unlocking (`HasItem`) and dynamic physical collision synchronization with `CollisionManager` (`SetColliderEnabled`), instantaneously opening passageways to players and entities without reallocating colliders.
- **Transactional Loot Containers & Ground Collectibles (`LootContainer`, `WorldItem`)**: Container opening with atomic `LootAll` inventory transfer, configurable pickup policies (`Manual [E]` vs `Automatic Proximity`), and strict two-phase transactional safety guaranteeing items are never destroyed or lost if inventory capacity is exhausted.
- **World Mechanisms Subsystem (`Mechanism`)**: Modular actuators supporting Levers, Switches, Pressure Plates, and Gate Turnstiles with manual vs automatic trigger modes, repeatable vs one-time locks, internal cooldowns, and customizable state callbacks (`MechanismStateCallback`).
- **Player-Facing Contextual Prompt UI**: Native raylib prompt banner integrated into `UIManager::RenderGameplayHUD` positioned cleanly above the hotbar, displaying dynamic action verbs, target names, localized hardware key glyphs (`[E]` on keyboard, `[X / Square]` on gamepad), and 2-second alert banners on validation failure.
- **Context-Aware Modal Input Integration**: Deep integration with Phase 21 `InputManager` and `InputAction::Interact` with hierarchical context suppression that silences gameplay interactions during Menu, Inventory, Wardrobe, or Pause modes.
- **Decoupled Observer Event Notification Bus (`IInteractionObserver`, `InteractionEvent`)**: Broadcasts lifecycle transitions (`TargetChanged`, `DoorOpened`, `ContainerOpened`, `ItemCollected`, `MechanismActivated`) to audio, quest, and telemetry systems.
- **DevUI Diagnostics & In-World Spatial Visualizer (Tab 15)**: Live interaction telemetry HUD, interactive proximity candidates table, door/mechanism debug controls, and in-world overlay rendering interaction radii circles, player facing vectors, and target bounding boxes.
- **Centralized Input Management Architecture (`InputManager`, `InputAction`)**: Robust, hardware-abstracted input subsystem fully decoupling physical hardware polling (Keyboard, Mouse, Gamepad via raylib) from gameplay simulation. Categorizes 33 logical actions across 7 discrete functional domains (`Movement`, `Combat`, `Inventory`, `Character`, `General`, `UI`, `DevUI`).
- **Coordinate Transformation Authority**: Centralized pipeline converting raw OS screen mouse coordinates into letterbox/pillarbox-adjusted $640 \times 360$ Virtual Canvas coordinates (`graphics::Viewport::ScreenToVirtual`) and world-space coordinates (`graphics::Camera2DSystem::ScreenToWorld`) with automatic window-focus-loss recovery.
- **Continuous Radial Deadzone Gamepad Pipeline (`GamepadInput`)**: Hardware-agnostic controller integration featuring continuous radial deadzones on dual analog sticks ($\frac{\text{mag} - d}{1 - d}$) that eliminates axial snapping and preserves full $360^\circ$ analog range, paired with thresholded analog trigger normalization.
- **Combat Input Buffering (`InputBuffer`)**: High-responsiveness input queuing with configurable per-action time-to-live windows (120 ms attack, 150 ms dodge, 150 ms interact) and strict single-consumption guarantees, eliminating dropped inputs across animation recovery and stun frames.
- **Hierarchical Modal Context Stack (`InputContextManager`)**: Priority-ranked context stack (`Gameplay` [10], `Menu` [40], `Inventory` [50], `Wardrobe` [60], `Settings` [80], `Paused` [100]) enforcing modal exclusivity to completely prevent cross-screen input bleed.
- **Configurable Rebinding & JSON Persistence (`InputSettings`)**: Interactive remapping state machine supporting primary and alternate bindings, modifier keys (`Shift`, `Ctrl`, `Alt`), conflict detection, and disk persistence (`config/input_bindings.json`).
- **DevUI Input Diagnostics (Tab 14)**: Live telemetry HUD displaying active device states, cursor coordinates across screen/virtual/world spaces, stick vector visualization, deadzone sliders, and an interactive action rebinding table.
- **Authoritative Physics & Collision Architecture (`CollisionManager`, `Collider`)**: High-performance, modular 2D collision subsystem decoupled from external physics dependencies. Features 10 configurable logical bitfield layers, bilateral consent filtering, AABB geometric bounds, local position offsets, and solid vs trigger classification.
- **Bilateral Consent Mask Filtering (`CollisionFilter`)**: Mutual consent invariant $(A.\text{mask} \ \& \ B.\text{layer} \ne 0) \ \land \ (B.\text{mask} \ \& \ A.\text{layer} \ne 0)$ with automatic self-collision exclusion, eliminating asymmetric collision bugs and unnecessary geometric tests.
- **Stateful Multi-Entity Trigger Volumes (`TriggerVolume`)**: Dedicated trigger volumes tracking entity sets across continuous frames, orchestrating authoritative `Enter`, `Stay`, and `Exit` lifecycle transitions.
- **Deferred Mutation Queue & Re-entrancy Safety**: Prevents iterator invalidation or memory corruption when callbacks dynamically spawn projectiles, destroy items, or modify colliders during event dispatch.
- **Deterministic 13-Stage Simulation Pipeline**: Canonical pair normalization $(\min(id_A, id_B), \max(id_A, id_B))$ and lexical event sorting guarantee 100% identical simulation results across platforms.
- **In-World Debug Visualizer & DevUI Inspector (Tab 13)**: Color-coded raylib collider rendering with active overlap highlighting (hotkey `F3`), accompanied by real-time simulation metrics, interactive bilateral filter testing, and an active overlaps table in DevUI Tab 13 (`F1`).
- **Zero-Allocation Steady-State Performance**: Contiguous storage and pre-allocated capacity reservations delivering ~0.048 ms tick execution times for 120 dynamic colliders (<0.3% of 16.66ms frame budget in Release).
- **Authoritative Item & Inventory Architecture (`Inventory`, `ItemInstance`)**: Decoupled immutable data definitions (`ItemDefinition`) from mutable runtime state (`ItemInstance` with unique runtime ID, quantity, and durability). Features a fixed 24-slot container with atomic auto-stacking, empty slot allocation, split stack, slot swapping, and overflow protection ensuring complete transactional integrity.
- **World Ground Pickups & Loot Containers (`WorldItem`, `LootContainer`)**: Interactive ground pickups rendered with grounded elliptical contact drop shadows, sinusoidal vertical hovering, and stack quantity badges. Interactive environmental chests supporting lock states, key item requirements (`CanOpen`), transactional `LootAll`, and procedural audio dispatch (`"Interact.ChestOpen"`).
- **Physical Ranged Projectile System (`ProjectileSystem`)**: Authoritative high-speed ballistic arrow simulation (480 px/s velocity) supporting 4-way cardinal firing, directional sprite rotation, lifetime expiration, spatial collision sweep against combatant hurtboxes, base weapon damage calculation, and audio event dispatch (`"Combat.BowHit"`).
- **Integrated Dual-Discipline Combat**: Cohesive melee and ranged weapon capabilities in `Player::TryAttack`. Swords trigger dynamic directional AABB hitboxes and slash audio (`"Combat.SwordWhoosh"`), while bows consume attack cooldowns and spawn physical arrows (`"Combat.BowRelease"`).
- **Centralized Armor Defense & Diminishing Returns Mitigation (`EquipmentSystem`)**: Dedicated equipment slots (Helmet, Chest Armor, Pants, Boots, Offhand/Shield). Aggregates total defense rating and calculates damage reduction via the standard diminishing returns formula $\frac{\text{defense}}{\text{defense} + 100}$. Seamlessly injects mitigation into `DamageSystem::CalculateDamage` and synchronizes equipment visuals with the player's modular paperdoll layers in lockstep.
- **Consumable & Resource Restoration**: Configurable health and stamina item effects (Health Potions, Stamina Potions, Bread, Apples, Cooked Meat, Healing Herbs) with transactional inventory deduction, combatant health clamp validation, and audio feedback (`"Interact.PotionDrink"`).
- **Player-Facing UI Gameplay Integration (`UIManager`)**:
  - *Dynamic Gameplay HUD*: Real-time health meter ($HP / maxHP$), stamina meter ($SP / maxSP$), current biome badge, defense armor rating readout, and 4-slot hotbar with numerical indicators `[1]`-`[4]`.
  - *Interactive Inventory Window*: Grid display of the player's 24 inventory slots, category filter tabs (All, Weapons, Armor, Consumables, Materials, Special), item inspection detail card, and functional "USE", "EQUIP", and "CLOSE" buttons with zero input bleed.
  - *Hotbar Quick Access*: Instant slot selection and item usage via keyboard keys `1`, `2`, `3`, `4`.
- **Integrated Audio Event Routing**: 4-bus procedural sound event dispatching (`Master`, `Music`, `SFX`, `UI`) tied to ground pickups (`"Interact.ItemPickup"`), chest opening (`"Interact.ChestOpen"`), sword swings, bow releases, arrow impacts, potion drinking, and button clicks.
- **Complete Game UI Design System (`UIManager`)**: Unified retro-fantasy pixel-art design system operating natively within the $640 \times 360$ virtual canvas (scaled $2\times$ point-filtered to $1280 \times 720$). Decoupled completely from developer tools (`DevUI`). Features zero input bleed when navigating modal screens, preventing character movement or weapon attacks.
- **Full-Canvas Backgrounds (10 Assets)**: Bespoke $640 \times 360$ px pixel-art backgrounds for Main Menu, Character Select, Wardrobe, Inventory, Settings, Loading Screen, Lobby, Match Results, Pause Menu, and Game Over.
- **Reusable Panel & Button Component Collection**: 5 scalable nine-slice frames (`panel_main_64`, `panel_dark_64`, `panel_tooltip_32`, `panel_card_48`, `panel_hud_bar`), alongside a multi-state button system featuring 6 distinct interactive states (Normal, Hover, Pressed, Disabled, Selected, Focused) across Primary, Destructive, Confirm, and Tab styles.
- **25 Dedicated Pixel-Art UI Icons ($24 \times 24$ px)**: Visual glyphs covering combat vitals (Health, Stamina), equipment (Armor, Weapons, Items), navigation (Map, Nav Compass), configuration (Settings, Audio, Graphics, Controls), interface actions (Exit, Confirm, Pause, Search, Notifications, Error), and inventory categories (Inventory, Character, Cosmetics, Collectibles, Consumables, Currency, Backpacks, Clothing).
- **Eight Integrated Player-Facing Screens**:
  - *Main Menu*: Title banner, dynamic background, and seamless navigation to all submenus.
  - *Character Selection*: Pedestal character presentation with live modular appearance preview.
  - *Modular Wardrobe*: Category tabs across all 8 Phase 17 cosmetic types with instant paperdoll visual updating.
  - *Inventory & Collectibles*: Filter tabs for all 40 Phase 16 items, $6 \times 4$ slot inventory grid, and inspection detail card.
  - *Audio & Settings*: Interactive progress-bar sliders for all 6 audio buses, mute-all toggle, and disk persistence.
  - *Pause Menu*: Frosted dark modal overlay with quick resume and settings access.
  - *Gameplay HUD*: Top-left Health ($90\%$) and Stamina ($75\%$) meters, top-right active biome indicator, and bottom-center 4-slot hotbar.
  - *Match Results & Game Over*: Stat readouts (eliminations, damage dealt, survival time, placement rank) and rematch triggers.
- **Centralized Procedural Audio Architecture (`AudioSystem`)**: High-performance C++20 audio management engine managing 6 logical audio buses (`Master`, `Music`, `UI`, `SFX`, `Environment`, `Voice`) with independent attenuation, global muting, and JSON settings persistence (`data/config/audio_settings.json`).
- **68 Original Procedural Audio Assets (100% Royalty-Free)**: Pure mathematical waveform synthesis in Python standard library (sine, triangle, saw, square, noise, ADSR envelopes, resonant low-pass filters, stereo delay) in 44.1 kHz 16-bit PCM WAV:
  - *10 Original Music Tracks*: Main Menu, Character Select, Wardrobe, Inventory, Lobby, Results, Defeat, Forest, Desert, Snow.
  - *7 Environmental Ambience Loops*: Forest Birds, Forest Wind, Desert Wind, Blizzard Howl, Swamp Insects, Mountain Draft, Campfire Crackle.
  - *15 UI Interface Sounds*: Button click, hover, confirm, cancel, tab switch, window open/close, item select, cosmetic equip/unequip, slider tick, error, success, back, modal confirm.
  - *14 Player & Movement Sounds*: Dual footsteps across 6 surface terrains (grass, dirt, stone, snow, sand, mud), player hurt, and death.
  - *9 Weapon & Combat Sounds*: Fast sword whoosh, heavy cleave, flesh strike, metal parry clash, shield block, bow draw, arrow release, arrow whistle, arrow impact.
  - *8 Interaction Sounds*: Item pickup/drop, gold coin clink, gem chime, potion drink, door open/close, chest open.
  - *5 Match Event Stingers*: 3-2-1 countdown, match start horn, zone alert siren, victory fanfare, defeat lament.
- **Interactive Audio Suite (DevUI Tab 12)**: Real-time developer overlay providing bus volume sliders, mute toggle, procedural music player with play/pause/stop and track selector, biome ambience preview switcher, and an interactive SFX soundboard.
- **Expanded Asset Management System (AMS)**: 68 new procedural audio assets registered in `asset_manifest.json`, elevating total registered engine assets from 512 to **580**.
- **Comprehensive Costumes & Armor Collection (50 Designs)**: Complete pixel-art production library featuring 8 Clothing designs, 8 Armor designs, 6 Pants designs, 5 Footwear designs, 6 Headwear designs, 5 Face Accessories, 5 Backpacks, and 7 Character Variations.
- **Independent Modular Layering Architecture (`ModularCharacter`)**: Real-time multi-layer compositing engine extended with dedicated `CosmeticCategory::Armor` (`Z-Index = 52`) and `CosmeticCategory::Face` (`Z-Index = 62`), enabling players to equip civilian clothing and metallic chest armor simultaneously with zero clipping or slot conflicts.
- **Data-Driven Occlusion & Directional Depth Sorting**: Automatic hair suppression (`hair_hide_all`) under closed iron and steel helmets, eyewear suppression (`mask_hide_goggles`) under porcelain mystic masks, and directional Z-ordering (backpacks render behind torso when facing South, East, West, but flip to foreground `Z = 85` when facing North).
- **Expanded Asset Management System (AMS)**: 100 new cosmetic textures and spritesheet descriptors registered in `asset_manifest.json`, elevating total registered engine assets from 412 to **512**.
- **Interactive Wardrobe & Customization Suite (DevUI Tab 6)**: Real-time developer tooling supporting 8 representative test preset builds (Default, Explorer, Armored, Mystic, Forest Guardian, Desert Nomad, Frost Knight, Runebound), discrete slot dropdowns for all 50 Phase 17 cosmetics, 4-way facing direction toggles, and live layer stack table inspection.
- **Comprehensive Item & Collectible Asset Collection (40 Items)**: Complete pixel-art production library featuring 8 Collectibles (Gold/Silver Coins, Ruby, Sapphire, Emerald, Ancient Crystal, Sunken Relic, Royal Emblem), 10 Consumables (Health, Mana, Stamina Potions, Antidote, Herbs, Bread, Apple, Cooked Meat, Water Flask), 8 Crafting Materials (Timber Log, Granite Stone, Iron/Gold Ores, Leather, Cloth, Iron/Gold Ingots), 7 Natural Resources (Red/Blue Mushrooms, Sunblossom Flower, Forest Herb, Wild Berries, Prickly Pear Cactus, Frostbloom Sprig), and 7 Special Items (Celestial & Rusty Keys, Ancient Scroll, Treasure Map, Rune Stone, Sun Amulet, Chrono Artifact).
- **Dual Representation per Item (80 Production Textures)**:
  - *Inventory Icons (64×64 px)*: Circular translucent display badge framing (`#1A1E29CC`), specular highlights, dark steel border, and centered silhouette.
  - *World Representations (32×32 px)*: Top-down angled perspective (30° tilt) resting on an elliptical contact drop-shadow for grounded terrain placement across all 5 biomes.
- **C++ Item Architecture (`ItemManager`)**: High-performance subsystem managing item metadata, category and biome queries, and automated AMS asset resolution validation.
- **Item Preview Suite (DevUI Tab 11)**: Real-time in-engine developer suite allowing interactive category filtering, instant search, item selection, dual icon/world preview, metadata inspection, and live AMS validation indicators.
- **Expanded Asset Management System (AMS)**: 80 new item textures registered in `asset_manifest.json`, elevating total registered engine assets to **412**.
- **Comprehensive Weapon Asset Collection (11 Weapons + Arrow)**: Complete pixel-art production library featuring 6 distinct swords (Wooden Training, Rusty Short, Iron Arming, Steel Longsword, Curved Saber, Ancient Runeblade), 5 distinct bows (Wooden Hunting, Yew Longbow, Reinforced Composite, Equestrian Recurve, Ancient Runebow), and 1 broadhead arrow projectile.
- **Four Dedicated Representations per Weapon**:
  - *Inventory Icons (64×64 px)*: High-readability diagonal weapon presentation inside translucent circular UI frames.
  - *Ground Representations (32×32 px)*: Flat top-down world perspective with contact drop shadows for natural drop placement across any terrain.
  - *Player-Held Sprites (128×32 px, 4-Way)*: Cardinal directions (Down, Up, Left, Right) aligned to player hand sockets with defined grip anchor points.
  - *Action Animation Pose Strips (128×32 px, 4 Keyframes)*: Dedicated 4-stage action keyframes (Idle, Prep/Draw, Swing/FullDraw, Recovery/Release).
- **Directional Z-Ordering & Occlusion Matrix**: Standardized depth-sorting ensuring weapons render in front of player torso when facing South, Left, or Right (Z = 85), and correctly behind the torso/shoulders when facing North (Z = 18).
- **C++ Weapon Architecture (`WeaponManager`)**: Robust subsystem managing weapon metadata, category querying, grip sockets, and automated AMS asset resolution validation.
- **Weapon Preview Suite (DevUI Tab 10)**: Real-time in-engine developer suite allowing interactive inspection of weapon designs, 4-way direction toggle, grip crosshairs, action pose scrubber, and automated AMS validation indicators.
- **Expanded Asset Management System (AMS)**: 72 new weapon textures and spritesheet descriptors registered in `asset_manifest.json`, elevating total registered engine assets to **332**.
- **Multi-Biome World Architecture (`BiomeManager`)**: Dynamic biome management system supporting 5 full biomes: Verdant Forest, Sunscorched Wastes (Desert), Frostfall Taiga (Snow), Murkwood Mire (Swamp), and Iron Crag Highlands (Mountain). Coordinates map loading, player spawn resolution, camera viewport boundary clamping, and environmental animation/atmospheric reconfiguration.
- **Dedicated Biome Tilesets (AMS Registered)**: 4 new bespoke $256 \times 256$ px pixel-art tilesets (64 tiles each) adhering strictly to Phase 6 Art Bible palettes: `desert_tileset.png`, `snow_tileset.png`, `swamp_tileset.png`, and `mountain_tileset.png`. Total registered AMS assets: **260 assets**.
- **Four Biome Prototype Maps ($60 \times 45$ Tiles, $1920 \times 1440$ Px)**: Fully realized, playable representative maps for Desert (`desert_01.json`), Snow (`snow_01.json`), Swamp (`swamp_01.json`), and Mountain (`mountain_01.json`) featuring bespoke terrain transitions, environmental objects, and spatial collision indexing.
- **Atmospheric & Weather Customization**: Tailored weather parameters, fog densities (from 0.05 in Desert to 0.65 in Swamp), cloud drift velocities (6.0 to 24.0 units/sec), and ambient RGBA lighting tints synchronized seamlessly with `EnvironmentalAnimationSystem`.
- **Biome Selector & World Explorer (DevUI Tab 3)**: Interactive developer suite controls enabling instant hot-swapping between all 5 biomes, live metadata inspection, and instant player/camera repositioning.
- **Complete Greenwood Village Settlement (13 Buildings)**: Organic medieval-inspired settlement in Subregion 3 (Greenwood Hamlet & Commons) featuring 4 dedicated zones: Central Market Square, Commercial & Crafting District, Civic & Manor District, and Residential Outskirts.
- **Dedicated Modular Interior Tileset (`interior_tileset.png`)**: 64 high-resolution 32x32 pixel-art tiles ($256 \times 256$ px) supporting polished wood floors, flagstone slate, heavy timber and stone masonry walls, carpets, counters, potion racks, fireplaces, and household furniture. Registered in AMS asset manifest.
- **6 Enterable Buildings with Bespoke Functional Interiors**: Fully playable interiors for General Store ($20 \times 15$ tiles), Blacksmith Workshop ($20 \times 15$), Town Hall Manor ($24 \times 18$), Forester's Cottage ($18 \times 14$), Two-Story Residence ($22 \times 16$), and Village Granary & Warehouse ($20 \times 16$).
- **Seamless Indoor/Outdoor Transition Architecture (`BuildingManager`)**: Zero-bleed map transitions, automatic camera boundary clamping, exterior player coordinate restoration, and dynamic indoor collision cache rebuilding.
- **Interactive Gameplay Integration**: Instant doorway interaction via `E` or `F` key with gold-accented contextual on-screen HUD prompts (`Press [E] or [F] to Enter/Exit <Building>`).
- **Village & Buildings Inspector (DevUI Tab 3)**: Interactive developer tools displaying settlement status, outdoor/indoor location readouts, exterior teleportation, and direct interior entry/exit controls.
- **Expanded Forest Biome ($160 \times 120$ Tiles, $5120 \times 3840$ Px)**: Fourfold expansion of the prototype world into a $19.66$ megapixel landscape featuring nine interconnected macro-subregions (Whispering Pines, Northern Ridge & Waterfall Cove, Greenwood Hamlet, Silver Lake, Emerald Grove Crossroads, Riverwood Outpost, Sunlit Meadows, Serpentine River Basin, Forgotten Ruins).
- **Three Distinct Small Settlements**: Complete architectural layouts for Greenwood Hamlet (Town Hall, residential cottages, storage barn, public stone well, lamp posts, picnic grounds), Riverwood Outpost (trading shop, hunter's lodge, wooden bridge crossing, outdoor campfire), and Highland Homestead (pioneer cabin, granary, animal trough, dry stone perimeter).
- **Five Abandoned Storytelling Structures**: Immersive narrative landmarks across the world including the Ruined Stone Watchtower with smoldering smoke plumes, the Overgrown Prototype Cabin of Emerald Grove, the Sunken Shrine on Silver Lake Islet, the Destroyed Bridge & Evacuated Camp, and the Overgrown Hermit's Shed.
- **Five Hidden Exploration Locations**: Rewarding concealed discovery sites including the Secluded Waterfall Cove, Concealed Meadow Clearing with rare golden butterflies, Sunken Shrine Islet, Smuggler's Rock Cache, and Ancient Woodland Altar.
- **Connected Road & Bridge Network**: Logical highway system with the King's Highway running North-South, East-West arterial routes, woodland footpaths, and traversable wooden bridge decks across the Serpentine River.
- **Data-Driven Environmental Animation Integration**: Map objects dynamically populate `EnvironmentalAnimationSystem` with 14 animated environmental objects (waterfalls, campfires, water flows, ripples, tree and grass sway), 3 atmospheric smoke plumes, and 15 ambient wildlife wanderers.
- **Landmark Inspector & Teleport Tool (DevUI Tab 3)**: Interactive subregion and landmark navigator allowing developers to instantaneously teleport character and camera across all 18 major points of interest.
- **32 Production Environmental Animation Assets (Phase 11)**: Comprehensive living-world pixel-art animation collection across Water, Waterfalls, Fire, Grass, Trees, Clouds, Fog, Smoke, and Airplane flyovers.
- **Unified Environmental Animation Architecture**: High-performance C++20 subsystem (`EnvironmentalAnimationSystem`) orchestrating multi-pass rendering across Ground, Object, Overhead, and UI virtual canvas layers without dynamic allocations in the game loop.
- **Ambient Wildlife Simulation (Category B)**: Autonomous wandering behavior for small songbirds and colorful butterflies (`forest_butterfly`, `blue_butterfly`, `golden_butterfly`) featuring territorial boundary bounding boxes (`wanderBounds`), procedural flight flapping, resting perches, and elevation offset ground shadows.
- **Multi-Layered Atmospheric Weather (Category C)**: Continuous overhead cloud drift at varying velocities and altitudes, infinite boundary wrapping, smoking chimney and ruin emitters, and screen-space ground fog tiling with real-time opacity density modulation.
- **Scripted Environmental Events (Category D)**: High-altitude recon airplane flyovers (`airplane_recon_001`) with linear trajectory heading calculation, continuous 16.7 fps propeller animation, and elevation-offset ground shadows.
- **Living World DevUI (Tab 9)**: Real-time developer overlay providing simulation speed control (0.1x to 3.0x), Category A animation toggles, on-demand wildlife spawner, fog opacity slider, and flyover dispatch triggers.
- **77 Production Environment Assets (Phase 10)**: Substantial pixel-art environment collection spanning 35 Nature assets, 18 Buildings, and 24 Props with decoupled visual canopies and physical collision footprints.
- **Decoupled Visual and Collision Footprints**: Overhanging tree canopies, roofs, and eaves are walkable with zero collision obstruction, while physical collision AABBs are mapped strictly to physical ground contact bases.
- **Building Architectural Specifications**: Detailed metadata standards covering doorway coordinates, window positions, roof dimensions, chimney offsets, and interior map references (`interior_<id>`) for future transitions.
- **Environment Preview DevUI (Tab 8)**: In-engine developer suite tool supporting category filtering ("All (77)", "Nature (35)", "Buildings (18)", "Props (24)"), spatial coordinate readouts, collision boundaries, and real-time native/scaled texture rendering.
- **Enemy & Wildlife Asset Collection**: Diverse fantasy creature roster featuring 5 combat-capable adversaries (Forest Goblin, Undead Skeleton, Forest Wolf, Wild Boar, and Dusk Bat) and 4 ambient wildlife species (Small Songbird, Forest Butterfly, Blue Butterfly, Golden Butterfly).
- **Quadruped, Biped & Aerial Animation Systems**: Dedicated locomotion cycles for ground quadrupeds, bipeds, and aerial flyers with standard walk/idle pacing, accelerated sprint pursuit runs, directional attack sequences, and elevation offset rendering.
- **Future Biome Creature Catalogue**: Comprehensive design roadmap covering 15+ planned creature species across Desert, Tundra, Swamp, Mountain, and Deep Forest biomes.
- **Creature Preview DevUI**: Interactive Tab 7 ("Creatures") in the developer overlay for live in-engine inspection of all 9 creatures, animation clips, frame indices, and collision bounds.
- **32 Production Cosmetic Assets**: Full initial collection of 32 pixel-art customization assets spanning 8 categories (5 Hairstyles, 5 Shirts, 5 Pants, 3 Shoes, 5 Hats, 3 Goggles, 3 Masks, 3 Backpacks).
- **Metadata-Driven Compatibility Matrix**: Seamless occlusion handling (`hair_hide_all` suppresses hair under full bascinets, `mask_hide_goggles` suppresses eyewear under porcelain masks) and directional depth sorting.
- **5 Representative Configurations**: Integrated presets in DevUI demonstrating Default Adventurer, Explorer, Armored Knight, Mystic Sorcerer, and Mixed Scout builds.
- **Modular Paperdoll Compositing**: Real-time multi-layer character rendering supporting 8 independently equipped visual slots over a standardized $32 \times 48$ px humanoid frame grid.
- **Master-Driven Cell Synchronization**: Guarantees 100% lockstep animation across all layers via a single master `AnimationController`, eliminating animation drift and layer separation.
- **Directional Occlusion & Dynamic Z-Sorting**: Dynamic depth sorting handles directional occlusion (e.g. backpacks render behind torso when facing South, but in front of torso when facing North; face accessories hidden when facing North).
- **JSON Appearance Persistence**: Character loadouts serialize cleanly to and from JSON (`data/characters/player_appearance.json`) for local persistence and future multiplayer replication.
- **Shared Combat Architecture**: Human-controlled `Player` and AI-controlled `Enemy` entities both implement the unified `Combatant` interface, sharing identical damage formulas, hurtboxes, attack pipelines, and event dispatch.
- **Authoritative Combat Engine**: Decouples attack requests and gameplay intentions from authoritative calculation. `CombatSystem` validates actions, enforces cooldowns, checks friendly fire and PvP rules, detects AABB hits, and calculates damage.
- **Directional Melee Attacks & AABB Hitboxes**: Dynamic directional hitboxes (`slash`, `goblin_club`, `skeleton_blade`) calculated in front of attacking entities with range offsets, hurtbox intersections, and contact point calculations.
- **Autonomous Hostile Enemies**: Goblin and Skeleton monster archetypes with 4-directional animated pixel-art spritesheets, collision against static terrain, and Y-sorted world depth rendering.
- **Dual-Layer Enemy AI**: Sandboxed Lua 5.4 state machine (`Idle`, `Patrol`, `DetectTarget`, `Chase`, `Attack`, `Retreat`, `Dead`) with instant fallback to a native C++ state machine if scripts are absent or fail.
- **Collision-Aware Enemy Spawner**: Automated and manual spawning with candidate footprint collision checking, world boundary safety, player proximity distance bounds ($180$–$450$ px), and active enemy limits.
- **100% Automated Test Coverage**: 20 CTest suites (including `test_ui`, `test_audio`, `test_costumes_armor`, `test_items`, `test_weapons`, `test_biomes`, `test_village_buildings`, `test_environmental_animation`, `test_environment_assets`, `test_creatures`, `test_production_cosmetics`, `test_modular_character`, `test_combat`, and smoke test) passing across both Debug and Release configurations.

## Technology Stack
- **Language**: C++20 (MSVC `/std:c++20`)
- **Graphics & Audio**: raylib 5.5
- **Scripting**: Lua 5.4.7 + sol2 v3.3.1
- **Developer UI**: Dear ImGui v1.93.0 + rlImGui
- **Map Editor**: Tiled Map Editor 1.10+
- **JSON Parsing**: nlohmann/json v3.11.3
- **Build System**: CMake 3.20+ with FetchContent
- **Testing**: CTest (22 test suites, 100% pass)

## Quick Start
```powershell
# Configure & fetch dependencies
cmake -B build -S .

# Compile (Debug)
cmake --build build --config Debug

# Run automated tests (100% pass)
ctest --test-dir build -C Debug --output-on-failure

# Launch Wildbound
.\build\Debug\wildbound.exe
```

### In-Game Controls
| Action | Key / Input |
|---|---|
| **Move** | `W`, `A`, `S`, `D` / Arrow Keys |
| **Sprint** | `Left Shift` |
| **Attack (Sword Slash / Bow Shoot)** | `Spacebar` / `Left Mouse Button` |
| **Interact / Pickup / Open Chest / Door** | `E` or `F` |
| **Hotbar Slots 1–4** | `1`, `2`, `3`, `4` |
| **Inventory & Collectibles** | `I` or `Tab` |
| **Modular Wardrobe** | `C` |
| **Pause Menu / Back** | `Escape` |
| **Toggle DevUI** | `F1` |
| **Toggle Collision Debug** | `F3` |
| **Toggle Player Anchor** | `F4` |
| **Toggle Animation HUD** | `F5` |
| **Toggle Combat Hitboxes** | `F6` |
| **Toggle Enemy Spawner** | `F7` |
| **Respawn / Reset Player** | `R` |

## Documentation
- [Phase 23 Implementation Report](docs/inventory/PHASE_23_IMPLEMENTATION_REPORT.md)
- [Pre-Implementation Inventory Audit](docs/inventory/PHASE_23_INVENTORY_AUDIT.md)
- [Inventory System Architecture](docs/inventory/INVENTORY_SYSTEM_ARCHITECTURE.md)
- [Item Definition & Registry Specification](docs/inventory/ITEM_DEFINITION_AND_REGISTRY.md)
- [Inventory Slot & Stacking Specification](docs/inventory/INVENTORY_SLOT_AND_STACKING_SPECIFICATION.md)
- [Ground Loot & Pickup System](docs/inventory/GROUND_LOOT_AND_PICKUP_SYSTEM.md)
- [Item Dropping & Transfer Transactions](docs/inventory/ITEM_DROPPING_AND_TRANSFER_TRANSACTIONS.md)
- [Consumable & Healing System](docs/inventory/CONSUMABLE_AND_HEALING_SYSTEM.md)
- [Ammunition Integration](docs/inventory/AMMUNITION_INTEGRATION.md)
- [Inventory UI Specification](docs/inventory/INVENTORY_UI_SPECIFICATION.md)
- [Loot Container Integration](docs/inventory/LOOT_CONTAINER_INTEGRATION.md)
- [Inventory Debugging Guide & DevUI Tab 16](docs/inventory/INVENTORY_DEBUGGING_GUIDE.md)
- [Phase 22 Implementation Report](docs/interaction/PHASE_22_IMPLEMENTATION_REPORT.md)
- [Pre-Implementation Interaction Audit](docs/interaction/PHASE_22_INTERACTION_AUDIT.md)
- [Interaction System Architecture](docs/interaction/INTERACTION_SYSTEM_ARCHITECTURE.md)
- [Interactable Object Specification](docs/interaction/INTERACTABLE_OBJECT_SPECIFICATION.md)
- [Interaction Discovery and Candidate Selection](docs/interaction/INTERACTION_DISCOVERY_AND_SELECTION.md)
- [Interaction Validation and Execution](docs/interaction/INTERACTION_VALIDATION_AND_EXECUTION.md)
- [Door and Building Interactions](docs/interaction/DOOR_AND_BUILDING_INTERACTIONS.md)
- [Container and Collectible Interactions](docs/interaction/CONTAINER_AND_COLLECTIBLE_INTERACTIONS.md)
- [Mechanism and Environmental Interactions](docs/interaction/MECHANISM_AND_ENVIRONMENTAL_INTERACTIONS.md)
- [Interaction Prompt UI Specification](docs/interaction/INTERACTION_PROMPT_UI.md)
- [Interaction Event Specification](docs/interaction/INTERACTION_EVENT_SPECIFICATION.md)
- [Interaction Debugging Guide & DevUI Tab 15](docs/interaction/INTERACTION_DEBUGGING_GUIDE.md)
- [Phase 21 Implementation Report](docs/input/PHASE_21_IMPLEMENTATION_REPORT.md)
- [Pre-Implementation Input Audit](docs/input/PHASE_21_INPUT_AUDIT.md)
- [Input System Architecture](docs/input/INPUT_SYSTEM_ARCHITECTURE.md)
- [Input Action Specification](docs/input/INPUT_ACTION_SPECIFICATION.md)
- [Keyboard & Mouse Input Specification](docs/input/KEYBOARD_AND_MOUSE_INPUT.md)
- [Gamepad Input Specification](docs/input/GAMEPAD_INPUT_SPECIFICATION.md)
- [Input Rebinding & Settings Specification](docs/input/INPUT_REBINDING_AND_SETTINGS.md)
- [Input Buffering Specification](docs/input/INPUT_BUFFERING_SPECIFICATION.md)
- [Input Context Management](docs/input/INPUT_CONTEXT_MANAGEMENT.md)
- [Input Update Order & Determinism](docs/input/INPUT_UPDATE_ORDER.md)
- [Input Debugging Guide & DevUI Tab 14](docs/input/INPUT_DEBUGGING_GUIDE.md)
- [Phase 20 Implementation Report](docs/physics/PHASE_20_IMPLEMENTATION_REPORT.md)
- [Pre-Implementation Collision Audit](docs/physics/PHASE_20_COLLISION_AUDIT.md)
- [Collision System Architecture](docs/physics/COLLISION_SYSTEM_ARCHITECTURE.md)
- [Collision Layer and Mask Specification](docs/physics/COLLISION_LAYER_AND_MASK_SPECIFICATION.md)
- [Trigger Volume Specification](docs/physics/TRIGGER_VOLUME_SPECIFICATION.md)
- [Collision Event and Callback Specification](docs/physics/COLLISION_EVENT_AND_CALLBACK_SPECIFICATION.md)
- [Collision Update Order & Determinism](docs/physics/COLLISION_UPDATE_ORDER.md)
- [Collision Debugging & Visualization Guide](docs/physics/COLLISION_DEBUGGING_GUIDE.md)
- [Collision Performance & Optimization Notes](docs/physics/COLLISION_PERFORMANCE_NOTES.md)
- [Phase 19 Implementation Report](docs/PHASE_19_IMPLEMENTATION_REPORT.md)
- [Gameplay Asset Inventory](docs/integration/GAMEPLAY_ASSET_INVENTORY.md)
- [Gameplay System Dependency Map](docs/integration/GAMEPLAY_SYSTEM_DEPENDENCY_MAP.md)
- [Item and Inventory Integration](docs/integration/ITEM_AND_INVENTORY_INTEGRATION.md)
- [Weapon and Combat Integration](docs/integration/WEAPON_AND_COMBAT_INTEGRATION.md)
- [Equipment and Armor Integration](docs/integration/EQUIPMENT_AND_ARMOR_INTEGRATION.md)
- [Consumable and Resource Integration](docs/integration/CONSUMABLE_AND_RESOURCE_INTEGRATION.md)
- [Character Customization Integration](docs/integration/CHARACTER_CUSTOMIZATION_INTEGRATION.md)
- [UI and Audio Integration](docs/integration/UI_AND_AUDIO_INTEGRATION.md)
- [Biome and Environment Integration](docs/integration/BIOME_AND_ENVIRONMENT_INTEGRATION.md)
- [Phase 18 Implementation Report](docs/PHASE_18_IMPLEMENTATION_REPORT.md)
- [UI Design System Specification](docs/ui/WILDBOUND_UI_DESIGN_SYSTEM.md)
- [UI Asset Catalog](docs/ui/UI_ASSET_CATALOG.md)
- [UI Component Specification](docs/ui/UI_COMPONENT_SPECIFICATION.md)
- [Audio System Architecture](docs/audio/AUDIO_SYSTEM_ARCHITECTURE.md)
- [Audio Asset Catalog (68 Procedural Assets)](docs/audio/AUDIO_ASSET_CATALOG.md)
- [Procedural Audio Generation Pipeline](docs/audio/AUDIO_GENERATION_PIPELINE.md)
- [Audio Mixing & Settings Specification](docs/audio/AUDIO_MIXING_AND_SETTINGS.md)
- [Audio Provenance & Licensing Declaration](docs/audio/AUDIO_PROVENANCE_AND_LICENSING.md)
- [Phase 17 Implementation Report](docs/PHASE_17_IMPLEMENTATION_REPORT.md)
- [Phase 16 Implementation Report](docs/PHASE_16_IMPLEMENTATION_REPORT.md)
- [Phase 15 Implementation Report](docs/PHASE_15_IMPLEMENTATION_REPORT.md)
- [Phase 14 Implementation Report](docs/PHASE_14_IMPLEMENTATION_REPORT.md)
- [Phase 13 Implementation Report](docs/PHASE_13_IMPLEMENTATION_REPORT.md)
- [Greenwood Village Design Specification](docs/world/VILLAGE_DESIGN.md)
- [Building & Interior Transition System](docs/world/BUILDING_AND_INTERIOR_SYSTEM.md)
- [Village Interior Catalog](docs/world/VILLAGE_INTERIOR_CATALOG.md)
- [Phase 12 Implementation Report](docs/PHASE_12_IMPLEMENTATION_REPORT.md)
- [Forest Biome Design Specification](docs/world/FOREST_BIOME_DESIGN.md)
- [Forest Biome Asset Placement Specification](docs/world/FOREST_BIOME_ASSET_PLACEMENT.md)
- [Phase 11 Implementation Report](docs/environment/PHASE_11_IMPLEMENTATION_REPORT.md)
- [Environmental Animation Architecture](docs/environment/ENVIRONMENTAL_ANIMATION_ARCHITECTURE.md)
- [Environmental Animation Asset Catalog](docs/environment/ENVIRONMENTAL_ANIMATION_ASSET_CATALOG.md)
- [Ambient Wildlife Specification](docs/environment/AMBIENT_WILDLIFE_SPECIFICATION.md)
- [Atmospheric Effects Specification](docs/environment/ATMOSPHERIC_EFFECTS_SPECIFICATION.md)
- [Scripted Environmental Event Specification](docs/environment/ENVIRONMENTAL_EVENT_SPECIFICATION.md)
- [Phase 10 Implementation Report](docs/PHASE_10_IMPLEMENTATION_REPORT.md)
- [Environment Asset Catalog (77 Assets)](docs/ENVIRONMENT_ASSET_CATALOG.md)
- [Building Asset Specification](docs/BUILDING_ASSET_SPECIFICATION.md)
- [Environment Placement & Collision Specification](docs/ENVIRONMENT_PLACEMENT_AND_COLLISION.md)
- [Environment Asset Integration](docs/ENVIRONMENT_ASSET_INTEGRATION.md)
- [Phase 9 Implementation Report](docs/PHASE_9_IMPLEMENTATION_REPORT.md)
- [Creature & Wildlife Asset Catalog](docs/CREATURE_ASSET_CATALOG.md)
- [Creature Animation Specification](docs/CREATURE_ANIMATION_SPECIFICATION.md)
- [Future Biome Creature Catalogue](docs/art/FUTURE_CREATURE_CATALOGUE.md)
- [Phase 8 Implementation Report](docs/PHASE_8_IMPLEMENTATION_REPORT.md)
- [Character Collection Catalog (32 Production Cosmetics)](docs/CHARACTER_COLLECTION_CATALOG.md)
- [Cosmetic Compatibility & Occlusion Matrix](docs/COSMETIC_COMPATIBILITY_MATRIX.md)
- [Phase 7 Implementation Report](docs/PHASE_7_IMPLEMENTATION_REPORT.md)
- [Modular Character System Architecture](docs/MODULAR_CHARACTER_SYSTEM.md)
- [Character Layer Architecture & Occlusion](docs/CHARACTER_LAYER_ARCHITECTURE.md)
- [Cosmetic Equipment System](docs/COSMETIC_EQUIPMENT_SYSTEM.md)
- [Character Animation Synchronization](docs/CHARACTER_ANIMATION_SYNCHRONIZATION.md)
- [Phase 6 Implementation Report & Art Bible](docs/PHASE_6_IMPLEMENTATION_REPORT.md)
- [Phase 5 Implementation Report](docs/PHASE_5_IMPLEMENTATION_REPORT.md)
- [Combat System Architecture](docs/COMBAT_SYSTEM.md)
- [Combatant Architecture](docs/COMBATANT_ARCHITECTURE.md)
- [Health & Damage System](docs/HEALTH_AND_DAMAGE.md)
- [Attack System & Hitboxes](docs/ATTACK_SYSTEM.md)
- [Hit Detection Specification](docs/HIT_DETECTION.md)
- [Enemy Subsystem](docs/ENEMY_SYSTEM.md)
- [Enemy AI Architecture](docs/ENEMY_AI.md)
- [Lua AI Scripting API](docs/LUA_AI_API.md)
- [Enemy Spawner Specification](docs/ENEMY_SPAWNING.md)
- [Player Character System](docs/PLAYER_SYSTEM.md)
- [Animation Subsystem](docs/ANIMATION_SYSTEM.md)
- [Character Controller](docs/CHARACTER_CONTROLLER.md)
- [Collision & Wall Sliding](docs/COLLISION_SYSTEM.md)
- [Player Input & Key Bindings](docs/PLAYER_INPUT.md)
- [Camera Follow Tracking](docs/CAMERA_FOLLOW.md)
- [World Architecture](docs/WORLD_ARCHITECTURE.md)
- [Tilemap System](docs/TILEMAP_SYSTEM.md)
- [Tiled Map Editor Integration](docs/TILED_INTEGRATION.md)
- [Environmental Objects](docs/ENVIRONMENTAL_OBJECTS.md)
- [Collision Metadata](docs/COLLISION_METADATA.md)
- [Map Format Specification](docs/MAP_FORMAT.md)
- [Architecture Overview](docs/ARCHITECTURE.md)
- [Rendering Architecture](docs/RENDERING_ARCHITECTURE.md)
- [Camera System](docs/CAMERA_SYSTEM.md)
- [Coordinate Systems](docs/COORDINATE_SYSTEMS.md)
- [Asset Management](docs/ASSET_MANAGEMENT.md)
- [Spritesheet Format](docs/SPRITESHEET_FORMAT.md)
- [Asset Licensing](docs/ASSET_LICENSING.md)
- [Tech Stack](docs/TECH_STACK.md)
- [Coding Standards](docs/CODING_STANDARDS.md)
- [Build Guide](docs/BUILD_GUIDE.md)
- [Performance Guidelines](docs/PERFORMANCE_GUIDELINES.md)
