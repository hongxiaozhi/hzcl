/**
 * @file pet_face.c
 * @brief Cartoon pet face built from LVGL basic shapes
 *
 * A cute round creature with animated expressions for each state.
 * All parts are LVGL objects (circles, arcs) so no external assets needed.
 */

#include "pet_face.h"
#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "app.h"

#include <stdio.h>

/*===========================================================================
 * Face geometry (all positions relative to container center)
 *===========================================================================*/
#define HEAD_R       26          /* head radius */
#define EYE_R        6           /* eye radius */
#define PUPIL_R      3           /* pupil radius */
#define MOUTH_R      10          /* mouth arc radius */
#define BLUSH_R      5           /* blush radius */
#define EAR_W        8           /* ear width  */
#define EAR_H        12          /* ear height */

/*===========================================================================
 * Widget references
 *===========================================================================*/
static lv_obj_t *s_cont;         /* face container */
static lv_obj_t *s_head;         /* main head circle */
static lv_obj_t *s_ear_l, *s_ear_r;  /* ear circles */
static lv_obj_t *s_eye_l, *s_eye_r;  /* white of eyes */
static lv_obj_t *s_pupil_l, *s_pupil_r;  /* dark pupils */
static lv_obj_t *s_mouth;        /* mouth arc */
static lv_obj_t *s_blush_l, *s_blush_r;  /* cheek blush */
static lv_obj_t *s_tear_l, *s_tear_r;    /* tears (sad/sick) */
static lv_obj_t *s_zzz;          /* Zzz text (sleeping) */
static lv_obj_t *s_extra;        /* extra indicator */

/*===========================================================================
 * Helper: create a filled circle
 *===========================================================================*/
static lv_obj_t *make_circle(lv_obj_t *parent, int32_t size, lv_color_t color)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_size(obj, size, size);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

/* Forward declaration (function is non-static, declared in pet_face.h) */
void pet_face_set_state(hz_state_t state, int32_t hunger, int32_t energy, int32_t mood);

/*===========================================================================
 * Create all face parts
 *===========================================================================*/
void pet_face_create(lv_obj_t *parent)
{
    /* Container — holds all face parts, can be moved/animated as a unit */
    s_cont = lv_obj_create(parent);
    lv_obj_set_size(s_cont, 120, 130);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_remove_flag(s_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(s_cont);

    /* Ears (behind head) */
    s_ear_l = make_circle(s_cont, EAR_W, lv_color_hex(0x555555));
    lv_obj_align(s_ear_l, LV_ALIGN_CENTER, -20, -HEAD_R - 2);
    s_ear_r = make_circle(s_cont, EAR_W, lv_color_hex(0x555555));
    lv_obj_align(s_ear_r, LV_ALIGN_CENTER, 20, -HEAD_R - 2);

    /* Head */
    s_head = make_circle(s_cont, HEAD_R * 2, lv_color_hex(0xCCCCCC));
    lv_obj_align(s_head, LV_ALIGN_CENTER, 0, 0);

    /* Eyes */
    s_eye_l = make_circle(s_cont, EYE_R * 2, lv_color_hex(0xFFFFFF));
    lv_obj_align(s_eye_l, LV_ALIGN_CENTER, -8, -6);
    s_eye_r = make_circle(s_cont, EYE_R * 2, lv_color_hex(0xFFFFFF));
    lv_obj_align(s_eye_r, LV_ALIGN_CENTER, 8, -6);

    /* Pupils */
    s_pupil_l = make_circle(s_cont, PUPIL_R * 2, lv_color_hex(0x222222));
    lv_obj_align(s_pupil_l, LV_ALIGN_CENTER, -8, -6);
    s_pupil_r = make_circle(s_cont, PUPIL_R * 2, lv_color_hex(0x222222));
    lv_obj_align(s_pupil_r, LV_ALIGN_CENTER, 8, -6);

    /* Mouth (arc) — hidden initially, shown on update */
    s_mouth = lv_arc_create(s_cont);
    lv_obj_set_size(s_mouth, MOUTH_R * 2 + 4, MOUTH_R * 2 + 4);
    lv_arc_set_range(s_mouth, 0, 360);
    lv_arc_set_bg_angles(s_mouth, 0, 0);
    lv_arc_set_angles(s_mouth, 200, 340);  /* default: small smile */
    lv_obj_set_style_arc_width(s_mouth, 3, 0);
    lv_obj_set_style_arc_color(s_mouth, lv_color_hex(0x444444), 0);
    lv_obj_set_style_arc_opa(s_mouth, LV_OPA_COVER, 0);
    lv_obj_remove_flag(s_mouth, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(s_mouth, LV_ALIGN_CENTER, 0, 6);

    /* Blush */
    s_blush_l = make_circle(s_cont, BLUSH_R * 2, lv_color_hex(0xFFAAAA));
    lv_obj_align(s_blush_l, LV_ALIGN_CENTER, -16, 2);
    s_blush_r = make_circle(s_cont, BLUSH_R * 2, lv_color_hex(0xFFAAAA));
    lv_obj_align(s_blush_r, LV_ALIGN_CENTER, 16, 2);

    /* Tears */
    s_tear_l = make_circle(s_cont, 4, lv_color_hex(0x66AAFF));
    lv_obj_align(s_tear_l, LV_ALIGN_CENTER, -12, 2);
    s_tear_r = make_circle(s_cont, 4, lv_color_hex(0x66AAFF));
    lv_obj_align(s_tear_r, LV_ALIGN_CENTER, 12, 2);

    /* Zzz label (sleeping) */
    s_zzz = lv_label_create(s_cont);
    lv_label_set_text(s_zzz, "Zzz");
    lv_obj_set_style_text_color(s_zzz, lv_color_hex(0x8888FF), 0);
    lv_obj_align(s_zzz, LV_ALIGN_CENTER, 30, -HEAD_R - 8);

    /* Extra indicator label (for special states) */
    s_extra = lv_label_create(s_cont);
    lv_obj_set_style_text_font(s_extra, &lv_font_montserrat_14, 0);
    lv_obj_align(s_extra, LV_ALIGN_CENTER, 0, HEAD_R + 8);

    /* Default state */
    pet_face_set_state(PET_STATE_IDLE, 30, 80, 60);
}

/*===========================================================================
 * Update face for given state
 *===========================================================================*/
void pet_face_set_state(hz_state_t state, int32_t hunger, int32_t energy, int32_t mood)
{
    (void)hunger; (void)energy;

    /* --- Reset everything --- */
    lv_obj_clear_flag(s_tear_l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_tear_r, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_zzz, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_blush_l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_blush_r, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_extra, LV_OBJ_FLAG_HIDDEN);

    /* Default: open eyes with pupils, white head */
    lv_obj_set_style_bg_color(s_head, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_color(s_ear_l, lv_color_hex(0x555555), 0);
    lv_obj_set_style_bg_color(s_ear_r, lv_color_hex(0x555555), 0);
    lv_obj_set_size(s_eye_l, EYE_R * 2, EYE_R * 2);
    lv_obj_set_size(s_eye_r, EYE_R * 2, EYE_R * 2);
    lv_obj_set_size(s_pupil_l, PUPIL_R * 2, PUPIL_R * 2);
    lv_obj_set_size(s_pupil_r, PUPIL_R * 2, PUPIL_R * 2);
    lv_obj_clear_flag(s_pupil_l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_pupil_r, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_style_text_color(s_extra, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(s_extra, &lv_font_montserrat_14, 0);

    switch (state) {

    case PET_STATE_IDLE:
        /* Gentle eyes, small smile */
        lv_arc_set_angles(s_mouth, 200, 340);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, true);
        break;

    case PET_STATE_HAPPY:
        /* Big smile, blush, squinty happy eyes */
        lv_arc_set_angles(s_mouth, 190, 350);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_extra, true);
        /* Squint: smaller eyes */
        lv_obj_set_size(s_eye_l, EYE_R * 2, EYE_R * 1);
        lv_obj_set_size(s_eye_r, EYE_R * 2, EYE_R * 1);
        lv_obj_set_size(s_pupil_l, PUPIL_R * 2, 2);
        lv_obj_set_size(s_pupil_r, PUPIL_R * 2, 2);
        break;

    case PET_STATE_EATING:
        /* Open mouth (full circle), wider eyes */
        lv_arc_set_angles(s_mouth, 0, 360);
        lv_obj_set_style_arc_color(s_mouth, lv_color_hex(0xCC4444), 0);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, true);
        break;

    case PET_STATE_SLEEPING:
        /* Closed eyes (horizontal lines), Zzz, flat mouth */
        lv_arc_set_angles(s_mouth, 90, 270);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, true);
        /* Closed eyes: thin horizontal rectangles */
        lv_obj_set_size(s_eye_l, EYE_R * 2, 3);
        lv_obj_set_size(s_eye_r, EYE_R * 2, 3);
        lv_obj_set_hidden(s_pupil_l, true);
        lv_obj_set_hidden(s_pupil_r, true);
        lv_obj_set_style_bg_color(s_eye_l, lv_color_hex(0x666666), 0);
        lv_obj_set_style_bg_color(s_eye_r, lv_color_hex(0x666666), 0);
        break;

    case PET_STATE_SAD:
        /* Downward mouth, tears visible, no blush */
        lv_arc_set_angles(s_mouth, 20, 160);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, true);
        /* Half-closed eyes */
        lv_obj_set_size(s_pupil_l, PUPIL_R * 1, PUPIL_R * 1);
        lv_obj_set_size(s_pupil_r, PUPIL_R * 1, PUPIL_R * 1);
        break;

    case PET_STATE_SICK:
        /* Green tint, wobbly mouth, tears */
        lv_obj_set_style_bg_color(s_head, lv_color_hex(0xBBCC99), 0);
        lv_obj_set_style_bg_color(s_ear_l, lv_color_hex(0x889966), 0);
        lv_obj_set_style_bg_color(s_ear_r, lv_color_hex(0x889966), 0);
        lv_arc_set_angles(s_mouth, 45, 135);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, false);
        lv_label_set_text(s_extra, ">_<");
        lv_obj_set_style_text_color(s_extra, lv_color_hex(0x88AA66), 0);
        break;

    case PET_STATE_HYPER:
        /* Star eyes, huge smile, blush */
        lv_arc_set_angles(s_mouth, 180, 360);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_extra, false);
        lv_label_set_text(s_extra, "\xE2\xAD\x90");  /* ⭐ */
        lv_obj_set_style_text_color(s_extra, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_text_font(s_extra, &lv_font_montserrat_16, 0);
        /* Star eyes: gold sparkle */
        lv_obj_set_style_bg_color(s_eye_l, lv_color_hex(0xFFD700), 0);
        lv_obj_set_style_bg_color(s_eye_r, lv_color_hex(0xFFD700), 0);
        break;

    case PET_STATE_TRAINING:
        /* Determined expression, sweat drop */
        lv_arc_set_angles(s_mouth, 200, 340);
        lv_obj_set_style_arc_width(s_mouth, 4, 0);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_blush_l, true);
        lv_obj_set_hidden(s_blush_r, true);
        lv_obj_set_hidden(s_extra, false);
        lv_label_set_text(s_extra, "\xF0\x9F\x92\xA6");  /* 💦 sweat */
        lv_obj_set_style_text_font(s_extra, &lv_font_montserrat_16, 0);
        /* Determined: angled eyebrows (simplified as smaller pupils offset) */
        lv_obj_set_size(s_pupil_l, PUPIL_R * 2, PUPIL_R * 1);
        lv_obj_set_size(s_pupil_r, PUPIL_R * 2, PUPIL_R * 1);
        lv_obj_align(s_pupil_l, LV_ALIGN_CENTER, -9, -4);
        lv_obj_align(s_pupil_r, LV_ALIGN_CENTER, 9, -4);
        break;

    case PET_STATE_CLEANING:
        /* Sparkle clean, big smile */
        lv_obj_set_style_bg_color(s_head, lv_color_hex(0xEEEEFF), 0);
        lv_arc_set_angles(s_mouth, 200, 340);
        lv_obj_set_hidden(s_tear_l, true);
        lv_obj_set_hidden(s_tear_r, true);
        lv_obj_set_hidden(s_zzz, true);
        lv_obj_set_hidden(s_extra, false);
        lv_label_set_text(s_extra, "\xC2\xA8\xE2\x97\x8B\xC2\xA8");  /* ¨○¨ clean */
        lv_obj_set_style_text_color(s_extra, lv_color_hex(0x88BBFF), 0);
        break;

    default:
        break;
    }

    /* Mood-affected blush intensity */
    if (mood > 60) {
        lv_obj_set_style_bg_color(s_blush_l, lv_color_hex(0xFFAAAA), 0);
        lv_obj_set_style_bg_color(s_blush_r, lv_color_hex(0xFFAAAA), 0);
    } else if (mood > 30) {
        lv_obj_set_style_bg_color(s_blush_l, lv_color_hex(0xFFCCCC), 0);
        lv_obj_set_style_bg_color(s_blush_r, lv_color_hex(0xFFCCCC), 0);
    }

    /* Restore mouth style if it was changed for EATING */
    if (state != PET_STATE_EATING)
        lv_obj_set_style_arc_color(s_mouth, lv_color_hex(0x444444), 0);
    lv_obj_set_style_arc_width(s_mouth, 3, 0);
    /* Reset pupil position if changed for TRAINING */
    if (state != PET_STATE_TRAINING) {
        lv_obj_align(s_pupil_l, LV_ALIGN_CENTER, -8, -6);
        lv_obj_align(s_pupil_r, LV_ALIGN_CENTER, 8, -6);
    }
}

lv_obj_t *pet_face_get_cont(void)
{
    return s_cont;
}
