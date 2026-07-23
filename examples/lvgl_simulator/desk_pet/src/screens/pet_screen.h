#ifndef PET_SCREEN_H
#define PET_SCREEN_H
#include "lvgl/lvgl.h"

void pet_screen_init(void);
void pet_screen_switch(int idx);
void pet_screen_update_mode(void);

#endif
