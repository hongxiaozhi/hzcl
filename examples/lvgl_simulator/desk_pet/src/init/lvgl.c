#include "init/lvgl.h"
#include "app.h"
#include "hz_fsm.h"
#include "hz_mode.h"
#include "lvgl/lvgl.h"
#include "lvgl/drivers/sdl/lv_sdl_window.h"
#include "SDL.h"
#include "screens/pet_screen.h"
#include <stdio.h>

#define SCREEN_HOR_RES  480
#define SCREEN_VER_RES  350  /* 320 + 30px debug toolbar */

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

    /* Disable window resize */
    SDL_Window *sdl_win = lv_sdl_window_get_window(disp);
    if (sdl_win) SDL_SetWindowResizable(sdl_win, SDL_FALSE);

    /* Init FSM + mode */
    hz_fsm_transition(PET_STATE_IDLE);
    hz_mode_switch(MODE_NORMAL);

    /* Single-screen UI with all 5 pages + auto-switch */
    pet_screen_init();
}
