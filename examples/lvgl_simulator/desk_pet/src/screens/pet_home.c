/**
 * @file pet_home.c
 * @brief Home — matches design/desk_pet_prototype.html pixel-for-pixel
 *
 * Status cards: flex-row, icon+bar+pct, 110×24, r=6, bg=#1c273b
 * Action buttons: flex-row, icon+text horizontal, 72×26, r=8, border 1px
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
 * Colors — from design HTML tokens
 *===========================================================================*/
#define C_CARD_BG    0x1c273b  /* --card-bg */
#define C_BTN_BG     0x1e293b  /* --btn-bg */
#define C_BTN_BD     0x334155  /* --btn-bd */
#define C_ACCENT     0x22d3ee  /* --accent */
#define C_ACCENT2    0xfb923c  /* --accent2 */
#define C_GREEN      0x22c55e  /* --green */
#define C_YELLOW     0xfacc15  /* --yellow */
#define C_PURPLE     0xa78bfa  /* --purple */
#define C_LABEL      0x9ca3af  /* --label */
#define C_WHITE      0xffffff  /* --title */
/* Mode btn bg: rgba(34,211,238,.08) on #1e293b ≈ #1e3544 */
#define C_MODE_BG    0x1e3544

/*===========================================================================
 * Layout — computed from design: page 284px, pad 2/14/0/14, gap 4px
 *===========================================================================*/
/* Status cards: 4×(110×24), gap=4, x0=14 */
#define CARD_W   110       /* (480-14*2-4*3)/4 = 440/4 */
#define CARD_H   24
#define CARD_Y   226       /* page(284)-24-4(gap)-26(btn)-4(margin)=226 */
#define CARD_GAP 4
#define CARD_X0  14
#define CARD_R   6

/* Track inside card: h=3, r=2, bg=#334155 */
#define TRACK_W  52        /* card(110)-pad(6)-icon(~20)-pct(~26)-gaps(6) */
#define TRACK_H  3
#define TRACK_X  26        /* pad(3)+icon_visible(~20)+gap(3) */
#define TRACK_Y  10        /* (CARD_H - TRACK_H)/2 = 10.5→10 */

/* Action buttons: 6×(72×26), gap=4, x0=14 */
#define BTN_W    72        /* (480-14*2-4*5)/6 = 432/6 */
#define BTN_H    26
#define BTN_Y    254       /* page(284)-26-4=254 */
#define BTN_GAP  4
#define BTN_X0   14
#define BTN_R    8

/*===========================================================================
 * Globals
 *===========================================================================*/
static lv_obj_t *s_hunger_bar, *s_energy_bar, *s_mood_bar, *s_hygiene_bar;
static lv_obj_t *s_hunger_pct, *s_energy_pct, *s_mood_pct, *s_hygiene_pct;
static lv_obj_t *s_mode_btn;

/*===========================================================================
 * Status card — flex-row: icon | progress-bar | pct
 *===========================================================================*/
static void make_bar_item(lv_obj_t *parent, int x, const char *icon, lv_color_t color,
                           lv_obj_t **bar_out, lv_obj_t **pct_out)
{
    /* Card: 110×24, bg=#1c273b, r=6, pad=3 */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_set_pos(card, x, CARD_Y);
    lv_obj_set_style_bg_color(card, lv_color_hex(C_CARD_BG), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, CARD_R, 0);
    lv_obj_set_style_pad_all(card, 3, 3);
    lv_obj_set_style_outline_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, 0, 0);
    lv_obj_remove_flag(card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    /* Icon — mdi_icons_20, color-coded, left-aligned */
    lv_obj_t *ic = lv_label_create(card);
    lv_label_set_text(ic, icon);
    lv_obj_set_style_text_color(ic, color, 0);
    lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 2, 0);

    /* Track — exact pos, h=3, r=2 */
    lv_obj_t *track = lv_obj_create(card);
    lv_obj_set_size(track, TRACK_W, TRACK_H);
    lv_obj_set_pos(track, TRACK_X, TRACK_Y);
    lv_obj_set_style_bg_color(track, lv_color_hex(C_BTN_BD), 0);
    lv_obj_set_style_border_width(track, 0, 0);
    lv_obj_set_style_radius(track, 2, 0);
    lv_obj_set_style_outline_width(track, 0, 0);
    lv_obj_set_style_shadow_width(track, 0, 0);
    lv_obj_remove_flag(track, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(track, LV_SCROLLBAR_MODE_OFF);

    /* Fill — same origin as track, left→right */
    lv_obj_t *fill = lv_obj_create(card);
    lv_obj_set_size(fill, 0, TRACK_H);
    lv_obj_set_pos(fill, TRACK_X, TRACK_Y);
    lv_obj_set_style_bg_color(fill, color, 0);
    lv_obj_set_style_border_width(fill, 0, 0);
    lv_obj_set_style_radius(fill, 2, 0);
    lv_obj_set_style_outline_width(fill, 0, 0);
    lv_obj_set_style_shadow_width(fill, 0, 0);
    lv_obj_remove_flag(fill, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(fill, LV_SCROLLBAR_MODE_OFF);

    /* Percentage — right-aligned, 12px, #9ca3af */
    lv_obj_t *pct = lv_label_create(card);
    lv_label_set_text(pct, "0%");
    lv_obj_set_style_text_color(pct, lv_color_hex(C_LABEL), 0);
    lv_obj_set_style_text_font(pct, &lv_font_montserrat_12, 0);
    lv_obj_align(pct, LV_ALIGN_RIGHT_MID, -3, 0);

    *bar_out = fill;
    *pct_out = pct;
}

/*===========================================================================
 * Action button — flex-row: icon+text horizontal, 72×26, r=8, border 1px
 *===========================================================================*/
static lv_obj_t *make_btn(lv_obj_t *parent, int x, int y, const char *icon,
                           const char *label, lv_event_cb_t cb, int is_mode)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, BTN_W, BTN_H);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_style_bg_color(btn,
        lv_color_hex(is_mode ? C_MODE_BG : C_BTN_BG), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn,
        lv_color_hex(is_mode ? C_ACCENT : C_BTN_BD), 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, BTN_R, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_style_pad_ver(btn, 4, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    /* Use flex row for icon+text horizontal layout */
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn, 3, 0);  /* gap:3px */

    /* Icon — mdi_icons_20 */
    lv_obj_t *ic = lv_label_create(btn);
    lv_label_set_text(ic, icon);
    lv_obj_set_style_text_font(ic, &mdi_icons_20, 0);
    lv_obj_set_style_text_color(ic,
        lv_color_hex(is_mode ? C_ACCENT : C_WHITE), 0);

    /* Label — montserrat_12 */
    lv_obj_t *lb = lv_label_create(btn);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_font(lb, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lb,
        lv_color_hex(is_mode ? C_ACCENT : C_WHITE), 0);

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
 * Update bars — fill width scaled 0..TRACK_W
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
 * Mode changed — update mode button border color
 *===========================================================================*/
static void on_mode_changed(void *d, void *u)
{
    (void)d; (void)u;
    int cur = hz_mode_current();
    if (cur == MODE_NORMAL) {
        lv_obj_set_style_border_color(s_mode_btn, lv_color_hex(C_ACCENT), 0);
    } else {
        static const uint32_t mc[] = {0, C_ACCENT, C_ACCENT2, C_PURPLE};
        if (cur >= 0 && cur < 4)
            lv_obj_set_style_border_color(s_mode_btn, lv_color_hex(mc[cur]), 0);
    }
}

/*===========================================================================
 * State changed — refresh bars
 *===========================================================================*/
static void on_fsm_change(void *d, void *u) { (void)d; (void)u; update_bars(); }

/*===========================================================================
 * Create home screen
 *===========================================================================*/
void pet_home_create(lv_obj_t *parent)
{
    lv_obj_remove_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    pet_face_create(parent);

    /* Status cards: 4 across, x = 14, 128, 242, 356 */
    int cx[4];
    cx[0] = CARD_X0;
    cx[1] = CARD_X0 + CARD_W + CARD_GAP;                /* 14+110+4=128 */
    cx[2] = CARD_X0 + 2 * (CARD_W + CARD_GAP);          /* 14+228=242 */
    cx[3] = CARD_X0 + 3 * (CARD_W + CARD_GAP);          /* 14+342=356 */

    make_bar_item(parent, cx[0], MDI_FOOD,          lv_color_hex(C_ACCENT2), &s_hunger_bar,  &s_hunger_pct);
    make_bar_item(parent, cx[1], MDI_LIGHTNING_BOLT, lv_color_hex(C_ACCENT),  &s_energy_bar,  &s_energy_pct);
    make_bar_item(parent, cx[2], MDI_EMOTICON,       lv_color_hex(C_YELLOW),  &s_mood_bar,    &s_mood_pct);
    make_bar_item(parent, cx[3], MDI_SHOWER,         lv_color_hex(C_GREEN),   &s_hygiene_bar, &s_hygiene_pct);

    /* Action buttons: 6 across, x = 14, 90, 166, 242, 318, 394 */
    int bx[6];
    for (int i = 0; i < 6; i++)
        bx[i] = BTN_X0 + i * (BTN_W + BTN_GAP);

    make_btn(parent, bx[0], BTN_Y, MDI_FOOD_APPLE, "Feed",  btn_feed_cb,  0);
    make_btn(parent, bx[1], BTN_Y, MDI_HAND_HEART, "Pet",   btn_pet_cb,   0);
    make_btn(parent, bx[2], BTN_Y, MDI_SLEEP,      "Sleep", btn_sleep_cb, 0);
    make_btn(parent, bx[3], BTN_Y, MDI_SPRAY,      "Clean", btn_clean_cb, 0);
    make_btn(parent, bx[4], BTN_Y, MDI_DUMBBELL,   "Train", btn_train_cb, 0);
    s_mode_btn = make_btn(parent, bx[5], BTN_Y, MDI_SWAP, "Mode", btn_mode_cb, 1);

    hz_event_subscribe(EV_FSM_STATE_CHANGED, on_fsm_change, NULL);
    hz_event_subscribe(EV_MODE_CHANGED,      on_mode_changed, NULL);

    pet_face_set_state(hz_fsm_current());
    update_bars();
    on_mode_changed(NULL, NULL);
}

void pet_home_tick(hz_u32 ms) { pet_face_tick(ms); }
