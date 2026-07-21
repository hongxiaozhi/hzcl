/**
 * @file screen_sleep.c
 * @brief Sleep screen — dark screen with wake hint
 */
#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "hz_screen.h"
#include "hz_fsm.h"
#include "fsm.h"

static lv_obj_t *s_cont;

static void sleep_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_color(s_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_hidden(s_cont, true);
    lv_obj_remove_flag(s_cont, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, "\xF0\x9F\x92\xA4");
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl);

    lv_obj_t *hint = lv_label_create(s_cont);
    lv_label_set_text(hint, "Click to wake");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x333333), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -40);
}

static void sleep_on_enter(void) { lv_obj_set_hidden(s_cont, false); }
static void sleep_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

hz_screen_t g_scr_sleep = {
    .name = "sleep", .on_create = sleep_on_create,
    .on_enter = sleep_on_enter, .on_exit = sleep_on_exit,
};
