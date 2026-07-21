/**
 * @file screen_stats.c
 * @brief Stats / Cancel screen — shows brushing result or cancel message
 */
#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "hz_screen.h"
#include "hz_fsm.h"
#include "fsm.h"
#include <stdio.h>

static lv_obj_t *s_cont;
static lv_obj_t *s_title;
static lv_obj_t *s_score_label;
static lv_obj_t *s_time_label;
static lv_obj_t *s_score_arc;

static void update_ui(void)
{
    hz_state_t cur = hz_fsm_current();
    char buf[32];

    if (cur == STATE_CANCEL) {
        lv_label_set_text(s_title, "Brushing Cancelled");
        lv_label_set_text(s_score_label, "< 30s");
        lv_label_set_text(s_time_label, "Too short to evaluate");
        lv_arc_set_value(s_score_arc, 0);
    } else {
        lv_label_set_text(s_title, "Brushing Complete!");
        snprintf(buf, sizeof(buf), "Score: %d", g_app.score);
        lv_label_set_text(s_score_label, buf);

        hz_u32 min = g_app.work_sec / 60;
        hz_u32 sec = g_app.work_sec % 60;
        snprintf(buf, sizeof(buf), "Time  %02lu:%02lu", (unsigned long)min, (unsigned long)sec);
        lv_label_set_text(s_time_label, buf);
        lv_arc_set_value(s_score_arc, g_app.score * 360 / 100);
    }
}

/*===========================================================================
 * Lifecycle
 *===========================================================================*/
static void stats_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_color(s_cont, lv_color_hex(0x0a1628), 0);
    lv_obj_set_hidden(s_cont, true);
    lv_obj_remove_flag(s_cont, LV_OBJ_FLAG_CLICKABLE);

    /* Title */
    s_title = lv_label_create(s_cont);
    lv_label_set_text(s_title, "Brushing Complete!");
    lv_obj_set_style_text_color(s_title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(s_title, &lv_font_montserrat_20, 0);
    lv_obj_align(s_title, LV_ALIGN_TOP_MID, 0, 20);

    /* Score arc */
    s_score_arc = lv_arc_create(s_cont);
    lv_obj_set_size(s_score_arc, 140, 140);
    lv_obj_align(s_score_arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_range(s_score_arc, 0, 360);
    lv_arc_set_value(s_score_arc, 0);
    lv_arc_set_bg_angles(s_score_arc, 0, 360);
    lv_arc_set_angles(s_score_arc, 0, 0);
    lv_obj_set_style_arc_color(s_score_arc, lv_color_hex(0x1a2a4a), LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_score_arc, lv_color_hex(0x4ecdc4), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(s_score_arc, 10, 0);
    lv_obj_set_style_arc_width(s_score_arc, 10, LV_PART_INDICATOR);

    /* Score label */
    s_score_label = lv_label_create(s_cont);
    lv_label_set_text(s_score_label, "Score: --");
    lv_obj_set_style_text_color(s_score_label, lv_color_hex(0x4ecdc4), 0);
    lv_obj_set_style_text_font(s_score_label, &lv_font_montserrat_24, 0);
    lv_obj_align(s_score_label, LV_ALIGN_CENTER, 0, 0);

    /* Time label */
    s_time_label = lv_label_create(s_cont);
    lv_label_set_text(s_time_label, "Time  00:00");
    lv_obj_set_style_text_color(s_time_label, lv_color_hex(0xcccccc), 0);
    lv_obj_align(s_time_label, LV_ALIGN_CENTER, 0, 80);

    /* Hint */
    lv_obj_t *hint = lv_label_create(s_cont);
    lv_label_set_text(hint, "Short press → Continue brushing");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x666666), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    update_ui();
}

static void stats_on_enter(void) { lv_obj_set_hidden(s_cont, false); update_ui(); }
static void stats_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

hz_screen_t g_scr_stats = {
    .name = "stats", .on_create = stats_on_create,
    .on_enter = stats_on_enter, .on_exit = stats_on_exit,
};
