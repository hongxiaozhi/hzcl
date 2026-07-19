/*
 * HZCL - Horizon Zone Control Library
 * hz_sensor.c - 传感器数据流水线实现
 */

#include "hz_sensor.h"
#include "hz_sensor_filter.h"

#if HZ_ENABLE_SENSOR

#include <string.h>

/* ============================================================
 * 传感器通道
 * ============================================================ */

typedef struct {
    hz_sensor_driver_t    *driver;
    hz_u32                 interval_ms;
    hz_u32                 elapsed_ms;
    float                  last_value;
    hz_s32                 last_raw;
    hz_filter_type_t       filter_type;
    float                  threshold_upper;
    float                  threshold_lower;
    hz_bool_t              inited;

    /* 滤波器状态 */
    hz_moving_avg_filter_t ma_filter;
    hz_lowpass_filter_t    lp_filter;
} hz_channel_t;

/* ============================================================
 * 全局状态
 * ============================================================ */

static hz_channel_t s_channels[HZ_SENSOR_MAX_CHANNELS];
static hz_u8        s_channel_count = 0;
static hz_bool_t    s_inited = false;

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_sensor_init(void)
{
    s_channel_count = 0;
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 注册传感器通道
 * ============================================================ */

hz_err_t hz_sensor_channel_register(const hz_sensor_channel_cfg_t *cfg,
                                    hz_sensor_channel_id_t *out_id)
{
    hz_channel_t *ch;

    if (!s_inited || !cfg || !cfg->driver || !out_id) {
        return HZ_ERR_INVALID;
    }

    if (s_channel_count >= HZ_SENSOR_MAX_CHANNELS) {
        return HZ_ERR_NOMEM;
    }

    ch = &s_channels[s_channel_count];
    memset(ch, 0, sizeof(hz_channel_t));

    ch->driver          = cfg->driver;
    ch->interval_ms     = cfg->interval_ms;
    ch->elapsed_ms      = 0;
    ch->filter_type     = cfg->filter_type;
    ch->threshold_upper = cfg->threshold_upper;
    ch->threshold_lower = cfg->threshold_lower;

    /* 初始化滤波器 */
    if (cfg->filter_type == HZ_FILTER_MOVING_AVG) {
        hz_moving_avg_filter_init(&ch->ma_filter, cfg->filter_window);
    } else if (cfg->filter_type == HZ_FILTER_LOWPASS) {
        hz_lowpass_filter_init(&ch->lp_filter, 0.3f); /* 默认alpha */
    }

    /* 调用驱动 init */
    if (cfg->driver->init) {
        hz_err_t err = cfg->driver->init(NULL);
        if (err != HZ_OK) {
            return err;
        }
    }

    ch->inited = true;
    *out_id = (hz_sensor_channel_id_t)s_channel_count;
    s_channel_count++;

    return HZ_OK;
}

/* ============================================================
 * 读取传感器最新物理值
 * ============================================================ */

hz_err_t hz_sensor_channel_read(hz_sensor_channel_id_t id, float *value)
{
    if (!s_inited || id >= s_channel_count || !value) {
        return HZ_ERR_INVALID;
    }
    *value = s_channels[id].last_value;
    return HZ_OK;
}

/* ============================================================
 * 读取传感器原始值
 * ============================================================ */

hz_err_t hz_sensor_read_raw(hz_sensor_channel_id_t id, hz_s32 *raw)
{
    if (!s_inited || id >= s_channel_count || !raw) {
        return HZ_ERR_INVALID;
    }
    *raw = s_channels[id].last_raw;
    return HZ_OK;
}

/* ============================================================
 * 周期性采集与处理
 * ============================================================ */

void hz_sensor_tick(hz_u32 ms)
{
    hz_u8 i;

    if (!s_inited) return;

    for (i = 0; i < s_channel_count; i++) {
        hz_channel_t *ch = &s_channels[i];
        if (!ch->inited) continue;

        ch->elapsed_ms += ms;
        if (ch->elapsed_ms < ch->interval_ms) continue;
        ch->elapsed_ms = 0;

        /* 读取原始值 */
        if (ch->driver->read_raw) {
            if (ch->driver->read_raw(&ch->last_raw) != HZ_OK) {
                continue;
            }
        }

        /* 单位转换 */
        float physical = (float)ch->last_raw;
        if (ch->driver->raw_to_physical) {
            physical = ch->driver->raw_to_physical(ch->last_raw);
        }

        /* 滤波 */
        switch (ch->filter_type) {
        case HZ_FILTER_MOVING_AVG:
            physical = hz_moving_avg_filter_process(&ch->ma_filter, physical);
            break;
        case HZ_FILTER_LOWPASS:
            physical = hz_lowpass_filter_process(&ch->lp_filter, physical);
            break;
        default:
            break;
        }

        ch->last_value = physical;

        /* 发布数据就绪事件 */
        hz_event_publish(EV_SENSOR_DATA_READY, (void *)(hz_u32)i);

        /* 阈值检测 */
        if (ch->threshold_upper > 0.0f && physical > ch->threshold_upper) {
            hz_sensor_threshold_event_t evt;
            evt.channel_id     = i;
            evt.sensor_type    = ch->driver->type;
            evt.current_value  = physical;
            evt.threshold_value = ch->threshold_upper;
            evt.direction      = 0;
            hz_event_publish(EV_SENSOR_THRESHOLD, &evt);
        }
        if (physical < ch->threshold_lower) {
            hz_sensor_threshold_event_t evt;
            evt.channel_id     = i;
            evt.sensor_type    = ch->driver->type;
            evt.current_value  = physical;
            evt.threshold_value = ch->threshold_lower;
            evt.direction      = 1;
            hz_event_publish(EV_SENSOR_THRESHOLD, &evt);
        }
    }
}

#endif /* HZ_ENABLE_SENSOR */
