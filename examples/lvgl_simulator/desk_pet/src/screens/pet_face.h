/**
 * @file pet_face.h
 * @brief Pet face emotion system — drawn with LVGL basic shapes
 *
 * Each state maps to a distinct facial expression with per-state
 * lv_style_t for border/glow (like CSS classes).
 * All face objects are children of a container you position/scale.
 */

#ifndef PET_FACE_H
#define PET_FACE_H

#include "lvgl/lvgl.h"
#include "app.h"
#include "hz_fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create the face objects (call once at setup) */
void pet_face_create(lv_obj_t *parent);

/** Update face expression for the given pet state */
void pet_face_set_state(hz_state_t state);

/** Periodic tick for blink timing (call from screen on_tick) */
void pet_face_tick(hz_u32 ms);

/** Get the face container (for positioning / animation) */
lv_obj_t *pet_face_get_cont(void);

#ifdef __cplusplus
}
#endif

#endif /* PET_FACE_H */
