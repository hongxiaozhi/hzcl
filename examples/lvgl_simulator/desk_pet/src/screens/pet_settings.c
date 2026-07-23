#include "pet_settings.h"
#include "pet_screen.h"
#include "mdi_icons.h"
#include "hz_config_mgr.h"
#include "app.h"

#define C_CARD 0x1c273b
#define C_ACC 0x22d3ee
#define C_LBL 0x9ca3af

static lv_obj_t *s_c;

static void toggle_cb(lv_event_t *e)
{
    lv_obj_t *t=lv_event_get_target(e);
    lv_color_t c = lv_obj_get_style_bg_color(t,0);
    lv_obj_set_style_bg_color(t,
        c.red==0x22&&c.green==0xd3&&c.blue==0xee
        ? lv_color_hex(0x334155) : lv_color_hex(C_ACC), 0);
}

void pet_settings_create(lv_obj_t *parent)
{
    int i;
    lv_obj_t *o,*ic,*lb,*ds,*tg,*sl;
    static const char *lbls[]={"Voice Interaction","AI Backend","Theme Accent","Notifications","Auto-save"};
    static const char *dscs[]={"Enable speech input/output","DeepSeek / OpenAI / Local","Color scheme","Remind when needed","30 seconds"};
    static const char *si[]={MDI_ACCOUNT_VOICE,MDI_ROBOT,MDI_PALETTE,MDI_BELL,MDI_CLOCK};

    s_c=lv_obj_create(parent);
    lv_obj_set_size(s_c,480,284);
    lv_obj_set_style_bg_opa(s_c,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_c,0,0);
    lv_obj_set_style_pad_all(s_c,10,14);

    for(i=0;i<5;i++){
        o=lv_obj_create(s_c);
        lv_obj_set_size(o,452,46);
        lv_obj_set_style_bg_color(o,lv_color_hex(C_CARD),0);
        lv_obj_set_style_radius(o,10,0);
        lv_obj_set_style_border_width(o,0,0);
        lv_obj_set_style_pad_all(o,8,12);
        lv_obj_align(o,LV_ALIGN_TOP_MID,0,2+i*50);

        ic=lv_label_create(o);
        lv_label_set_text(ic,si[i]);
        lv_obj_set_style_text_font(ic,&mdi_icons_20,0);
        lv_obj_set_style_text_color(ic,lv_color_hex(C_ACC),0);
        lv_obj_align(ic,LV_ALIGN_LEFT_MID,8,0);

        lb=lv_label_create(o);
        lv_label_set_text(lb,lbls[i]);
        lv_obj_set_style_text_color(lb,lv_color_hex(0xffffff),0);
        lv_obj_set_style_text_font(lb,&lv_font_montserrat_12,0);
        lv_obj_align(lb,LV_ALIGN_LEFT_MID,32,0);

        ds=lv_label_create(o);
        lv_label_set_text(ds,dscs[i]);
        lv_obj_set_style_text_color(ds,lv_color_hex(C_LBL),0);
            lv_obj_align(ds,LV_ALIGN_LEFT_MID,32,18);

        if(i==0||i==3){
            tg=lv_obj_create(o);
            lv_obj_set_size(tg,44,24);
            lv_obj_set_style_bg_color(tg,lv_color_hex(C_ACC),0);
            lv_obj_set_style_radius(tg,12,0);
            lv_obj_set_style_border_width(tg,0,0);
            lv_obj_align(tg,LV_ALIGN_RIGHT_MID,-6,0);
            lv_obj_add_flag(tg,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(tg,toggle_cb,LV_EVENT_CLICKED,NULL);

            lv_obj_t *knob=lv_obj_create(tg);
            lv_obj_set_size(knob,20,20);
            lv_obj_set_style_bg_color(knob,lv_color_hex(0xffffff),0);
            lv_obj_set_style_radius(knob,LV_RADIUS_CIRCLE,0);
            lv_obj_set_style_border_width(knob,0,0);
            lv_obj_align(knob,LV_ALIGN_RIGHT_MID,-2,0);
            lv_obj_remove_flag(knob,LV_OBJ_FLAG_CLICKABLE);
        }

        if(i==1){
            lb=lv_label_create(o);
            lv_label_set_text(lb,"DeepSeek");
            lv_obj_set_style_text_color(lb,lv_color_hex(C_ACC),0);
            lv_obj_set_style_text_font(lb,&lv_font_montserrat_12,0);
            lv_obj_align(lb,LV_ALIGN_RIGHT_MID,-10,0);
        }

        if(i==2){
            int c;
                        for(c=0;c<4;c++){
                lv_obj_t *dot=lv_obj_create(o);
                lv_obj_set_size(dot,16,16);
                lv_obj_set_style_radius(dot,LV_RADIUS_CIRCLE,0);
                lv_obj_set_style_border_width(dot,c==0?2:0,0);
                lv_obj_set_style_border_color(dot,lv_color_hex(0xffffff),0);
                lv_obj_align(dot,LV_ALIGN_RIGHT_MID,-10-c*22,0);
                lv_obj_remove_flag(dot,LV_OBJ_FLAG_CLICKABLE);
                switch(c){
                    case 0: lv_obj_set_style_bg_color(dot,lv_color_hex(0x22d3ee),0); break;
                    case 1: lv_obj_set_style_bg_color(dot,lv_color_hex(0xfb923c),0); break;
                    case 2: lv_obj_set_style_bg_color(dot,lv_color_hex(0xa78bfa),0); break;
                    case 3: lv_obj_set_style_bg_color(dot,lv_color_hex(0xf472b6),0); break;
                }
            }
        }

        if(i==4){
            sl=lv_slider_create(o);
            lv_obj_set_size(sl,80,4);
            lv_obj_align(sl,LV_ALIGN_RIGHT_MID,-10,0);
            lv_slider_set_range(sl,5,120);
            lv_slider_set_value(sl,30,LV_ANIM_OFF);
            lv_obj_set_style_bg_color(sl,lv_color_hex(0x334155),LV_PART_MAIN);
            lv_obj_set_style_bg_color(sl,lv_color_hex(C_ACC),LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(sl,lv_color_hex(C_ACC),LV_PART_KNOB);
            lv_obj_set_style_radius(sl,2,LV_PART_MAIN|LV_PART_INDICATOR);
            lv_obj_set_style_radius(sl,LV_RADIUS_CIRCLE,LV_PART_KNOB);
        }
    }
}

static hz_screen_t s_scr;
static void scr_create(void) {
    lv_obj_t *s=lv_scr_act(); lv_obj_set_style_bg_color(s,lv_color_hex(0x0b1120),0);
    pet_status_bar(s,"Settings",3); pet_settings_create(s);
    pet_swipe_init(s); pet_register_nav(3,&s_scr);
}
hz_screen_t *pet_settings_screen_get(void){return &s_scr;}
void pet_settings_screen_create(void){s_scr=(hz_screen_t){.name="settings",.on_create=scr_create};scr_create();}
