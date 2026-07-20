/**
 * @file main.c
 * @brief Pro Desk Pet — comprehensive HZCL demo application
 *
 * Exercises: hz_fsm, hz_screen, hz_event, hz_timer, hz_mode,
 *            hz_sensor, hz_alarm, hz_config_mgr, hz_power
 *
 * Build:
 *   cd examples/lvgl_simulator/desk_pet
 *   mkdir build && cd build
 *   cmake .. -G "MinGW Makefiles"
 *   mingw32-make -j8
 *   ./desk_pet.exe
 */

#include "lvgl/lvgl.h"
#include "hz_config.h"
#include "hz_types.h"
#include "app.h"

#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_timer.h"
#include "hz_screen.h"
#include "hz_mode.h"
#include "hz_sensor.h"
#include "hz_sensor_filter.h"
#include "hz_alarm.h"
#include "hz_config_mgr.h"
#include "hz_power.h"
#include "hz_platform.h"

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*===========================================================================
 * Display / Constants
 *===========================================================================*/
#define SCREEN_HOR_RES      480
#define SCREEN_VER_RES      320
#define TICK_INTERVAL_MS   1000

/*===========================================================================
 * Global Data (extern'd in app.h)
 *===========================================================================*/
pet_status_t          g_pet          = { 30, 80, 60, 100 };
pet_state_counters_t  g_counters     = { 0 };
mode_factors_t        g_mode_factors = { 1.0f, 1.0f, 1.0f };
theme_colors_t        g_theme        = { 0x1a1a2e, 0x00d2ff };
app_config_t          g_app_cfg;
activity_log_t        g_activity_log = { 0 };
chart_history_t       g_chart_history = { 0 };
achievement_flags_t   g_achievements = { 0 };

const char *g_state_emojis[PET_STATE_COUNT] = {
    "\xF0\x9F\x98\x90",  /* IDLE     */
    "\xF0\x9F\x98\x8A",  /* HAPPY    */
    "\xF0\x9F\x98\x8B",  /* EATING   */
    "\xF0\x9F\x98\xB4",  /* SLEEPING */
    "\xF0\x9F\x98\xA2",  /* SAD      */
    "\xF0\x9F\xA4\x92",  /* SICK     */
    "\xF0\x9F\xA4\xA9",  /* HYPER    */
    "\xF0\x9F\x92\xAA",  /* TRAINING */
    "\xF0\x9F\xA7\xB9",  /* CLEANING */
};

const char *g_state_moods[PET_STATE_COUNT] = {
    "Chillin...", "Feeling great!", "Yummy!", "Zzz...",
    "So sad...", "Not well...", "Awesome!", "Getting stronger!", "Sparkling!",
};

/*===========================================================================
 * Screen factory declarations (implemented in screens/*.c)
 *===========================================================================*/
void screen_home_create(void);
hz_screen_t *screen_home_get(void);
void screen_stats_create(void);
hz_screen_t *screen_stats_get(void);
void screen_settings_create(void);
hz_screen_t *screen_settings_get(void);
void screen_about_create(void);
hz_screen_t *screen_about_get(void);

/*===========================================================================
 * Forward Declarations
 *===========================================================================*/
static void pet_status_tick(void);

/*===========================================================================
 * Action Handlers
 *===========================================================================*/
static hz_err_t act_feed_handler(void *arg)
{
    (void)arg;
    g_pet.hunger = 0;
    g_pet.mood += 10;
    if (g_pet.mood > 100) g_pet.mood = 100;
    g_counters.eating_duration = 0;
    app_notify_activity();
    app_log_action("Fed the pet");
    return HZ_OK;
}

static hz_err_t act_pet_handler(void *arg)
{
    (void)arg;
    g_pet.mood = 100;
    g_counters.happy_duration = 0;
    app_notify_activity();
    app_log_action("Petted the pet");
    return HZ_OK;
}

static hz_err_t act_sleep_handler(void *arg)   { (void)arg; app_log_action("Fell asleep"); return HZ_OK; }
static hz_err_t act_wake_handler(void *arg)    { (void)arg; app_log_action("Woke up"); return HZ_OK; }
static hz_err_t act_clean_handler(void *arg)
{
    (void)arg;
    g_counters.cleaning_duration = 0;
    app_notify_activity();
    app_log_action("Cleaned the pet");
    return HZ_OK;
}
static hz_err_t act_train_handler(void *arg)
{
    (void)arg;
    g_counters.training_duration = 0;
    app_notify_activity();
    app_log_action("Started training");
    return HZ_OK;
}
static hz_err_t act_stop_handler(void *arg)
{
    (void)arg;
    app_log_action("Stopped activity");
    return HZ_OK;
}
static hz_err_t act_tick_handler(void *arg) { (void)arg; pet_status_tick(); return HZ_OK; }

/*===========================================================================
 * Per-State Action Tables
 *===========================================================================*/
static const hz_action_t s_actions_idle[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING   },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_GO_SLEEP, "go_sleep", act_sleep_handler, PET_STATE_SLEEPING },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_happy[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HYPER },
    { ACT_GO_SLEEP, "go_sleep", act_sleep_handler, PET_STATE_SLEEPING },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_eating[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sleeping[] = {
    { ACT_PET, "pet", act_pet_handler, PET_STATE_IDLE },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sad[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sick[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_hyper[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_training[] = {
    { ACT_STOP, "stop", act_stop_handler, PET_STATE_IDLE },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_cleaning[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};

/*===========================================================================
 * State Entry Callbacks
 *===========================================================================*/
static void on_idle_enter(hz_state_t from)   { (void)from; }
static void on_happy_enter(hz_state_t from)  { (void)from; g_counters.happy_duration = 0; }
static void on_eating_enter(hz_state_t from) { (void)from; g_counters.eating_duration = 0; }
static void on_sleep_enter(hz_state_t from)  { (void)from; }
static void on_sad_enter(hz_state_t from)    { (void)from; }
static void on_sick_enter(hz_state_t from)   { (void)from; }
static void on_hyper_enter(hz_state_t from)  { (void)from; }
static void on_train_enter(hz_state_t from)  { (void)from; g_counters.training_duration = 0; }
static void on_clean_enter(hz_state_t from)  { (void)from; g_counters.cleaning_duration = 0; }

/*===========================================================================
 * FSM State Table
 *===========================================================================*/
static const hz_state_node_t s_states[] = {
    { .id = PET_STATE_IDLE,     .parent = 0xFF, .name = "idle",     .on_enter = on_idle_enter,  .allowed_actions = s_actions_idle     },
    { .id = PET_STATE_HAPPY,    .parent = 0xFF, .name = "happy",    .on_enter = on_happy_enter, .allowed_actions = s_actions_happy    },
    { .id = PET_STATE_EATING,   .parent = 0xFF, .name = "eating",   .on_enter = on_eating_enter,.allowed_actions = s_actions_eating   },
    { .id = PET_STATE_SLEEPING, .parent = 0xFF, .name = "sleeping", .on_enter = on_sleep_enter, .allowed_actions = s_actions_sleeping },
    { .id = PET_STATE_SAD,      .parent = 0xFF, .name = "sad",      .on_enter = on_sad_enter,   .allowed_actions = s_actions_sad      },
    { .id = PET_STATE_SICK,     .parent = 0xFF, .name = "sick",     .on_enter = on_sick_enter,  .allowed_actions = s_actions_sick     },
    { .id = PET_STATE_HYPER,    .parent = 0xFF, .name = "hyper",    .on_enter = on_hyper_enter, .allowed_actions = s_actions_hyper    },
    { .id = PET_STATE_TRAINING, .parent = 0xFF, .name = "training", .on_enter = on_train_enter, .allowed_actions = s_actions_training },
    { .id = PET_STATE_CLEANING, .parent = 0xFF, .name = "cleaning", .on_enter = on_clean_enter, .allowed_actions = s_actions_cleaning },
};

/*===========================================================================
 * Pet Status Simulation
 *===========================================================================*/
static void pet_status_tick(void)
{
    hz_state_t cur = hz_fsm_current();

    switch (cur) {
    case PET_STATE_SLEEPING:
        g_pet.energy  += (int32_t)(5 * g_mode_factors.energy_decay);
        g_pet.hunger  += (int32_t)(1 * g_mode_factors.hunger_decay);
        break;
    case PET_STATE_EATING:
        g_pet.hunger  += (int32_t)(2 * g_mode_factors.hunger_decay);
        g_pet.energy  += (int32_t)(2 * g_mode_factors.energy_decay);
        if (++g_counters.eating_duration >= 4) hz_fsm_transition(PET_STATE_IDLE);
        break;
    case PET_STATE_TRAINING:
        g_pet.energy  -= (int32_t)(3 * g_mode_factors.energy_decay);
        g_pet.hunger  += (int32_t)(3 * g_mode_factors.hunger_decay);
        g_pet.mood     = HZ_CLAMP(g_pet.mood + 2, 0, 100);
        if (++g_counters.training_duration >= 5) hz_fsm_transition(PET_STATE_IDLE);
        break;
    case PET_STATE_CLEANING:
        g_pet.hygiene = 100;
        if (++g_counters.cleaning_duration >= 2) hz_fsm_transition(PET_STATE_IDLE);
        break;
    case PET_STATE_SICK:
        g_pet.hunger  += (int32_t)(3 * g_mode_factors.hunger_decay);
        g_pet.energy  -= (int32_t)(3 * g_mode_factors.energy_decay);
        g_pet.mood    -= (int32_t)(3 * g_mode_factors.mood_decay);
        g_counters.sick_duration++;
        if (g_counters.sick_duration > 10) hz_alarm_trigger(ALARM_SICK_PROLONGED);
        break;
    default:
        g_pet.hunger  += (int32_t)(1 * g_mode_factors.hunger_decay);
        g_pet.energy  -= (int32_t)(1 * g_mode_factors.energy_decay);
        break;
    }

    g_pet.hunger  = HZ_CLAMP(g_pet.hunger,  0, 100);
    g_pet.energy  = HZ_CLAMP(g_pet.energy,  0, 100);
    g_pet.mood    = HZ_CLAMP(g_pet.mood,    0, 100);
    g_pet.hygiene = HZ_CLAMP(g_pet.hygiene, 0, 100);

    if (cur != PET_STATE_SICK) {
        if (g_pet.hunger > 70 || g_pet.energy < 20)
            g_pet.mood -= (int32_t)(2 * g_mode_factors.mood_decay);
        else if (cur == PET_STATE_HAPPY || cur == PET_STATE_HYPER)
            g_pet.mood += (int32_t)(1 * g_mode_factors.mood_decay);
    }
    if (cur != PET_STATE_CLEANING) g_pet.hygiene -= 1;

    /* Auto transitions */
    if (cur == PET_STATE_HAPPY && ++g_counters.happy_duration >= 4)
        hz_fsm_transition(PET_STATE_IDLE);
    if (cur == PET_STATE_IDLE && (g_pet.hunger > 80 || g_pet.mood < 15))
        hz_fsm_transition(PET_STATE_SAD);
    if (cur == PET_STATE_SAD && ++g_counters.sad_duration > 8)
        hz_fsm_transition(PET_STATE_SICK);
    if (cur != PET_STATE_SAD) g_counters.sad_duration = 0;
    if ((cur == PET_STATE_IDLE || cur == PET_STATE_SAD || cur == PET_STATE_SICK) && g_pet.energy <= 5)
        hz_fsm_transition(PET_STATE_SLEEPING);
    if (cur == PET_STATE_SLEEPING && g_pet.energy >= 100)
        hz_fsm_transition(PET_STATE_IDLE);
    if (cur == PET_STATE_IDLE && g_pet.mood > 85 && g_pet.energy > 80)
        hz_fsm_transition(PET_STATE_HYPER);
    if (cur == PET_STATE_HYPER && (g_pet.energy < 50 || g_pet.mood < 50))
        hz_fsm_transition(PET_STATE_IDLE);

    /* Alarms */
    if (g_pet.hunger > g_app_cfg.hunger_thresh) hz_alarm_trigger(ALARM_HUNGER_HIGH);
    else hz_alarm_clear(ALARM_HUNGER_HIGH);
    if (g_pet.energy < g_app_cfg.energy_thresh) hz_alarm_trigger(ALARM_ENERGY_LOW);
    else hz_alarm_clear(ALARM_ENERGY_LOW);

    /* Chart history */
    g_chart_history.mood[g_chart_history.index]     = g_pet.mood;
    g_chart_history.energy[g_chart_history.index]   = g_pet.energy;
    g_chart_history.fullness[g_chart_history.index] = 100 - g_pet.hunger;
    g_chart_history.index = (g_chart_history.index + 1) % CHART_DATA_POINTS;

    app_check_achievements();
    g_pet.mood = HZ_CLAMP(g_pet.mood, 0, 100);
}

/*===========================================================================
 * HZCL Timers
 *===========================================================================*/
static hz_timer_t s_pet_timer;
static hz_timer_t s_auto_save_timer;

static void pet_timer_cb(void *arg)
{
    (void)arg;
    hz_fsm_execute_action(ACT_TICK, NULL);
    /* Notify screens via event bus — hz_fsm internally publishes EV_FSM_STATE_CHANGED */
}

static void auto_save_cb(void *arg)
{
    (void)arg;
    hz_config_mgr_save();
}

/*===========================================================================
 * Event Subscribers
 *===========================================================================*/
static void on_sensor_threshold(void *data, void *user)
{
    (void)user;
    hz_sensor_threshold_event_t *evt = (hz_sensor_threshold_event_t *)data;
    if (evt->channel_id == SENSOR_CH_WELLNESS) {
        if (evt->direction == 0)
            g_pet.mood = HZ_MIN(100, g_pet.mood + 3);
        else
            g_pet.mood = HZ_MAX(0, g_pet.mood - 3);
    }
    if (evt->channel_id == SENSOR_CH_ACTIVITY && evt->direction == 1) {
        if (hz_fsm_current() == PET_STATE_IDLE)
            hz_alarm_trigger(ALARM_LOW_ACTIVITY);
    }
}

static void on_config_changed(void *data, void *user)
{
    (void)user;
    hz_config_id_t id = (hz_config_id_t)(hz_u32)data;
    switch (id) {
    case CFG_HUNGER_DECAY:  hz_config_mgr_get(CFG_HUNGER_DECAY, &g_app_cfg.hunger_decay); break;
    case CFG_ENERGY_DECAY:  hz_config_mgr_get(CFG_ENERGY_DECAY, &g_app_cfg.energy_decay); break;
    case CFG_MOOD_DECAY:    hz_config_mgr_get(CFG_MOOD_DECAY, &g_app_cfg.mood_decay); break;
    case CFG_HUNGER_THRESH: hz_config_mgr_get(CFG_HUNGER_THRESH, &g_app_cfg.hunger_thresh); break;
    case CFG_ENERGY_THRESH: hz_config_mgr_get(CFG_ENERGY_THRESH, &g_app_cfg.energy_thresh); break;
    case CFG_THEME_COLOR:   hz_config_mgr_get(CFG_THEME_COLOR, &g_app_cfg.theme_color); break;
    case CFG_AUTO_SAVE_MS:  hz_config_mgr_get(CFG_AUTO_SAVE_MS, &g_app_cfg.auto_save_ms); break;
    }
}

/*===========================================================================
 * Virtual Sensor Drivers
 *===========================================================================*/
static hz_err_t wellness_init(void *param)        { (void)param; return HZ_OK; }
static hz_err_t wellness_deinit(void)             { return HZ_OK; }
static hz_err_t wellness_read_raw(hz_s32 *raw)    { *raw = (hz_s32)(rand() % 100); return HZ_OK; }
static float    wellness_raw_to_physical(hz_s32 raw) { return (float)raw; }
static hz_err_t wellness_self_check(void)         { return HZ_OK; }

static hz_sensor_driver_t s_wellness_driver = {
    .name = "wellness", .type = HZ_SENSOR_TYPE_USER,
    .init = wellness_init, .deinit = wellness_deinit,
    .read_raw = wellness_read_raw, .raw_to_physical = wellness_raw_to_physical,
    .self_check = wellness_self_check,
};

static int s_activity_counter = 0;
void app_notify_activity(void) { s_activity_counter++; }

static hz_err_t activity_init(void *param)        { (void)param; return HZ_OK; }
static hz_err_t activity_deinit(void)             { return HZ_OK; }
static hz_err_t activity_read_raw(hz_s32 *raw)    { *raw = (hz_s32)s_activity_counter; s_activity_counter = 0; return HZ_OK; }
static float    activity_raw_to_physical(hz_s32 raw) { return (float)raw; }
static hz_err_t activity_self_check(void)         { return HZ_OK; }

static hz_sensor_driver_t s_activity_driver = {
    .name = "activity", .type = HZ_SENSOR_TYPE_USER,
    .init = activity_init, .deinit = activity_deinit,
    .read_raw = activity_read_raw, .raw_to_physical = activity_raw_to_physical,
    .self_check = activity_self_check,
};

static hz_sensor_channel_id_t s_ch_ids[SENSOR_CH_COUNT];

static void register_sensors(void)
{
    hz_sensor_channel_cfg_t wellness_cfg = {
        .driver = &s_wellness_driver, .interval_ms = 3000,
        .filter_type = HZ_FILTER_MOVING_AVG, .filter_window = 5,
        .threshold_upper = 80.0f, .threshold_lower = 20.0f,
    };
    hz_sensor_channel_register(&wellness_cfg, &s_ch_ids[SENSOR_CH_WELLNESS]);

    hz_sensor_channel_cfg_t activity_cfg = {
        .driver = &s_activity_driver, .interval_ms = 5000,
        .filter_type = HZ_FILTER_MOVING_AVG, .filter_window = 3,
        .threshold_upper = 0, .threshold_lower = 0.5f,
    };
    hz_sensor_channel_register(&activity_cfg, &s_ch_ids[SENSOR_CH_ACTIVITY]);
}

/*===========================================================================
 * Mode Definitions
 *===========================================================================*/
static void mode_enter_normal(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 1.0f; g_mode_factors.energy_decay = 1.0f; g_mode_factors.mood_decay = 1.0f;
    g_theme.bg_color = 0x1a1a2e; g_theme.accent_color = 0x00d2ff;
}
static void mode_enter_focus(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 0.5f; g_mode_factors.energy_decay = 0.7f; g_mode_factors.mood_decay = 0.8f;
    g_theme.bg_color = 0x2d3436; g_theme.accent_color = 0x74b9ff;
}
static void mode_enter_play(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 1.5f; g_mode_factors.energy_decay = 1.5f; g_mode_factors.mood_decay = 1.2f;
    g_theme.bg_color = 0x2d1b1b; g_theme.accent_color = 0xe17055;
}
static void mode_enter_night(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 0.3f; g_mode_factors.energy_decay = 0.3f; g_mode_factors.mood_decay = 1.0f;
    g_theme.bg_color = 0x0a0a1a; g_theme.accent_color = 0x6c5ce7;
}

static const hz_mode_param_t s_params_normal[] = { { 0, 1.0f }, { 1, 1.0f }, { 2, 1.0f } };
static const hz_mode_param_t s_params_focus[]  = { { 0, 0.5f }, { 1, 0.7f }, { 2, 0.8f } };
static const hz_mode_param_t s_params_play[]   = { { 0, 1.5f }, { 1, 1.5f }, { 2, 1.2f } };
static const hz_mode_param_t s_params_night[]  = { { 0, 0.3f }, { 1, 0.3f }, { 2, 1.0f } };

static const hz_mode_t s_modes[] = {
    { .id = MODE_NORMAL, .name = "NORMAL", .transition_ms = 0, .on_enter = mode_enter_normal, .params = s_params_normal, .param_count = 3 },
    { .id = MODE_FOCUS,  .name = "FOCUS",  .transition_ms = 0, .on_enter = mode_enter_focus,  .params = s_params_focus,  .param_count = 3 },
    { .id = MODE_PLAY,   .name = "PLAY",   .transition_ms = 0, .on_enter = mode_enter_play,   .params = s_params_play,   .param_count = 3 },
    { .id = MODE_NIGHT,  .name = "NIGHT",  .transition_ms = 0, .on_enter = mode_enter_night,  .params = s_params_night,  .param_count = 3 },
};

/*===========================================================================
 * Alarm Definitions
 *===========================================================================*/
static const hz_alarm_cfg_t s_alarm_defs[] = {
    { .id = ALARM_HUNGER_HIGH,        .name = "HUNGER_HIGH",        .level = HZ_ALARM_WARNING,  .display_timeout_ms = 5000,  .auto_recover = true,  .recover_delay_ms = 5000  },
    { .id = ALARM_ENERGY_LOW,         .name = "ENERGY_LOW",         .level = HZ_ALARM_WARNING,  .display_timeout_ms = 5000,  .auto_recover = true,  .recover_delay_ms = 5000  },
    { .id = ALARM_SICK_PROLONGED,     .name = "SICK_PROLONGED",     .level = HZ_ALARM_CRITICAL, .display_timeout_ms = 0,     .auto_recover = false, .recover_delay_ms = 0     },
    { .id = ALARM_ACHIEVEMENT_UNLOCK, .name = "ACHIEVEMENT_UNLOCK", .level = HZ_ALARM_INFO,     .display_timeout_ms = 3000,  .auto_recover = true,  .recover_delay_ms = 3000  },
    { .id = ALARM_MODE_AUTO_SWITCH,   .name = "MODE_AUTO_SWITCH",   .level = HZ_ALARM_INFO,     .display_timeout_ms = 2000,  .auto_recover = true,  .recover_delay_ms = 2000  },
    { .id = ALARM_LOW_ACTIVITY,       .name = "LOW_ACTIVITY",       .level = HZ_ALARM_INFO,     .display_timeout_ms = 4000,  .auto_recover = true,  .recover_delay_ms = 4000  },
};

/*===========================================================================
 * Config Items
 *===========================================================================*/
static float   s_def_hunger_decay  = 1.0f;
static float   s_def_energy_decay  = 1.0f;
static float   s_def_mood_decay    = 1.0f;
static hz_u8   s_def_hunger_thresh = 75;
static hz_u8   s_def_energy_thresh = 20;
static hz_u32  s_def_theme_color   = 0x00D2FF;
static hz_u16  s_def_auto_save_ms  = 30000;

static hz_bool_t validate_decay(const void *val) {
    float f = *(const float *)val; return (f >= 0.1f && f <= 5.0f) ? 1 : 0;
}
static hz_bool_t validate_thresh_u8(const void *val) {
    hz_u8 v = *(const hz_u8 *)val; return (v <= 100) ? 1 : 0;
}

static const hz_config_item_t s_config_items[] = {
    { .id = CFG_HUNGER_DECAY,  .name = "hunger_decay",  .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_hunger_decay,  .validator = validate_decay     },
    { .id = CFG_ENERGY_DECAY,  .name = "energy_decay",  .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_energy_decay,  .validator = validate_decay     },
    { .id = CFG_MOOD_DECAY,    .name = "mood_decay",    .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_mood_decay,    .validator = validate_decay     },
    { .id = CFG_HUNGER_THRESH, .name = "hunger_thresh", .type = HZ_CONFIG_TYPE_U8,    .default_val = &s_def_hunger_thresh, .validator = validate_thresh_u8 },
    { .id = CFG_ENERGY_THRESH, .name = "energy_thresh", .type = HZ_CONFIG_TYPE_U8,    .default_val = &s_def_energy_thresh, .validator = validate_thresh_u8 },
    { .id = CFG_THEME_COLOR,   .name = "theme_color",   .type = HZ_CONFIG_TYPE_U32,   .default_val = &s_def_theme_color,   .validator = NULL               },
    { .id = CFG_AUTO_SAVE_MS,  .name = "auto_save_ms",  .type = HZ_CONFIG_TYPE_U16,   .default_val = &s_def_auto_save_ms,  .validator = NULL               },
};

static void config_cache_init(void)
{
    g_app_cfg.hunger_decay  = s_def_hunger_decay;
    g_app_cfg.energy_decay  = s_def_energy_decay;
    g_app_cfg.mood_decay    = s_def_mood_decay;
    g_app_cfg.hunger_thresh = s_def_hunger_thresh;
    g_app_cfg.energy_thresh = s_def_energy_thresh;
    g_app_cfg.theme_color   = s_def_theme_color;
    g_app_cfg.auto_save_ms  = s_def_auto_save_ms;
}

/*===========================================================================
 * Public Functions
 *===========================================================================*/
void app_log_action(const char *action)
{
    activity_log_t *log = &g_activity_log;
    size_t len = strlen(action);
    if (len >= sizeof(log->entries[0])) len = sizeof(log->entries[0]) - 1;
    memcpy(log->entries[log->head], action, len);
    log->entries[log->head][len] = '\0';
    log->head = (log->head + 1) % ACTIVITY_LOG_SIZE;
    if (log->count < ACTIVITY_LOG_SIZE) log->count++;
}

const char *app_mood_label(int32_t mood)
{
    if (mood > 70) return "Happy!";
    if (mood > 40) return "Okay";
    if (mood > 15) return "Down...";
    return "Very sad";
}

void app_check_achievements(void)
{
    static int s_feed_count = 0;
    static int s_train_count = 0;
    hz_state_t cur = hz_fsm_current();

    if (cur == PET_STATE_EATING && ++s_feed_count >= 10 && !g_achievements.fed_10_times) {
        g_achievements.fed_10_times = 1;
        hz_alarm_trigger(ALARM_ACHIEVEMENT_UNLOCK);
    }
    if (cur == PET_STATE_TRAINING && ++s_train_count >= 5 && !g_achievements.trained_5_times) {
        g_achievements.trained_5_times = 1;
        hz_alarm_trigger(ALARM_ACHIEVEMENT_UNLOCK);
    }
    if (cur == PET_STATE_HYPER && !g_achievements.reached_hyper) {
        g_achievements.reached_hyper = 1;
        hz_alarm_trigger(ALARM_ACHIEVEMENT_UNLOCK);
    }

    static hz_u8 s_states_seen_mask = 0;
    s_states_seen_mask |= (1 << cur);
    if (s_states_seen_mask == 0x1FF && !g_achievements.all_states_seen) {
        g_achievements.all_states_seen = 1;
        hz_alarm_trigger(ALARM_ACHIEVEMENT_UNLOCK);
    }

    static hz_u8 s_mood_good_ticks = 0;
    if (g_pet.mood > 80) s_mood_good_ticks++; else s_mood_good_ticks = 0;
    if (s_mood_good_ticks >= 10 && !g_achievements.mood_80_for_10) {
        g_achievements.mood_80_for_10 = 1;
        g_pet.mood = HZ_MIN(100, g_pet.mood + 5);
    }
}

/*===========================================================================
 * Config Set Wrapper (used by settings screen)
 *===========================================================================*/
hz_err_t config_set(hz_config_id_t id, const void *value)
{
    hz_err_t err = hz_config_mgr_set(id, value);
    if (err == HZ_OK) {
        hz_event_publish(EV_CONFIG_CHANGED, (void *)(hz_u32)id);
    }
    return err;
}

/*===========================================================================
 * Main
 *===========================================================================*/
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    /* ---- Init HZCL subsystems ---- */
    hz_platform_init();
    hz_event_init();
    hz_timer_init();
    hz_screen_init();
    hz_fsm_init(s_states, HZ_ARRAY_SIZE(s_states));
    hz_mode_init();
    hz_sensor_init();
    hz_alarm_init(s_alarm_defs, ALARM_COUNT);
    hz_config_mgr_init(s_config_items, CFG_COUNT);

    hz_power_config_t power_cfg = {
        .idle_timeout_ms = 30000, .sleep_timeout_ms = 60000, .deep_sleep_timeout_ms = 120000,
    };
    hz_power_init(&power_cfg);
    config_cache_init();

    /* ---- Register modes ---- */
    for (int i = 0; i < MODE_COUNT; i++) hz_mode_register(&s_modes[i]);

    /* ---- Register sensor channels ---- */
    register_sensors();

    /* ---- Subscribe to global events ---- */
    hz_event_subscribe(EV_SENSOR_THRESHOLD, on_sensor_threshold, NULL);
    hz_event_subscribe(EV_CONFIG_CHANGED,   on_config_changed,   NULL);

    /* ---- Create HZCL soft timers ---- */
    s_pet_timer.interval_ms = TICK_INTERVAL_MS; s_pet_timer.remaining_ms = 0;
    s_pet_timer.callback = pet_timer_cb; s_pet_timer.arg = NULL;
    s_pet_timer.repeat = 1; s_pet_timer.active = 0; s_pet_timer.next = NULL;
    hz_timer_start(&s_pet_timer);

    s_auto_save_timer.interval_ms = g_app_cfg.auto_save_ms; s_auto_save_timer.remaining_ms = 0;
    s_auto_save_timer.callback = auto_save_cb; s_auto_save_timer.arg = NULL;
    s_auto_save_timer.repeat = 1; s_auto_save_timer.active = 0; s_auto_save_timer.next = NULL;
    hz_timer_start(&s_auto_save_timer);

    /* ---- Init LVGL ---- */
    lv_init();
    lv_display_t *disp = lv_sdl_window_create(SCREEN_HOR_RES, SCREEN_VER_RES);
    if (!disp) { fprintf(stderr, "Failed to create SDL display!\n"); return -1; }
    lv_sdl_window_set_title(disp, "Pro Desk Pet");
    lv_sdl_mouse_create();

    /* ---- Create screens and push home ---- */
    screen_home_create();
    screen_stats_create();
    screen_settings_create();
    screen_about_create();

    hz_screen_push(screen_home_get(), NULL);

    /* ---- Initial state & mode ---- */
    hz_fsm_transition(PET_STATE_IDLE);
    hz_mode_switch(MODE_NORMAL);

    /* ---- Main loop ---- */
    hz_u32 last_tick = hz_platform_get_tick_ms();
    while (1) {
        hz_u32 now = hz_platform_get_tick_ms();
        hz_u32 elapsed = now - last_tick;
        if (elapsed == 0) { hz_platform_delay_ms(5); continue; }
        last_tick = now;
        if (elapsed > 100) elapsed = 100;

        hz_timer_tick(elapsed);
        hz_fsm_tick(elapsed);
        hz_sensor_tick(elapsed);
        hz_mode_tick(elapsed);
        hz_alarm_tick(elapsed);
        hz_power_tick(elapsed);
        hz_screen_tick(elapsed);

        lv_timer_handler();
        lv_tick_inc(elapsed);
        hz_platform_delay_ms(5);
    }

    lv_sdl_quit();
    return 0;
}
