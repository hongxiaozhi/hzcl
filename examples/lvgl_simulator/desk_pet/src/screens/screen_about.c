/**
 * @file screen_about.c
 * @brief About / Help screen — controls, tips, credits
 */

#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "app.h"
#include "hz_screen.h"

/*===========================================================================
 * Widget references
 *===========================================================================*/
static lv_obj_t *s_cont;

/*===========================================================================
 * Back button
 *===========================================================================*/
static void btn_back_cb(lv_event_t *e)
{
    (void)e;
    hz_screen_pop();
}

/*===========================================================================
 * Screen lifecycle
 *===========================================================================*/
static void about_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_hidden(s_cont, true);

    /* Back button */
    lv_obj_t *btn = lv_btn_create(s_cont);
    lv_obj_set_size(btn, 50, 25);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0);
    lv_obj_add_event_cb(btn, btn_back_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "< Back");
    lv_obj_center(lbl);

    /* Title */
    lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, "About / Help");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 8);

    /* Content */
    const char *text =
        "\xF0\x9F\x96\xA5 Pro Desk Pet v1.0\n\n"
        "-- Controls --\n"
        "Feed    - Lowers hunger\n"
        "Pet     - Boosts mood\n"
        "Sleep   - Restores energy\n"
        "Clean   - Improves hygiene\n"
        "Train   - Active training mode\n"
        "Mode    - Cycle device modes\n\n"
        "-- Tips --\n"
        "Keep hunger below 50%%\n"
        "Keep energy above 30%%\n"
        "Interact regularly!\n"
        "Focus mode slows decay\n"
        "Play mode is more demanding\n\n"
        "-- HZCL Modules --\n"
        "Y FSM   Y Screen   Y Event\n"
        "Y Timer Y Mode     Y Sensor\n"
        "Y Alarm Y Config   Y Power\n\n"
        "Built with LVGL + SDL2 + MinGW";

    lbl = lv_label_create(s_cont);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xcccccc), 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, 40);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl, 450);
}

static void about_on_enter(void) { lv_obj_set_hidden(s_cont, false); }
static void about_on_exit(void)  { lv_obj_set_hidden(s_cont, true); }

/*===========================================================================
 * Screen descriptor & factory
 *===========================================================================*/
static hz_screen_t s_about_screen;

void screen_about_create(void)
{
    s_about_screen = (hz_screen_t){
        .name     = "about",
        .on_create = about_on_create,
        .on_enter  = about_on_enter,
        .on_exit   = about_on_exit,
    };
    about_on_create();
}

hz_screen_t *screen_about_get(void)
{
    return &s_about_screen;
}
