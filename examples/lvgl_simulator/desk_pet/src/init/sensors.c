#include "init/sensors.h"
#include "app.h"
#include "hz_sensor.h"
#include "hz_sensor_filter.h"
#include <stdlib.h>

static int s_activity_counter = 0;

void app_notify_activity(void) { s_activity_counter++; }

/*===========================================================================
 * Wellness sensor driver
 *===========================================================================*/
static hz_err_t wellness_init(void *param)        { (void)param; return HZ_OK; }
static hz_err_t wellness_deinit(void)             { return HZ_OK; }
static hz_err_t wellness_read_raw(hz_s32 *raw)    { *raw = (hz_s32)(rand() % 100); return HZ_OK; }
static float    wellness_raw_to_physical(hz_s32 raw) { return (float)raw; }
static hz_err_t wellness_self_check(void)         { return HZ_OK; }

static hz_sensor_driver_t s_wellness_driver = {
    .name = "wellness", .type = HZ_SENSOR_TYPE_USER,
    .init = wellness_init, .deinit = wellness_deinit,
    .read_raw = wellness_read_raw, .raw_to_physical = wellness_raw_to_physical,
    .self_check = wellness_self_check,
};

/*===========================================================================
 * Activity sensor driver
 *===========================================================================*/
static hz_err_t activity_init(void *param)        { (void)param; return HZ_OK; }
static hz_err_t activity_deinit(void)             { return HZ_OK; }
static hz_err_t activity_read_raw(hz_s32 *raw)    { *raw = (hz_s32)s_activity_counter; s_activity_counter = 0; return HZ_OK; }
static float    activity_raw_to_physical(hz_s32 raw) { return (float)raw; }
static hz_err_t activity_self_check(void)         { return HZ_OK; }

static hz_sensor_driver_t s_activity_driver = {
    .name = "activity", .type = HZ_SENSOR_TYPE_USER,
    .init = activity_init, .deinit = activity_deinit,
    .read_raw = activity_read_raw, .raw_to_physical = activity_raw_to_physical,
    .self_check = activity_self_check,
};

/*===========================================================================
 * Sensor channel IDs (returned by register, used by events)
 *===========================================================================*/
hz_sensor_channel_id_t g_ch_wellness;
hz_sensor_channel_id_t g_ch_activity;

void init_sensors(void)
{
    hz_sensor_channel_cfg_t wellness_cfg = {
        .driver = &s_wellness_driver, .interval_ms = 3000,
        .filter_type = HZ_FILTER_MOVING_AVG, .filter_window = 5,
        .threshold_upper = 80.0f, .threshold_lower = 20.0f,
    };
    hz_sensor_channel_register(&wellness_cfg, &g_ch_wellness);

    hz_sensor_channel_cfg_t activity_cfg = {
        .driver = &s_activity_driver, .interval_ms = 5000,
        .filter_type = HZ_FILTER_MOVING_AVG, .filter_window = 3,
        .threshold_upper = 0, .threshold_lower = 0.5f,
    };
    hz_sensor_channel_register(&activity_cfg, &g_ch_activity);
}
