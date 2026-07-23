/**
 * @file util_screenshot.c
 * @brief Reusable LVGL+SDL2 screenshot utility — implementation
 */
#include "util_screenshot.h"
#include "lvgl/lvgl.h"
#include "lvgl/drivers/sdl/lv_sdl_window.h"
#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MKDIR(d) mkdir(d, 0755)
#endif

static char  s_dir[128] = "screenshots";
static int   s_interval = 0;
static int   s_counter = 0;
static lv_timer_t *s_timer = NULL;

/* ---- Take a single screenshot ---- */
void util_screenshot_take(const char *name)
{
    MKDIR(s_dir);

    lv_display_t *disp = lv_display_get_default();
    if (!disp) return;

    SDL_Window   *win = lv_sdl_window_get_window(disp);
    SDL_Renderer *ren = lv_sdl_window_get_renderer(disp);
    if (!win || !ren) return;

    int ww, wh;
    SDL_GetWindowSize(win, &ww, &wh);

    SDL_Surface *surf = SDL_CreateRGBSurface(0, ww, wh, 24,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0);
    if (!surf) return;

    SDL_RenderReadPixels(ren, NULL, surf->format->format, surf->pixels, surf->pitch);

    char path[256];
    snprintf(path, sizeof(path), "%s/%s.bmp", s_dir, name);
    SDL_SaveBMP(surf, path);

    SDL_FreeSurface(surf);
}

/* ---- Auto-capture timer callback ---- */
static void auto_cb(lv_timer_t *tm)
{
    (void)tm;
    char name[32];
    s_counter++;
    snprintf(name, sizeof(name), "auto_%03d", s_counter);
    util_screenshot_take(name);
}

/* ---- Init ---- */
void util_screenshot_init(const char *dir, int interval_sec)
{
    if (dir && dir[0]) {
        strncpy(s_dir, dir, sizeof(s_dir) - 1);
        s_dir[sizeof(s_dir) - 1] = '\0';
    }

    s_interval = interval_sec;

    if (s_interval > 0) {
        s_timer = lv_timer_create(auto_cb, s_interval * 1000, NULL);
    }
}

/* ---- Tick (placeholder for future use) ---- */
void util_screenshot_tick(void)
{
    /* Auto-capture is driven by lv_timer, no manual tick needed */
}
