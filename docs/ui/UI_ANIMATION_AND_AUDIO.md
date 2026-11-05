# UI Animation and Audio Specification

## 1. Overview

Visual and auditory responsiveness are critical for player immersion and feedback. Wildbound's UI implements lightweight, frame-rate independent procedural animations and comprehensive event dispatching to the `AudioSystem`.

---

## 2. Procedural Animation Pipelines

### 2.1 Delayed Damage Lag Bar Catch-Down
- **Hold Duration**: $0.5\text{s}$ stationary pause.
- **Drain Velocity**: $65.0\text{ HP/s}$ linear catch-down.
- **Handling Frame Jitter**: Excess delta time is captured upon timer expiration to ensure sub-frame smooth continuity.

### 2.2 Low Health Sinusoidal Pulse
- **Condition**: $HP \le 0.25 \times HP_{max}$.
- **Formula**:
  $$\text{Alpha}(t) = \text{base} + \text{amplitude} \cdot \sin(2\pi \cdot f \cdot t)$$
  where $f = 0.8\text{ Hz}$ ($4.0\text{ rad/s}$), $\text{base} = 0.35$, $\text{amplitude} = 0.35$.
- Produces a natural biological heartbeat sensation on the screen perimeter.

### 2.3 Non-Blocking Toast Notification Slide & Fade
- **Entry Slide**: Toasts spawn with an offset of $+20\text{ px}$ from the right screen edge and slide into place over $0.25\text{s}$ using cubic ease-out:
  $$x(t) = x_{target} + 20 \cdot (1 - t)^3$$
- **Hold Duration**: Sustained for $3.0\text{s}$.
- **Exit Fade**: Over the final $0.5\text{s}$, opacity $\alpha(t)$ fades linearly from $1.0$ to $0.0$.

### 2.4 Button Hover Scale & Accent Expansion
- Hovering over buttons smoothly expands the gold border indicator from $0\text{ px}$ to $3\text{ px}$ width over $0.1\text{s}$.
- Inactive buttons maintain low alpha ($60\%$), transitioning to full opacity ($100\%$) upon selection.

---

## 3. AudioSystem Event Dispatching

Every UI interaction dispatches strongly typed audio triggers consumed by `wildbound::audio::AudioSystem`:

| UI Event | Audio Event Trigger | Default SFX Characteristics |
|---|---|---|
| Menu Button Navigate | `ui.button.hover` | Soft high-pass click, $1200\text{ Hz}$, $0.02\text{s}$, $-12\text{ dB}$ |
| Menu Button Click | `ui.button.click` | Solid resonant wooden thud, $440\text{ Hz}$, $0.08\text{s}$ |
| Panel Open | `ui.panel.open` | Whoosh / scroll unroll sound, $0.15\text{s}$ |
| Panel Close | `ui.panel.close` | Subdued slide / click, $0.12\text{s}$ |
| Tab Switch | `ui.tab.switch` | Sharp metallic tick, $0.04\text{s}$ |
| Item Move / Select | `ui.item.select` | Leather bag rustle |
| Item Equip | `item.equip` | Metal armor clink / leather buckle |
| Item Drop | `item.drop` | Ground impact thud |
| Potion Consumed | `item.consume.potion` | Liquid swallow / glass clink |
| Notification Toast | `ui.notification.ping` | Soft chime, major third chord |

### Bus Routing & Volume Attenuation
All UI audio is routed strictly to the `AudioBus::UI` channel, honoring player volume adjustments in the Settings menu independently from SFX, Music, and Environmental ambience.

