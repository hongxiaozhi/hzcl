/**
 * @file screen_stats.c
 * @brief Stats screen — detailed pet data, chart, achievements, activity log
 */

#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "app.h"
#include "hz_screen.h"
#include "hz_event.h"
#include "hz_fsm.h"

#include <stdio.h>
#include <string.h>

static lv_obj_t *s_cont;
static lv_obj_t *s_hunger_bar, *s_energy_bar, *s_mood_bar, *s_hygiene_bar;
static lv_obj_t *s_hunger_val, *s_energy_val, *s_mood_val, *s_hygiene_val;
static lv_obj_t *s_chart;
static lv_obj_t *s_achievements_label;
static lv_obj_t *s_log_label;
static lv_chart_series_t *s_series_mood, *s_series_energy, *s_series_fullness;

/*===========================================================================
 * Helpers
 *===========================================================================*/
static void stats_update_ui(void)
{
    char buf[64];

    lv_bar_set_value(s_hunger_bar,  100 - g_pet.hunger, LV_ANIM_OFF);
    lv_bar_set_value(s_energy_bar,  g_pet.energy,       LV_ANIM_OFF);
    lv_bar_set_value(s_mood_bar,    g_pet.mood,         LV_ANIM_OFF);
    lv_bar_set_value(s_hygiene_bar, g_pet.hygiene,      LV_ANIM_OFF);

    snprintf(buf, sizeof(buf), "%d/100", 100 - g_pet.hunger); lv_label_set_text(s_hunger_val, buf);
    snprintf(buf, sizeof(buf), "%d/100", g_pet.energy);       lv_label_set_text(s_energy_val, buf);
    snprintf(buf, sizeof(buf), "%d/100", g_pet.mood);          lv_label_set_text(s_mood_val, buf);
    snprintf(buf, sizeof(buf), "%d/100", g_pet.hygiene);       lv_label_set_text(s_hygiene_val, buf);

    lv_chart_set_next_value(s_chart, s_series_mood,     g_pet.mood);
    lv_chart_set_next_value(s_chart, s_series_energy,   g_pet.energy);
    lv_chart_set_next_value(s_chart, s_series_fullness, 100 - g_pet.hunger);

    char ach[128] = "";
    strcat(ach, g_achievements.fed_10_times    ? "Y Full Belly\n"    : "N Full Belly\n");
    strcat(ach, g_achievements.mood_80_for_10  ? "Y Happy Kid\n"     : "N Happy Kid\n");
    strcat(ach, g_achievements.trained_5_times ? "Y Trainer\n"       : "N Trainer\n");
    strcat(ach, g_achievements.all_states_seen ? "Y Explorer\n"      : "N Explorer\n");
    strcat(ach, g_achievements.reached_hyper   ? "Y Hyper!\n"        : "N Hyper!\n");
    lv_label_set_text(s_achievements_label, ach);

    char log[256] = "";
    for (hz_u8 i = 0; i < g_activity_log.count; i++) {
        hz_u8 idx = (g_activity_log.head + ACTIVITY_LOG_SIZE - 1 - i) % ACTIVITY_LOG_SIZE;
        strcat(log, g_activity_log.entries[idx]); strcat(log, "\n");
    }
    if (g_activity_log.count == 0) strcat(log, "(no activity yet)");
    lv_label_set_text(s_log_label, log);
}

static void stats_update_event(void *data, void *user) { (void)data; (void)user; stats_update_ui(); }
static void btn_back_cb(lv_event_t *e) { (void)e; hz_screen_pop(); }

/*===========================================================================
 * Lifecycle
 *===========================================================================*/
static void stats_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_hidden(s_cont, true);

    /* Back */
    lv_obj_t *btn = lv_btn_create(s_cont);
    lv_obj_set_size(btn, 60, 25);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
    lv_obj_add_event_cb(btn, btn_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(btn); lv_label_set_text(lbl, "< Back"); lv_obj_center(lbl);

    lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, "Pet Statistics");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 8);

    /* Bars */
    const char *names[] = { "Fullness", "Energy", "Mood", "Hygiene" };
    hz_u32 colors[] = { 0xff6b6b, 0x4ecdc4, 0xffd93d, 0x74b9ff };
    lv_obj_t **bars[] = { &s_hunger_bar, &s_energy_bar, &s_mood_bar, &s_hygiene_bar };
    lv_obj_t **vals[] = { &s_hunger_val, &s_energy_val, &s_mood_val, &s_hygiene_val };

    for (int i = 0; i < 4; i++) {
        lbl = lv_label_create(s_cont); lv_label_set_text(lbl, names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(colors[i]), 0);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, 40 + i * 22);

        *vals[i] = lv_label_create(s_cont); lv_label_set_text(*vals[i], "0/100");
        lv_obj_set_style_text_color(*vals[i], lv_color_hex(0xcccccc), 0);
        lv_obj_align(*vals[i], LV_ALIGN_TOP_LEFT, 90, 40 + i * 22);

        *bars[i] = lv_bar_create(s_cont);
        lv_obj_set_size(*bars[i], 150, 8);
        lv_obj_align(*bars[i], LV_ALIGN_TOP_LEFT, 140, 42 + i * 22);
        lv_obj_set_style_bg_color(*bars[i], lv_color_hex(0x333333), 0);
        lv_bar_set_range(*bars[i], 0, 100);
        lv_bar_set_value(*bars[i], 50, LV_ANIM_OFF);
    }

    /* Chart */
    lbl = lv_label_create(s_cont); lv_label_set_text(lbl, "-- Recent History --");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, 130);

    s_chart = lv_chart_create(s_cont);
    lv_obj_set_size(s_chart, 300, 80);
    lv_obj_align(s_chart, LV_ALIGN_TOP_LEFT, 10, 150);
    lv_chart_set_type(s_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_axis_range(s_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_point_count(s_chart, CHART_DATA_POINTS);
    s_series_mood     = lv_chart_add_series(s_chart, lv_color_hex(0xffd93d), LV_CHART_AXIS_PRIMARY_Y);
    s_series_energy   = lv_chart_add_series(s_chart, lv_color_hex(0x4ecdc4), LV_CHART_AXIS_PRIMARY_Y);
    s_series_fullness = lv_chart_add_series(s_chart, lv_color_hex(0xff6b6b), LV_CHART_AXIS_PRIMARY_Y);

    /* Achievements */
    lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, "-- Achievements --");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, 240);

    s_achievements_label = lv_label_create(s_cont);
    lv_label_set_text(s_achievements_label, "N Full Belly\nN Happy Kid\nN Trainer\nN Explorer\nN Hyper!");
    lv_obj_set_style_text_color(s_achievements_label, lv_color_hex(0xcccccc), 0);
    lv_obj_align(s_achievements_label, LV_ALIGN_TOP_LEFT, 10, 260);

    /* Activity log */
    lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, "-- Activity Log --");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, 340);

    s_log_label = lv_label_create(s_cont);
    lv_label_set_text(s_log_label, "(no activity yet)");
    lv_obj_set_style_text_color(s_log_label, lv_color_hex(0xcccccc), 0);
    lv_obj_align(s_log_label, LV_ALIGN_TOP_LEFT, 10, 360);
}

static void stats_on_load(void *ctx)
{
    (void)ctx;
    hz_event_subscribe(EV_FSM_STATE_CHANGED, stats_update_event, NULL);
    stats_update_ui();
}

static void stats_on_unload(void) { hz_event_unsubscribe(EV_FSM_STATE_CHANGED, stats_update_event); }
static void stats_on_enter(void)  { lv_obj_set_hidden(s_cont, false); stats_update_ui(); }
static void stats_on_exit(void)   { lv_obj_set_hidden(s_cont, true); }

/*===========================================================================
 * Screen descriptor & factory
 *===========================================================================*/
static hz_screen_t s_stats_screen;

void screen_stats_create(void)
{
    s_stats_screen = (hz_screen_t){
        .name = "stats", .on_create = stats_on_create,
        .on_load = stats_on_load, .on_unload = stats_on_unload,
        .on_enter = stats_on_enter, .on_exit = stats_on_exit,
    };
    stats_on_create();
}

hz_screen_t *screen_stats_get(void) { return &s_stats_screen; }
