#include "pet_screen.h"
#include "pet_home.h"
#include "pet_chat.h"
#include "pet_dash.h"
#include "pet_settings.h"
#include "pet_about.h"
#include "app.h"
#include "hz_mode.h"
#include "hz_screen.h"
#include "lvgl/lvgl.h"

#define W 480
#define BH 36
#define PAGE_H (320-BH)

static hz_screen_t *s_nav[5];
static int s_cur = 0;

static void nav_go(int dir)
{
    int next = s_cur + dir;
    if (next < 0 || next > 4) return;
    s_cur = next;
    hz_screen_replace(s_nav[next], NULL);
}

/* Gesture detection via press/release coordinates */
static int s_touch_x = -1;

static void press_cb(lv_event_t *e)
{
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    s_touch_x = p.x;
}

static void release_cb(lv_event_t *e)
{
    (void)e;
    if (s_touch_x < 0) return;
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    int dx = p.x - s_touch_x;
    s_touch_x = -1;
    if (dx > 50) nav_go(-1);   /* swipe right → prev page */
    else if (dx < -50) nav_go(1);  /* swipe left → next page */
}

lv_obj_t *pet_status_bar(lv_obj_t *parent, const char *title, int active)
{
    int i;
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, W, BH);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x111c30), 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 8, 14);

    lv_obj_t *t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, lv_color_hex(0xffffff), 0);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *bd = lv_label_create(bar);
    static const char *mn[] = {"NORMAL","FOCUS","PLAY","NIGHT"};
    lv_label_set_text(bd, mn[hz_mode_current()]);
    lv_obj_set_style_text_color(bd, lv_color_hex(0x22d3ee), 0);
    lv_obj_set_style_text_font(bd, &lv_font_montserrat_12, 0);
    lv_obj_set_style_bg_color(bd, lv_color_hex(0x223544), 0);
    lv_obj_set_style_bg_opa(bd, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(bd, 2, 8);
    lv_obj_set_style_radius(bd, 6, 0);
    lv_obj_align(bd, LV_ALIGN_LEFT_MID, 120, 0);

    for (i = 0; i < 5; i++) {
        lv_obj_t *d = lv_obj_create(bar);
        lv_obj_set_size(d, i == active ? 9 : 7, i == active ? 9 : 7);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(d, 0, 0);
        lv_obj_set_style_bg_color(d,
            i == active ? lv_color_hex(0x22d3ee) : lv_color_hex(0x64748b), 0);
        lv_obj_align(d, LV_ALIGN_RIGHT_MID, -80 + i * 13, 0);
        lv_obj_remove_flag(d, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_t *sg = lv_label_create(bar);
    lv_label_set_text(sg, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(sg, lv_color_hex(0x22c55e), 0);
    lv_obj_align(sg, LV_ALIGN_RIGHT_MID, -4, 0);

    lv_obj_t *wf = lv_label_create(bar);
    lv_label_set_text(wf, LV_SYMBOL_BARS);
    lv_obj_set_style_text_color(wf, lv_color_hex(0x22c55e), 0);
    lv_obj_align(wf, LV_ALIGN_RIGHT_MID, -22, 0);

    return bar;
}

void pet_swipe_init(lv_obj_t *obj)
{
    lv_obj_add_event_cb(obj, press_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(obj, release_cb, LV_EVENT_RELEASED, NULL);
}

void pet_register_nav(int idx, hz_screen_t *scr)
{
    if (idx >= 0 && idx < 5) s_nav[idx] = scr;
}
