#include "InteractionExecutor.hpp"
#include "Door.hpp"
#include "Mechanism.hpp"
#include "../entities/Player.hpp"
#include "../world/World.hpp"
#include "../world/BuildingManager.hpp"
#include "../world/LootContainer.hpp"
#include "../world/WorldItem.hpp"
#include "../items/ItemManager.hpp"
#include "../audio/AudioSystem.hpp"
#include "../core/Logger.hpp"

using wildbound::core::Logger;

namespace wildbound::interaction {

InteractionResult InteractionExecutor::Execute(InteractableComponent* target,
                                              InteractionContext& context,
                                              Door* door,
                                              Mechanism* mechanism) {
    if (!target) {
        return InteractionResult::InvalidTarget;
    }

    target->isExecuting = true;
    InteractionResult result = InteractionResult::Failed;

    // 1. Custom handler override takes top precedence
    if (target->customHandler) {
        result = target->customHandler(target->id, context);
    }
    // 2. Door Category
    else if (target->definition.category == InteractionCategory::Door && door) {
        if (context.player) {
            result = door->TryInteract(*context.player, context.audioSystem, context.collisionManager);
        } else {
            result = door->Toggle(context.audioSystem, context.collisionManager)
                         ? InteractionResult::Success : InteractionResult::Failed;
        }
    }
    // 3. Building Category
    else if (target->definition.category == InteractionCategory::Building) {
        if (context.buildingManager && context.world && context.player &&
            context.camera && context.assetManager) {
            if (context.buildingManager->IsIndoors()) {
                bool ok = context.buildingManager->ExitBuilding(*context.world, *context.player,
                                                                *context.camera, *context.assetManager);
                result = ok ? InteractionResult::Success : InteractionResult::Failed;
                if (ok && context.audioSystem) {
                    context.audioSystem->DispatchEvent("Environment.DoorOpen");
                }
            } else {
                std::string bId = target->linkedEntityId;
                if (bId.empty()) {
                    const auto* b = context.buildingManager->CheckEntrancePrompt(context.player->GetPosition());
                    if (b) bId = b->id;
                }
                if (!bId.empty()) {
                    bool ok = context.buildingManager->EnterBuilding(bId, *context.world, *context.player,
                                                                    *context.camera, *context.assetManager);
                    result = ok ? InteractionResult::Success : InteractionResult::Failed;
                    if (ok && context.audioSystem) {
                        context.audioSystem->DispatchEvent("Environment.DoorOpen");
                    }
                } else {
                    result = InteractionResult::InvalidTarget;
                }
            }
        }
    }
    // 4. Container Category
    else if (target->definition.category == InteractionCategory::Container) {
        if (context.world && context.player) {
            auto* container = context.world->GetLootContainer(target->linkedEntityId);
            if (!container) {
                container = context.world->GetLootContainerNear(target->position, target->definition.range);
            }
            if (container) {
                if (container->IsLocked()) {
                    if (!container->TryOpen(*context.player, context.audioSystem)) {
                        target->isExecuting = false;
                        return InteractionResult::MissingRequirement;
                    }
                } else if (!container->IsOpen()) {
                    container->Open(context.audioSystem);
                }

                int lootedCount = container->LootAll(context.player->GetInventory(), context.itemManager, context.audioSystem);
                if (lootedCount > 0) {
                    result = InteractionResult::Success;
                } else {
                    // Container already empty
                    result = InteractionResult::Success;
                }
            } else {
                result = InteractionResult::InvalidTarget;
            }
        }
    }
    // 5. Collectible Category (Transactional Ground Item Pickup)
    else if (target->definition.category == InteractionCategory::Collectible) {
        if (context.world && context.player) {
            auto* worldItem = context.world->GetWorldItem(target->linkedEntityId);
            if (worldItem && !worldItem->IsCollected()) {
                auto& inv = context.player->GetInventory();
                auto& item = worldItem->GetItem();

                int unadded = inv.AddItem(item.definitionId, item.quantity, context.itemManager);
                if (unadded == 0) {
                    worldItem->MarkCollected();
                    if (context.audioSystem) {
                        context.audioSystem->DispatchEvent("Interact.ItemPickup");
                    }
                    result = InteractionResult::Success;
                    Logger::Info("COLLECTIBLE", "Collected [" + item.definitionId + " x" +
                                 std::to_string(item.quantity) + "] into inventory.");
                } else if (unadded < item.quantity) {
                    int collected = item.quantity - unadded;
                    item.quantity = unadded;
                    if (context.audioSystem) {
                        context.audioSystem->DispatchEvent("Interact.ItemPickup");
                    }
                    result = InteractionResult::Success;
                    Logger::Info("COLLECTIBLE", "Partially collected [" + item.definitionId + " x" +
                                 std::to_string(collected) + "], " + std::to_string(unadded) + " remain on ground.");
                } else {
                    Logger::Warn("COLLECTIBLE", "Cannot collect [" + item.definitionId + "]: Inventory full.");
                    result = InteractionResult::Blocked;
                }
            } else {
                result = InteractionResult::InvalidTarget;
            }
        }
    }
    // 6. Mechanism Category
    else if (target->definition.category == InteractionCategory::Mechanism && mechanism) {
        bool ok = mechanism->Toggle(context.audioSystem);
        result = ok ? InteractionResult::Success : InteractionResult::Failed;
    }
    // 7. Environmental & Generic Objects
    else if (target->definition.category == InteractionCategory::Environmental) {
        if (!target->definition.audioEvent.empty() && context.audioSystem) {
            context.audioSystem->DispatchEvent(target->definition.audioEvent);
        }
        Logger::Info("INTERACTION", "Interacted with environmental object [" + target->definition.displayName + "].");
        result = InteractionResult::Success;
    }

    target->isExecuting = false;

    if (result == InteractionResult::Success) {
        target->hasBeenExecuted = true;
        target->currentCooldown = target->definition.cooldown;
        if (target->definition.oneTimeOnly) {
            target->isAvailable = false;
        }
        if (!target->definition.audioEvent.empty() && context.audioSystem) {
            context.audioSystem->DispatchEvent(target->definition.audioEvent);
        }
    }

    return result;
}

} // namespace wildbound::interaction
