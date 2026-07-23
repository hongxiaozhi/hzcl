/**
 * @file pet_dash.c
 * @brief Dashboard — 2x2 sensor cards + chart (absolute pos, no lv_obj_align)
 */
#include "lvgl/lvgl.h"
#include "mdi_icons.h"
#include <stdlib.h>

#define C_CARD 0x2d3b4f
#define C_LBL  0x9ca3af
#define C_ACC  0x22d3ee
#define C_ACC2 0xfb923c
#define C_PUR  0xa78bfa
#define C_RED  0xef4444

void pet_dash_create(lv_obj_t *parent)
{
    static const char *icons[] = {MDI_THERMOMETER, MDI_WATER_PERCENT, MDI_GAUGE, MDI_WEATHER};
    static const char *vals[]  = {"26.5C", "65.2%", "1013", "12"};
    static const char *names[] = {"Temperature", "Humidity", "Pressure hPa", "PM2.5"};
    uint32_t colors[] = {C_ACC2, C_ACC, C_PUR, C_RED};
    int px[4] = {14, 248, 14, 248};
    int py[4] = {6, 6, 66, 66};

    for (int i = 0; i < 4; i++) {
        lv_obj_t *card = lv_obj_create(parent);
        lv_obj_set_size(card, 218, 56);
        lv_obj_set_pos(card, px[i], py[i]);
        lv_obj_set_style_bg_color(card, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE);

        /* icon */
        lv_obj_t *ic = lv_label_create(card);
        lv_label_set_text(ic, icons[i]);
        lv_obj_set_pos(ic, 90, 4);
        lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
        lv_obj_set_style_text_color(ic, lv_color_hex(colors[i]), 0);

        /* value */
        lv_obj_t *v = lv_label_create(card);
        lv_label_set_text(v, vals[i]);
        lv_obj_set_pos(v, 90, 26);
        lv_obj_set_style_text_color(v, lv_color_hex(colors[i]), 0);
        lv_obj_set_style_text_font(v, &lv_font_montserrat_16, 0);

        /* label */
        lv_obj_t *l = lv_label_create(card);
        lv_label_set_text(l, names[i]);
        lv_obj_set_pos(l, 90, 44);
        lv_obj_set_style_text_color(l, lv_color_hex(C_LBL), 0);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
    }

    /* Activity chart */
    lv_obj_t *chart = lv_obj_create(parent);
    lv_obj_set_size(chart, 452, 56);
    lv_obj_set_pos(chart, 14, 130);
    lv_obj_set_style_bg_color(chart, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_radius(chart, 10, 0);
    lv_obj_set_style_border_width(chart, 0, 0);

    lv_obj_t *lb = lv_label_create(chart);
    lv_label_set_text(lb, "Activity — last 24h");
    lv_obj_set_pos(lb, 10, 6);
    lv_obj_set_style_text_color(lb, lv_color_hex(C_LBL), 0);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);

    /* Chart bars */
    for (int i = 0; i < 10; i++) {
        lv_obj_t *bar = lv_obj_create(chart);
        int h = 6 + rand() % 28;
        lv_obj_set_size(bar, 16, h);
        lv_obj_set_pos(bar, 20 + i * 42, 48 - h);
        lv_obj_set_style_bg_color(bar, i % 2 ? lv_color_hex(C_PUR) : lv_color_hex(C_ACC), 0);
        lv_obj_set_style_radius(bar, 4, 0);
        lv_obj_set_style_border_width(bar, 0, 0);
        lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    }
}
