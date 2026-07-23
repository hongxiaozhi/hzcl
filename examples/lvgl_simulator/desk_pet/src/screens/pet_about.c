#include "pet_about.h"
#include "pet_screen.h"
#include "app.h"

#define C_LBL 0x9ca3af

static lv_obj_t *s_c;

void pet_about_create(lv_obj_t *parent)
{
    lv_obj_t *o,*v,*l;

    s_c=lv_obj_create(parent);
    lv_obj_set_size(s_c,480,284);
    lv_obj_set_style_bg_opa(s_c,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_c,0,0);
    lv_obj_set_style_pad_all(s_c,10,14);

    o=lv_obj_create(s_c);
    lv_obj_set_size(o,200,200);
    lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(o,0,0);
    lv_obj_center(o);

    o=lv_label_create(s_c);
    lv_label_set_text(o,"\xF0\x9F\xA4\x96");
    lv_obj_set_style_text_font(o,&lv_font_montserrat_48,0);
    lv_obj_align(o,LV_ALIGN_CENTER,0,-50);

    v=lv_label_create(s_c);
    lv_label_set_text(v,"Pro Desk Pet");
    lv_obj_set_style_text_color(v,lv_color_hex(0xffffff),0);
    lv_obj_set_style_text_font(v,&lv_font_montserrat_16,0);
    lv_obj_align(v,LV_ALIGN_CENTER,0,-8);

    l=lv_label_create(s_c);
    lv_label_set_text(l,"v1.0.0 - HZCL + LVGL + SDL2");
    lv_obj_set_style_text_color(l,lv_color_hex(C_LBL),0);
    lv_obj_set_style_text_font(l,&lv_font_montserrat_12,0);
    lv_obj_align(l,LV_ALIGN_CENTER,0,14);

    l=lv_label_create(s_c);
    lv_label_set_text(l,"\xF0\x9F\x92\xA1 AI-powered desktop companion\n\xF0\x9F\x94\x84 FSM-driven personality engine\n\xE2\x9C\xA8 9 emotional states\n\xF0\x9F\x93\xA1 4 interaction modes\n\xE2\x9D\xA4 Built with HZCL Framework");
    lv_obj_set_style_text_color(l,lv_color_hex(C_LBL),0);
    lv_obj_set_style_text_font(l,&lv_font_montserrat_12,0);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_align(l,LV_ALIGN_CENTER,0,50);
}

static hz_screen_t s_scr;
static void scr_create(void) {
    lv_obj_t *s=lv_scr_act(); lv_obj_set_style_bg_color(s,lv_color_hex(0x0b1120),0);
    pet_status_bar(s,"About",4); pet_about_create(s);
    pet_swipe_init(s); pet_register_nav(4,&s_scr);
}
hz_screen_t *pet_about_screen_get(void){return &s_scr;}
void pet_about_screen_create(void){s_scr=(hz_screen_t){.name="about",.on_create=scr_create};scr_create();}
