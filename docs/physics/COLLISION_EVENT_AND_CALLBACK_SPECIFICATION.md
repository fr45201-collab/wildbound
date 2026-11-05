# Wildbound — Collision Event & Callback Specification

> **Document ID**: `WB-SPEC-PHYS-004`  
> **Status**: APPROVED  
> **Date**: 2026-11-04  
> **Subsystem**: `wildbound::physics`

---

## 1. Event Data Model

The `CollisionEvent` structure encapsulates all relevant contact and participant metadata:

```cpp
struct CollisionEvent {
    CollisionEventType type;         // CollisionEnter/Stay/Exit or TriggerEnter/Stay/Exit
    ColliderId colliderIdA;          // First participating collider
    ColliderId colliderIdB;          // Second participating collider
    EntityId entityIdA;              // Owner entity ID of A (or 0 if static geometry)
    EntityId entityIdB;              // Owner entity ID of B (or 0 if static geometry)
    CollisionLayer layerA;           // Layer classification of A
    CollisionLayer layerB;           // Layer classification of B
    Rectangle intersection;          // World-space intersection rectangle
    bool isTrigger;                  // True if either collider is a Trigger
};
```

---

## 2. Observer Models

The subsystem supports two decoupled observer paradigms:

### 2.1 Interface Listener (`ICollisionListener`)
Objects or systems implementing `ICollisionListener` register directly with `CollisionManager`:
```cpp
class ICollisionListener {
public:
    virtual ~ICollisionListener() = default;
    virtual void OnCollisionEvent(const CollisionEvent& event) = 0;
};
```
Ideal for monolithic managers (`CombatSystem`, `QuestManager`, `BuildingManager`).

### 2.2 Functional Callback (`CollisionCallback`)
Lightweight lambdas and function objects can be registered dynamically, returning a `CallbackId` for unregistration:
```cpp
auto cbId = collisionMgr.RegisterCallback([](const CollisionEvent& ev) {
    if (ev.type == CollisionEventType::TriggerEnter && ev.InvolvesEntity(1)) {
        // Player entered a trigger
    }
});
```

---

## 3. Safe Reentrancy & Deferred Mutation Queue

### The Problem
During event dispatch, game logic frequently requests structural changes:
- An arrow collider impacts an enemy $\implies$ arrow entity destroyed $\implies$ arrow collider unregistered.
- A player picks up a key $\implies$ chest trigger enabled.
- An enemy enters an ambush trigger $\implies$ new enemy colliders spawned.

If these modifications were applied immediately while iterating active collider lists or pair arrays, **iterator invalidation**, **reallocation corruption**, or **dangling pointers** would occur.

### The Solution: Deferred Mutation
When `CollisionManager::Update` executes, it sets `m_inUpdateTick = true`. Any mutation API called during this window is pushed into `m_deferredMutations`:
- `RegisterCollider` $\implies$ `MutationType::AddCollider`
- `UnregisterCollider` $\implies$ `MutationType::RemoveCollider`
- `SetColliderEnabled` $\implies$ `MutationType::SetEnabled`
- `SetColliderLayerAndMask` $\implies$ `MutationType::SetLayerMask`

At the beginning and completion of each update tick, `ProcessDeferredMutations()` drains the queue safely.

