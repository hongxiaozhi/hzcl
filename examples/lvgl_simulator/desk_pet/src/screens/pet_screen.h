#ifndef PET_SCREEN_H
#define PET_SCREEN_H
#include "hz_screen.h"
#include "lvgl/lvgl.h"

lv_obj_t *pet_status_bar(lv_obj_t *parent, const char *title, int active_idx);
void pet_swipe_init(lv_obj_t *obj);
void pet_register_nav(int idx, hz_screen_t *scr);

#endif
