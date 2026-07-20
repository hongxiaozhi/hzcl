/**
 * @file screen_settings.c
 * @brief Settings screen — configurable parameters via sliders and buttons
 */

#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "app.h"
#include "hz_screen.h"
#include "hz_config_mgr.h"

#include <stdio.h>
#include <string.h>

static lv_obj_t *s_cont;
static lv_obj_t *s_hunger_slider, *s_energy_slider, *s_mood_slider;
static lv_obj_t *s_hunger_label, *s_energy_label, *s_mood_label;

/*===========================================================================
 * Helpers
 *===========================================================================*/
static void set_slider(lv_obj_t *s, float val)   { lv_slider_set_value(s, (int32_t)(val * 20.0f), LV_ANIM_OFF); }
static float get_slider(lv_obj_t *s)              { return (float)lv_slider_get_value(s) / 20.0f; }
static void update_slider_lbl(lv_obj_t *s, lv_obj_t *l) { char b[16]; snprintf(b, sizeof(b), "%.1fx", get_slider(s)); lv_label_set_text(l, b); }

extern hz_err_t config_set(hz_config_id_t id, const void *value);

/*===========================================================================
 * Slider callbacks
 *===========================================================================*/
static void hunger_cb(lv_event_t *e) { lv_obj_t *s = lv_event_get_target(e); float v = get_slider(s); config_set(CFG_HUNGER_DECAY, &v); update_slider_lbl(s, s_hunger_label); }
static void energy_cb(lv_event_t *e) { lv_obj_t *s = lv_event_get_target(e); float v = get_slider(s); config_set(CFG_ENERGY_DECAY, &v); update_slider_lbl(s, s_energy_label); }
static void mood_cb(lv_event_t *e)   { lv_obj_t *s = lv_event_get_target(e); float v = get_slider(s); config_set(CFG_MOOD_DECAY, &v); update_slider_lbl(s, s_mood_label); }

/*===========================================================================
 * Color preset colors and callback
 *===========================================================================*/
static const hz_u32 s_color_presets[] = { 0x00d2ff, 0x74b9ff, 0xe17055, 0x6c5ce7, 0x00b894 };

static void color_cb(lv_event_t *e)
{
    int idx = (int)(hz_u32)lv_event_get_user_data(e);
    if (idx >= 0 && idx < 5)
        config_set(CFG_THEME_COLOR, &s_color_presets[idx]);
}

/*===========================================================================
 * Reset
 *===========================================================================*/
static void reset_cb(lv_event_t *e)
{
    (void)e;
    hz_config_mgr_reset();
    hz_config_mgr_get(CFG_HUNGER_DECAY,  &g_app_cfg.hunger_decay);
    hz_config_mgr_get(CFG_ENERGY_DECAY,  &g_app_cfg.energy_decay);
    hz_config_mgr_get(CFG_MOOD_DECAY,    &g_app_cfg.mood_decay);
    set_slider(s_hunger_slider, g_app_cfg.hunger_decay); update_slider_lbl(s_hunger_slider, s_hunger_label);
    set_slider(s_energy_slider, g_app_cfg.energy_decay); update_slider_lbl(s_energy_slider, s_energy_label);
    set_slider(s_mood_slider,   g_app_cfg.mood_decay);   update_slider_lbl(s_mood_slider,   s_mood_label);
}

static void btn_back_cb(lv_event_t *e) { (void)e; hz_screen_pop(); }

/*===========================================================================
 * Lifecycle
 *===========================================================================*/
static void settings_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_hidden(s_cont, true);

    lv_obj_t *btn = lv_btn_create(s_cont);
    lv_obj_set_size(btn, 60, 25); lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
    lv_obj_add_event_cb(btn, btn_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(btn); lv_label_set_text(lbl, "< Back"); lv_obj_center(lbl);

    lbl = lv_label_create(s_cont); lv_label_set_text(lbl, "Settings");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 8);

    int y = 45;
    lbl = lv_label_create(s_cont); lv_label_set_text(lbl, "-- Decay Rates --");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, y); y += 25;

    /* Sliders */
    lv_obj_t **sliders[] = { &s_hunger_slider, &s_energy_slider, &s_mood_slider };
    lv_obj_t **labels[] = { &s_hunger_label, &s_energy_label, &s_mood_label };
    lv_event_cb_t cbs[] = { hunger_cb, energy_cb, mood_cb };
    const char *names[] = { "Hunger:", "Energy:", "Mood:" };

    for (int i = 0; i < 3; i++) {
        lbl = lv_label_create(s_cont); lv_label_set_text(lbl, names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xcccccc), 0);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 20, y);

        *sliders[i] = lv_slider_create(s_cont);
        lv_obj_set_size(*sliders[i], 150, 12);
        lv_obj_align(*sliders[i], LV_ALIGN_TOP_LEFT, 90, y - 3);
        lv_slider_set_range(*sliders[i], 2, 100);
        lv_slider_set_value(*sliders[i], 20, LV_ANIM_OFF);
        lv_obj_add_event_cb(*sliders[i], cbs[i], LV_EVENT_VALUE_CHANGED, NULL);

        *labels[i] = lv_label_create(s_cont); lv_label_set_text(*labels[i], "1.0x");
        lv_obj_set_style_text_color(*labels[i], lv_color_hex(0x00d2ff), 0);
        lv_obj_align(*labels[i], LV_ALIGN_TOP_LEFT, 250, y);
        y += 25;
    }

    y += 10;
    lbl = lv_label_create(s_cont); lv_label_set_text(lbl, "-- Theme --");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, y); y += 25;

    for (int i = 0; i < 5; i++) {
        btn = lv_btn_create(s_cont); lv_obj_set_size(btn, 35, 35);
        lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20 + i * 45, y);
        lv_obj_set_style_bg_color(btn, lv_color_hex(s_color_presets[i]), 0);
        lv_obj_add_event_cb(btn, color_cb, LV_EVENT_CLICKED, (void *)(hz_u32)i);
    }

    y += 55;
    btn = lv_btn_create(s_cont); lv_obj_set_size(btn, 160, 30);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 20, y);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x990000), 0);
    lv_obj_add_event_cb(btn, reset_cb, LV_EVENT_CLICKED, NULL);
    lbl = lv_label_create(btn); lv_label_set_text(lbl, "Reset to Defaults"); lv_obj_center(lbl);
}

static void settings_on_load(void *ctx)
{
    (void)ctx;
    hz_config_mgr_get(CFG_HUNGER_DECAY, &g_app_cfg.hunger_decay);
    hz_config_mgr_get(CFG_ENERGY_DECAY, &g_app_cfg.energy_decay);
    hz_config_mgr_get(CFG_MOOD_DECAY,   &g_app_cfg.mood_decay);
    set_slider(s_hunger_slider, g_app_cfg.hunger_decay);
    set_slider(s_energy_slider, g_app_cfg.energy_decay);
    set_slider(s_mood_slider,   g_app_cfg.mood_decay);
    update_slider_lbl(s_hunger_slider, s_hunger_label);
    update_slider_lbl(s_energy_slider, s_energy_label);
    update_slider_lbl(s_mood_slider,   s_mood_label);
}

static void settings_on_enter(void) { lv_obj_set_hidden(s_cont, false); }
static void settings_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

/*===========================================================================
 * Screen descriptor & factory
 *===========================================================================*/
static hz_screen_t s_settings_screen;

void screen_settings_create(void)
{
    s_settings_screen = (hz_screen_t){
        .name = "settings", .on_create = settings_on_create,
        .on_load = settings_on_load, .on_enter = settings_on_enter, .on_exit = settings_on_exit,
    };
    settings_on_create();
}

hz_screen_t *screen_settings_get(void) { return &s_settings_screen; }
