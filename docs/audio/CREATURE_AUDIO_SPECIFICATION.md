# Creature Audio Specification

## 1. Overview & Objectives

Wildbound's creature audio subsystem models vocalizations, locomotion footfalls, attack actions, and death cries for all 5 implemented enemy archetypes, plus ambient wildlife:
1. **Goblin**
2. **Skeleton**
3. **Wolf**
4. **Wild Boar**
5. **Bat**
6. **Ambient Wildlife (Birds & Butterflies)**

All creature audio is routed to the dedicated `AudioBus::Creatures` channel with 2D spatial positioning relative to the listener.

---

## 2. Creature Archetype Specifications

### 2.1 Goblin
- **Idle (`creature.goblin.idle`):** Raspy throat wobble ($280\text{ Hz} \pm 90\text{ Hz}$) with subtle malice ($0.40\text{s}$).
- **Step (`creature.goblin.step`):** Shuffling light footfall on dirt ($0.12\text{s}$).
- **Attack (`creature.goblin.attack`):** Heavy wooden club swing whoosh with breath grunt ($0.22\text{s}$).
- **Hurt (`creature.goblin.hurt`):** High-pitched squealing yelp ($0.25\text{s}$).
- **Death (`creature.goblin.death`):** Descending screech and body drop thud ($0.55\text{s}$).

### 2.2 Skeleton
- **Movement (`creature.skeleton.step`):** Dry hollow bone clinking ($1.2\text{ kHz} - 1.8\text{ kHz}$) ($0.14\text{s}$).
- **Attack (`creature.skeleton.attack`):** Fast bone blade thrust whoosh with bone rattle ($0.24\text{s}$).
- **Hurt (`creature.skeleton.hurt`):** Sharp calcium fracture crack and splinter ($0.20\text{s}$).
- **Death (`creature.skeleton.death`):** Complete collapse of bones cascading onto the ground ($0.60\text{s}$).

### 2.3 Wolf
- **Idle (`creature.wolf.idle`):** Low guttural throat snarl ($75\text{ Hz} + 20\text{ Hz}$ vibrato) ($0.50\text{s}$).
- **Growl (`creature.wolf.growl`):** Aggressive threat growl signaling player detection ($0.65\text{s}$).
- **Attack (`creature.wolf.attack`):** Fast jaw snap and lunging bite ($0.25\text{s}$).
- **Hurt (`creature.wolf.hurt`):** Sharp canine pain yelp ($650\text{ Hz} \to 350\text{ Hz}$) ($0.28\text{s}$).
- **Death (`creature.wolf.death`):** Mournful whimper fading into silence ($0.70\text{s}$).

### 2.4 Wild Boar
- **Idle (`creature.boar.idle`):** Deep nasal snort and guttural grunt ($80\text{ Hz}$) ($0.45\text{s}$).
- **Step (`creature.boar.step`):** Heavy clopped hoof impact on earth ($0.15\text{s}$).
- **Attack (`creature.boar.attack`):** Ferocious headbutt charge with tusk slash ($0.35\text{s}$).
- **Hurt (`creature.boar.hurt`):** High piercing swine squeal ($1.1\text{ kHz} \to 700\text{ Hz}$) ($0.32\text{s}$).
- **Death (`creature.boar.death`):** Heavy collapse squeal and ground impact ($0.65\text{s}$).

### 2.5 Bat
- **Flight (`creature.bat.flight`):** Rapid leathery wing flutter ($60\text{ Hz}$ modulation) ($0.25\text{s}$).
- **Attack (`creature.bat.attack`):** High-frequency ultrasonic swoop dive ($0.22\text{s}$).
- **Hurt (`creature.bat.hurt`):** Shrill ultrasonic screech ($3.2\text{ kHz} \to 2.0\text{ kHz}$) ($0.18\text{s}$).
- **Death (`creature.bat.death`):** Soft muffled fall thud ($0.35\text{s}$).

### 2.6 Ambient Wildlife
- `wildlife.bird.takeoff`: Rapid wing flutter takeoff triggered when approached by the player ($0.35\text{s}$).
- `wildlife.bird.landing`: Soft branch contact and flutter ($0.20\text{s}$).
- `wildlife.butterfly.flutter`: Extremely subtle atmospheric wing murmur ($0.25\text{s}$).

---

## 3. Spatialization & Voice Clamping Rules

1. **Distance Culling:** All creature sound events are culled if the creature is farther than $500\text{ units}$ from the player listener.
2. **Idle Cooldowns:** Idle vocalizations have a minimum $2.0\text{s}$ cooldown to prevent audio crowding in multi-enemy packs.
3. **Stereo Localization:** Attacking or hurt creatures are stereo-panned based on their world coordinates, giving the player instant situational awareness of flanking monsters.

