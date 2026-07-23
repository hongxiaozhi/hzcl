/**
 * @file pet_home.c
 * @brief Desk Pet home screen — pet_face + action buttons
 */
#include "pet_home.h"
#include "pet_face.h"
#include "hz_screen.h"
#include "lvgl/lvgl.h"

#define C_DARK    0x0b1120
#define C_BTN_BG  0x1e293b
#define C_ACCENT  0x22d3ee

static lv_obj_t *s_c;
static int s_state = 0;

static void on_prev(lv_event_t *e)   {(void)e;s_state=s_state?s_state-1:8;pet_face_set_state(s_state);}

static void create_btns(lv_obj_t *p)
{
    lv_obj_t *lb = lv_label_create(p);
    lv_label_set_text(lb, "<");
    lv_obj_set_style_text_color(lb, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_16, 0);
    lv_obj_align(lb, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_flag(lb, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lb, on_prev, LV_EVENT_CLICKED, NULL);
}

void pet_home_create(lv_obj_t *parent)
{
    s_c = lv_obj_create(parent);
    lv_obj_set_size(s_c, 480, 284);
    lv_obj_set_style_bg_opa(s_c, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_c, 0, 0);
    lv_obj_set_style_pad_all(s_c, 0, 0);
    lv_obj_remove_flag(s_c, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scrollbar_mode(s_c, LV_SCROLLBAR_MODE_OFF);

    pet_face_create(s_c);
    create_btns(s_c);
    s_state = 0;
    pet_face_set_state(0);
}

void pet_home_update(hz_u32 ms)
{
    pet_face_tick(ms);
}

static hz_screen_t s_scr;
static void scr_create(void)
{
    lv_obj_t *s = lv_scr_act();
    lv_obj_set_style_bg_color(s, lv_color_hex(C_DARK), 0);
    lv_obj_set_style_pad_all(s, 0, 0);
    lv_obj_remove_flag(s, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s, LV_SCROLLBAR_MODE_OFF);
    pet_home_create(s);
}

hz_screen_t *pet_home_screen_get(void) { return &s_scr; }
void pet_home_screen_create(void)
{
    s_scr = (hz_screen_t){
        .name = "home",
        .on_create = scr_create,
        .on_tick = pet_home_update,
    };
    scr_create();
}
