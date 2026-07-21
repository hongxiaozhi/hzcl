/**
 * @file screen_ready.c
 * @brief Ready / Settings screen — shows mode, used for both READY and SETTINGS states
 */
#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "hz_screen.h"
#include "hz_fsm.h"
#include "fsm.h"

static lv_obj_t *s_cont;
static lv_obj_t *s_mode_name;
static lv_obj_t *s_title;

static const char *mode_names[] = { "Care", "Clean", "White" };

static void update_ui(void)
{
    hz_state_t cur = hz_fsm_current();
    if (cur == STATE_SETTINGS) {
        lv_label_set_text(s_title, "SETTINGS");
        lv_label_set_text(s_mode_name, "Short press → exit");
    } else {
        lv_label_set_text(s_title, "TOOTHBRUSH");
        lv_label_set_text(s_mode_name, mode_names[g_app.mode]);
    }
}

static void ready_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_color(s_cont, lv_color_hex(0x0a1628), 0);
    lv_obj_set_hidden(s_cont, true);
    lv_obj_remove_flag(s_cont, LV_OBJ_FLAG_CLICKABLE);

    s_title = lv_label_create(s_cont);
    lv_label_set_text(s_title, "TOOTHBRUSH");
    lv_obj_set_style_text_color(s_title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(s_title, &lv_font_montserrat_20, 0);
    lv_obj_align(s_title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *img = lv_obj_create(s_cont);
    lv_obj_set_size(img, 80, 80);
    lv_obj_set_style_bg_color(img, lv_color_hex(0x1a2a4a), 0);
    lv_obj_set_style_border_width(img, 2, 0);
    lv_obj_set_style_border_color(img, lv_color_hex(0x00d2ff), 0);
    lv_obj_set_style_radius(img, 12, 0);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *il = lv_label_create(img);
    lv_label_set_text(il, "\xF0\x9F\xAA\xA5");
    lv_obj_set_style_text_font(il, &lv_font_montserrat_48, 0);
    lv_obj_center(il);

    s_mode_name = lv_label_create(s_cont);
    lv_label_set_text(s_mode_name, "Clean");
    lv_obj_set_style_text_color(s_mode_name, lv_color_hex(0x00d2ff), 0);
    lv_obj_set_style_text_font(s_mode_name, &lv_font_montserrat_24, 0);
    lv_obj_align(s_mode_name, LV_ALIGN_CENTER, 0, -90);

    lv_obj_t *ml = lv_label_create(s_cont);
    lv_label_set_text(ml, "Default Mode");
    lv_obj_set_style_text_color(ml, lv_color_hex(0x888888), 0);
    lv_obj_align(ml, LV_ALIGN_CENTER, 0, -110);

    lv_obj_t *hint = lv_label_create(s_cont);
    lv_label_set_text(hint, "UP=start  DOWN=settings");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x666666), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_obj_t *bat = lv_label_create(s_cont);
    lv_label_set_text(bat, "\xF0\x9F\x94\x8B 85%");
    lv_obj_set_style_text_color(bat, lv_color_hex(0x4ecdc4), 0);
    lv_obj_align(bat, LV_ALIGN_TOP_RIGHT, -10, 10);

    update_ui();
}

static void ready_on_enter(void) { lv_obj_set_hidden(s_cont, false); update_ui(); }
static void ready_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

hz_screen_t g_scr_ready = {
    .name = "ready", .on_create = ready_on_create,
    .on_enter = ready_on_enter, .on_exit = ready_on_exit,
};
