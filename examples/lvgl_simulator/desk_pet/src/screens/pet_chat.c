/**
 * @file pet_chat.c
 * @brief AI Chat page — absolute positioning
 */
#include "lvgl/lvgl.h"
#include <string.h>

#define C_CARD 0x2d3b4f
#define C_BTN  0x334155
#define C_ACC  0x22d3ee

static lv_obj_t *s_area, *s_inp;
static int s_msg_y = 4;

static void add_msg(const char *txt, int is_user)
{
    lv_obj_t *m = lv_label_create(s_area);
    lv_label_set_text(m, txt);
    lv_obj_set_style_bg_color(m, is_user ? lv_color_hex(C_ACC) : lv_color_hex(C_CARD), 0);
    lv_obj_set_style_text_color(m, is_user ? lv_color_hex(0x000000) : lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(m, 8, 0);
    lv_obj_set_style_pad_all(m, 4, 10);
    lv_obj_set_style_text_font(m, &lv_font_montserrat_12, 0);
    lv_obj_set_width(m, 420);
    lv_obj_set_pos(m, is_user ? 20 : 10, s_msg_y);
    s_msg_y += 30;
    /* Scroll to bottom */
    lv_obj_scroll_to_y(s_area, s_msg_y, LV_ANIM_OFF);
}

static void send_cb(lv_event_t *e)
{
    (void)e;
    const char *t = lv_textarea_get_text(s_inp);
    if (!t || !t[0]) return;
    add_msg(t, 1);
    lv_textarea_set_text(s_inp, "");
}

void pet_chat_create(lv_obj_t *parent)
{
    /* Message area */
    s_area = lv_obj_create(parent);
    lv_obj_set_size(s_area, 480, 234);
    lv_obj_set_pos(s_area, 0, 0);
    lv_obj_set_style_bg_opa(s_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_area, 0, 0);
    lv_obj_set_style_pad_all(s_area, 0, 0);
    lv_obj_set_scrollbar_mode(s_area, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(s_area, LV_DIR_VER);
    s_msg_y = 4;

    add_msg("Hi! I'm your desk pet. Ask me anything!", 0);

    /* Input row at bottom */
    s_inp = lv_textarea_create(parent);
    lv_obj_set_size(s_inp, 370, 32);
    lv_obj_set_pos(s_inp, 5, 248);
    lv_obj_set_style_bg_color(s_inp, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_border_color(s_inp, lv_color_hex(C_BTN), 0);
    lv_obj_set_style_border_width(s_inp, 1, 0);
    lv_obj_set_style_radius(s_inp, 8, 0);
    lv_obj_set_style_pad_all(s_inp, 4, 10);
    lv_obj_set_style_text_color(s_inp, lv_color_hex(0xffffff), 0);
    lv_textarea_set_placeholder_text(s_inp, "Say something...");
    lv_textarea_set_one_line(s_inp, true);

    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 80, 32);
    lv_obj_set_pos(btn, 385, 248);
    lv_obj_set_style_bg_color(btn, lv_color_hex(C_ACC), 0);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_add_event_cb(btn, send_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lb = lv_label_create(btn);
    lv_label_set_text(lb, "Send");
    lv_obj_set_style_text_color(lb, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
    lv_obj_center(lb);
}
