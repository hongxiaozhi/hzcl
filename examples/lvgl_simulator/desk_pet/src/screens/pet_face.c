/**
 * @file pet_face.c
 * @brief Pet face emotion system — direct property sets (no lv_style_t)
 *
 * Each state sets border/glow/eyes/mouth directly via lv_obj_set_style_*.
 * Reliable approach: no style cascading, no lv_style_t version issues.
 */

#include "pet_face.h"
#include "lvgl/lvgl.h"

/*===========================================================================
 * Color constants
 *===========================================================================*/
#define C_ACCENT  0x22d3ee
#define C_ACCENT2 0xfb923c
#define C_PURPLE  0xa78bfa
#define C_SICK    0x65a30d
#define C_BD      0x334155
#define C_BG      0x1e293b
#define C_PANEL   0x172033
#define C_DARK    0x0b1120

/*===========================================================================
 * Proportional face geometry — change HEAD_SZ to scale everything
 * All ratios are % of HEAD_SZ for readability:
 *   (HEAD_SZ * 18 / 100) = 18% of head size
 *===========================================================================*/
#define HEAD_SZ   90           /* change this to scale the whole face */
#define EYE_SZ    (HEAD_SZ * 18 / 100)   /* eye diameter: 18% of head */
#define EYE_X     (HEAD_SZ * 14 / 100)   /* eye x offset: 14% of head */
#define EYE_Y     (HEAD_SZ * 11 / 100)   /* eye y offset: 11% of head */
#define PUPIL_SZ  (HEAD_SZ * 7 / 100)    /* pupil diameter: 7% of head */
#define MOUTH_W   (HEAD_SZ * 31 / 100)   /* mouth arc box: 31% of head (matches HTML 28px) */
#define MOUTH_H   MOUTH_W                 /* square box for circular arc curvature */
#define MOUTH_Y   (HEAD_SZ * 19 / 100)   /* mouth y: center + 19% of head */
#define BLUSH_SZ  (HEAD_SZ * 9 / 100)    /* blush diameter: 9% of head */
#define BLUSH_X   (HEAD_SZ * 22 / 100)   /* blush x offset: 22% of head */
#define BLUSH_Y   (HEAD_SZ * 7 / 100)    /* blush y offset: 7% of head */
#define DECOR_Y   (HEAD_SZ * -13 / 100)  /* decoration y: -13% (above) */
#define DECOR_X   (HEAD_SZ * 31 / 100)   /* decoration x: 31% of head */
#define BUBBLE_Y  (HEAD_SZ * -9 / 100)   /* bubble y: -9% (above head) */

/*===========================================================================
 * Widget references
 *===========================================================================*/
static lv_obj_t *s_wrap;
static lv_obj_t *s_head;
static lv_obj_t *s_el, *s_er;
static lv_obj_t *s_pl, *s_pr;
static lv_obj_t *s_m;
static lv_obj_t *s_bl, *s_br;
static lv_obj_t *s_zz;
static lv_obj_t *s_sw;
static lv_obj_t *s_sk;
static lv_obj_t *s_bub;

static lv_anim_t s_anim;
static int s_cur_anim = -1;
static int s_base_y = 0;
static int s_base_x = 0;

/* Blink timing */
static hz_u32 s_blink_acc = 0;

/*===========================================================================
 * Animation types
 *===========================================================================*/
enum { ANIM_NONE = -1, ANIM_FLOAT, ANIM_BOUNCE, ANIM_TREMBLE, ANIM_PULSE };

static const int s_anim_map[] = {
    ANIM_FLOAT, ANIM_FLOAT, ANIM_PULSE, ANIM_FLOAT, ANIM_FLOAT,
    ANIM_TREMBLE, ANIM_BOUNCE, ANIM_BOUNCE, ANIM_FLOAT
};

/*===========================================================================
 * Helper
 *===========================================================================*/
static lv_obj_t *make_circle(lv_obj_t *p, int sz, lv_color_t c)
{
    lv_obj_t *o = lv_obj_create(p);
    lv_obj_set_size(o, sz, sz);
    lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(o, c, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
    return o;
}

/*===========================================================================
 * Apply head visual — directly, no styles
 *===========================================================================*/
static void apply_head(uint32_t border_col, int shadow_w, lv_opa_t shadow_opa)
{
    /* Base properties (every state needs these) */
    lv_obj_set_style_radius(s_head, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_head, lv_color_hex(C_BG), 0);
    lv_obj_set_style_bg_grad_color(s_head, lv_color_hex(0x0b1525), 0);
    lv_obj_set_style_bg_grad_dir(s_head, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(s_head, LV_OPA_COVER, 0);

    /* State-specific border */
    lv_obj_set_style_border_color(s_head, lv_color_hex(border_col), 0);
    lv_obj_set_style_border_width(s_head, 2, 0);

    /* State-specific glow */
    if (shadow_opa > 0) {
        lv_obj_set_style_shadow_color(s_head, lv_color_hex(border_col), 0);
        lv_obj_set_style_shadow_opa(s_head, shadow_opa, 0);
        lv_obj_set_style_shadow_width(s_head, shadow_w, 0);
        lv_obj_set_style_shadow_spread(s_head, 4, 0);
        lv_obj_set_style_shadow_ofs_x(s_head, 0, 0);
        lv_obj_set_style_shadow_ofs_y(s_head, 0, 0);
    } else {
        lv_obj_set_style_shadow_opa(s_head, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_width(s_head, 0, 0);
    }
}

/*===========================================================================
 * Animation
 *===========================================================================*/
static void anim_y(void *o, int32_t v) { lv_obj_set_y((lv_obj_t *)o, v); }
static void anim_x(void *o, int32_t v) { lv_obj_set_x((lv_obj_t *)o, v); }
static void anim_sz(void *o, int32_t v) { lv_obj_set_size((lv_obj_t *)o, v, v); }
static void blink_eye(void *o, int32_t v)
{
    (void)o;
    lv_obj_set_height(s_el, v);
    lv_obj_set_height(s_er, v);
}

static void switch_anim(int type)
{
    lv_anim_del(s_wrap, NULL);
    lv_anim_del(s_head, NULL);
    lv_obj_set_y(s_wrap, s_base_y);
    lv_obj_set_x(s_wrap, s_base_x);
    lv_obj_set_size(s_head, HEAD_SZ, HEAD_SZ);

    s_cur_anim = type;
    if (type == ANIM_NONE) return;

    lv_anim_init(&s_anim);
    lv_anim_set_repeat_count(&s_anim, LV_ANIM_REPEAT_INFINITE);

    switch (type) {
    case ANIM_FLOAT:
        lv_anim_set_var(&s_anim, s_wrap);
        lv_anim_set_exec_cb(&s_anim, anim_y);
        lv_anim_set_values(&s_anim, s_base_y, s_base_y - HEAD_SZ * 10 / 90);
        lv_anim_set_time(&s_anim, 3000);
        lv_anim_set_playback_time(&s_anim, 3000);
        lv_anim_set_path_cb(&s_anim, lv_anim_path_ease_in_out);
        break;
    case ANIM_BOUNCE:
        lv_anim_set_var(&s_anim, s_wrap);
        lv_anim_set_exec_cb(&s_anim, anim_y);
        lv_anim_set_values(&s_anim, s_base_y, s_base_y - HEAD_SZ * 16 / 90);
        lv_anim_set_time(&s_anim, 700);
        lv_anim_set_playback_time(&s_anim, 700);
        lv_anim_set_path_cb(&s_anim, lv_anim_path_bounce);
        break;
    case ANIM_TREMBLE:
        lv_anim_set_var(&s_anim, s_wrap);
        lv_anim_set_exec_cb(&s_anim, anim_x);
        lv_anim_set_values(&s_anim, s_base_x - HEAD_SZ * 3 / 90, s_base_x + HEAD_SZ * 3 / 90);
        lv_anim_set_time(&s_anim, 200);
        lv_anim_set_playback_time(&s_anim, 200);
        lv_anim_set_path_cb(&s_anim, lv_anim_path_linear);
        break;
    case ANIM_PULSE:
        lv_anim_set_var(&s_anim, s_head);
        lv_anim_set_exec_cb(&s_anim, anim_sz);
        lv_anim_set_values(&s_anim, HEAD_SZ, HEAD_SZ * 96 / 90);
        lv_anim_set_time(&s_anim, 300);
        lv_anim_set_playback_time(&s_anim, 300);
        lv_anim_set_path_cb(&s_anim, lv_anim_path_ease_in_out);
        break;
    }
    lv_anim_start(&s_anim);
}

/*===========================================================================
 * Blink
 *===========================================================================*/
static void do_blink(void)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_el);
    lv_anim_set_exec_cb(&a, blink_eye);
    lv_anim_set_values(&a, EYE_SZ, 2);
    lv_anim_set_time(&a, 75);
    lv_anim_set_playback_time(&a, 75);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_start(&a);
}

/*===========================================================================
 * Create face
 *===========================================================================*/
void pet_face_create(lv_obj_t *parent)
{
    /* Wrap */
    s_wrap = lv_obj_create(parent);
    lv_obj_set_size(s_wrap, HEAD_SZ, HEAD_SZ);
    lv_obj_set_style_bg_opa(s_wrap, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_wrap, 0, 0);
    lv_obj_remove_flag(s_wrap, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_wrap, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(s_wrap);
    s_base_y = lv_obj_get_y(s_wrap);
    s_base_x = lv_obj_get_x(s_wrap);

    /* Head */
    s_head = lv_obj_create(s_wrap);
    lv_obj_set_size(s_head, HEAD_SZ, HEAD_SZ);
    lv_obj_center(s_head);
    lv_obj_remove_flag(s_head, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_head, LV_SCROLLBAR_MODE_OFF);

    /* Eyes */
    s_el = make_circle(s_head, EYE_SZ, lv_color_hex(0xFFFFFF));
    lv_obj_align(s_el, LV_ALIGN_CENTER, -EYE_X, -EYE_Y);
    s_er = make_circle(s_head, EYE_SZ, lv_color_hex(0xFFFFFF));
    lv_obj_align(s_er, LV_ALIGN_CENTER, EYE_X, -EYE_Y);

    /* Pupils */
    s_pl = make_circle(s_el, PUPIL_SZ, lv_color_hex(C_DARK));
    lv_obj_align(s_pl, LV_ALIGN_CENTER, 0, 1);
    s_pr = make_circle(s_er, PUPIL_SZ, lv_color_hex(C_DARK));
    lv_obj_align(s_pr, LV_ALIGN_CENTER, 0, 1);

    /* Mouth */
    s_m = lv_arc_create(s_head);
    lv_obj_set_size(s_m, MOUTH_W, MOUTH_H);
    lv_arc_set_range(s_m, 0, 360);
    lv_arc_set_bg_angles(s_m, 0, 0);
    lv_arc_set_angles(s_m, 170, 370);
    lv_obj_set_style_arc_color(s_m, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    /* Ultra-fine mouth line: 1px arc width with rounded caps */
    lv_obj_set_style_arc_width(s_m, HEAD_SZ * 1 / 90, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(s_m, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(s_m, true, LV_PART_INDICATOR);
    lv_obj_set_style_opa(s_m, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_remove_flag(s_m, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(s_m, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(s_m, LV_ALIGN_CENTER, 0, MOUTH_Y);

    /* Blush */
    s_bl = make_circle(s_head, BLUSH_SZ, lv_color_hex(C_ACCENT2));
    lv_obj_set_style_bg_opa(s_bl, LV_OPA_30, 0);
    lv_obj_align(s_bl, LV_ALIGN_CENTER, -BLUSH_X, BLUSH_Y);
    lv_obj_add_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
    s_br = make_circle(s_head, BLUSH_SZ, lv_color_hex(C_ACCENT2));
    lv_obj_set_style_bg_opa(s_br, LV_OPA_30, 0);
    lv_obj_align(s_br, LV_ALIGN_CENTER, BLUSH_X, BLUSH_Y);
    lv_obj_add_flag(s_br, LV_OBJ_FLAG_HIDDEN);

    /* Decorations */
    s_zz = lv_label_create(s_wrap);
    lv_label_set_text(s_zz, "Zzz");
    lv_obj_set_style_text_color(s_zz, lv_color_hex(C_PURPLE), 0);
    lv_obj_align(s_zz, LV_ALIGN_TOP_MID, DECOR_X, DECOR_Y);
    lv_obj_add_flag(s_zz, LV_OBJ_FLAG_HIDDEN);
    s_sw = lv_label_create(s_wrap);
    lv_label_set_text(s_sw, "\xF0\x9F\x92\xA7");
    lv_obj_align(s_sw, LV_ALIGN_TOP_MID, -DECOR_X, DECOR_Y);
    lv_obj_add_flag(s_sw, LV_OBJ_FLAG_HIDDEN);
    s_sk = lv_label_create(s_wrap);
    lv_label_set_text(s_sk, "\xE2\x9C\xA8");
    lv_obj_align(s_sk, LV_ALIGN_TOP_MID, DECOR_X, DECOR_Y);
    lv_obj_add_flag(s_sk, LV_OBJ_FLAG_HIDDEN);

    /* Bubble */
    s_bub = lv_label_create(s_wrap);
    lv_label_set_text(s_bub, "");
    lv_obj_set_style_bg_color(s_bub, lv_color_hex(C_PANEL), 0);
    lv_obj_set_style_border_color(s_bub, lv_color_hex(C_BD), 0);
    lv_obj_set_style_border_width(s_bub, 1, 0);
    lv_obj_set_style_radius(s_bub, 8, 0);
    lv_obj_set_style_pad_all(s_bub, 4, 10);
    lv_obj_set_style_text_color(s_bub, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(s_bub, LV_ALIGN_TOP_MID, 0, BUBBLE_Y);
    lv_obj_add_flag(s_bub, LV_OBJ_FLAG_HIDDEN);

    s_blink_acc = 3500;
}

/*===========================================================================
 * Set state
 *===========================================================================*/
void pet_face_set_state(hz_state_t state)
{
    /* Reset decorations */
    lv_obj_add_flag(s_zz, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_sw, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_sk, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_br, LV_OBJ_FLAG_HIDDEN);

    /* Reset eyes */
    lv_obj_set_size(s_el, EYE_SZ, EYE_SZ);
    lv_obj_set_size(s_er, EYE_SZ, EYE_SZ);
    lv_obj_set_style_radius(s_el, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_radius(s_er, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(s_el, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(s_er, lv_color_hex(0xFFFFFF), 0);
    lv_obj_clear_flag(s_pl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_pr, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(s_pl, PUPIL_SZ, PUPIL_SZ);
    lv_obj_set_size(s_pr, PUPIL_SZ, PUPIL_SZ);

    int anim = ANIM_FLOAT;
    if (state >= 0 && state < 9) anim = s_anim_map[state];

    switch (state) {

    case PET_STATE_IDLE:
        apply_head(C_BD, 0, LV_OPA_TRANSP);
        lv_arc_set_angles(s_m, 170, 370);
        break;

    case PET_STATE_HAPPY:
        apply_head(C_ACCENT, 28, LV_OPA_30);
        lv_arc_set_angles(s_m, 165, 350);
        lv_obj_set_size(s_el, EYE_SZ, EYE_SZ / 2);
        lv_obj_set_size(s_er, EYE_SZ, EYE_SZ / 2);
        lv_obj_set_size(s_pl, PUPIL_SZ, 2);
        lv_obj_set_size(s_pr, PUPIL_SZ, 2);
        lv_obj_clear_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_br, LV_OBJ_FLAG_HIDDEN);
        break;

    case PET_STATE_HYPER:
        apply_head(C_ACCENT2, 38, LV_OPA_40);
        lv_arc_set_angles(s_m, 0, 360);
        lv_obj_set_size(s_el, EYE_SZ, EYE_SZ / 2);
        lv_obj_set_size(s_er, EYE_SZ, EYE_SZ / 2);
        lv_obj_set_size(s_pl, PUPIL_SZ, 2);
        lv_obj_set_size(s_pr, PUPIL_SZ, 2);
        lv_obj_clear_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_br, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_sw, LV_OBJ_FLAG_HIDDEN);
        break;

    case PET_STATE_EATING:
        apply_head(C_ACCENT2, 22, LV_OPA_20);
        lv_arc_set_angles(s_m, 0, 360);
        lv_obj_set_size(s_el, EYE_SZ * 3 / 4, EYE_SZ * 3 / 4);
        lv_obj_set_size(s_er, EYE_SZ * 3 / 4, EYE_SZ * 3 / 4);
        lv_obj_set_size(s_pl, PUPIL_SZ / 2, PUPIL_SZ / 2);
        lv_obj_set_size(s_pr, PUPIL_SZ / 2, PUPIL_SZ / 2);
        break;

    case PET_STATE_SLEEPING:
        apply_head(C_PURPLE, 18, LV_OPA_20);
        lv_arc_set_angles(s_m, 0, 360);
        lv_obj_set_size(s_el, EYE_SZ, 2);
        lv_obj_set_size(s_er, EYE_SZ, 2);
        lv_obj_set_style_bg_color(s_el, lv_color_hex(0x666666), 0);
        lv_obj_set_style_bg_color(s_er, lv_color_hex(0x666666), 0);
        lv_obj_add_flag(s_pl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_pr, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_zz, LV_OBJ_FLAG_HIDDEN);
        break;

    case PET_STATE_SAD:
        apply_head(C_PURPLE, 22, LV_OPA_20);
        lv_arc_set_angles(s_m, 20, 160);
        lv_obj_set_size(s_el, EYE_SZ, EYE_SZ * 5 / 4);
        lv_obj_set_size(s_er, EYE_SZ, EYE_SZ * 5 / 4);
        break;

    case PET_STATE_SICK:
        apply_head(C_SICK, 16, LV_OPA_20);
        lv_arc_set_angles(s_m, 45, 135);
        lv_obj_set_style_bg_opa(s_el, LV_OPA_70, 0);
        lv_obj_set_style_bg_opa(s_er, LV_OPA_70, 0);
        break;

    case PET_STATE_TRAINING:
        apply_head(C_ACCENT, 30, LV_OPA_30);
        lv_arc_set_angles(s_m, 170, 370);
        lv_obj_clear_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_br, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_sw, LV_OBJ_FLAG_HIDDEN);
        break;

    case PET_STATE_CLEANING:
        apply_head(C_ACCENT, 18, LV_OPA_20);
        lv_arc_set_angles(s_m, 170, 370);
        lv_obj_clear_flag(s_bl, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_br, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_sk, LV_OBJ_FLAG_HIDDEN);
        break;

    default:
        apply_head(C_BD, 0, LV_OPA_TRANSP);
        lv_arc_set_angles(s_m, 170, 370);
        break;
    }

    switch_anim(anim);
}

/*===========================================================================
 * Tick
 *===========================================================================*/
void pet_face_tick(hz_u32 ms)
{
    s_blink_acc += ms;
    if (s_blink_acc >= 3500) {
        s_blink_acc = 0;
        if (lv_obj_has_flag(s_zz, LV_OBJ_FLAG_HIDDEN))
            do_blink();
    }
}

lv_obj_t *pet_face_get_cont(void) { return s_wrap; }
