# Toothbrush Simulator — Requirements Document

> Derived from reference product [BYS-M9P], adapted for LVGL PC simulator.
> **Single source of truth.** Code follows this document. When in doubt, this wins.

---

## 1. Hardware & Environment

| Item | Reference (Real HW) | Simulator (This Project) |
|------|--------------------|-------------------------|
| CPU | M0, 8 KB RAM, 60 KB Flash | PC (Windows / Linux / macOS) |
| Display | SPI LCD, 80×160 px | SDL2 window, 240×280 px |
| Input | 1 physical button | `↑` short press / `↓` long press |
| Sensor | 3-axis accelerometer | Not simulated |
| USB | Charging event | Not simulated |
| Motor | BLDC, 3 modes + boost | Simulated (logic only) |

---

## 2. Global Rules

Apply to **all states unless explicitly overridden**.

| # | Rule | Detail |
|---|------|--------|
| G1 | **Idle timeout** | 8 s without key press → SLEEP. **Skipped during WORKING.** |
| G2 | **Wake behavior** | Wake from SLEEP → READY, restoring `last_mode`. |
| G3 | **Mode memory** | Any motor mode used > 10 s is saved as `last_mode`; restored on next wake. |
| G4 | **Timer display** | During WORKING, elapsed time counts 0 → 120 s, refreshed every 1 s. |

---

## 3. Motor Modes

| ID | Name | Display Label |
|----|------|---------------|
| 0 | Care | Care |
| 1 | Clean | Clean |
| 2 | White | White |

No boost mode in the simulator.

---

## 4. Screen Inventory

### 4.1 READY Screen

```
┌──────────────────────────┐
│        TOOTHBRUSH        │  ← title
│                          │
│        ┌──────┐          │  ← mode icon placeholder (80×80)
│        │ 🪥   │          │
│        └──────┘          │
│          Clean           │  ← current mode name (accent color)
│      Default Mode        │  ← subtitle
│                          │
│ UP=start  DOWN=settings  │  ← hint (bottom)
│                    🔋 85%│  ← battery placeholder (top-right)
└──────────────────────────┘
```

### 4.2 WORKING Screen

```
┌──────────────────────────┐
│        🪥                 │  ← toothbrush icon (top)
│                          │
│      ╭──────────╮        │
│     ╱   00:42   ╲        │  ← timer at center of progress ring
│     │           │        │
│     ╲           ╱        │  ← ring goes 0→360° over 2 min
│      ╰──────────╯        │
│          Clean           │  ← mode name (below ring)
│                          │
│ UP=pause/mode  DOWN=stop │  ← hint (bottom)
└──────────────────────────┘
```

**Timer rule**: READY → WORKING starts from 0. STATS/CANCEL → WORKING continues accumulating.

### 4.3 STATS / CANCEL Screen

```
┌──────────────────────────┐
│   Brushing Complete!     │  ← "Complete" or "Cancelled"
│                          │
│       ╭──────╮           │
│      ╱ Score:82╲         │  ← score in center of arc, or "<30s"
│      │         │         │
│      ╲         ╱         │  ← score arc (0→360° proportional)
│       ╰──────╯           │
│      Time  01:12         │  ← elapsed time
│                          │
│ UP=continue brushing     │  ← hint
└──────────────────────────┘
```

| Condition | Title | Score | Arc |
|-----------|-------|-------|-----|
| WORKING stopped, t < 30 s | "Brushing Cancelled" | "< 30s" | 0 |
| WORKING stopped, t ≥ 30 s | "Brushing Complete!" | 70 + t/4 (max 100) | proportional |

### 4.4 SLEEP Screen

```
┌──────────────────────────┐
│                          │
│           💤              │
│                          │
│      Click to wake       │
└──────────────────────────┘
```

Black background. Any key → wake.

### 4.5 SETTINGS Screen

Reuses READY container. Title changes to "SETTINGS", hint changes to "Short press → exit". Navigation and sub-menus are **out of scope** for the simulator.

---

## 5. Operation Matrix (Core)

> Format: **Current State × Event → Action / Next State.**
> If "Action" is empty, only a state transition occurs.

| # | Current State | Event | Condition | Action | Next State |
|---|-------------|-------|-----------|--------|-----------|
| R1 | READY | ↑ short | — | Reset timer to 0 | WORKING |
| R2 | READY | ↓ long | — | — | SETTINGS |
| R3 | READY | idle 8s | — | Save current mode to `last_mode` | SLEEP |
| | | | | | |
| W1 | WORKING | ↑ short | t < 10 s, not last mode | `mode += 1` | WORKING |
| W2 | WORKING | ↑ short | t < 10 s, mode == White | — | READY |
| W3 | WORKING | ↑ short | 10 s ≤ t < 30 s | — | CANCEL |
| W4 | WORKING | ↑ short | t ≥ 30 s | Compute score | STATS |
| W5 | WORKING | ↓ long | t < 10 s | — | READY |
| W6 | WORKING | ↓ long | t ≥ 10 s | Same as W3 / W4 | STATS _or_ CANCEL |
| W7 | WORKING | timer 2 min | — | — | COVER |
| | | | | | |
| C1 | COVER | ↑ short | — | — | WORKING |
| C2 | COVER | idle 8s | — | — | SLEEP |
| | | | | | |
| S1 | STATS | ↑ short | — | Resume, keep accumulated time | WORKING |
| S2 | STATS | idle 8s | — | — | SLEEP |
| | | | | | |
| X1 | CANCEL | ↑ short | — | Resume, keep accumulated time | WORKING |
| X2 | CANCEL | idle 8s | — | — | SLEEP |
| | | | | | |
| G1 | SETTINGS | ↑ short | — | — | READY |
| G2 | SETTINGS | idle 8s | — | — | SLEEP |
| | | | | | |
| Z1 | SLEEP | any key | — | Restore `last_mode` | READY |

---

## 6. State Transition Diagram

```
                    ┌──────────┐
              any key│  SLEEP   │ idle 8s
            ┌───────│ (black)  │◄──────────────┐
            │       └──────────┘               │
            │                                  │
      ┌─────▼──────────┐                       │
      │     READY      │  ↓ long               │
      │                │──────────┐            │
      └──┬─────────────┘          │            │
         │ ↑ short                 ▼            │
         ▼                   ┌──────────┐       │
   ┌──────────────┐          │ SETTINGS │       │
   │   WORKING    │  2 min   │          │       │
   │              │─────────┐│ ↑→READY  │       │
   │ t<10: cycle  │        │└────┬─────┘       │
   │ t≥10: stop   │        │     │             │
   └──┬───┬───┬───┘        ▼     │             │
      │   │   │        ┌────────┐ │             │
      │   │   │        │ COVER  │ │             │
      │   │   │        └───┬────┘ │             │
      │   │   │       ↑→WORKING  │             │
      ▼   ▼   ▼            │     │             │
  ┌──────┐ ┌──────┐        │     │             │
  │CANCEL│ │STATS │        │     │             │
  │<30s  │ │≥30s  │        │     │             │
  └──┬───┘ └──┬───┘        │     │             │
     │        │            │     │             │
     └─↑short─┘            │     │             │
     →WORKING (accumulate) │     │             │
                           │     │             │
            idle 8s ───────┴─────┘             │
            (all non-WORKING states)           │
                                               │
            WORKING skips idle counter;         │
            will NOT auto-sleep.                │
                                               │
            idle 8s ──────────────────────────┘
            (READY / COVER / STATS / CANCEL / SETTINGS)
```

---

## 7. Key Mapping

| Keyboard | Maps To | Notes |
|----------|---------|-------|
| `↑` (Up Arrow) | Short press | — |
| `↓` (Down Arrow) | Long press | — |

---

## 8. Timing Rules

| Rule | Trigger | Action |
|------|---------|--------|
| Idle → SLEEP | 8 s no key press, state ≠ WORKING | Enter SLEEP |
| Cover reminder | `work_sec` reaches 120 | WORKING → COVER |
| Mode memory | A single mode runs > 10 s | Save to `last_mode` |
| Timer reset | READY → WORKING | `work_sec = 0` |
| Timer resume | STATS or CANCEL → WORKING | `work_sec` unchanged (accumulate) |
| Score | WORKING stopped at t ≥ 30 s | `score = 70 + t/4`, capped at 100 |

---

## 9. HZCL Modules Used

| Module | Purpose |
|--------|---------|
| `hz_fsm` | State machine — 7 states, 9 actions |
| `hz_screen` | Screen stack — 4 screens (ready, working, stats, sleep) |
| `hz_timer` | 1-second tick for work timer + idle countdown |

---

## 10. Out of Scope

Reference product features **not implemented** in this simulator:

- USB charge detection / charge screen
- Travel lock (flight mode)
- 6-axis sensor / wake-on-motion
- Wi-Fi provisioning
- Language switch (CN / EN)
- Factory reset
- Brush head replacement counter
- Pressure sensor feedback
- RLE image rendering
- Audible buzzer

---

## 11. Revision History

| Date | Change |
|------|--------|
| 2026-07-21 | Initial version — adapted from reference product for PC simulator |
