/**
 * @file pet_settings.c
 * @brief Settings — toggles, AI backend, theme, notifications, auto-save (absolute layout)
 */
#include "lvgl/lvgl.h"
#include "mdi_icons.h"

#define C_CARD 0x2d3b4f
#define C_ACC  0x22d3ee
#define C_LBL  0x9ca3af

static void toggle_cb(lv_event_t *e)
{
    lv_obj_t *t = lv_event_get_target(e);
    lv_color_t c = lv_obj_get_style_bg_color(t, 0);
    int on = (c.red == 0x22 && c.green == 0xd3 && c.blue == 0xee);
    lv_obj_set_style_bg_color(t,
        on ? lv_color_hex(0x334155) : lv_color_hex(C_ACC), 0);
    lv_obj_t *knob = lv_event_get_user_data(e);
    if (knob) {
        lv_obj_align(knob, on ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID,
            on ? 2 : -2, 0);
    }
}

void pet_settings_create(lv_obj_t *parent)
{
    static const char *lbls[] = {"Voice Interaction", "AI Backend", "Theme Accent",
                                  "Notifications", "Auto-save"};
    static const char *dscs[] = {"Speech input/output", "DeepSeek / OpenAI / Local",
                                  "Color scheme", "Remind when needed", "30 seconds"};
    static const char *si[]  = {MDI_ACCOUNT_VOICE, MDI_ROBOT, MDI_PALETTE, MDI_ALERT_OUTLINE, MDI_BRIGHTNESS_5};

    for (int i = 0; i < 5; i++) {
        int y = 4 + i * 52;
        lv_obj_t *card = lv_obj_create(parent);
        lv_obj_set_size(card, 452, 48);
        lv_obj_set_pos(card, 14, y);
        lv_obj_set_style_bg_color(card, lv_color_hex(C_CARD), 0);
        lv_obj_set_style_radius(card, 10, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_pad_all(card, 6, 10);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE);

        /* Icon */
        lv_obj_t *ic = lv_label_create(card);
        lv_label_set_text(ic, si[i]);
        lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
        lv_obj_set_style_text_color(ic, lv_color_hex(C_ACC), 0);
        lv_obj_align(ic, LV_ALIGN_LEFT_MID, 6, -6);

        /* Label */
        lv_obj_t *lb = lv_label_create(card);
        lv_label_set_text(lb, lbls[i]);
        lv_obj_set_style_text_color(lb, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 30, -6);

        /* Description */
        lv_obj_t *ds = lv_label_create(card);
        lv_label_set_text(ds, dscs[i]);
        lv_obj_set_style_text_color(ds, lv_color_hex(C_LBL), 0);
        lv_obj_set_style_text_font(ds, &lv_font_montserrat_12, 0);
        lv_obj_align(ds, LV_ALIGN_LEFT_MID, 30, 6);

        /* Toggle switch (items 0, 3) */
        if (i == 0 || i == 3) {
            lv_obj_t *tg = lv_obj_create(card);
            lv_obj_set_size(tg, 44, 24);
            lv_obj_set_style_bg_color(tg, lv_color_hex(C_ACC), 0);
            lv_obj_set_style_radius(tg, 12, 0);
            lv_obj_set_style_border_width(tg, 0, 0);
            lv_obj_align(tg, LV_ALIGN_RIGHT_MID, -4, 0);
            lv_obj_add_flag(tg, LV_OBJ_FLAG_CLICKABLE);

            lv_obj_t *knob = lv_obj_create(tg);
            lv_obj_set_size(knob, 20, 20);
            lv_obj_set_style_bg_color(knob, lv_color_hex(0xffffff), 0);
            lv_obj_set_style_radius(knob, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(knob, 0, 0);
            lv_obj_align(knob, LV_ALIGN_RIGHT_MID, -2, 0);
            lv_obj_remove_flag(knob, LV_OBJ_FLAG_CLICKABLE);

            lv_obj_add_event_cb(tg, toggle_cb, LV_EVENT_CLICKED, knob);
        }

        /* AI Backend label */
        if (i == 1) {
            lv_obj_t *lai = lv_label_create(card);
            lv_label_set_text(lai, "DeepSeek");
            lv_obj_set_style_text_color(lai, lv_color_hex(C_ACC), 0);
            lv_obj_set_style_text_font(lai, &lv_font_montserrat_12, 0);
            lv_obj_align(lai, LV_ALIGN_RIGHT_MID, -6, 0);
        }

        /* Theme color dots */
        if (i == 2) {
            lv_color_t dots[] = {lv_color_hex(0x22d3ee), lv_color_hex(0xfb923c),
                                 lv_color_hex(0xa78bfa), lv_color_hex(0xf472b6)};
            for (int c = 0; c < 4; c++) {
                lv_obj_t *dot = lv_obj_create(card);
                lv_obj_set_size(dot, 16, 16);
                lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_bg_color(dot, dots[c], 0);
                lv_obj_set_style_border_width(dot, c == 0 ? 2 : 0, 0);
                lv_obj_set_style_border_color(dot, lv_color_hex(0xffffff), 0);
                lv_obj_align(dot, LV_ALIGN_RIGHT_MID, -8 - c * 22, 0);
                lv_obj_remove_flag(dot, LV_OBJ_FLAG_CLICKABLE);
            }
        }

        /* Auto-save slider */
        if (i == 4) {
            lv_obj_t *sl = lv_slider_create(card);
            lv_obj_set_size(sl, 80, 4);
            lv_obj_align(sl, LV_ALIGN_RIGHT_MID, -6, 0);
            lv_slider_set_range(sl, 5, 120);
            lv_slider_set_value(sl, 30, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(sl, lv_color_hex(0x334155), LV_PART_MAIN);
            lv_obj_set_style_bg_color(sl, lv_color_hex(C_ACC), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(sl, lv_color_hex(C_ACC), LV_PART_KNOB);
            lv_obj_set_style_radius(sl, 2, LV_PART_MAIN | LV_PART_INDICATOR);
            lv_obj_set_style_radius(sl, LV_RADIUS_CIRCLE, LV_PART_KNOB);
        }
    }
}
