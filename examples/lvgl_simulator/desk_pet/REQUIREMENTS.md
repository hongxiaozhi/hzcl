# Pro Desk Pet — Requirements Document

## 1. Overview

Pro Desk Pet is an advanced desktop pet simulation built on **HZCL + LVGL v9 + SDL2**.
It serves as both a fun interactive demo and a comprehensive testbed for HZCL modules.

### 1.1 Objectives

- Exercise **9 of 12** HZCL modules (fsm, screen, event, timer, mode, sensor, alarm, config_mgr, power)
- Validate HZCL architecture in a realistic multi-page, multi-state application
- Identify bugs and improvement opportunities in HZCL source code
- Serve as a reference example for new HZCL users

### 1.2 Platform

- PC simulator: LVGL v9 + SDL2
- Build: CMake + MinGW GCC (MSYS2) or any GCC
- Resolution: 480 x 320 pixels

---

## 2. Screens

The app uses `hz_screen` stack-based navigation. Four screens:

### 2.1 Home Screen (root, never popped)

Main pet view. Layout (480x320):

```
+----------------------------------------------------------+
| [NORMAL]  Pro Desk Pet           [!Hungry]               |
|                                                          |
|       😊                                                    |
|    "Feeling great!"     Fullness ████████░░  80%          |
|                         Energy   ██████░░░░  60%          |
|                         Mood     █████████░  90%          |
|                         Hygiene  ████████░░  80%          |
|                                                          |
|  [Feed]  [Pet]  [Sleep]  [Clean]  [Train]  [Mode]       |
|                                                          |
|         [Stats]     [Settings]     [About]                |
+----------------------------------------------------------+
```

**Elements:**

| Element | Type | Details |
|---------|------|---------|
| Title | `lv_label` | "Pro Desk Pet", centered top |
| Mode badge | `lv_label` | e.g. "[NORMAL]", color-coded per mode, top-left |
| Alarm banner | `lv_label` | Red text, top-right, visible when alarm active |
| Pet emoji | `lv_label` | Font: montserrat_48, changes with FSM state |
| Mood text | `lv_label` | Below emoji, e.g. "Feeling great!" |
| 4 stat bars | `lv_label` + `lv_bar` | Right column: Fullness, Energy, Mood, Hygiene |
| 6 action buttons | `lv_btn` | Feed, Pet, Sleep, Clean, Train, Mode cycle |
| 3 nav buttons | `lv_btn` | Stats, Settings, About |

**State table for emoji and mood text:**

| FSM State | Emoji | Mood Text |
|-----------|-------|-----------|
| IDLE | 😐 | "Chillin..." |
| HAPPY | 😊 | "Feeling great!" |
| EATING | 😋 | "Yummy!" |
| SLEEPING | 😴 | "Zzz..." |
| SAD | 😢 | "So sad..." |
| SICK | 🤒 | "Not well..." |
| HYPER | 🤩 | "Awesome!" |
| TRAINING | 💪 | "Getting stronger!" |
| CLEANING | 🧹 | "Sparkling!" |

**Behavior:**
- on_load: create all LVGL widgets
- on_enter: show container (`lv_obj_clear_flag(cont, LV_OBJ_FLAG_HIDDEN)`)
- on_exit: hide container
- on_tick: update bars and emoji from pet_status_t data
- on_event: respond to EV_FSM_STATE_CHANGED, EV_ALARM_TRIGGERED, EV_ALARM_CLEARED, EV_MODE_CHANGED
- Action buttons call hz_event_publish(EV_USER_BASE + N) then hz_fsm_execute_action()
- Nav buttons call hz_screen_push()

### 2.2 Stats Screen

Pushed from Home. Shows detailed pet data.

```
+----------------------------------------------------------+
| [< Back]  Pet Statistics                   [Refresh]     |
+----------------------------------------------------------+
|  Fullness:  42/100  ████░░░░░░                            |
|  Energy:    78/100  ████████░░                            |
|  Mood:      65/100  ██████░░░░                            |
|  Hygiene:   90/100  █████████░                            |
|                                                          |
|  -- Recent History --                                     |
|  Mood:     ▁▂▃▄▅▆▇██▇▆   (lv_chart, 3 series)           |
|  Energy:   ▇▆▅▄▃▂▁▂▃▄▅                                    |
|                                                          |
|  -- Achievements --                                       |
|  ✓ Full Belly (fed 10 times)                              |
|  □ Happy Kid (mood>80 for 10 ticks, 5/10)                |
|  ✓ Night Walker (played at night)                         |
|                                                          |
|  -- Activity Log --                                       |
|  [14:23] Fed the pet                                      |
|  [14:10] Petted the pet                                   |
|  [13:55] Fell asleep                                      |
+----------------------------------------------------------+
```

**Elements:**
- Back button → `hz_screen_pop()`
- 4 detailed stat bars with numeric labels
- `lv_chart` type LINE, 3 series (Mood/Energy/Fullness), 10 data points, auto-scroll
- Achievement list: `lv_label` with ✓ (unlocked) / □ (locked) markers
- Activity log: circular buffer of 5 entries, displayed as text

**Behavior:**
- on_enter: populate chart from history buffer
- on_tick: push new data to chart every 3 ticks
- on_event: EV_FSM_ACTION_EXECUTED → add to activity log

### 2.3 Settings Screen

Pushed from Home. User-configurable parameters.

```
+----------------------------------------------------------+
| [< Back]  Settings                                        |
+----------------------------------------------------------+
|                                                          |
|  -- Decay Rates --                                        |
|  Hunger:  ◀━━━━━━●━━━━▶  1.2x   [lv_slider]              |
|  Energy:  ◀━━●━━━━━━━━▶  0.7x                            |
|  Mood:    ◀━━━━━━━●━▶  1.5x                              |
|                                                          |
|  -- Alarm Thresholds --                                   |
|  Hunger warn @ [ 75 ]  +/-                                |
|  Energy warn @ [ 20 ]  +/-                                |
|                                                          |
|  -- Theme --                                              |
|  Accent color:  [● Cyan] [● Blue] [● Red] [● Purple]    |
|                                                          |
|  -- System --                                             |
|  Auto-save:  [ON]  ├────●────┤  30s interval             |
|                                                          |
|  [Reset to Defaults]                                      |
+----------------------------------------------------------+
```

**Elements:**
- lv_slider for each decay rate (float 0.1-5.0, mapped to slider 0-100)
- lv_roller or lv_spinbox for thresholds (u8 0-100)
- Color preset buttons (4-5 preset colors)
- Auto-save toggle + interval display
- Reset button

**Behavior:**
- on_create: build UI
- on_enter: read current config values via hz_config_mgr_get()
- Slider change → config_set_wrapper() → hz_config_mgr_set() → EV_CONFIG_CHANGED
- Color button → set CFG_THEME_COLOR
- Reset → hz_config_mgr_reset() → EV_CONFIG_RESET

### 2.4 About Screen

Pushed from Home. Static information.

```
+----------------------------------------------------------+
| [< Back]  About / Help                                    |
+----------------------------------------------------------+
|                                                          |
|         🖥 Pro Desk Pet v1.0                               |
|                                                          |
|  -- Controls --                                           |
|  Feed    - Lowers hunger                                  |
|  Pet     - Boosts mood                                    |
|  Sleep   - Restores energy                                |
|  Clean   - Improves hygiene                               |
|  Train   - Active training mode                           |
|                                                          |
|  -- Tips --                                               |
|  Keep hunger < 50%                                        |
|  Keep energy > 30%                                        |
|  Interact regularly!                                      |
|  Focus mode slows decay                                   |
|                                                          |
|  -- HZCL Modules in Use --                                |
|  ✓ FSM  ✓ Screen  ✓ Event  ✓ Timer                        |
|  ✓ Mode  ✓ Sensor  ✓ Alarm  ✓ Config                     |
|  ✓ Power                                                  |
|                                                          |
|  Built with LVGL + SDL2 + MinGW                           |
+----------------------------------------------------------+
```

---

## 3. Finite State Machine

### 3.1 State IDs

```c
PET_STATE_IDLE       = 0
PET_STATE_HAPPY      = 1
PET_STATE_EATING     = 2
PET_STATE_SLEEPING   = 3
PET_STATE_SAD        = 4
PET_STATE_SICK       = 5
PET_STATE_HYPER      = 6
PET_STATE_TRAINING   = 7
PET_STATE_CLEANING   = 8
```

### 3.2 Action IDs

```c
ACT_FEED     = 1
ACT_PET      = 2
ACT_GO_SLEEP = 3
ACT_TICK     = 4  // Internal: 1-second status update
ACT_WAKE     = 5
ACT_CLEAN    = 6
ACT_TRAIN    = 7
ACT_STOP     = 8  // Stop training/cleaning
```

### 3.3 Per-State Action Tables

#### IDLE
| Action | Target State | Effect |
|--------|-------------|--------|
| FEED | EATING | Hunger=0, Mood+=10 |
| PET | HAPPY | Mood=100, start happy counter |
| GO_SLEEP | SLEEPING | Enter sleep |
| CLEAN | CLEANING | Start cleaning |
| TRAIN | TRAINING | Start training |
| TICK | (none) | Standard decay |

#### HAPPY
| Action | Target State | Effect |
|--------|-------------|--------|
| FEED | EATING | Eat while happy |
| PET | HYPER | Consecutive pet → hyper |
| GO_SLEEP | SLEEPING | Sleep |
| TRAIN | TRAINING | Train |
| TICK | (none) | Auto→IDLE after 4 ticks |

#### EATING
| Action | Target State | Effect |
|--------|-------------|--------|
| TICK | (none) | Auto→IDLE after 4 ticks |

#### SLEEPING
| Action | Target State | Effect |
|--------|-------------|--------|
| PET | IDLE | Wake up (pet to wake) |
| WAKE | IDLE | Wake up |
| TICK | (none) | Auto→IDLE when energy >= 100 |

#### SAD
| Action | Target State | Effect |
|--------|-------------|--------|
| FEED | EATING | Comfort feed |
| PET | HAPPY | Comfort pet |
| CLEAN | CLEANING | Distract with cleaning |
| TICK | (none) | Auto→SICK after 8 ticks |

#### SICK
| Action | Target State | Effect |
|--------|-------------|--------|
| FEED | EATING | Feed medicine |
| PET | HAPPY | Comfort |
| CLEAN | CLEANING | Clean to help recovery |
| TICK | (none) | Faster decay, triggers alarm |

#### HYPER
| Action | Target State | Effect |
|--------|-------------|--------|
| FEED | EATING | Eat |
| PET | HAPPY | Reset to happy |
| TRAIN | TRAINING | Train |
| TICK | (none) | Auto→IDLE when energy/mood drops |

#### TRAINING
| Action | Target State | Effect |
|--------|-------------|--------|
| STOP | IDLE | Manual stop |
| TICK | (none) | Auto→IDLE after 5 ticks |

#### CLEANING
| Action | Target State | Effect |
|--------|-------------|--------|
| TICK | (none) | Auto→IDLE after 2 ticks, hygiene=100 |

### 3.4 Auto-Transition Logic (in pet_status_tick)

Called via ACT_TICK handler every 1 second.

```
1. Apply stat decay/growth based on current state and mode factors:
   - SLEEPING:   energy += 5, hunger += 1
   - EATING:     hunger = 0 then += 2, energy += 2, counter→IDLE at 4
   - TRAINING:   energy -= 3, hunger += 3, mood += 2, counter→IDLE at 5
   - CLEANING:   hygiene = 100, counter→IDLE at 2
   - SICK:       hunger += 3, energy -= 3, mood -= 3
   - IDLE/HAPPY/HYPER/SAD: hunger += 1, energy -= 1

2. Clamp all values to [0, 100]

3. Passive mood adjustment:
   - If NOT sick and (hunger > 70 or energy < 20): mood -= 2
   - If HAPPY or HYPER: mood += 1

4. Hygiene passive decay: hygiene -= 1 (if not CLEANING)

5. Auto state transitions:
   - HAPPY → IDLE after 4 ticks
   - IDLE + (hunger > 80 or mood < 15) → SAD
   - SAD + duration > 8 → SICK
   - IDLE/SAD/SICK + energy <= 5 → SLEEPING
   - SLEEPING + energy >= 100 → IDLE
   - IDLE + mood > 85 + energy > 80 → HYPER
   - HYPER + (energy < 50 or mood < 50) → IDLE

6. Alarm checks:
   - hunger > 75 → ALARM_HUNGER_HIGH
   - energy < 20 → ALARM_ENERGY_LOW
```

---

## 4. Modes

### 4.1 Mode IDs & Parameters

```c
MODE_NORMAL  = 0  // hunger=1.0x, energy=1.0x, mood=1.0x  (blue theme)
MODE_FOCUS   = 1  // hunger=0.5x, energy=0.7x, mood=0.8x  (calm blue-grey)
MODE_PLAY    = 2  // hunger=1.5x, energy=1.5x, mood=1.2x  (warm orange)
MODE_NIGHT   = 3  // hunger=0.3x, energy=0.3x, mood=1.0x  (dark purple)
```

### 4.2 Mode Callbacks

Each mode's on_enter:
- Updates `s_mode_factors` (hunger_decay, energy_decay, mood_decay)
- Calls `apply_theme(bg_color, accent_color)` to update global theme struct
- Theme colors are used by Home screen on EV_MODE_CHANGED

### 4.3 Mode Switching

- Mode cycle button on Home screen: `hz_mode_switch((current + 1) % 4)`
- hz_mode_tick() handles transitions
- EV_MODE_CHANGED published on completion → Home screen updates colors

---

## 5. Sensors

### 5.1 Wellness Sensor (virtual)

| Property | Value |
|----------|-------|
| Type | HZ_SENSOR_TYPE_USER |
| Interval | 3000 ms |
| Filter | Moving average, window=5 |
| Raw value | `rand() % 100` |
| Threshold upper | 80.0 (good wellness → mood boost) |
| Threshold lower | 20.0 (bad wellness → mood penalty) |

**On EV_SENSOR_THRESHOLD:**
- Exceeded upper: `s_pet.mood = MIN(100, s_pet.mood + 3)`
- Exceeded lower: `s_pet.mood = MAX(0, s_pet.mood - 3)`

### 5.2 Activity Sensor (virtual)

| Property | Value |
|----------|-------|
| Type | HZ_SENSOR_TYPE_USER |
| Interval | 5000 ms |
| Filter | Moving average, window=3 |
| Raw value | Activity counter (reset on read) |
| Threshold lower | 0.5 (low activity → alarm) |

**Activity counter:** Incremented on every user action (btn callbacks call `app_notify_activity()`).

**On EV_SENSOR_THRESHOLD (lower exceeded):**
- If current state is IDLE: trigger ALARM_LOW_ACTIVITY

---

## 6. Alarms

| ID | Name | Level | Auto-recover | Display timeout |
|----|------|-------|-------------|-----------------|
| 0 | HUNGER_HIGH | WARNING | yes | 5000 ms |
| 1 | ENERGY_LOW | WARNING | yes | 5000 ms |
| 2 | SICK_PROLONGED | CRITICAL | no | 30000 ms |
| 3 | ACHIEVEMENT_UNLOCK | INFO | yes | 3000 ms |
| 4 | MODE_AUTO_SWITCH | INFO | yes | 2000 ms |
| 5 | LOW_ACTIVITY | INFO | yes | 4000 ms |

**Trigger conditions:**
- HUNGER_HIGH: `s_pet.hunger > 75` (checked every tick)
- ENERGY_LOW: `s_pet.energy < 20` (checked every tick)
- SICK_PROLONGED: sick_duration > 10 ticks (checked in pet_status_tick)
- ACHIEVEMENT_UNLOCK: when achievement conditions met
- LOW_ACTIVITY: activity sensor threshold exceeded

---

## 7. Configuration Parameters

| ID | Name | Type | Default | Validator |
|----|------|------|---------|-----------|
| 1 | hunger_decay | float | 1.0 | [0.1, 5.0] |
| 2 | energy_decay | float | 1.0 | [0.1, 5.0] |
| 3 | mood_decay | float | 1.0 | [0.1, 5.0] |
| 4 | hunger_thresh | u8 | 75 | [0, 100] |
| 5 | energy_thresh | u8 | 20 | [0, 100] |
| 6 | theme_color | u32 | 0x00D2FF | none |
| 7 | auto_save_ms | u16 | 30000 | [5000, 120000] |

**Implementation note:** `hz_config_mgr_set()` currently validates but doesn't store runtime values (TODOs in hz_config_mgr.c). The demo will use a parallel runtime cache (`app_config_t s_app_cfg`) that is synced with hz_config_mgr_set/get via a wrapper function.

---

## 8. Event Map

### 8.1 Subscriptions

| Event | Subscriber | Where Registered | Purpose |
|-------|-----------|-----------------|---------|
| EV_FSM_STATE_CHANGED | screen_home | on_load | Update emoji, mood text, stat bars |
| EV_FSM_ACTION_EXECUTED | screen_stats | on_load | Add to activity log |
| EV_SENSOR_THRESHOLD | global cb | main startup | Affect mood, trigger alarms |
| EV_MODE_CHANGED | screen_home | on_load | Update theme colors, mode badge |
| EV_ALARM_TRIGGERED | screen_home | on_load | Show alarm banner |
| EV_ALARM_CLEARED | screen_home | on_load | Hide alarm banner |
| EV_CONFIG_CHANGED | global cb | main startup | Reload config cache |
| EV_POWER_SLEEP | global cb | main startup | Dim display |
| EV_POWER_WAKEUP | global cb | main startup | Restore display |
| EV_SCREEN_PUSHED | global cb | main startup | Reset power idle timer |
| EV_SCREEN_POPPED | global cb | main startup | Reset power idle timer |

### 8.2 Published Events

| Event | Publisher | Payload |
|-------|-----------|---------|
| EV_USER_BASE + 0..6 | Action button callbacks | NULL |
| EV_FSM_STATE_CHANGED | hz_fsm internally | (state data) |
| EV_FSM_ACTION_EXECUTED | hz_fsm internally | (action data) |
| EV_SENSOR_THRESHOLD | hz_sensor internally | hz_sensor_threshold_event_t* |
| EV_MODE_CHANGED | hz_mode internally | (mode data) |
| EV_ALARM_TRIGGERED | hz_alarm internally | alarm_id (as void*) |
| EV_CONFIG_CHANGED | config_set_wrapper | config_id (as void*) |

---

## 9. Data Structures

### 9.1 Pet Status

```c
typedef struct {
    int32_t hunger;    // 0-100
    int32_t energy;    // 0-100
    int32_t mood;      // 0-100
    int32_t hygiene;   // 0-100
} pet_status_t;
```

### 9.2 Mode Factors

```c
typedef struct {
    float hunger_decay;   // multiplier for hunger increase
    float energy_decay;   // multiplier for energy decrease
    float mood_decay;     // multiplier for mood change
} mode_factors_t;
```

### 9.3 Config Cache

```c
typedef struct {
    float  hunger_decay;
    float  energy_decay;
    float  mood_decay;
    hz_u8  hunger_thresh;
    hz_u8  energy_thresh;
    hz_u32 theme_color;
    hz_u16 auto_save_ms;
} app_config_t;
```

### 9.4 State Counters

```c
typedef struct {
    int happy_duration;     // ticks in HAPPY
    int eating_duration;    // ticks in EATING
    int training_duration;  // ticks in TRAINING
    int cleaning_duration;  // ticks in CLEANING
    int sad_duration;       // ticks in SAD (cumulative)
    int sick_duration;      // ticks in SICK (cumulative)
} pet_state_counters_t;
```

### 9.5 Activity Log

```c
#define ACTIVITY_LOG_SIZE 5
typedef struct {
    char entries[ACTIVITY_LOG_SIZE][32];  // circular buffer
    hz_u8 head;
    hz_u8 count;
} activity_log_t;
```

### 9.6 Chart History

```c
#define CHART_DATA_POINTS 10
typedef struct {
    int32_t mood[CHART_DATA_POINTS];
    int32_t energy[CHART_DATA_POINTS];
    int32_t fullness[CHART_DATA_POINTS];  // 100 - hunger
    hz_u8 index;
} chart_history_t;
```

---

## 10. File Structure

```
desk_pet/
├── REQUIREMENTS.md              # This document
├── CMakeLists.txt               # Updated with screen sources
├── inc/
│   └── lv_conf.h                # Unchanged
├── src/
│   ├── app.h                    # Shared types, enums, data structs
│   ├── main.c                   # Rewrite: init + main loop + FSM + sensors + modes
│   ├── hz_platform_sdl2.c       # Unchanged
│   └── screens/
│       ├── screen_home.c        # Home screen UI + event handlers
│       ├── screen_stats.c       # Stats/achievements/log
│       ├── screen_settings.c    # Config sliders/spinners
│       └── screen_about.c       # Help/about info
```

---

## 11. Implementation Order

| Phase | Files | Description |
|-------|-------|-------------|
| 1 | app.h, main.c, CMakeLists.txt | Foundation: shared types, module init, basic FSM (5 states), single-screen UI |
| 2 | main.c | Extended states: SICK, HYPER, TRAINING, CLEANING + all transitions |
| 3 | main.c | Modes (4) + virtual sensors (2) + event subscriptions |
| 4 | main.c | Alarms (6) + config (7 params) + runtime cache + auto-save |
| 5 | screen_*.c, main.c, CMakeLists.txt | 4 screen descriptors with navigation + screen lifecycle |
| 6 | screen_home.c | Full home screen UI with all elements + event handlers |
| 7 | screen_stats.c, screen_settings.c, screen_about.c | Remaining screens complete |
| 8 | All | Polish: edge cases, achievements, HZCL issue fixes |

---

## 12. HZCL Optimization Targets

During development, these HZCL source files may need fixes:

- `src/hz_config_mgr.c` — Implement runtime value storage in hz_config_mgr_set()
- Any other bugs or API gaps discovered during development

All HZCL fixes will be made directly in the library source (not worked around in demo code) and documented.
