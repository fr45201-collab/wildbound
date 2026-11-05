# Safe Zone Specification

## 1. Geometry and Representations

A Safe Zone within Wildbound is defined as a 2D Euclidean disc characterized by:
- Center coordinate $C = (X_c, Y_c) \in \mathbb{R}^2$ in world coordinates.
- Radius $R \ge 0 \in \mathbb{R}$.

```
                     World Playable Boundary [0, 0] to [W, H]
     +-----------------------------------------------------------------------+
     |                                                                       |
     |                     Current Safe Zone (Radius R1)                     |
     |                  . - ~ ~ ~ - .                                        |
     |              . '               ' .                                    |
     |            /                       \                                  |
     |           /    Next Target Zone     \                                 |
     |          |       . - ~ - .           |                                |
     |          |     /     C2    \         |                                |
     |          |    (  Radius R2  )        |                                |
     |          |     \           /         |                                |
     |           \      ' - ~ - '          /                                 |
     |            \                       /                                  |
     |              . '               ' .                                    |
     |                  ' - ~ ~ ~ - '                                        |
     |                                                                       |
     +-----------------------------------------------------------------------+
```

### 1.1 Containment Test
A point $P = (X_p, Y_p)$ is considered inside the safe zone if:
$$\|P - C\|^2 = (X_p - X_c)^2 + (Y_p - Y_c)^2 \le R^2$$

### 1.2 Signed Distance to Boundary
The Euclidean distance to the perimeter is calculated as:
$$d(P) = \|P - C\| - R$$
- If $d(P) \le 0$, the point is within the safe zone ($|d(P)|$ is depth inside).
- If $d(P) > 0$, the point is in the storm ($d(P)$ is penetration depth outside).

### 1.3 Closest Point on Boundary
For an entity at point $P$ outside the perimeter ($\|P - C\| > 0$), the closest point $S$ on the safe zone circle is projected via:
$$S = C + \frac{P - C}{\|P - C\|} \cdot R$$

---

## 2. Dynamic Center Migration & Boundary Constraints

To guarantee realistic, playable circles that never migrate into unplayable areas or outside the island boundary, `SafeZoneHelper::CalculateNextCenter` applies a two-tier containment constraint:

1. **Drift Clamping**:
   The distance between current center $C_1$ and next center $C_2$ cannot exceed the difference in radii scaled by the configured drift factor:
   $$\|C_2 - C_1\| \le (R_1 - R_2) \cdot \text{driftRatio}$$
   where $\text{driftRatio} \in [0.0, 1.0]$. When $\text{driftRatio} = 1.0$, the next circle is fully inscribed within the current circle.

2. **World Boundary Clamping**:
   Given world playable bounds $[X_{\min}, Y_{\min}, X_{\max}, Y_{\max}]$, the candidate center $(X, Y)$ is clamped to:
   $$X \in [X_{\min} + R_2, X_{\max} - R_2]$$
   $$Y \in [Y_{\min} + R_2, Y_{\max} - R_2]$$
   If $2 \cdot R_2$ exceeds the world dimension, $C_2$ defaults to the center of the world bounds.

