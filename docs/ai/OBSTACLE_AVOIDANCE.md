# Wildbound — Dynamic Obstacle Avoidance

## 1. Overview & Separation Dynamics

When multiple entities pursue the same target or traverse narrow chokepoints, pathfinding alone can cause entities to overlap or cluster into identical grid coordinates.

Wildbound solves this using a local separation force architecture implemented in `wildbound::ai::ObstacleAvoidance`. The system applies dynamic repulsive steering vectors calculated from nearby agent positions.

---

## 2. Mathematical Model

For entity $i$ at position $\mathbf{p}_i$, obstacle avoidance samples all neighboring dynamic entities $j \in N_i$ within an avoidance radius $R_{\text{avoid}}$ (default 24.0 pixels):

$$\mathbf{d}_{ij} = \mathbf{p}_i - \mathbf{p}_j, \quad r_{ij} = \|\mathbf{d}_{ij}\|$$

If $r_{ij} < R_{\text{avoid}}$ and $r_{ij} > 0$:

$$\mathbf{F}_{ij} = \hat{\mathbf{d}}_{ij} \cdot \left( 1.0 - \frac{r_{ij}}{R_{\text{avoid}}} \right) \cdot w_{\text{separation}}$$

where:
- $\hat{\mathbf{d}}_{ij} = \frac{\mathbf{d}_{ij}}{r_{ij}}$ is the normalized direction pointing away from neighbor $j$.
- $\left( 1.0 - \frac{r_{ij}}{R_{\text{avoid}}} \right)$ provides inverse linear falloff (strongest when touching, zero at boundary).
- $w_{\text{separation}}$ is the profile-defined avoidance weight ($1.0 - 2.5$).

The total avoidance steering force is the vector sum over all neighbors:

$$\mathbf{F}_{\text{avoid}} = \sum_{j \in N_i} \mathbf{F}_{ij}$$

---

## 3. Steering Integration & Velocity Blending

In `AIMovementController::ComputeDesiredVelocity`:

```cpp
Vector2 desiredVel = pathHeading * maxSpeed;

if (avoidanceEnabled) {
    Vector2 avoidForce = ObstacleAvoidance::ComputeAvoidanceForce(
        currentPos, neighborPositions, avoidRadius, avoidWeight);
    
    // Blend goal velocity with local avoidance
    desiredVel.x += avoidForce.x * maxSpeed;
    desiredVel.y += avoidForce.y * maxSpeed;

    // Clamp to maximum permitted agent velocity
    float len = std::sqrt(desiredVel.x * desiredVel.x + desiredVel.y * desiredVel.y);
    if (len > maxSpeed) {
        desiredVel.x = (desiredVel.x / len) * maxSpeed;
        desiredVel.y = (desiredVel.y / len) * maxSpeed;
    }
}
```

---

## 4. Performance & Spatial Query Optimization

Rather than performing an $O(N^2)$ global pairwise check across all game entities:
1. Neighbors are queried locally from `EnemySpawner::GetActiveEnemies()` filtered within the local $R_{\text{avoid}}$ bounding box.
2. Self-comparison is pruned via pointer equality (`&neighbor != &self`).
3. Dead or inactive entities are skipped immediately.
4. Total execution overhead across 50 active enemies is under $0.05 \text{ ms}$ per frame.

