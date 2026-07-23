/**
 * @file pet_about.c
 * @brief About page
 */
#include "lvgl/lvgl.h"

#define C_LBL 0x9ca3af

void pet_about_create(lv_obj_t *parent)
{
    lv_obj_remove_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *emoji = lv_label_create(parent);
    lv_label_set_text(emoji, "Pro Desk Pet");
    lv_obj_set_style_text_color(emoji, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(emoji, &lv_font_montserrat_20, 0);
    lv_obj_align(emoji, LV_ALIGN_CENTER, 0, -50);

    lv_obj_t *ver = lv_label_create(parent);
    lv_label_set_text(ver, "v1.0.0 — HZCL + LVGL + SDL2");
    lv_obj_set_style_text_color(ver, lv_color_hex(C_LBL), 0);
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_12, 0);
    lv_obj_align(ver, LV_ALIGN_CENTER, 0, -24);

    lv_obj_t *desc = lv_label_create(parent);
    lv_label_set_text(desc,
        "AI-powered desktop companion\n"
        "Cross-platform · Embeddable\n"
        "FSM-driven personality engine\n"
        "9 emotional states · 4 modes\n"
        "Built with HZCL Framework");
    lv_obj_set_style_text_color(desc, lv_color_hex(C_LBL), 0);
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(desc, LV_ALIGN_CENTER, 0, 30);
}
