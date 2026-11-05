#pragma once

namespace wildbound::entities {
    class Player;
}

namespace wildbound::world {
    class World;
    class BuildingManager;
}

namespace wildbound::physics {
    class CollisionManager;
}

namespace wildbound::items {
    class ItemManager;
}

namespace wildbound::audio {
    class AudioSystem;
}

namespace wildbound::graphics {
    class Camera2DSystem;
}

namespace wildbound::resources {
    class AssetManager;
}

namespace wildbound::interaction {

/**
 * Encapsulates environmental and engine context passed to interaction executors.
 */
struct InteractionContext {
    entities::Player* player = nullptr;
    world::World* world = nullptr;
    world::BuildingManager* buildingManager = nullptr;
    physics::CollisionManager* collisionManager = nullptr;
    items::ItemManager* itemManager = nullptr;
    audio::AudioSystem* audioSystem = nullptr;
    graphics::Camera2DSystem* camera = nullptr;
    resources::AssetManager* assetManager = nullptr;
};

} // namespace wildbound::interaction

