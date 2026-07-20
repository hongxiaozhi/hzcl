/**
 * @file screen_home.c
 * @brief Home screen — main pet view with emoji, stats, action buttons, and animations
 */

#include "lvgl/lvgl.h"
#include "hz_types.h"
#include "app.h"
#include "hz_fsm.h"
#include "hz_screen.h"
#include "hz_event.h"
#include "hz_mode.h"
#include "hz_alarm.h"
#include "pet_face.h"

#include <stdio.h>

/*===========================================================================
 * Widget references
 *===========================================================================*/
static lv_obj_t *s_cont;
static lv_obj_t *s_pet_emoji;
static lv_obj_t *s_mood_label;
static lv_obj_t *s_status_label;
static lv_obj_t *s_hunger_bar;
static lv_obj_t *s_energy_bar;
static lv_obj_t *s_mood_bar;
static lv_obj_t *s_hygiene_bar;
static lv_obj_t *s_alarm_label;
static lv_obj_t *s_mode_badge;

/*===========================================================================
 * Animation
 *===========================================================================*/
static lv_anim_t s_float_anim;
static int32_t s_emoji_base_y;

/* Animation exec callback: move emoji Y */
static void anim_emoji_y(void *obj, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)obj, v);
}

/* Start floating animation with given amplitude (px) and period (ms) */
static void start_float_anim(int32_t amplitude, uint32_t period)
{
    lv_anim_init(&s_float_anim);
    lv_anim_set_var(&s_float_anim, s_pet_emoji);
    lv_anim_set_exec_cb(&s_float_anim, anim_emoji_y);
    lv_anim_set_values(&s_float_anim, s_emoji_base_y - amplitude, s_emoji_base_y + amplitude);
    lv_anim_set_time(&s_float_anim, period);
    lv_anim_set_playback_time(&s_float_anim, period);
    lv_anim_set_repeat_count(&s_float_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&s_float_anim);
}

/* Update animation based on current FSM state */
static void update_pet_animation(void)
{
    hz_state_t cur = hz_fsm_current();
    switch (cur) {
    case PET_STATE_IDLE:
        start_float_anim(8, 2000);       /* Gentle float */
        break;
    case PET_STATE_HAPPY:
        start_float_anim(14, 800);       /* Happy bounce */
        break;
    case PET_STATE_EATING:
        start_float_anim(5, 300);        /* Fast tiny nom-nom */
        break;
    case PET_STATE_SLEEPING:
        start_float_anim(12, 3500);      /* Slow deep drift */
        break;
    case PET_STATE_SAD:
        start_float_anim(6, 1500);       /* Slow weak wobble */
        break;
    case PET_STATE_SICK:
        start_float_anim(4, 1000);       /* Weak tremble */
        break;
    case PET_STATE_HYPER:
        start_float_anim(18, 500);       /* Excited big bounce */
        break;
    case PET_STATE_TRAINING:
        start_float_anim(10, 600);       /* Energetic pace */
        break;
    case PET_STATE_CLEANING:
        start_float_anim(6, 1200);       /* Gentle freshness */
        break;
    default:
        start_float_anim(8, 2000);
        break;
    }
}

/* Forward declarations for functions defined later */
static void home_update(void);
static void home_update_ui_wrapper(void *data, void *user);
static void update_pet_animation(void);

/*===========================================================================
 * Button callbacks
 *===========================================================================*/
static void btn_feed_cb(lv_event_t *e)   { (void)e; hz_event_publish(EV_PET_FEED, NULL); hz_fsm_execute_action(ACT_FEED, NULL); }
static void btn_pet_cb(lv_event_t *e)    { (void)e; hz_event_publish(EV_PET_PET, NULL); hz_fsm_execute_action(ACT_PET, NULL); }
static void btn_sleep_cb(lv_event_t *e)  { (void)e; hz_event_publish(EV_PET_SLEEP, NULL); hz_fsm_execute_action(ACT_GO_SLEEP, NULL); }
static void btn_clean_cb(lv_event_t *e)  { (void)e; hz_event_publish(EV_PET_CLEAN, NULL); hz_fsm_execute_action(ACT_CLEAN, NULL); }
static void btn_train_cb(lv_event_t *e)  { (void)e; hz_event_publish(EV_PET_TRAIN, NULL); hz_fsm_execute_action(ACT_TRAIN, NULL); }

static void btn_mode_cb(lv_event_t *e)
{
    (void)e;
    hz_mode_id_t cur = hz_mode_current();
    hz_mode_id_t next = (cur + 1) % MODE_COUNT;
    hz_mode_switch(next);
}

/*===========================================================================
 * Navigation callbacks
 *===========================================================================*/
hz_screen_t *screen_stats_get(void);
hz_screen_t *screen_settings_get(void);
hz_screen_t *screen_about_get(void);

static void btn_stats_cb(lv_event_t *e)     { (void)e; hz_screen_push(screen_stats_get(), NULL); }
static void btn_settings_cb(lv_event_t *e)  { (void)e; hz_screen_push(screen_settings_get(), NULL); }
static void btn_about_cb(lv_event_t *e)    { (void)e; hz_screen_push(screen_about_get(), NULL); }

/*===========================================================================
 * Screen lifecycle
 *===========================================================================*/
static void home_on_create(void)
{
    s_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(s_cont, 0, 0);
    lv_obj_set_style_bg_opa(s_cont, LV_OPA_TRANSP, 0);

    lv_obj_t *title = lv_label_create(s_cont);
    lv_label_set_text(title, "Pro Desk Pet");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    s_mode_badge = lv_label_create(s_cont);
    lv_label_set_text(s_mode_badge, "[NORMAL]");
    lv_obj_set_style_text_color(s_mode_badge, lv_color_hex(0x00d2ff), 0);
    lv_obj_align(s_mode_badge, LV_ALIGN_TOP_LEFT, 5, 8);

    s_alarm_label = lv_label_create(s_cont);
    lv_label_set_text(s_alarm_label, "");
    lv_obj_set_style_text_color(s_alarm_label, lv_color_hex(0xff0000), 0);
    lv_obj_align(s_alarm_label, LV_ALIGN_TOP_RIGHT, -5, 8);
    lv_obj_set_hidden(s_alarm_label, true);

    /* Pet face (cartoon character drawn with shapes) */
    pet_face_create(s_cont);
    s_pet_emoji = pet_face_get_cont();
    lv_obj_align(s_pet_emoji, LV_ALIGN_CENTER, -60, -25);
    s_emoji_base_y = lv_obj_get_y(s_pet_emoji);

    s_mood_label = lv_label_create(s_cont);
    lv_label_set_text(s_mood_label, "Chillin...");
    lv_obj_set_style_text_color(s_mood_label, lv_color_hex(0xcccccc), 0);
    lv_obj_align(s_mood_label, LV_ALIGN_CENTER, -60, 40);

    s_status_label = lv_label_create(s_cont);
    lv_label_set_text(s_status_label, "Click buttons to interact!");
    lv_obj_set_style_text_color(s_status_label, lv_color_hex(0x888888), 0);
    lv_obj_align(s_status_label, LV_ALIGN_BOTTOM_MID, 0, -60);

    /* Right-side bars */
    const char *names[] = { "Fullness", "Energy", "Mood", "Hygiene" };
    hz_u32 colors[] = { 0xff6b6b, 0x4ecdc4, 0xffd93d, 0x74b9ff };
    lv_obj_t *bars[] = { NULL, NULL, NULL, NULL };

    for (int i = 0; i < 4; i++) {
        lv_obj_t *lbl = lv_label_create(s_cont);
        lv_label_set_text(lbl, names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(colors[i]), 0);
        lv_obj_align(lbl, LV_ALIGN_RIGHT_MID, -120, -70 + i * 25);

        lv_obj_t *bar = lv_bar_create(s_cont);
        lv_obj_set_size(bar, 90, 8);
        lv_obj_align(bar, LV_ALIGN_RIGHT_MID, -20, -70 + i * 25);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x444444), 0);
        lv_obj_set_style_anim_time(bar, 300, 0);
        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 50, LV_ANIM_OFF);
        bars[i] = bar;
    }
    s_hunger_bar  = bars[0];
    s_energy_bar  = bars[1];
    s_mood_bar    = bars[2];
    s_hygiene_bar = bars[3];

    /* Bottom action buttons (row 1) */
    lv_obj_t *btn;
    const char *btn_labels[] = { "\xF0\x9F\x8D\x96 Feed", "\xF0\x9F\x91\x8B Pet", "\xF0\x9F\x92\xA4 Sleep" };
    hz_u32 btn_colors[] = { 0xe17055, 0x00b894, 0x6c5ce7 };
    lv_event_cb_t btn_cbs[] = { btn_feed_cb, btn_pet_cb, btn_sleep_cb };
    int btn_x[] = { -130, 0, 130 };

    for (int i = 0; i < 3; i++) {
        btn = lv_btn_create(s_cont);
        lv_obj_set_size(btn, 90, 35);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, btn_x[i], -35);
        lv_obj_set_style_bg_color(btn, lv_color_hex(btn_colors[i]), 0);
        lv_obj_add_event_cb(btn, btn_cbs[i], LV_EVENT_CLICKED, NULL);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, btn_labels[i]);
        lv_obj_center(lbl);
    }

    /* Row 2: Clean, Train, Mode */
    const char *btn_labels2[] = { "\xF0\x9F\xA7\xB9 Clean", "\xF0\x9F\x92\xAA Train", "\xF0\x9F\x94\x81 Mode" };
    hz_u32 btn_colors2[] = { 0x0984e3, 0xe17055, 0x636e72 };
    lv_event_cb_t btn_cbs2[] = { btn_clean_cb, btn_train_cb, btn_mode_cb };

    for (int i = 0; i < 3; i++) {
        btn = lv_btn_create(s_cont);
        lv_obj_set_size(btn, 90, 30);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, btn_x[i], 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(btn_colors2[i]), 0);
        lv_obj_add_event_cb(btn, btn_cbs2[i], LV_EVENT_CLICKED, NULL);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, btn_labels2[i]);
        lv_obj_center(lbl);
    }

    /* Bottom nav buttons */
    const char *nav_labels[] = { "Stats", "Settings", "About" };
    lv_event_cb_t nav_cbs[] = { btn_stats_cb, btn_settings_cb, btn_about_cb };
    int nav_x[] = { -80, 0, 80 };

    for (int i = 0; i < 3; i++) {
        btn = lv_btn_create(s_cont);
        lv_obj_set_size(btn, 65, 22);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, nav_x[i], -85);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x444444), 0);
        lv_obj_add_event_cb(btn, nav_cbs[i], LV_EVENT_CLICKED, NULL);
        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, nav_labels[i]);
        lv_obj_center(lbl);
    }
}

static void home_on_load(void *ctx)
{
    (void)ctx;
    hz_event_subscribe(EV_FSM_STATE_CHANGED, home_update_ui_wrapper, NULL);
    hz_event_subscribe(EV_ALARM_TRIGGERED,   home_update_ui_wrapper, NULL);
    hz_event_subscribe(EV_ALARM_CLEARED,     home_update_ui_wrapper, NULL);
    hz_event_subscribe(EV_MODE_CHANGED,      home_update_ui_wrapper, NULL);
    home_update();
    update_pet_animation();   /* Start floating animation */
}

static void home_on_unload(void)
{
    lv_anim_del(s_pet_emoji, NULL);          /* Stop animation */
    hz_event_unsubscribe(EV_FSM_STATE_CHANGED, home_update_ui_wrapper);
    hz_event_unsubscribe(EV_ALARM_TRIGGERED,   home_update_ui_wrapper);
    hz_event_unsubscribe(EV_ALARM_CLEARED,     home_update_ui_wrapper);
    hz_event_unsubscribe(EV_MODE_CHANGED,      home_update_ui_wrapper);
}

static void home_on_enter(void) { lv_obj_set_hidden(s_cont, false); home_update(); update_pet_animation(); }
static void home_on_exit(void)  { lv_obj_set_hidden(s_cont, true); lv_anim_del(s_pet_emoji, NULL); }

/*===========================================================================
 * Event callback wrapper
 *===========================================================================*/
static void home_update_ui_wrapper(void *data, void *user)
{
    (void)data; (void)user; home_update();
}

/*===========================================================================
 * UI update
 *===========================================================================*/
static void update_theme(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(g_theme.bg_color), 0);
}

static void home_update(void)
{
    hz_state_t cur = hz_fsm_current();

    update_theme();
    pet_face_set_state(cur, g_pet.hunger, g_pet.energy, g_pet.mood);
    lv_label_set_text(s_mood_label, g_state_moods[cur]);

    lv_bar_set_value(s_hunger_bar,  100 - g_pet.hunger, LV_ANIM_ON);
    lv_bar_set_value(s_energy_bar,  g_pet.energy,       LV_ANIM_ON);
    lv_bar_set_value(s_mood_bar,    g_pet.mood,         LV_ANIM_ON);
    lv_bar_set_value(s_hygiene_bar, g_pet.hygiene,      LV_ANIM_ON);

    const char *mode_name = hz_mode_current_name();
    char buf[32];
    snprintf(buf, sizeof(buf), "[%s]", mode_name ? mode_name : "?");
    lv_label_set_text(s_mode_badge, buf);
    lv_obj_set_style_text_color(s_mode_badge, lv_color_hex(g_theme.accent_color), 0);

    hz_bool_t any_alarm = 0;
    for (hz_u8 i = 0; i < ALARM_COUNT; i++) {
        if (hz_alarm_is_active(i)) { any_alarm = 1; break; }
    }
    lv_obj_set_hidden(s_alarm_label, !any_alarm);
    if (any_alarm) lv_label_set_text(s_alarm_label, "!ALERT!");

    const char *mood_str = app_mood_label(g_pet.mood);
    const char *state_name = hz_fsm_current_name();
    snprintf(buf, sizeof(buf), "[%s] %s", state_name, mood_str);
    lv_label_set_text(s_status_label, buf);

    update_pet_animation();   /* Change animation style to match new state */
}

/*===========================================================================
 * Screen descriptor & factory
 *===========================================================================*/
static hz_screen_t s_home_screen;

void screen_home_create(void)
{
    s_home_screen = (hz_screen_t){
        .name      = "home",
        .on_create = home_on_create,
        .on_load   = home_on_load,
        .on_unload = home_on_unload,
        .on_enter  = home_on_enter,
        .on_exit   = home_on_exit,
    };
    home_on_create();
}

hz_screen_t *screen_home_get(void)
{
    return &s_home_screen;
}
