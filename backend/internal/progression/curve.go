package progression

import "fmt"

// XPRequiredForLevel returns total accumulated XP required to reach the given level.
func XPRequiredForLevel(level int) int {
	if level <= 1 {
		return 0
	}
	l := level - 1
	return (l * 100) + (l * (l - 1) * 25)
}

// CalculateLevelFromXP returns current level, progress in current level, XP required for next level, and progress percentage.
func CalculateLevelFromXP(totalXP int) (level int, currentLevelXP int, nextLevelXP int, progressPct float32) {
	if totalXP <= 0 {
		return 1, 0, XPRequiredForLevel(2), 0.0
	}

	lvl := 1
	for {
		nextThreshold := XPRequiredForLevel(lvl + 1)
		if totalXP < nextThreshold {
			break
		}
		lvl++
	}

	currentThreshold := XPRequiredForLevel(lvl)
	nextThreshold := XPRequiredForLevel(lvl + 1)

	currentXP := totalXP - currentThreshold
	neededForNext := nextThreshold - currentThreshold
	pct := float32(currentXP) / float32(neededForNext) * 100.0

	return lvl, currentXP, neededForNext, pct
}

// CalculateMatchXP calculates earned XP and reward points from authoritative match statistics.
func CalculateMatchXP(placement, eliminations int, damageDealt, survivalSeconds float32) (int, int, string) {
	xp := 50 // Base match completion XP
	points := 5

	// Placement bonus
	switch placement {
	case 1:
		xp += 250
		points += 25
	case 2:
		xp += 150
		points += 15
	case 3:
		xp += 100
		points += 10
	default:
		if placement <= 5 && placement > 0 {
			xp += 50
			points += 5
		}
	}

	// Eliminations bonus
	if eliminations > 0 {
		xp += eliminations * 30
		points += eliminations * 2
	}

	// Damage dealt bonus (1 XP per 10 damage)
	if damageDealt > 0 {
		xp += int(damageDealt / 10.0)
	}

	// Survival time bonus (1 XP per 10 seconds)
	if survivalSeconds > 0 {
		xp += int(survivalSeconds / 10.0)
	}

	reason := fmt.Sprintf("Match Completed — Placement #%d, %d Eliminations, %.0f Damage", placement, eliminations, damageDealt)
	return xp, points, reason
}

