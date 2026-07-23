/**
 * @file pet_home.c
 * @brief Home — pet face + status bar cards + action buttons
 *
 * Visual refresh: rounded status cards with icon(left) + capsule progress
 * bar(center) + percentage(right), vertical icon-over-text action buttons.
 * All three card sections use explicit positioning — no overlap.
 */
#include "pet_face.h"
#include "app.h"
#include "hz_fsm.h"
#include "hz_mode.h"
#include "hz_event.h"
#include "lvgl/lvgl.h"
#include "mdi_icons.h"
#include <stdio.h>

/*===========================================================================
 * Unified color palette
 *===========================================================================*/
#define C_BG       0x0b1120
#define C_CARD     0x1e293b
#define C_BTN_BD   0x334155
#define C_ACCENT   0x22d3ee
#define C_ACCENT2  0xfb923c
#define C_GREEN    0x22c55e
#define C_YELLOW   0xfacc15
#define C_PURPLE   0xa78bfa
#define C_LABEL    0x9ca3af
#define C_WHITE    0xffffff

/*===========================================================================
 * Layout constants — all computed relative to 480px width
 *===========================================================================*/
/* Status cards: 4×108 across 480px, unified 10px side margin */
#define CARD_W   108
#define CARD_H   38
#define CARD_Y   188       /* face bottom ≈ 184, gap = 4px */
#define CARD_GAP 9
#define CARD_X0  10        /* left margin = 10, matches right margin */
#define CARD_PAD 5         /* internal padding */
#define CARD_RADIUS 10

/* Track within card — 46px wide, centered between icon and % label */
#define TRACK_W  44        /* slightly narrower for better gaps */
#define TRACK_H  6
#define TRACK_X  30        /* x: 5(pad) + 22(icon area) + 3(gap) */
/* TRACK_Y computed as (CARD_H - TRACK_H) / 2 = (38-6)/2 = 16 */
#define TRACK_Y  16

/* Action buttons: 6×70 across 480px, unified 10px side margin */
#define BTN_W   70
#define BTN_H   44
#define BTN_Y   232         /* CARD_Y(188)+CARD_H(38)=226, +6px gap=232, bottom=276 ≤ PH(278) */
#define BTN_GAP 8
#define BTN_X0  10
#define BTN_RADIUS 14

/*===========================================================================
 * Globals
 *===========================================================================*/
static lv_obj_t *s_hunger_bar, *s_energy_bar, *s_mood_bar, *s_hygiene_bar;
static lv_obj_t *s_hunger_pct, *s_energy_pct, *s_mood_pct, *s_hygiene_pct;
static lv_obj_t *s_mode_btn;

/*===========================================================================
 * Status bar card — three-section layout:
 *   [icon left]  [===progress bar===]  [NN% right]
 *
 * All positions computed explicitly to guarantee no overlap.
 *===========================================================================*/
static void make_bar_item(lv_obj_t *parent, int x, const char *icon, lv_color_t color,
                           lv_obj_t **bar_out, lv_obj_t **pct_out)
{
    /* ---- Card container ---- */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_set_pos(card, x, CARD_Y);
    lv_obj_set_style_bg_color(card, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, CARD_RADIUS, 0);
    lv_obj_set_style_pad_all(card, CARD_PAD, CARD_PAD);
    lv_obj_set_style_outline_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, 0, 0);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    /* ---- Section 1: Icon (left-aligned, vertical center) ---- */
    lv_obj_t *ic = lv_label_create(card);
    lv_label_set_text(ic, icon);
    lv_obj_set_style_text_color(ic, color, 0);
    lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 2, 0);   /* x ≈ 5+2 = 7 */

    /* ---- Section 2: Progress bar track (explicit position) ---- */
    lv_obj_t *track = lv_obj_create(card);
    lv_obj_set_size(track, TRACK_W, TRACK_H);
    lv_obj_set_pos(track, TRACK_X, TRACK_Y);
    lv_obj_set_style_bg_color(track, lv_color_hex(C_BTN_BD), 0);
    lv_obj_set_style_border_width(track, 0, 0);
    lv_obj_set_style_radius(track, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_outline_width(track, 0, 0);
    lv_obj_set_style_shadow_width(track, 0, 0);
    lv_obj_remove_flag(track, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(track, LV_SCROLLBAR_MODE_OFF);

    /* ---- Section 2b: Progress bar fill (overlaps track, fills left→right) ---- */
    lv_obj_t *fill = lv_obj_create(card);
    lv_obj_set_size(fill, 23, TRACK_H);          /* initial 50% fill */
    lv_obj_set_pos(fill, TRACK_X, TRACK_Y);       /* same origin as track */
    lv_obj_set_style_bg_color(fill, color, 0);
    lv_obj_set_style_border_width(fill, 0, 0);
    lv_obj_set_style_radius(fill, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_outline_width(fill, 0, 0);
    lv_obj_set_style_shadow_width(fill, 0, 0);
    lv_obj_remove_flag(fill, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(fill, LV_SCROLLBAR_MODE_OFF);

    /* ---- Section 3: Percentage label (right-aligned, vertical center) ---- */
    lv_obj_t *pct = lv_label_create(card);
    lv_label_set_text(pct, "50%");
    lv_obj_set_style_text_color(pct, lv_color_hex(C_LABEL), 0);
    lv_obj_set_style_text_font(pct, &lv_font_montserrat_12, 0);
    lv_obj_align(pct, LV_ALIGN_RIGHT_MID, -3, 0);  /* right margin = 3 */

    *bar_out = fill;
    *pct_out = pct;
}

/*===========================================================================
 * Action button — vertical layout: icon on top, label below
 *===========================================================================*/
static lv_obj_t *make_btn(lv_obj_t *parent, int x, int y, const char *icon,
                           const char *label, lv_event_cb_t cb, int highlight)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, BTN_W, BTN_H);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn, lv_color_hex(C_CARD), 0);
    lv_obj_set_style_border_width(btn, highlight ? 2 : 0, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(C_ACCENT), 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, BTN_RADIUS, 0);
    lv_obj_set_style_pad_all(btn, 2, 4);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    /* Icon — top center */
    lv_obj_t *ic = lv_label_create(btn);
    lv_label_set_text(ic, icon);
    lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
    lv_obj_set_style_text_color(ic, lv_color_hex(C_WHITE), 0);
    lv_obj_align(ic, LV_ALIGN_TOP_MID, 0, 4);

    /* Label — bottom center */
    lv_obj_t *lb = lv_label_create(btn);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lb, lv_color_hex(C_WHITE), 0);
    lv_obj_align(lb, LV_ALIGN_BOTTOM_MID, 0, -4);

    return btn;
}

/*===========================================================================
 * Button callbacks — business logic unchanged
 *===========================================================================*/
static void btn_feed_cb(lv_event_t *e)  { (void)e; hz_fsm_execute_action(ACT_FEED, NULL); }
static void btn_pet_cb(lv_event_t *e)   { (void)e; hz_fsm_execute_action(ACT_PET, NULL); }
static void btn_sleep_cb(lv_event_t *e) { (void)e; hz_fsm_execute_action(ACT_GO_SLEEP, NULL); }
static void btn_clean_cb(lv_event_t *e) { (void)e; hz_fsm_execute_action(ACT_CLEAN, NULL); }
static void btn_train_cb(lv_event_t *e) { (void)e; hz_fsm_execute_action(ACT_TRAIN, NULL); }
static void btn_mode_cb(lv_event_t *e)
{ (void)e; hz_mode_switch((hz_mode_current() + 1) % MODE_COUNT); }

/*===========================================================================
 * Update progress bars — scales fill width 0..TRACK_W
 *===========================================================================*/
static void update_bars(void)
{
    extern pet_status_t g_pet;
    char b[8];
    lv_obj_set_size(s_hunger_bar,  TRACK_W * (100 - g_pet.hunger)  / 100, TRACK_H);
    snprintf(b, sizeof(b), "%d%%", 100 - (int)g_pet.hunger); lv_label_set_text(s_hunger_pct, b);
    lv_obj_set_size(s_energy_bar,  TRACK_W * g_pet.energy  / 100, TRACK_H);
    snprintf(b, sizeof(b), "%d%%", (int)g_pet.energy); lv_label_set_text(s_energy_pct, b);
    lv_obj_set_size(s_mood_bar,    TRACK_W * g_pet.mood    / 100, TRACK_H);
    snprintf(b, sizeof(b), "%d%%", (int)g_pet.mood); lv_label_set_text(s_mood_pct, b);
    lv_obj_set_size(s_hygiene_bar, TRACK_W * g_pet.hygiene / 100, TRACK_H);
    snprintf(b, sizeof(b), "%d%%", (int)g_pet.hygiene); lv_label_set_text(s_hygiene_pct, b);
}

/*===========================================================================
 * Mode change — toggle border highlight on mode button
 *===========================================================================*/
static void on_mode_changed(void *d, void *u)
{
    (void)d; (void)u;
    int cur = hz_mode_current();
    if (cur == MODE_NORMAL) {
        lv_obj_set_style_border_width(s_mode_btn, 0, 0);
    } else {
        lv_obj_set_style_border_width(s_mode_btn, 2, 0);
        static const uint32_t mc[] = {0, C_ACCENT, C_ACCENT2, C_PURPLE};
        if (cur >= 0 && cur < 4)
            lv_obj_set_style_border_color(s_mode_btn, lv_color_hex(mc[cur]), 0);
    }
}

/*===========================================================================
 * State change — refresh bars
 *===========================================================================*/
static void on_fsm_change(void *d, void *u) { (void)d; (void)u; update_bars(); }

/*===========================================================================
 * Create the home screen
 *===========================================================================*/
void pet_home_create(lv_obj_t *parent)
{
    lv_obj_remove_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    /* Pet face — auto-centered in the page container */
    pet_face_create(parent);

    /* ---- Status bar cards (4 across) ---- */
    int cx[4];
    cx[0] = CARD_X0;
    cx[1] = CARD_X0 + CARD_W + CARD_GAP;                /* = 10+108+9 = 127 */
    cx[2] = CARD_X0 + 2 * (CARD_W + CARD_GAP);          /* = 10+234 = 244 */
    cx[3] = CARD_X0 + 3 * (CARD_W + CARD_GAP);          /* = 10+351 = 361 */

    make_bar_item(parent, cx[0], MDI_FOOD,          lv_color_hex(C_ACCENT2), &s_hunger_bar,  &s_hunger_pct);
    make_bar_item(parent, cx[1], MDI_LIGHTNING_BOLT, lv_color_hex(C_ACCENT),  &s_energy_bar,  &s_energy_pct);
    make_bar_item(parent, cx[2], MDI_EMOTICON,       lv_color_hex(C_YELLOW),  &s_mood_bar,    &s_mood_pct);
    make_bar_item(parent, cx[3], MDI_SHOWER,         lv_color_hex(C_GREEN),   &s_hygiene_bar, &s_hygiene_pct);

    /* ---- Action buttons (6 across) ---- */
    int bx[6];
    for (int i = 0; i < 6; i++)
        bx[i] = BTN_X0 + i * (BTN_W + BTN_GAP);
    /* bx = {10, 88, 166, 244, 322, 400} */

    make_btn(parent, bx[0], BTN_Y, MDI_FOOD_APPLE, "Feed",  btn_feed_cb,  0);
    make_btn(parent, bx[1], BTN_Y, MDI_HAND_HEART, "Pet",   btn_pet_cb,   0);
    make_btn(parent, bx[2], BTN_Y, MDI_SLEEP,      "Sleep", btn_sleep_cb, 0);
    make_btn(parent, bx[3], BTN_Y, MDI_SPRAY,      "Clean", btn_clean_cb, 0);
    make_btn(parent, bx[4], BTN_Y, MDI_DUMBBELL,   "Train", btn_train_cb, 0);
    s_mode_btn = make_btn(parent, bx[5], BTN_Y, MDI_SWAP, "Mode", btn_mode_cb, 1);

    /* Subscribe to events */
    hz_event_subscribe(EV_FSM_STATE_CHANGED, on_fsm_change, NULL);
    hz_event_subscribe(EV_MODE_CHANGED,      on_mode_changed, NULL);

    /* Initial state */
    pet_face_set_state(hz_fsm_current());
    update_bars();
    on_mode_changed(NULL, NULL);
}

/*===========================================================================
 * Tick (delegates to face animation + blink)
 *===========================================================================*/
void pet_home_tick(hz_u32 ms) { pet_face_tick(ms); }
