/**
 * @file main.c
 * @brief HZCL Desk Pet - LVGL PC Simulator Demo
 *
 * A smart desktop pet demonstrating HZCL + LVGL v9 integration.
 *
 * Features:
 *   - Pet states via HZCL FSM (idle / happy / eating / sleeping / sad)
 *   - Periodic status updates via HZCL timer
 *   - Button events via HZCL event bus
 *   - Status monitoring (hunger / energy / mood)
 *   - LVGL UI with animated emoji face + status bars
 *
 * Build:
 *   cd examples/lvgl_simulator/desk_pet
 *   mkdir build && cd build
 *   cmake .. -G "MinGW Makefiles"
 *   mingw32-make
 *
 * Requires: SDL2, LVGL (submodule at ../lvgl)
 */

#include "lvgl/lvgl.h"
#include "hz_config.h"
#include "hz_types.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_timer.h"
#include "hz_power.h"
#include "hz_alarm.h"
#include "hz_platform.h"
#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*===========================================================================
 * Display / UI constants
 *===========================================================================*/
#define SCREEN_HOR_RES     480
#define SCREEN_VER_RES      320
#define TICK_INTERVAL_MS   1000    /* 1-second pet status tick */

/*===========================================================================
 * Pet status data
 *===========================================================================*/
typedef struct {
    int32_t hunger;     /* 0~100, higher = hungrier */
    int32_t energy;     /* 0~100, higher = more energetic */
    int32_t mood;       /* 0~100, higher = happier */
} pet_status_t;

static pet_status_t s_pet = { 30, 80, 60 };
static int s_happy_counter = 0;
static int s_eat_counter   = 0;

/*===========================================================================
 * LVGL widgets
 *===========================================================================*/
static lv_obj_t *s_pet_emoji;
static lv_obj_t *s_status_label;
static lv_obj_t *s_hunger_bar;
static lv_obj_t *s_energy_bar;
static lv_obj_t *s_mood_bar;

/*===========================================================================
 * Pet state IDs
 *===========================================================================*/
enum {
    PET_STATE_IDLE     = 0,
    PET_STATE_HAPPY    = 1,
    PET_STATE_EATING   = 2,
    PET_STATE_SLEEPING = 3,
    PET_STATE_SAD      = 4,
};

/*===========================================================================
 * Pet action IDs
 *===========================================================================*/
enum {
    ACT_FEED     = 1,
    ACT_PET      = 2,
    ACT_GO_SLEEP = 3,
    ACT_TICK     = 4,
    ACT_WAKE     = 5,
};

/*===========================================================================
 * Forward declarations
 *===========================================================================*/
static void ui_update_all(void);
static void pet_status_tick(void);

/*===========================================================================
 * Emoji helpers
 *===========================================================================*/
static const char *emoji_for_state(hz_state_t state)
{
    switch (state) {
    case PET_STATE_IDLE:     return "\xF0\x9F\x98\x90";
    case PET_STATE_HAPPY:    return "\xF0\x9F\x98\x8A";
    case PET_STATE_EATING:   return "\xF0\x9F\x98\x8B";
    case PET_STATE_SLEEPING: return "\xF0\x9F\x98\xB4";
    case PET_STATE_SAD:      return "\xF0\x9F\x98\xA2";
    default:                 return "\xF0\x9F\x99\x82";
    }
}

/*===========================================================================
 * FSM action handlers
 *===========================================================================*/
static hz_err_t act_feed_handler(void *arg)
{
    (void)arg;
    s_pet.hunger = 0;
    s_pet.mood += 10;
    if (s_pet.mood > 100) s_pet.mood = 100;
    s_eat_counter = 0;
    ui_update_all();
    return HZ_OK;
}

static hz_err_t act_pet_handler(void *arg)
{
    (void)arg;
    s_pet.mood = 100;
    s_happy_counter = 0;
    ui_update_all();
    return HZ_OK;
}

static hz_err_t act_sleep_handler(void *arg)
{
    (void)arg;
    ui_update_all();
    return HZ_OK;
}

static hz_err_t act_wake_handler(void *arg)
{
    (void)arg;
    ui_update_all();
    return HZ_OK;
}

static hz_err_t act_tick_handler(void *arg)
{
    (void)arg;
    pet_status_tick();
    return HZ_OK;
}

/*===========================================================================
 * Allowed actions per state
 *===========================================================================*/
static const hz_action_t s_actions_idle[] = {
    { ACT_FEED,     "feed",     act_feed_handler,     PET_STATE_EATING   },
    { ACT_PET,      "pet",      act_pet_handler,      PET_STATE_HAPPY    },
    { ACT_GO_SLEEP, "go_sleep", act_sleep_handler,    PET_STATE_SLEEPING },
    { ACT_TICK,     "tick",     act_tick_handler,      HZ_STATE_NONE      },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_happy[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_eating[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_sleeping[] = {
    { ACT_PET,  "pet",  act_pet_handler,  PET_STATE_IDLE     },
    { ACT_TICK, "tick", act_tick_handler,  HZ_STATE_NONE      },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_sad[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING   },
    { ACT_PET,  "pet",  act_pet_handler,  PET_STATE_HAPPY    },
    { 0, NULL, NULL, 0 },
};

/*===========================================================================
 * State table
 *===========================================================================*/
static void on_idle_enter(hz_state_t from)    { (void)from; ui_update_all(); }
static void on_happy_enter(hz_state_t from)   { (void)from; s_happy_counter = 0; ui_update_all(); }
static void on_eating_enter(hz_state_t from)  { (void)from; s_eat_counter = 0; ui_update_all(); }
static void on_sleep_enter(hz_state_t from)   { (void)from; ui_update_all(); }
static void on_sad_enter(hz_state_t from)     { (void)from; ui_update_all(); }

static const hz_state_node_t s_states[] = {
    {
        .id              = PET_STATE_IDLE,
        .parent          = 0xFF,
        .name            = "idle",
        .on_enter        = on_idle_enter,
        .allowed_actions = s_actions_idle,
    },
    {
        .id              = PET_STATE_HAPPY,
        .parent          = 0xFF,
        .name            = "happy",
        .on_enter        = on_happy_enter,
        .allowed_actions = s_actions_happy,
    },
    {
        .id              = PET_STATE_EATING,
        .parent          = 0xFF,
        .name            = "eating",
        .on_enter        = on_eating_enter,
        .allowed_actions = s_actions_eating,
    },
    {
        .id              = PET_STATE_SLEEPING,
        .parent          = 0xFF,
        .name            = "sleeping",
        .on_enter        = on_sleep_enter,
        .allowed_actions = s_actions_sleeping,
    },
    {
        .id              = PET_STATE_SAD,
        .parent          = 0xFF,
        .name            = "sad",
        .on_enter        = on_sad_enter,
        .allowed_actions = s_actions_sad,
    },
};

#define STATE_COUNT (sizeof(s_states) / sizeof(s_states[0]))

/*===========================================================================
 * HZCL timer
 *===========================================================================*/
static hz_timer_t s_pet_timer;

static void pet_timer_cb(void *arg)
{
    (void)arg;
    hz_fsm_execute_action(ACT_TICK, NULL);

    hz_state_t cur = hz_fsm_current();
    if (cur == PET_STATE_HAPPY) {
        s_happy_counter++;
        if (s_happy_counter >= 3)
            hz_fsm_transition(PET_STATE_IDLE);
    }
    if (cur == PET_STATE_EATING) {
        s_eat_counter++;
        if (s_eat_counter >= 3)
            hz_fsm_transition(PET_STATE_IDLE);
    }
}

/*===========================================================================
 * Pet status simulation
 *===========================================================================*/
static void pet_status_tick(void)
{
    hz_state_t cur = hz_fsm_current();

    if (cur == PET_STATE_SLEEPING) {
        s_pet.energy += 3;
        if (s_pet.energy > 100) {
            s_pet.energy = 100;
            hz_fsm_transition(PET_STATE_IDLE);
        }
    } else {
        if (s_pet.hunger < 100) s_pet.hunger += 1;
        if (s_pet.energy > 0)   s_pet.energy -= 1;
    }

    if (s_pet.hunger > 70 || s_pet.energy < 20) {
        if (s_pet.mood > 0) s_pet.mood -= 2;
    } else if (s_pet.mood < 60) {
        s_pet.mood += 1;
    }

    if (s_pet.hunger >= 80 && cur != PET_STATE_SAD && cur != PET_STATE_EATING)
        hz_fsm_transition(PET_STATE_SAD);
    if (s_pet.energy <= 10 && cur != PET_STATE_SLEEPING)
        hz_fsm_transition(PET_STATE_SLEEPING);

    ui_update_all();
}

/*===========================================================================
 * UI
 *===========================================================================*/
static void ui_update_all(void)
{
    hz_state_t cur = hz_fsm_current();
    lv_label_set_text(s_pet_emoji, emoji_for_state(cur));

    lv_bar_set_value(s_hunger_bar, 100 - s_pet.hunger, LV_ANIM_ON);
    lv_bar_set_value(s_energy_bar, s_pet.energy, LV_ANIM_ON);
    lv_bar_set_value(s_mood_bar,   s_pet.mood,   LV_ANIM_ON);

    const char *mood_text;
    if (s_pet.mood > 70)      mood_text = "Happy! \xF0\x9F\x98\x8A";
    else if (s_pet.mood > 40) mood_text = "Okay \xF0\x9F\x98\x90";
    else                      mood_text = "Sad \xF0\x9F\x98\xA2";

    const char *state_name = hz_fsm_current_name();
    char buf[80];
    snprintf(buf, sizeof(buf), "[%s] %s  |  Click to interact!",
             state_name, mood_text);
    lv_label_set_text(s_status_label, buf);
}

static void btn_feed_cb(lv_event_t *e)
{
    (void)e;
    hz_event_publish(EV_USER_BASE + 1, NULL);
    hz_fsm_execute_action(ACT_FEED, NULL);
}

static void btn_pet_cb(lv_event_t *e)
{
    (void)e;
    hz_event_publish(EV_USER_BASE + 2, NULL);
    hz_fsm_execute_action(ACT_PET, NULL);
}

static void btn_sleep_cb(lv_event_t *e)
{
    (void)e;
    hz_event_publish(EV_USER_BASE + 3, NULL);
    hz_fsm_execute_action(ACT_GO_SLEEP, NULL);
}

static void create_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "HZCL Desk Pet");
    lv_obj_set_style_text_color(title, lv_color_hex(0x00d2ff), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    s_pet_emoji = lv_label_create(scr);
    lv_label_set_text(s_pet_emoji, "\xF0\x9F\x98\x90");
    lv_obj_set_style_text_font(s_pet_emoji, &lv_font_montserrat_48, 0);
    lv_obj_align(s_pet_emoji, LV_ALIGN_CENTER, 0, -30);

    s_status_label = lv_label_create(scr);
    lv_label_set_text(s_status_label, "Click buttons to interact!");
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0xcccccc), 0);
    lv_obj_align(s_status_label, LV_ALIGN_CENTER, 0, 20);

    /* --- Right-side bars --- */
    lv_obj_t *lbl_hunger = lv_label_create(scr);
    lv_label_set_text(lbl_hunger, "Fullness");
    lv_obj_set_style_text_color(lbl_hunger, lv_color_hex(0xff6b6b), 0);
    lv_obj_align(lbl_hunger, LV_ALIGN_RIGHT_MID, -120, -70);

    s_hunger_bar = lv_bar_create(scr);
    lv_obj_set_size(s_hunger_bar, 100, 10);
    lv_obj_align(s_hunger_bar, LV_ALIGN_RIGHT_MID, -20, -70);
    lv_obj_set_style_bg_color(s_hunger_bar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_anim_time(s_hunger_bar, 300, 0);
    lv_bar_set_range(s_hunger_bar, 0, 100);
    lv_bar_set_value(s_hunger_bar, 70, LV_ANIM_OFF);

    lv_obj_t *lbl_energy = lv_label_create(scr);
    lv_label_set_text(lbl_energy, "Energy");
    lv_obj_set_style_text_color(lbl_energy, lv_color_hex(0x4ecdc4), 0);
    lv_obj_align(lbl_energy, LV_ALIGN_RIGHT_MID, -120, -40);

    s_energy_bar = lv_bar_create(scr);
    lv_obj_set_size(s_energy_bar, 100, 10);
    lv_obj_align(s_energy_bar, LV_ALIGN_RIGHT_MID, -20, -40);
    lv_obj_set_style_bg_color(s_energy_bar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_anim_time(s_energy_bar, 300, 0);
    lv_bar_set_range(s_energy_bar, 0, 100);
    lv_bar_set_value(s_energy_bar, 80, LV_ANIM_OFF);

    lv_obj_t *lbl_mood = lv_label_create(scr);
    lv_label_set_text(lbl_mood, "Mood");
    lv_obj_set_style_text_color(lbl_mood, lv_color_hex(0xffd93d), 0);
    lv_obj_align(lbl_mood, LV_ALIGN_RIGHT_MID, -120, -10);

    s_mood_bar = lv_bar_create(scr);
    lv_obj_set_size(s_mood_bar, 100, 10);
    lv_obj_align(s_mood_bar, LV_ALIGN_RIGHT_MID, -20, -10);
    lv_obj_set_style_bg_color(s_mood_bar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_anim_time(s_mood_bar, 300, 0);
    lv_bar_set_range(s_mood_bar, 0, 100);
    lv_bar_set_value(s_mood_bar, 60, LV_ANIM_OFF);

    /* --- Bottom buttons --- */
    lv_obj_t *btn;

    btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 25, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xff6b6b), 0);
    lv_obj_add_event_cb(btn, btn_feed_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "\xF0\x9F\x8D\x96 Feed");
    lv_obj_center(lbl);

    btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x4ecdc4), 0);
    lv_obj_add_event_cb(btn, btn_pet_cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "\xF0\x9F\x91\x8B Pet");
    lv_obj_center(lbl);

    btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 90, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -25, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x6c5ce7), 0);
    lv_obj_add_event_cb(btn, btn_sleep_cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "\xF0\x9F\x92\xA4 Sleep");
    lv_obj_center(lbl);

    ui_update_all();
}

/*===========================================================================
 * Main
 *===========================================================================*/
int main(int argc, char *argv[])
{
    /* ---- Init HZCL subsystems ---- */
    hz_platform_init();
    hz_event_init();
    hz_timer_init();

    hz_power_config_t power_cfg = {
        .idle_timeout_ms       = 30000,
        .sleep_timeout_ms      = 60000,
        .deep_sleep_timeout_ms = 120000,
    };
    hz_power_init(&power_cfg);

    /* ---- Init FSM (state table loaded, no transition yet — UI not ready) ---- */
    hz_fsm_init(s_states, STATE_COUNT);

    /* ---- Create HZCL soft timer ---- */
    s_pet_timer.interval_ms  = TICK_INTERVAL_MS;
    s_pet_timer.remaining_ms = 0;
    s_pet_timer.callback     = pet_timer_cb;
    s_pet_timer.arg          = NULL;
    s_pet_timer.repeat       = 1;
    s_pet_timer.active       = 0;
    s_pet_timer.next         = NULL;
    hz_timer_start(&s_pet_timer);

    /* ---- Init LVGL ---- */
    lv_init();

    lv_display_t *disp = lv_sdl_window_create(SCREEN_HOR_RES, SCREEN_VER_RES);
    if (!disp) {
        fprintf(stderr, "Failed to create SDL display!\n");
        return -1;
    }
    lv_sdl_window_set_title(disp, "HZCL Desk Pet");

    lv_sdl_mouse_create();

    create_ui();

    /* Enter initial FSM state (UI widgets exist now, on_enter will update them) */
    hz_fsm_transition(PET_STATE_IDLE);

    /* ---- Main loop ---- */
    hz_u32 last_tick = hz_platform_get_tick_ms();

    while (1) {
        hz_u32 now = hz_platform_get_tick_ms();
        hz_u32 elapsed = now - last_tick;
        last_tick = now;

        hz_timer_tick(elapsed);
        hz_power_tick(elapsed);

        lv_timer_handler();
        lv_tick_inc(elapsed);

        hz_platform_delay_ms(5);
    }

    lv_sdl_quit();
    return 0;
}
