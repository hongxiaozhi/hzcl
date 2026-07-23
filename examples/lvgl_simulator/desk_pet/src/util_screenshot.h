/**
 * @file util_screenshot.h
 * @brief Reusable LVGL+SDL2 screenshot utility
 *
 * Drop-in module for any LVGL v9 + SDL2 project.
 * Captures the SDL renderer output to BMP files.
 *
 * Usage:
 *   #include "util_screenshot.h"
 *   util_screenshot_init("screenshots", 5);  // auto-capture every 5s
 *   // or manually:
 *   util_screenshot_take("my_page");
 */

#ifndef UTIL_SCREENSHOT_H
#define UTIL_SCREENSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize screenshot system.
 * @param dir   Output directory (created if needed). NULL = "screenshots".
 * @param interval_sec  Auto-capture interval in seconds. 0 = manual only.
 */
void util_screenshot_init(const char *dir, int interval_sec);

/**
 * @brief Take a single screenshot.
 * @param name  Base filename (".bmp" appended). E.g. "home" → "home.bmp".
 */
void util_screenshot_take(const char *name);

/**
 * @brief Tick function — call periodically if using auto-capture.
 *        (The auto-capture timer runs internally, but call this as a fallback.)
 */
void util_screenshot_tick(void);

#ifdef __cplusplus
}
#endif

#endif
