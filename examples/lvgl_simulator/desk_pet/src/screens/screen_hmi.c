/**
 * @file screen_hmi.c
 * @brief HMI Smart Controller — dual-page industrial UI
 * Reference: MockingBot prototype "LVGL Embedded Controller"
 */

#include "lvgl/lvgl.h"
#include "hz_screen.h"
#include "mdi_icons.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define W           480
#define H           320
#define BAR_H       36
#define PGH         (H - BAR_H)

static lv_obj_t *s_cont;
static lv_obj_t *s_alarm_mq;
static lv_obj_t *s_pc;
static lv_obj_t *s_dots[2];
static int s_cur = 0;

/* Page 1 */
static lv_obj_t *s_time;
static lv_obj_t *s_date;
static lv_obj_t *s_temp_val;
static lv_obj_t *s_hum_val;
static lv_obj_t *s_quick[3];

/* Page 2 */
static lv_obj_t *s_led_i[3];
static lv_obj_t *s_led_s[3];
static lv_obj_t *s_dim_v;
static bool s_led[3] = {0,0,0};

static const lv_color_t LC[3] = {
    LV_COLOR_MAKE(0xFF,0x47,0x57),
    LV_COLOR_MAKE(0x2E,0xD5,0x73),
    LV_COLOR_MAKE(0x1E,0x90,0xFF),
};

/*=======================================================================*/
/* Clock                                                                  */
/*=======================================================================*/
static void upd_clock(void)
{
    time_t n = time(NULL);
    struct tm *t = localtime(&n);
    char b[64];
    strftime(b,sizeof(b),"%H:%M:%S",t);
    lv_label_set_text(s_time,b);
    strftime(b,sizeof(b),"%Y/%m/%d %a",t);
    lv_label_set_text(s_date,b);
}

static void tmr_cb(lv_timer_t *tm) { (void)tm; upd_clock(); }

/*=======================================================================*/
/* LED                                                                   */
/*=======================================================================*/
static void led_set(int i,int on)
{
    s_led[i] = on;
    lv_obj_set_style_bg_color(s_quick[i], on ? LC[i] : lv_color_hex(0x2f3542), 0);
    lv_obj_set_style_shadow_width(s_quick[i], on ? 12 : 0, 0);
    lv_obj_set_style_shadow_color(s_quick[i], LC[i], 0);
    lv_obj_set_style_shadow_opa(s_quick[i], on ? LV_OPA_60 : LV_OPA_0, 0);
    lv_obj_set_style_bg_color(s_led_s[i], on ? LC[i] : lv_color_hex(0x2f3542), 0);
    lv_obj_set_style_bg_color(s_led_i[i], on ? LC[i] : lv_color_hex(0x374151), 0);
    lv_obj_set_style_shadow_width(s_led_i[i], on ? 10 : 0, 0);
    lv_obj_set_style_shadow_color(s_led_i[i], LC[i], 0);
    lv_obj_set_style_shadow_opa(s_led_i[i], on ? LV_OPA_50 : LV_OPA_0, 0);
}

static void led_cb(lv_event_t *e)
{
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    led_set(i, !s_led[i]);
}

/*=======================================================================*/
/* Dimmer                                                                */
/*=======================================================================*/
static void dim_cb(lv_event_t *e)
{
    char b[8];
    snprintf(b,sizeof(b),"%d%%",(int)lv_slider_get_value(lv_event_get_target(e)));
    lv_label_set_text(s_dim_v,b);
}

/*=======================================================================*/
/* Alarm                                                                 */
/*=======================================================================*/
static void al_tg(lv_event_t *e) { (void)e; lv_obj_clear_flag(s_alarm_mq,LV_OBJ_FLAG_HIDDEN); }
static void al_cl(lv_event_t *e) { (void)e; lv_obj_add_flag(s_alarm_mq,LV_OBJ_FLAG_HIDDEN); }

/*=======================================================================*/
/* Swipe                                                                 */
/*=======================================================================*/
static void slide(void *o, int32_t v) { lv_obj_set_x((lv_obj_t*)o,v); }

static void go_page(int idx)
{
    if(idx==s_cur) return;
    s_cur=idx;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a,s_pc);
    lv_anim_set_exec_cb(&a,slide);
    lv_anim_set_values(&a,-idx*W,-idx*W);
    lv_anim_set_time(&a,250);
    lv_anim_set_path_cb(&a,lv_anim_path_ease_out);
    lv_anim_start(&a);
    int i;
    for(i=0;i<2;i++){
        lv_obj_set_style_bg_color(s_dots[i],i==idx?lv_color_hex(0x00d2ff):lv_color_hex(0x4b5563),0);
        lv_obj_set_size(s_dots[i],i==idx?10:8,i==idx?10:8);
    }
}

static void sw_cb(lv_event_t *e)
{
    (void)e;
    lv_dir_t d = lv_indev_get_gesture_dir(lv_indev_active());
    if(d==LV_DIR_LEFT && s_cur==0) go_page(1);
    if(d==LV_DIR_RIGHT && s_cur==1) go_page(0);
}

/*=======================================================================*/
/* Status bar                                                             */
/*=======================================================================*/
static void mk_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar,W,BAR_H);
    lv_obj_set_pos(bar,0,0);
    lv_obj_set_style_bg_color(bar,lv_color_hex(0x16213e),0);
    lv_obj_set_style_border_width(bar,0,0);
    lv_obj_set_style_radius(bar,0,0);
    lv_obj_set_style_pad_all(bar,0,0);

    lv_obj_t *t = lv_label_create(bar);
    lv_label_set_text(t,LV_SYMBOL_SETTINGS" Controller");
    lv_obj_set_style_text_color(t,lv_color_hex(0x00d2ff),0);
    lv_obj_set_style_text_font(t,&lv_font_montserrat_14,0);
    lv_obj_align(t,LV_ALIGN_LEFT_MID,8,0);

    lv_obj_t *rx = lv_label_create(bar);
    lv_label_set_text(rx,LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(rx,lv_color_hex(0x2ed573),0);
    lv_obj_align(rx,LV_ALIGN_RIGHT_MID,-8,0);

    lv_obj_t *sig = lv_label_create(bar);
    lv_label_set_text(sig,MDI_SIGNAL);
    lv_obj_set_style_text_color(sig,lv_color_hex(0x2ed573),0);
    lv_obj_set_style_text_font(sig,&mdi_icons_20,0);
    lv_obj_align(sig,LV_ALIGN_RIGHT_MID,-30,0);

    s_alarm_mq = lv_label_create(bar);
    lv_label_set_text(s_alarm_mq,LV_SYMBOL_BELL" ALARM TRIGGERED");
    lv_obj_set_style_text_color(s_alarm_mq,lv_color_hex(0xff4757),0);
    lv_obj_center(s_alarm_mq);
    lv_obj_add_flag(s_alarm_mq,LV_OBJ_FLAG_HIDDEN);
}

/*=======================================================================*/
/* Dashboard                                                              */
/*=======================================================================*/
static void mk_dash(lv_obj_t *parent)
{
    s_time = lv_label_create(parent);
    lv_label_set_text(s_time,"00:00:00");
    lv_obj_set_style_text_color(s_time,lv_color_hex(0xffffff),0);
    lv_obj_set_style_text_font(s_time,&lv_font_montserrat_48,0);
    lv_obj_set_style_text_letter_space(s_time,3,0);
    lv_obj_align(s_time,LV_ALIGN_TOP_MID,0,6);

    s_date = lv_label_create(parent);
    lv_label_set_text(s_date,"----/--/--");
    lv_obj_set_style_text_color(s_date,lv_color_hex(0x999999),0);
    lv_obj_set_style_text_font(s_date,&lv_font_montserrat_14,0);
    lv_obj_align(s_date,LV_ALIGN_TOP_MID,0,58);

    /* Env cards */
    lv_obj_t *ec = lv_obj_create(parent);
    lv_obj_set_size(ec,W-16,56);
    lv_obj_set_style_bg_opa(ec,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(ec,0,0);
    lv_obj_set_style_pad_all(ec,0,0);
    lv_obj_align(ec,LV_ALIGN_TOP_MID,0,86);

    /* Temp card */
    lv_obj_t *c1 = lv_obj_create(ec);
    lv_obj_set_size(c1,(W-28)/2,54);
    lv_obj_set_style_bg_color(c1,lv_color_hex(0x1a1a3a),0);
    lv_obj_set_style_border_color(c1,lv_color_hex(0x485460),0);
    lv_obj_set_style_border_width(c1,1,0);
    lv_obj_set_style_radius(c1,3,0);
    lv_obj_set_style_pad_all(c1,0,0);
    lv_obj_align(c1,LV_ALIGN_LEFT_MID,0,0);

    lv_obj_t *l1 = lv_label_create(c1);
    lv_label_set_text(l1,"Temperature");
    lv_obj_set_style_text_color(l1,lv_color_hex(0x999999),0);
    lv_obj_set_style_text_font(l1,&lv_font_montserrat_12,0);
    lv_obj_align(l1,LV_ALIGN_TOP_LEFT,10,8);

    s_temp_val = lv_label_create(c1);
    lv_label_set_text(s_temp_val,"26.5C");
    lv_obj_set_style_text_color(s_temp_val,lv_color_hex(0xff9f43),0);
    lv_obj_set_style_text_font(s_temp_val,&lv_font_montserrat_20,0);
    lv_obj_align(s_temp_val,LV_ALIGN_BOTTOM_LEFT,10,-8);

    lv_obj_t *ti = lv_label_create(c1);
    lv_label_set_text(ti,MDI_THERMOMETER);
    lv_obj_set_style_text_color(ti,lv_color_hex(0xff9f43),0);
    lv_obj_set_style_text_font(ti,&mdi_icons_20,0);
    lv_obj_align(ti,LV_ALIGN_TOP_RIGHT,-10,8);

    /* Humidity card */
    lv_obj_t *c2 = lv_obj_create(ec);
    lv_obj_set_size(c2,(W-28)/2,54);
    lv_obj_set_style_bg_color(c2,lv_color_hex(0x1a1a3a),0);
    lv_obj_set_style_border_color(c2,lv_color_hex(0x485460),0);
    lv_obj_set_style_border_width(c2,1,0);
    lv_obj_set_style_radius(c2,3,0);
    lv_obj_set_style_pad_all(c2,0,0);
    lv_obj_align(c2,LV_ALIGN_RIGHT_MID,0,0);

    lv_obj_t *l2 = lv_label_create(c2);
    lv_label_set_text(l2,"Humidity");
    lv_obj_set_style_text_color(l2,lv_color_hex(0x999999),0);
    lv_obj_set_style_text_font(l2,&lv_font_montserrat_12,0);
    lv_obj_align(l2,LV_ALIGN_TOP_LEFT,10,8);

    s_hum_val = lv_label_create(c2);
    lv_label_set_text(s_hum_val,"65.2%");
    lv_obj_set_style_text_color(s_hum_val,lv_color_hex(0x00d2ff),0);
    lv_obj_set_style_text_font(s_hum_val,&lv_font_montserrat_20,0);
    lv_obj_align(s_hum_val,LV_ALIGN_BOTTOM_LEFT,10,-8);

    lv_obj_t *hi = lv_label_create(c2);
    lv_label_set_text(hi,MDI_WATER_PERCENT);
    lv_obj_set_style_text_color(hi,lv_color_hex(0x00d2ff),0);
    lv_obj_set_style_text_font(hi,&mdi_icons_20,0);
    lv_obj_align(hi,LV_ALIGN_TOP_RIGHT,-10,8);

    /* Quick LED buttons */
    int i;
    for(i=0;i<3;i++){
        lv_obj_t *b = lv_btn_create(parent);
        lv_obj_set_size(b,34,34);
        lv_obj_set_style_bg_color(b,lv_color_hex(0x2f3542),0);
        lv_obj_set_style_border_color(b,lv_color_hex(0x57606f),0);
        lv_obj_set_style_border_width(b,2,0);
        lv_obj_set_style_radius(b,LV_RADIUS_CIRCLE,0);
        lv_obj_align(b,LV_ALIGN_BOTTOM_RIGHT,-20-i*48,-4);
        lv_obj_add_event_cb(b,led_cb,LV_EVENT_CLICKED,(void*)(intptr_t)i);
        lv_obj_t *lb = lv_label_create(b);
        char c[2] = {'R'+i,0};
        lv_label_set_text(lb,c);
        lv_obj_center(lb);
        s_quick[i]=b;
    }
}

/*=======================================================================*/
/* Control Panel                                                          */
/*=======================================================================*/
static void mk_ctrl(lv_obj_t *parent)
{
    int i;
    lv_obj_t *card, *ind, *lb, *sw, *knob;

    for(i=0;i<3;i++){
        card = lv_obj_create(parent);
        lv_obj_set_size(card,W-16,50);
        lv_obj_set_style_bg_color(card,lv_color_hex(0x1a1a3a),0);
        lv_obj_set_style_border_color(card,lv_color_hex(0x485460),0);
        lv_obj_set_style_border_width(card,1,0);
        lv_obj_set_style_radius(card,3,0);
        lv_obj_set_style_pad_all(card,6,12);
        lv_obj_align(card,LV_ALIGN_TOP_MID,0,8+i*56);

        ind = lv_obj_create(card);
        lv_obj_set_size(ind,20,20);
        lv_obj_set_style_bg_color(ind,lv_color_hex(0x374151),0);
        lv_obj_set_style_radius(ind,LV_RADIUS_CIRCLE,0);
        lv_obj_set_style_border_width(ind,1,0);
        lv_obj_set_style_border_color(ind,lv_color_hex(0xffffff),0);
        lv_obj_set_style_border_opa(ind,LV_OPA_20,0);
        lv_obj_align(ind,LV_ALIGN_LEFT_MID,12,0);
        lv_obj_remove_flag(ind,LV_OBJ_FLAG_CLICKABLE);
        s_led_i[i]=ind;

        lb = lv_label_create(card);
        lv_label_set_text(lb,i==0?"LED R":i==1?"LED G":"LED B");
        lv_obj_set_style_text_color(lb,lv_color_hex(0xcccccc),0);
        lv_obj_set_style_text_font(lb,&lv_font_montserrat_12,0);
        lv_obj_align(lb,LV_ALIGN_LEFT_MID,44,0);

        sw = lv_btn_create(card);
        lv_obj_set_size(sw,44,24);
        lv_obj_set_style_bg_color(sw,lv_color_hex(0x2f3542),0);
        lv_obj_set_style_radius(sw,12,0);
        lv_obj_set_style_border_color(sw,lv_color_hex(0x57606f),0);
        lv_obj_set_style_border_width(sw,1,0);
        lv_obj_align(sw,LV_ALIGN_RIGHT_MID,-10,0);
        lv_obj_add_event_cb(sw,led_cb,LV_EVENT_CLICKED,(void*)(intptr_t)i);
        s_led_s[i]=sw;

        knob = lv_obj_create(sw);
        lv_obj_set_size(knob,20,20);
        lv_obj_set_style_bg_color(knob,lv_color_hex(0xffffff),0);
        lv_obj_set_style_radius(knob,LV_RADIUS_CIRCLE,0);
        lv_obj_set_style_border_width(knob,0,0);
        lv_obj_align(knob,LV_ALIGN_LEFT_MID,2,0);
        lv_obj_remove_flag(knob,LV_OBJ_FLAG_CLICKABLE);
    }

    /* Brightness */
    card = lv_obj_create(parent);
    lv_obj_set_size(card,W-16,56);
    lv_obj_set_style_bg_color(card,lv_color_hex(0x1a1a3a),0);
    lv_obj_set_style_border_color(card,lv_color_hex(0x485460),0);
    lv_obj_set_style_border_width(card,1,0);
    lv_obj_set_style_radius(card,3,0);
    lv_obj_set_style_pad_all(card,8,16);
    lv_obj_align(card,LV_ALIGN_TOP_MID,0,8+3*56+4);

    lv_obj_t *tl = lv_label_create(card);
    lv_label_set_text(tl,"Brightness");
    lv_obj_set_style_text_color(tl,lv_color_hex(0xcccccc),0);
    lv_obj_set_style_text_font(tl,&lv_font_montserrat_12,0);
    lv_obj_align(tl,LV_ALIGN_TOP_LEFT,0,0);

    s_dim_v = lv_label_create(card);
    lv_label_set_text(s_dim_v,"72%");
    lv_obj_set_style_text_color(s_dim_v,lv_color_hex(0x00d2ff),0);
    lv_obj_set_style_text_font(s_dim_v,&lv_font_montserrat_12,0);
    lv_obj_align(s_dim_v,LV_ALIGN_TOP_RIGHT,0,0);

    lv_obj_t *sl = lv_slider_create(card);
    lv_obj_set_size(sl,W-64,6);
    lv_obj_align(sl,LV_ALIGN_BOTTOM_MID,0,-8);
    lv_slider_set_range(sl,0,100);
    lv_slider_set_value(sl,72,LV_ANIM_OFF);
    lv_obj_set_style_bg_color(sl,lv_color_hex(0x2f3542),LV_PART_MAIN);
    lv_obj_set_style_bg_color(sl,lv_color_hex(0xffffff),LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sl,lv_color_hex(0xffffff),LV_PART_KNOB);
    lv_obj_set_style_radius(sl,3,LV_PART_MAIN|LV_PART_INDICATOR);
    lv_obj_set_style_radius(sl,LV_RADIUS_CIRCLE,LV_PART_KNOB);
    lv_obj_add_event_cb(sl,dim_cb,LV_EVENT_VALUE_CHANGED,NULL);

    /* Alarm */
    card = lv_obj_create(parent);
    lv_obj_set_size(card,W-16,44);
    lv_obj_set_style_bg_color(card,lv_color_hex(0x1a1a3a),0);
    lv_obj_set_style_border_color(card,lv_color_hex(0x485460),0);
    lv_obj_set_style_border_width(card,1,0);
    lv_obj_set_style_radius(card,3,0);
    lv_obj_set_style_pad_all(card,4,12);
    lv_obj_align(card,LV_ALIGN_BOTTOM_MID,0,-4);

    lv_obj_t *ai = lv_label_create(card);
    lv_label_set_text(ai,LV_SYMBOL_BELL);
    lv_obj_set_style_text_color(ai,lv_color_hex(0xff4757),0);
    lv_obj_set_style_text_font(ai,&lv_font_montserrat_14,0);
    lv_obj_align(ai,LV_ALIGN_LEFT_MID,8,0);

    lb = lv_label_create(card);
    lv_label_set_text(lb,"Alarm System");
    lv_obj_set_style_text_color(lb,lv_color_hex(0xcccccc),0);
    lv_obj_set_style_text_font(lb,&lv_font_montserrat_12,0);
    lv_obj_align(lb,LV_ALIGN_LEFT_MID,32,0);

    lv_obj_t *btr = lv_btn_create(card);
    lv_obj_set_size(btr,60,26);
    lv_obj_set_style_bg_color(btr,lv_color_hex(0xe74c3c),0);
    lv_obj_set_style_radius(btr,3,0);
    lv_obj_align(btr,LV_ALIGN_RIGHT_MID,0,0);
    lv_obj_add_event_cb(btr,al_tg,LV_EVENT_CLICKED,NULL);
    lv_obj_t *ltr = lv_label_create(btr);
    lv_label_set_text(ltr,"TRIGGER");
    lv_obj_set_style_text_font(ltr,&lv_font_montserrat_12,0);
    lv_obj_center(ltr);

    lv_obj_t *bcl = lv_btn_create(card);
    lv_obj_set_size(bcl,60,26);
    lv_obj_set_style_bg_color(bcl,lv_color_hex(0x636e72),0);
    lv_obj_set_style_radius(bcl,3,0);
    lv_obj_align(bcl,LV_ALIGN_RIGHT_MID,-68,0);
    lv_obj_add_event_cb(bcl,al_cl,LV_EVENT_CLICKED,NULL);
    lv_obj_t *lcl = lv_label_create(bcl);
    lv_label_set_text(lcl,"CLEAR");
    lv_obj_set_style_text_font(lcl,&lv_font_montserrat_12,0);
    lv_obj_center(lcl);
}

/*=======================================================================*/
/* Screen lifecycle                                                       */
/*=======================================================================*/
static void scr_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    s_cont = lv_obj_create(scr);
    lv_obj_set_size(s_cont,W,H);
    lv_obj_set_pos(s_cont,0,0);
    lv_obj_set_style_bg_color(s_cont,lv_color_hex(0x0f0f23),0);
    lv_obj_set_style_border_width(s_cont,0,0);
    lv_obj_set_style_radius(s_cont,0,0);
    lv_obj_set_style_pad_all(s_cont,0,0);
    lv_obj_add_flag(s_cont,LV_OBJ_FLAG_HIDDEN);

    mk_bar(s_cont);

    s_pc = lv_obj_create(s_cont);
    lv_obj_set_size(s_pc,W*2,PGH);
    lv_obj_set_pos(s_pc,0,BAR_H);
    lv_obj_set_style_bg_opa(s_pc,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(s_pc,0,0);
    lv_obj_set_style_pad_all(s_pc,0,0);
    lv_obj_remove_flag(s_pc,LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *p1 = lv_obj_create(s_pc);
    lv_obj_set_size(p1,W,PGH);
    lv_obj_set_pos(p1,0,0);
    lv_obj_set_style_bg_opa(p1,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(p1,0,0);
    lv_obj_set_style_pad_all(p1,0,0);
    mk_dash(p1);

    lv_obj_t *p2 = lv_obj_create(s_pc);
    lv_obj_set_size(p2,W,PGH);
    lv_obj_set_pos(p2,W,0);
    lv_obj_set_style_bg_opa(p2,LV_OPA_TRANSP,0);
    lv_obj_set_style_border_width(p2,0,0);
    lv_obj_set_style_pad_all(p2,0,0);
    mk_ctrl(p2);

    int i;
    for(i=0;i<2;i++){
        s_dots[i]=lv_obj_create(s_cont);
        lv_obj_set_size(s_dots[i],8,8);
        lv_obj_set_style_radius(s_dots[i],LV_RADIUS_CIRCLE,0);
        lv_obj_set_style_border_width(s_dots[i],0,0);
        lv_obj_set_style_bg_color(s_dots[i],i==0?lv_color_hex(0x00d2ff):lv_color_hex(0x4b5563),0);
        lv_obj_align(s_dots[i],LV_ALIGN_BOTTOM_MID,-10+i*20,-4);
        lv_obj_remove_flag(s_dots[i],LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_add_flag(s_cont,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(s_cont,sw_cb,LV_EVENT_GESTURE,NULL);

    upd_clock();
    lv_timer_create(tmr_cb,1000,NULL);
}

static void scr_enter(void) { lv_obj_clear_flag(s_cont,LV_OBJ_FLAG_HIDDEN); }
static void scr_exit(void)  { lv_obj_add_flag(s_cont,LV_OBJ_FLAG_HIDDEN); }

static hz_screen_t s_scr;

hz_screen_t *hmi_get_screen(void) { return &s_scr; }

void screen_hmi_create(void)
{
    s_scr = (hz_screen_t){
        .name="hmi_controller", .on_create=scr_create,
        .on_enter=scr_enter, .on_exit=scr_exit,
    };
    scr_create();
}
