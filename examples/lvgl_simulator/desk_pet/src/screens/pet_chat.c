#include "pet_chat.h"
#include "pet_screen.h"
#include "mdi_icons.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define C_CARD 0x1c273b
#define C_BTN 0x334155
#define C_ACC 0x22d3ee

static lv_obj_t *s_c, *s_area, *s_inp;

static void add_msg(const char *txt, int is_user)
{
    lv_obj_t *m=lv_label_create(s_area);
    lv_label_set_text(m,txt);
    lv_obj_set_style_bg_color(m,is_user?lv_color_hex(C_ACC):lv_color_hex(C_CARD),0);
    lv_obj_set_style_text_color(m,is_user?lv_color_hex(0x000000):lv_color_hex(0xffffff),0);
    lv_obj_set_style_radius(m,8,0);
    lv_obj_set_style_pad_all(m,4,10);
    lv_obj_set_style_text_font(m,&lv_font_montserrat_12,0);
    lv_obj_set_width(m,LV_SIZE_CONTENT);
    lv_obj_align(m,is_user?LV_ALIGN_TOP_RIGHT:LV_ALIGN_TOP_LEFT,is_user?-10:10,0);
}

static void send_cb(lv_event_t *e)
{
    (void)e;
    const char *t=lv_textarea_get_text(s_inp);
    if(!t||!t[0])return;
    add_msg(t,1);
    lv_textarea_set_text(s_inp,"");
}

void pet_chat_create(lv_obj_t *parent)
{
    lv_obj_t *o,*btn,*lb;

    s_c=lv_obj_create(parent);
    lv_obj_set_size(s_c,480,284);
    lv_obj_set_style_bg_opa(s_c,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_c,0,0);
    lv_obj_set_style_pad_all(s_c,10,14);

    s_area=lv_obj_create(s_c);
    lv_obj_set_size(s_area,452,215);
    lv_obj_set_style_bg_opa(s_area,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_area,0,0);
    lv_obj_set_style_pad_all(s_area,5,10);
    lv_obj_align(s_area,LV_ALIGN_TOP_MID,0,0);
    lv_obj_set_flex_flow(s_area,LV_FLEX_FLOW_COLUMN_REVERSE);
    lv_obj_set_scrollbar_mode(s_area,LV_SCROLLBAR_MODE_AUTO);

    add_msg("Hi! I'm your desk pet. Ask me anything!",0);

    o=lv_obj_create(s_c);
    lv_obj_set_size(o,452,36);
    lv_obj_set_style_bg_opa(o,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(o,0,0);
    lv_obj_align(o,LV_ALIGN_BOTTOM_MID,0,4);

    s_inp=lv_textarea_create(o);
    lv_obj_set_size(s_inp,360,30);
    lv_obj_set_style_bg_color(s_inp,lv_color_hex(C_CARD),0);
    lv_obj_set_style_border_color(s_inp,lv_color_hex(C_BTN),0);
    lv_obj_set_style_border_width(s_inp,1,0);
    lv_obj_set_style_radius(s_inp,8,0);
    lv_obj_set_style_pad_all(s_inp,4,10);
    lv_obj_align(s_inp,LV_ALIGN_LEFT_MID,0,0);
    lv_textarea_set_placeholder_text(s_inp,"Say something...");
    lv_textarea_set_one_line(s_inp,true);

    btn=lv_btn_create(o);
    lv_obj_set_size(btn,70,30);
    lv_obj_set_style_bg_color(btn,lv_color_hex(C_ACC),0);
    lv_obj_set_style_radius(btn,8,0);
    lv_obj_align(btn,LV_ALIGN_RIGHT_MID,0,0);
    lv_obj_add_event_cb(btn,send_cb,LV_EVENT_CLICKED,NULL);

    lb=lv_label_create(btn);
    lv_label_set_text(lb,"Send");
    lv_obj_set_style_text_color(lb,lv_color_hex(0x000000),0);
    lv_obj_center(lb);
}

static hz_screen_t s_scr;
static void scr_create(void) {
    lv_obj_t *s=lv_scr_act(); lv_obj_set_style_bg_color(s,lv_color_hex(0x0b1120),0);
    pet_status_bar(s,"AI Chat",1); pet_chat_create(s);
    pet_swipe_init(s); pet_register_nav(1,&s_scr);
}
hz_screen_t *pet_chat_screen_get(void){return &s_scr;}
void pet_chat_screen_create(void){s_scr=(hz_screen_t){.name="chat",.on_create=scr_create};scr_create();}
