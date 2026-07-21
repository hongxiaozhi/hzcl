/**
 * @file lv_conf.h
 * Configuration for LVGL v9 - Desk Pet Simulator
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
 *  MEMORY SETTINGS
 *====================*/

#define LV_USE_STDLIB_MALLOC    LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING    LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_CLIB

#define LV_MEM_SIZE             (64 * 1024)

/*====================
 *  HAL SETTINGS
 *====================*/

#define LV_USE_SDL              1
#define LV_USE_SDL_RENDERER     1
#define LV_USE_SDL_GPU          0

/* SDL include path — bundled SDL2 headers are directly in include/, not in SDL2/ subdir */
#define LV_SDL_INCLUDE_PATH     "SDL.h"

/*====================
 *  DISPLAY SETTINGS
 *====================*/

#define LV_COLOR_DEPTH          32
#define LV_COLOR_16_SWAP        0
#define LV_DPI_DEF              160

/*====================
 *  OS SETTINGS
 *====================*/

#define LV_USE_OS               LV_OS_NONE

/*====================
 *  LOG SETTINGS
 *====================*/

#define LV_USE_LOG              1
#define LV_LOG_LEVEL            LV_LOG_LEVEL_INFO

/*====================
 *  WIDGET USAGE
 *====================*/

#define LV_USE_BTN              1
#define LV_USE_LABEL            1
#define LV_USE_IMG              1
#define LV_USE_ANIMIMG          1
#define LV_USE_BAR              1
#define LV_USE_SLIDER           1
#define LV_USE_LINE             1
#define LV_USE_ARC              1
#define LV_USE_CONT            1
#define LV_USE_PAGE             1
#define LV_USE_WIN              1
#define LV_USE_CANVAS           1
#define LV_USE_CHART            1
#define LV_USE_TABVIEW          1
#define LV_USE_LED              1

/*====================
 *  FONT USAGE
 *====================*/

#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_20    1
#define LV_FONT_MONTSERRAT_24    1
#define LV_FONT_MONTSERRAT_48    1

#define LV_USE_FONT_COMPRESSED   0

/*====================
 *  THEME USAGE
 *====================*/

#define LV_USE_THEME_DEFAULT     1
#define LV_USE_THEME_MONO        0
#define LV_USE_THEME_SIMPLE      0

/*====================
 *  EXAMPLES & DEMOS
 *====================*/

#define LV_BUILD_EXAMPLES        0

/*====================
 *  ANIMATION
 *====================*/

#define LV_USE_ANIMATION         1

/*====================
 *  GPU SETTINGS
 *====================*/

#define LV_USE_GPU_SDL           1

/*====================
 *  DRAW SETTINGS
 *====================*/

#define LV_DRAW_SW_SHADOW        1
#define LV_DRAW_SW_ARC_SMOOTH    1
#define LV_DRAW_SW_LINE_JOIN     1
#define LV_DRAW_SW_LINE_SMOOTH   1
#define LV_DRAW_SW_CIRCLE_SMOOTH 1

/*====================
 *  REFRESH RATE
 *====================*/

#define LV_DISP_DEF_REFR_PERIOD  30

#endif /* LV_CONF_H */
