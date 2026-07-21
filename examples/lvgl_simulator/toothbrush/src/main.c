/**
 * @file main.c
 * @brief Toothbrush LVGL Simulator — main entry and event loop
 *
 * Uses hz_fsm (state machine), hz_screen (screen stack), hz_timer (1s tick).
 * Keyboard: UP = short press, DOWN = long press
 */

#include "lvgl/lvgl.h"
#include "hz_config.h"
#include "hz_types.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_timer.h"
#include "hz_screen.h"
#include "hz_platform.h"
#include "fsm.h"

#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_W  240
#define SCREEN_H  280

/*===========================================================================
 * Screen descriptors (extern)
 *===========================================================================*/
extern hz_screen_t g_scr_ready;
extern hz_screen_t g_scr_working;
extern hz_screen_t g_scr_stats;
extern hz_screen_t g_scr_sleep;

/*===========================================================================
 * Pre-create all screens
 *===========================================================================*/
static void screens_init(void)
{
    if (g_scr_ready.on_create)   g_scr_ready.on_create();
    if (g_scr_working.on_create) g_scr_working.on_create();
    if (g_scr_stats.on_create)   g_scr_stats.on_create();
    if (g_scr_sleep.on_create)   g_scr_sleep.on_create();
}

/*===========================================================================
 * Timer
 *===========================================================================*/
static hz_timer_t s_tick_timer;

static void tick_cb(void *arg)
{
    (void)arg;
    fsm_tick_1s();
}

/*===========================================================================
 * Key handler — UP=short press, DOWN=long press
 *===========================================================================*/
static void handle_key(hz_bool_t is_long)
{
    fsm_reset_idle();
    hz_state_t cur = hz_fsm_current();

    /* Wake from sleep on any key */
    if (cur == STATE_SLEEP) {
        hz_fsm_execute_action(ACT_WAKE, NULL);
        return;
    }

    if (is_long) {
        /* Long press */
        switch (cur) {
        case STATE_READY:
            hz_fsm_execute_action(ACT_ENTER_SETTING, NULL);
            break;
        case STATE_WORKING:
            if (g_app.work_sec < 10) {
                /* Long press in first 10s → back to ready */
                hz_fsm_transition(STATE_READY);
            } else if (g_app.work_sec < 30) {
                hz_fsm_transition(STATE_CANCEL);
            } else {
                g_app.score = HZ_MIN(100, 70 + g_app.work_sec / 4);
                hz_fsm_transition(STATE_STATS);
            }
            break;
        default:
            break;
        }
    } else {
        /* Short press */
        switch (cur) {
        case STATE_READY:
            hz_fsm_execute_action(ACT_START, NULL);
            break;
        case STATE_WORKING:
            if (g_app.work_sec < 10) {
                /* In first 10s: cycle modes. After last mode → back to ready */
                if (g_app.mode >= MOTOR_COUNT - 1) {
                    hz_fsm_transition(STATE_READY);
                } else {
                    hz_fsm_execute_action(ACT_MODE_NEXT, NULL);
                }
            } else {
                if (g_app.work_sec < 30)
                    hz_fsm_transition(STATE_CANCEL);
                else {
                    g_app.score = HZ_MIN(100, 70 + g_app.work_sec / 4);
                    hz_fsm_transition(STATE_STATS);
                }
            }
            break;
        case STATE_COVER:
            hz_fsm_execute_action(ACT_COVER_DONE, NULL);
            break;
        case STATE_STATS:
        case STATE_CANCEL:
            hz_fsm_execute_action(ACT_START, NULL);
            break;
        case STATE_SETTINGS:
            hz_fsm_execute_action(ACT_EXIT_SETTING, NULL);
            break;
        default:
            break;
        }
    }
}

/*===========================================================================
 * Main
 *===========================================================================*/
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    /* Init LVGL */
    lv_init();
    lv_display_t *disp = lv_sdl_window_create(SCREEN_W, SCREEN_H);
    if (!disp) { fprintf(stderr, "Failed to create SDL display!\n"); return -1; }
    lv_sdl_window_set_title(disp, "Toothbrush Simulator [UP=short DOWN=long]");
    lv_sdl_mouse_create();

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a1628), 0);

    /* Init HZCL */
    hz_platform_init();
    hz_event_init();
    hz_timer_init();
    hz_screen_init();

    screens_init();
    fsm_init();

    /* 1-second timer */
    s_tick_timer.interval_ms  = 1000;
    s_tick_timer.remaining_ms = 0;
    s_tick_timer.callback     = tick_cb;
    s_tick_timer.arg          = NULL;
    s_tick_timer.repeat       = 1;
    s_tick_timer.active       = 0;
    s_tick_timer.next         = NULL;
    hz_timer_start(&s_tick_timer);

    /* Main loop */
    hz_u32 last_tick = hz_platform_get_tick_ms();

    while (1) {
        hz_u32 now = hz_platform_get_tick_ms();
        hz_u32 elapsed = now - last_tick;
        if (elapsed == 0) { hz_platform_delay_ms(5); continue; }
        last_tick = now;
        if (elapsed > 100) elapsed = 100;

        /* Poll SDL keyboard events */
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto quit;
            if (evt.type == SDL_KEYDOWN) {
                switch (evt.key.keysym.sym) {
                case SDLK_UP:
                    handle_key(0);   /* short press */
                    break;
                case SDLK_DOWN:
                    handle_key(1);   /* long press */
                    break;
                }
            }
        }

        hz_timer_tick(elapsed);
        hz_fsm_tick(elapsed);
        hz_screen_tick(elapsed);

        lv_timer_handler();
        lv_tick_inc(elapsed);

        hz_platform_delay_ms(5);
    }

quit:
    lv_sdl_quit();
    return 0;
}
