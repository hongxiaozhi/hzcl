/*
 * HZCL - Horizon Zone Control Library
 * hz_sensor.h - 传感器数据流水线
 *
 * 解决痛点：传感器数据采集/滤波/转换/阈值逻辑分散各处
 *
 * 数据流：
 *   驱动 read_raw → 滤波 → 单位转换 → 阈值检测 → 发布事件
 */

#ifndef HZ_SENSOR_H
#define HZ_SENSOR_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_SENSOR

#include "hz_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 传感器类型
 * ============================================================ */

typedef enum {
    HZ_SENSOR_TYPE_NONE      = 0,
    HZ_SENSOR_TYPE_PRESSURE  = 1,   /* 压力 */
    HZ_SENSOR_TYPE_FLOW      = 2,   /* 流量 */
    HZ_SENSOR_TYPE_TEMP      = 3,   /* 温度 */
    HZ_SENSOR_TYPE_ACCEL     = 4,   /* 加速度 */
    HZ_SENSOR_TYPE_BATTERY   = 5,   /* 电量 */
    HZ_SENSOR_TYPE_USER      = 0x80,/* 用户自定义类型起始 */
} hz_sensor_type_t;

/* ============================================================
 * 滤波类型
 * ============================================================ */

typedef enum {
    HZ_FILTER_NONE       = 0,   /* 无滤波 */
    HZ_FILTER_MOVING_AVG = 1,   /* 滑动平均 */
    HZ_FILTER_MEDIAN     = 2,   /* 中值滤波 */
    HZ_FILTER_LOWPASS    = 3,   /* 一阶低通 */
} hz_filter_type_t;

/* ============================================================
 * 传感器驱动接口（由具体硬件驱动实现）
 * ============================================================ */

typedef struct hz_sensor_driver {
    const char      *name;
    hz_sensor_type_t type;

    hz_err_t (*init)(void *param);
    hz_err_t (*deinit)(void);
    hz_err_t (*read_raw)(hz_s32 *raw);
    float    (*raw_to_physical)(hz_s32 raw);
    hz_err_t (*self_check)(void);
} hz_sensor_driver_t;

/* ============================================================
 * 传感器通道配置
 * ============================================================ */

typedef struct {
    hz_sensor_driver_t *driver;          /* 传感器驱动 */
    hz_u32              interval_ms;     /* 采集周期 */
    hz_filter_type_t    filter_type;     /* 滤波类型 */
    hz_u8               filter_window;   /* 滤波窗口大小 */
    float               threshold_upper; /* 上限阈值 */
    float               threshold_lower; /* 下限阈值 */
} hz_sensor_channel_cfg_t;

/* ============================================================
 * 传感器通道ID（注册时返回）
 * ============================================================ */

typedef hz_u8 hz_sensor_channel_id_t;

/* ============================================================
 * 阈值事件数据结构（EV_SENSOR_THRESHOLD 的 data）
 * ============================================================ */

typedef struct {
    hz_sensor_channel_id_t channel_id;
    hz_sensor_type_t       sensor_type;
    float                  current_value;
    float                  threshold_value;
    hz_u8                  direction;  /* 0=超上限 1=超下限 */
} hz_sensor_threshold_event_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_sensor_init(void);
hz_err_t hz_sensor_channel_register(const hz_sensor_channel_cfg_t *cfg,
                                    hz_sensor_channel_id_t *out_id);
hz_err_t hz_sensor_channel_read(hz_sensor_channel_id_t id, float *value);
hz_err_t hz_sensor_read_raw(hz_sensor_channel_id_t id, hz_s32 *raw);
void     hz_sensor_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_SENSOR */
#endif /* HZ_SENSOR_H */
