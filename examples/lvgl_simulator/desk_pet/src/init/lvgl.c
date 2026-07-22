#include "init/lvgl.h"
#include "app.h"
#include "hz_fsm.h"
#include "hz_mode.h"
#include "hz_screen.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

/* Screen factory declarations (implemented in screens/*.c) */
void screen_home_create(void);
hz_screen_t *screen_home_get(void);
void screen_stats_create(void);
hz_screen_t *screen_stats_get(void);
void screen_settings_create(void);
hz_screen_t *screen_settings_get(void);
void screen_about_create(void);
hz_screen_t *screen_about_get(void);
void screen_hmi_create(void);
hz_screen_t *hmi_get_screen(void);

#define SCREEN_HOR_RES  480
#define SCREEN_VER_RES  320

void init_lvgl(void)
{
    lv_init();

    lv_display_t *disp = lv_sdl_window_create(SCREEN_HOR_RES, SCREEN_VER_RES);
    if (!disp) {
        fprintf(stderr, "Failed to create SDL display!\n");
        return;
    }
    lv_sdl_window_set_title(disp, "Pro Desk Pet");
    lv_sdl_mouse_create();

    /* Create screens */
    screen_home_create();
    screen_stats_create();
    screen_settings_create();
    screen_about_create();
    screen_hmi_create();

    /* Enter initial FSM state and mode BEFORE pushing home screen */
    hz_fsm_transition(PET_STATE_IDLE);
    hz_mode_switch(MODE_NORMAL);

    hz_screen_push(screen_home_get(), NULL);
}
