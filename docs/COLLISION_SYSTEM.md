# Collision Detection & Wall Sliding Resolution

## 1. Overview

Wildbound uses an **axis-decoupled swept AABB collision system** combined with a spatial hash grid for real-time environment collision detection. It prevents character tunneling, eliminates corner sticking, and allows smooth wall sliding along diagonal obstacles.

---

## 2. Feet Bounding Box Geometry

To create believable 2D depth and perspective, character collision boxes must not envelop the entire sprite:
* **Sprite Bounds**: 32 pixels wide × 48 pixels high.
* **Collision Box**: 16 pixels wide × 12 pixels high, anchored at the bottom edge.
* **Benefits**:
  1. The character's head and shoulders can visually overlap behind tree canopies, fences, and building walls.
  2. Movement obstruction occurs only when the feet contact solid ground obstacles.

```
+------------------+
|                  |
|    Head & Body   |  (No collision - enables
|    (32 x 36 px)  |   visual depth overlap)
|                  |
+------------------+
|  [Feet Hitbox]   |  (16 x 12 px active collision)
+------------------+
         * (pos.x, pos.y: Logical ground anchor)
```

---

## 3. Axis-Decoupled Movement & Wall Sliding

Standard 2D collision algorithms often test diagonal movement as a single 2D vector $(dx, dy)$. If an intersection occurs, movement is completely zeroed, causing characters to "stick" to walls when moving diagonally into them.

Wildbound decomposes movement into separate X and Y steps:

```cpp
void Player::ResolveMovement(Vector2 delta, const world::World& world) {
    // 1. Move and resolve Horizontal Axis (X)
    if (std::fabs(delta.x) > 0.0001f) {
        m_position.x += delta.x;
        Rectangle colBounds = GetCollisionBounds();
        auto collisions = world.GetCollisionBoxes(colBounds);
        for (const auto& box : collisions) {
            if (box.IsSolid() && box.Intersects(colBounds)) {
                if (delta.x > 0.0f) {
                    // Rightward movement blocked: push back to left edge of obstacle
                    m_position.x = box.bounds.x - (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                } else if (delta.x < 0.0f) {
                    // Leftward movement blocked: push back to right edge of obstacle
                    m_position.x = (box.bounds.x + box.bounds.width) + (m_collisionConfig.width * 0.5f) - m_collisionConfig.offsetX;
                }
                colBounds = GetCollisionBounds();
            }
        }
    }

    // 2. Move and resolve Vertical Axis (Y) - independently of X!
    if (std::fabs(delta.y) > 0.0001f) {
        m_position.y += delta.y;
        Rectangle colBounds = GetCollisionBounds();
        auto collisions = world.GetCollisionBoxes(colBounds);
        for (const auto& box : collisions) {
            if (box.IsSolid() && box.Intersects(colBounds)) {
                if (delta.y > 0.0f) {
                    // Downward movement blocked: push back to top edge of obstacle
                    m_position.y = box.bounds.y - m_collisionConfig.offsetY;
                } else if (delta.y < 0.0f) {
                    // Upward movement blocked: push back to bottom edge of obstacle
                    m_position.y = (box.bounds.y + box.bounds.height) + m_collisionConfig.height - m_collisionConfig.offsetY;
                }
                colBounds = GetCollisionBounds();
            }
        }
    }
}
```

### Result
When moving diagonally into a vertical wall (e.g., $dx = +10, dy = +10$ against a wall on the right):
1. The X displacement is stopped and clamped to the wall's left edge.
2. The Y displacement continues unhindered, resulting in natural, responsive downward wall sliding.
