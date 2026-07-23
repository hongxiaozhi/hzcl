/**
 * @file pet_screen.c
 * @brief Unified screen manager — flex layout, transparent spacers
 *
 * Visual refresh: deeper header, capsule mode badge, signal+WiFi icons,
 * rounded toolbar with refined button styling.
 */
#include "pet_screen.h"
#include "app.h"
#include "hz_mode.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "util_screenshot.h"
#include "mdi_icons.h"
#include "lvgl/lvgl.h"
#include <stdio.h>

/*===========================================================================
 * Layout constants
 *===========================================================================*/
#define W   480
#define BH  44      /* header bar height */
#define PH  278     /* page content: 350 - 44 - 28 = 278 */
#define TBH 28      /* toolbar height (was 30) */

/*===========================================================================
 * Color palette (unified across all screens)
 *===========================================================================*/
#define C_BG           0x0b1120  /* page bg */
#define C_HEADER       0x0c1528  /* header bar bg */
#define C_CARD         0x1e293b  /* card / button bg */
#define C_TOOLBAR      0x0f172a  /* toolbar bg */
#define C_BTN_BD       0x334155  /* border / inactive */
#define C_ACCENT       0x22d3ee  /* cyan */
#define C_ACCENT2      0xfb923c  /* orange */
#define C_GREEN        0x22c55e  /* green */
#define C_LABEL        0x9ca3af  /* grey text */
#define C_WHITE        0xffffff  /* white */
#define C_DOT_INACTIVE 0x475569  /* inactive dot */
#define C_DEBUG_LABEL  0x7f8fa0  /* softer grey for DEBUG */

static lv_obj_t *s_pages[5];
static int       s_cur = 0;
static lv_obj_t *s_dots[5];
static lv_obj_t *s_mode_badge;
static lv_obj_t *s_title_label;

/* State display names for title */
static const char *s_state_names[] = {
    "IDLE", "HAPPY", "EATING", "SLEEPING",
    "SAD", "SICK", "HYPER", "TRAINING", "CLEANING"
};

extern void pet_home_create(lv_obj_t *parent);
extern void pet_chat_create(lv_obj_t *parent);
extern void pet_dash_create(lv_obj_t *parent);
extern void pet_settings_create(lv_obj_t *parent);
extern void pet_about_create(lv_obj_t *parent);

/*===========================================================================
 * Helper: transparent spacer for flex layout
 *===========================================================================*/
static lv_obj_t *make_spacer(lv_obj_t *parent, int w, int h)
{
    lv_obj_t *s = lv_obj_create(parent);
    lv_obj_set_size(s, w, h);
    lv_obj_set_style_bg_opa(s, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(s, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_outline_width(s, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(s, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(s, 0, 0);
    lv_obj_remove_flag(s, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s, LV_SCROLLBAR_MODE_OFF);
    return s;
}

/*===========================================================================
 * Title update: "Pro Desk Pet - <STATE>"
 *   Uses plain ASCII dash (-) to avoid font rendering issues with em-dash
 *===========================================================================*/
static void update_title(void)
{
    char buf[48];
    hz_state_t st = hz_fsm_current();
    const char *sn = (st >= 0 && st < PET_STATE_COUNT)
        ? s_state_names[st] : "IDLE";
    snprintf(buf, sizeof(buf), "Pro Desk Pet - %s", sn);
    lv_label_set_text(s_title_label, buf);
}

static void on_state_changed(void *d, void *u)
{
    (void)d; (void)u;
    update_title();
}

/*===========================================================================
 * Page dots
 *===========================================================================*/
static void update_dots(void)
{
    for (int i = 0; i < 5; i++) {
        lv_obj_set_size(s_dots[i], i == s_cur ? 8 : 6, i == s_cur ? 8 : 6);
        lv_obj_set_style_bg_color(s_dots[i],
            lv_color_hex(i == s_cur ? C_ACCENT : C_DOT_INACTIVE), 0);
    }
}

/*===========================================================================
 * Toolbar nav button (small square icon btn)
 *===========================================================================*/
static lv_obj_t *make_nav_btn(lv_obj_t *parent, const char *label, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 22, 22);
    lv_obj_set_style_bg_color(btn, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *lb = lv_label_create(btn);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lb, lv_color_hex(C_WHITE), 0);
    lv_obj_center(lb);
    return btn;
}

static void prev_cb(lv_event_t *e) { (void)e; if(s_cur>0) pet_screen_switch(s_cur-1); }
static void next_cb(lv_event_t *e) { (void)e; if(s_cur<4) pet_screen_switch(s_cur+1); }
static void shot_cb(lv_event_t *e) { (void)e; lv_timer_handler(); util_screenshot_take("screen"); }

/*===========================================================================
 * Header bar — flex layout
 *   [Title] [spacer] [NORMAL badge] [grow] [d●t●s] [signal] [WiFi]
 *===========================================================================*/
static void build_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, W, BH);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_hex(C_HEADER), 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_set_style_pad_left(bar, 10, 0);   /* left breathing room */
    lv_obj_set_style_pad_right(bar, 8, 0);   /* right breathing room */
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* ---- Title: "Pro Desk Pet - IDLE" (ASCII dash, no font issues) ---- */
    s_title_label = lv_label_create(bar);
    lv_label_set_text(s_title_label, "Pro Desk Pet - IDLE");
    lv_obj_set_style_text_color(s_title_label, lv_color_hex(C_WHITE), 0);
    lv_obj_set_style_text_font(s_title_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_pad_all(s_title_label, 0, 0);

    make_spacer(bar, 8, 1);

    /* ---- Mode capsule badge (pill shape with dark bg) ---- */
    s_mode_badge = lv_label_create(bar);
    lv_label_set_text(s_mode_badge, "NORMAL");
    lv_obj_set_style_text_color(s_mode_badge, lv_color_hex(C_ACCENT), 0);
    lv_obj_set_style_text_font(s_mode_badge, &lv_font_montserrat_12, 0);
    lv_obj_set_style_bg_color(s_mode_badge, lv_color_hex(0x12243e), 0);
    lv_obj_set_style_pad_all(s_mode_badge, 3, 12);    /* h-padding 12 for pill shape */
    lv_obj_set_style_radius(s_mode_badge, 14, 0);     /* pill: radius = height/2 + padding */

    /* Flex-grow spacer pushes right group to end */
    lv_obj_t *grow = make_spacer(bar, 1, 1);
    lv_obj_set_flex_grow(grow, 1);

    /* ---- Page dots (5) ---- */
    for (int i = 0; i < 5; i++) {
        s_dots[i] = lv_obj_create(bar);
        lv_obj_set_size(s_dots[i], 6, 6);
        lv_obj_set_style_radius(s_dots[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(s_dots[i], 0, 0);
        lv_obj_remove_flag(s_dots[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(s_dots[i], lv_color_hex(C_DOT_INACTIVE), 0);
    }

    make_spacer(bar, 9, 1);  /* gap between dots and signal */

    /* ---- Signal bars icon (between dots and WiFi) ---- */
    lv_obj_t *sig = lv_label_create(bar);
    lv_label_set_text(sig, MDI_SIGNAL);
    lv_obj_set_style_text_color(sig, lv_color_hex(C_GREEN), 0);
    lv_obj_set_style_text_font(sig, &mdi_icons_20, 0);
    lv_obj_set_style_pad_all(sig, 0, 0);

    make_spacer(bar, 6, 1);  /* gap between signal and WiFi */

    /* ---- WiFi icon ---- */
    lv_obj_t *wifi = lv_label_create(bar);
    lv_label_set_text(wifi, MDI_WIFI);
    lv_obj_set_style_text_color(wifi, lv_color_hex(C_GREEN), 0);
    lv_obj_set_style_text_font(wifi, &mdi_icons_20, 0);
    lv_obj_set_style_pad_all(wifi, 0, 0);

    update_dots();

    /* Subscribe to state changes for dynamic title */
    hz_event_subscribe(EV_FSM_STATE_CHANGED, on_state_changed, NULL);
    update_title();
}

/*===========================================================================
 * Debug toolbar — dark bg, soft grey label, unified button style
 *===========================================================================*/
static void build_toolbar(lv_obj_t *parent)
{
    lv_obj_t *tb = lv_obj_create(parent);
    lv_obj_set_size(tb, W, TBH);
    lv_obj_set_pos(tb, 0, 350 - TBH);
    lv_obj_set_style_bg_color(tb, lv_color_hex(C_TOOLBAR), 0);
    lv_obj_set_style_border_width(tb, 0, 0);
    lv_obj_set_style_radius(tb, 0, 0);
    lv_obj_set_style_pad_left(tb, 12, 0);
    lv_obj_set_style_pad_right(tb, 8, 0);
    lv_obj_set_flex_flow(tb, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tb, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* DEBUG label — soft grey, blends into dark theme */
    lv_obj_t *lb = lv_label_create(tb);
    lv_label_set_text(lb, "DEBUG");
    lv_obj_set_style_text_color(lb, lv_color_hex(C_DEBUG_LABEL), 0);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);

    /* Flex-grow spacer pushes buttons to right */
    lv_obj_t *grow = make_spacer(tb, 1, 1);
    lv_obj_set_flex_grow(grow, 1);

    make_nav_btn(tb, LV_SYMBOL_LEFT,  prev_cb);
    make_spacer(tb, 3, 1);
    make_nav_btn(tb, LV_SYMBOL_RIGHT, next_cb);
    make_spacer(tb, 6, 1);
    make_nav_btn(tb, "S", shot_cb);
}

/*===========================================================================
 * Screen switch
 *===========================================================================*/
void pet_screen_switch(int idx)
{
    if (idx < 0 || idx > 4 || idx == s_cur) return;
    lv_obj_add_flag(s_pages[s_cur], LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_pages[idx], LV_OBJ_FLAG_HIDDEN);
    s_cur = idx;
    update_dots();
    /* Title stays showing pet state — not page name */
}

/*===========================================================================
 * Screen init
 *===========================================================================*/
void pet_screen_init(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(C_BG), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    build_bar(scr);
    build_toolbar(scr);

    for (int i = 0; i < 5; i++) {
        s_pages[i] = lv_obj_create(scr);
        lv_obj_set_size(s_pages[i], W, PH);
        lv_obj_set_pos(s_pages[i], 0, BH);
        lv_obj_set_style_bg_opa(s_pages[i], LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(s_pages[i], 0, 0);
        lv_obj_set_style_pad_all(s_pages[i], 0, 0);
        lv_obj_remove_flag(s_pages[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(s_pages[i], LV_SCROLLBAR_MODE_OFF);
        if (i != 0) lv_obj_add_flag(s_pages[i], LV_OBJ_FLAG_HIDDEN);
    }

    pet_home_create(s_pages[0]);
    pet_chat_create(s_pages[1]);
    pet_dash_create(s_pages[2]);
    pet_settings_create(s_pages[3]);
    pet_about_create(s_pages[4]);
}

/*===========================================================================
 * Mode update (called externally when mode changes)
 *===========================================================================*/
void pet_screen_update_mode(void)
{
    static const char *mn[] = {"NORMAL", "FOCUS", "PLAY", "NIGHT"};
    int cur = hz_mode_current();
    if (cur >= 0 && cur < MODE_COUNT)
        lv_label_set_text(s_mode_badge, mn[cur]);
}
