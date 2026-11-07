# PLACEMENT CALCULATION & TOURNAMENT TIE SPECIFICATION

## 1. Overview
The Placement Calculation subsystem in `ResultCalculator::CalculatePlacements` establishes deterministic, monotonic tournament ranks for all competitors in a match.

---

## 2. Standard Placement Rules (Last Survivor Standing)
1. **The Victor**:
   - The sole living survivor receives **Rank #1**.
   - In a mutual draw (0 survivors), all participants eliminated in the final tick receive **Rank #1**.
2. **Reverse Chronological Elimination**:
   - Eliminated competitors are sorted descending by their `eliminationInfo.timestamp`.
   - Competitors who survived longer receive higher ranks (e.g. competitor eliminated at $t=180\text{s}$ ranks ahead of competitor eliminated at $t=60\text{s}$).

---

## 3. Tournament Tie-Breaking Policy
When two or more competitors are eliminated at the exact same logical timestamp (within epsilon $|\Delta t| < 0.001\text{s}$):
- **Equal Placement**: Both competitors share the identical placement rank.
- **Rank Skipping**: The subsequent placement advances by the size of the tied group.

### Concrete Example:
Suppose 4 competitors participate:
- `Winner`: Alive at match end $\rightarrow$ **Rank #1**
- `Player B`: Eliminated at $t=50.0\text{s}$ $\rightarrow$ **Rank #2 (Tie)**
- `Player C`: Eliminated at $t=50.0\text{s}$ $\rightarrow$ **Rank #2 (Tie)**
- `Player D`: Eliminated at $t=20.0\text{s}$ $\rightarrow$ **Rank #4** (Rank #3 is skipped)

This follows the official Olympic and tournament standard, preventing arbitrary ordering based on internal memory layout or entity ID.

