#include "pet_dash.h"
#include "pet_screen.h"
#include "lvgl/lvgl.h"
#include "mdi_icons.h"
#include <stdlib.h>

#define C_CARD 0x1c273b
#define C_LBL  0x9ca3af
#define C_ACC  0x22d3ee
#define C_ACC2 0xfb923c
#define C_PUR  0xa78bfa
#define C_RED  0xef4444

static lv_obj_t *s_c;

void pet_dash_create(lv_obj_t *parent)
{
    int i; lv_obj_t *o,*v,*l;
    /* MDI icons matching HTML design */
    static const char *di[]={MDI_THERMOMETER,MDI_WATER_PERCENT,MDI_GAUGE,MDI_AIR_FILTER};
    static const char *va[]={"26.5C","65.2%","1013","12"};
    static const char *ln[]={"Temperature","Humidity","Pressure hPa","PM2.5"};

    s_c=lv_obj_create(parent);
    lv_obj_set_size(s_c,480,284);
    lv_obj_set_style_bg_opa(s_c,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_c,0,0);
    lv_obj_set_style_pad_all(s_c,10,14);

    for(i=0;i<4;i++){
        lv_color_t ci=i==0?lv_color_hex(C_ACC2):i==1?lv_color_hex(C_ACC):
                       i==2?lv_color_hex(C_PUR):lv_color_hex(C_RED);
        o=lv_obj_create(s_c);
        lv_obj_set_size(o,(480-32)/2,56);
        lv_obj_set_style_bg_color(o,lv_color_hex(C_CARD),0);
        lv_obj_set_style_radius(o,10,0);
        lv_obj_set_style_border_width(o,0,0);
        lv_obj_align(o,LV_ALIGN_TOP_MID,i%2==0?-112:112,4+i/2*60);

        l=lv_label_create(o);lv_label_set_text(l,di[i]);
        lv_obj_set_style_text_font(l,&mdi_icons_20,0);
        lv_obj_set_style_text_color(l,ci,0);
        lv_obj_align(l,LV_ALIGN_TOP_MID,0,6);

        v=lv_label_create(o);lv_label_set_text(v,va[i]);
        lv_obj_set_style_text_color(v,ci,0);
        lv_obj_set_style_text_font(v,&lv_font_montserrat_16,0);
        lv_obj_align(v,LV_ALIGN_TOP_MID,0,26);

        l=lv_label_create(o);lv_label_set_text(l,ln[i]);
        lv_obj_set_style_text_color(l,lv_color_hex(C_LBL),0);
        lv_obj_align(l,LV_ALIGN_TOP_MID,0,44);
    }

    /* Activity chart */
    o=lv_obj_create(s_c);
    lv_obj_set_size(o,452,48);
    lv_obj_set_style_bg_color(o,lv_color_hex(C_CARD),0);
    lv_obj_set_style_radius(o,10,0);
    lv_obj_set_style_border_width(o,0,0);
    lv_obj_align(o,LV_ALIGN_TOP_MID,0,4+4*60+4);

    l=lv_label_create(o);
    lv_label_set_text(l,"Activity - last 24h");
    lv_obj_set_style_text_color(l,lv_color_hex(C_LBL),0);
    lv_obj_align(l,LV_ALIGN_TOP_LEFT,10,6);

    /* Simple chart */
    for(i=0;i<10;i++){
        lv_obj_t *bar=lv_obj_create(o);
        int h=6+rand()%28;
        lv_obj_set_size(bar,16,h);
        lv_obj_set_style_bg_color(bar,i%2==0?lv_color_hex(C_ACC):lv_color_hex(C_PUR),0);
        lv_obj_set_style_radius(bar,4,0);
        lv_obj_set_style_border_width(bar,0,0);
        lv_obj_align(bar,LV_ALIGN_BOTTOM_MID,-200+i*42,-4);
        lv_obj_remove_flag(bar,LV_OBJ_FLAG_CLICKABLE);
    }
}

static hz_screen_t s_scr;
static void scr_create(void){
    lv_obj_t *s=lv_scr_act();lv_obj_set_style_bg_color(s,lv_color_hex(0x0b1120),0);
    pet_status_bar(s,"Dashboard",2);pet_dash_create(s);
}
hz_screen_t *pet_dash_screen_get(void){return &s_scr;}
void pet_dash_screen_create(void){s_scr=(hz_screen_t){.name="dash",.on_create=scr_create};scr_create();}
