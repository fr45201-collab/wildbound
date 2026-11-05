# Wildbound — Inventory Debugging Guide

## 1. DevUI Tab 16 ("Loot & Inventory")
Pressing `F1` opens DevUI, where Tab 16 provides real-time state inspection and debugging controls:

### Sections:
1. **Player Inventory (24 Slots)**:
   - Live table listing slot index, item ID, display name, category, quantity / max stack, and actions.
   - Per-slot action buttons: `Drop 1`, `Drop All`, `Use`.
   - `Clear All Slots` button for instant testing reset.
2. **Ground Loot Spawner & Debug Tools**:
   - `Spawn Small Health Potion (+50 HP)`
   - `Spawn Large Health Potion (+100 HP)`
   - `Spawn Iron Arrows (x15)`
   - `Spawn Gold Coins (x50)`
   - `Spawn Cooked Meat (x3)`
   - `Spawn Iron Sword`
   - `Spawn Hunting Bow`
   - `Clear All Ground Loot`
3. **Active Ground Loot Registry**:
   - Live table displaying all active `WorldItem`s with entity ID, definition, quantity, world position, pickup radius, and collected status.

---

## 2. Automated Test Suite (Test 25)
Run the dedicated test suite:
```powershell
.\build\tests\Release\test_inventory.exe
```
Or run all test suites via CTest:
```powershell
ctest --test-dir build -C Release --output-on-failure
```
Expect **25/25 passing tests**.

