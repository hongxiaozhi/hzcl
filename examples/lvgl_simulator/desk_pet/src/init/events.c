#include "init/events.h"
#include "app.h"
#include "hz_event.h"
#include "hz_sensor.h"
#include "hz_config_mgr.h"
#include "hz_fsm.h"
#include "hz_alarm.h"

static void on_sensor_threshold(void *data, void *user)
{
    (void)user;
    hz_sensor_threshold_event_t *evt = (hz_sensor_threshold_event_t *)data;
    if (evt->channel_id == SENSOR_CH_WELLNESS) {
        if (evt->direction == 0)
            g_pet.mood = HZ_MIN(100, g_pet.mood + 3);
        else
            g_pet.mood = HZ_MAX(0, g_pet.mood - 3);
    }
    if (evt->channel_id == SENSOR_CH_ACTIVITY && evt->direction == 1) {
        if (hz_fsm_current() == PET_STATE_IDLE)
            hz_alarm_trigger(ALARM_LOW_ACTIVITY);
    }
}

static void on_config_changed(void *data, void *user)
{
    (void)user;
    hz_config_id_t id = (hz_config_id_t)(hz_u32)data;
    switch (id) {
    case CFG_HUNGER_DECAY:  hz_config_mgr_get(CFG_HUNGER_DECAY, &g_app_cfg.hunger_decay); break;
    case CFG_ENERGY_DECAY:  hz_config_mgr_get(CFG_ENERGY_DECAY, &g_app_cfg.energy_decay); break;
    case CFG_MOOD_DECAY:    hz_config_mgr_get(CFG_MOOD_DECAY, &g_app_cfg.mood_decay); break;
    case CFG_HUNGER_THRESH: hz_config_mgr_get(CFG_HUNGER_THRESH, &g_app_cfg.hunger_thresh); break;
    case CFG_ENERGY_THRESH: hz_config_mgr_get(CFG_ENERGY_THRESH, &g_app_cfg.energy_thresh); break;
    case CFG_THEME_COLOR:   hz_config_mgr_get(CFG_THEME_COLOR, &g_app_cfg.theme_color); break;
    case CFG_AUTO_SAVE_MS:  hz_config_mgr_get(CFG_AUTO_SAVE_MS, &g_app_cfg.auto_save_ms); break;
    }
}

void init_events(void)
{
    hz_event_subscribe(EV_SENSOR_THRESHOLD, on_sensor_threshold, NULL);
    hz_event_subscribe(EV_CONFIG_CHANGED,   on_config_changed,   NULL);
}
