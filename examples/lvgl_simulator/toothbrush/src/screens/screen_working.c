/**
 * @file screen_working.c
 * @brief Working screen — timer display, mode indicator, gear animation placeholder
 */
#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "hz_screen.h"
#include "hz_fsm.h"
#include "fsm.h"
#include <stdio.h>

static lv_obj_t *s_cont;
static lv_obj_t *s_timer_label;
static lv_obj_t *s_mode_label;
static lv_obj_t *s_circle;
static lv_obj_t *s_progress_arc;

static const char *mode_names[] = { "Care", "Clean", "White" };

static void update_ui(void)
{
    char buf[16];
    hz_u32 min = g_app.work_sec / 60;
    hz_u32 sec = g_app.work_sec % 60;
    snprintf(buf, sizeof(buf), "%02lu:%02lu", (unsigned long)min, (unsigned long)sec);
    lv_label_set_text(s_timer_label, buf);
    lv_label_set_text(s_mode_label, mode_names[g_app.mode]);

    /* Arc progress (0-120s → 0-360 deg) */
    int32_t progress = (g_app.work_sec > 120) ? 360 : (int32_t)(g_app.work_sec * 3);
    lv_arc_set_value(s_progress_arc, progress);
}

/*===========================================================================
 * Lifecycle
 *===========================================================================*/
static void working_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_color(s_cont, lv_color_hex(0x0a1628), 0);
    lv_obj_set_hidden(s_cont, true);
    lv_obj_remove_flag(s_cont, LV_OBJ_FLAG_CLICKABLE);

    /* Progress arc (ring) */
    s_progress_arc = lv_arc_create(s_cont);
    lv_obj_set_size(s_progress_arc, 180, 180);
    lv_obj_align(s_progress_arc, LV_ALIGN_CENTER, 0, -10);
    lv_arc_set_range(s_progress_arc, 0, 360);
    lv_arc_set_value(s_progress_arc, 0);
    lv_arc_set_bg_angles(s_progress_arc, 0, 360);
    lv_arc_set_angles(s_progress_arc, 0, 0);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x00d2ff), 0);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x00d2ff), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_progress_arc, 6, 0);
    lv_obj_set_style_arc_width(s_progress_arc, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(s_progress_arc, lv_color_hex(0x1a2a4a), LV_PART_MAIN);

    /* Timer in center of arc */
    s_timer_label = lv_label_create(s_cont);
    lv_label_set_text(s_timer_label, "00:00");
    lv_obj_set_style_text_color(s_timer_label, lv_color_hex(0x00d2ff), 0);
    lv_obj_set_style_text_font(s_timer_label, &lv_font_montserrat_48, 0);
    lv_obj_align(s_timer_label, LV_ALIGN_CENTER, 0, -10);

    /* Mode name below timer */
    s_mode_label = lv_label_create(s_cont);
    lv_label_set_text(s_mode_label, "Clean");
    lv_obj_set_style_text_color(s_mode_label, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(s_mode_label, &lv_font_montserrat_20, 0);
    lv_obj_align(s_mode_label, LV_ALIGN_CENTER, 0, -80);

    /* Toothbrush icon */
    lv_obj_t *icon = lv_label_create(s_cont);
    lv_label_set_text(icon, "\xF0\x9F\xAA\xA5");
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, 0);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -130);

    /* Hint */
    lv_obj_t *hint = lv_label_create(s_cont);
    lv_label_set_text(hint, "Short press → Pause/Mode    t<10s → cycle mode");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x666666), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    update_ui();
}

static void working_on_enter(void) { lv_obj_set_hidden(s_cont, false); update_ui(); }
static void working_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

static void working_tick(hz_u32 ms) { (void)ms; update_ui(); }

hz_screen_t g_scr_working = {
    .name = "working", .on_create = working_on_create,
    .on_enter = working_on_enter, .on_exit = working_on_exit,
    .on_tick  = working_tick,
};
