/**
 * @file main.c
 * @brief Pro Desk Pet — main entry, runtime glue, and main loop
 */

#include "app.h"
#include "hz_types.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_timer.h"
#include "hz_mode.h"
#include "hz_sensor.h"
#include "hz_alarm.h"
#include "hz_config_mgr.h"
#include "hz_power.h"
#include "hz_platform.h"
#include "hz_screen.h"

#include "init/fsm.h"
#include "init/modes.h"
#include "init/sensors.h"
#include "init/alarms.h"
#include "init/config.h"
#include "init/timers.h"
#include "init/events.h"
#include "init/lvgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*===========================================================================
 * Global Data (extern'd in app.h — data not owned by any init module)
 *===========================================================================*/
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
 * Pet Status Simulation (runtime, called every second via ACT_TICK)
 *===========================================================================*/
void pet_status_tick(void)
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
 * Activity Log
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

    /* ---- Init HZCL core subsystems ---- */
    hz_platform_init();
    hz_event_init();
    hz_timer_init();
    hz_screen_init();
    init_fsm();               /* register state table */
    hz_mode_init();
    hz_sensor_init();
    init_alarms();            /* register alarm definitions */
    init_config();            /* register config items + cache */

    hz_power_config_t power_cfg = {
        .idle_timeout_ms = 30000,
        .sleep_timeout_ms = 60000,
        .deep_sleep_timeout_ms = 120000,
    };
    hz_power_init(&power_cfg);

    init_modes();             /* register mode entries */
    init_sensors();           /* register sensor channels */
    init_events();            /* subscribe to event bus */
    init_timers();            /* start soft timers */

    /* ---- Init LVGL + screens + push home screen ---- */
    init_lvgl();

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
