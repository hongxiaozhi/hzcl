/*
 * HZCL - Horizon Zone Control Library
 * hz_sensor_filter.h - 传感器数据滤波算法
 */

#ifndef HZ_SENSOR_FILTER_H
#define HZ_SENSOR_FILTER_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_SENSOR

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 滑动平均滤波器
 * ============================================================ */

typedef struct {
    float  buffer[HZ_SENSOR_FILTER_WINDOW_MAX];
    hz_u8  window;
    hz_u8  index;
    hz_u8  count;
    float  sum;
} hz_moving_avg_filter_t;

void hz_moving_avg_filter_init(hz_moving_avg_filter_t *f, hz_u8 window);
float hz_moving_avg_filter_process(hz_moving_avg_filter_t *f, float value);

/* ============================================================
 * 一阶低通滤波器
 * ============================================================ */

typedef struct {
    float alpha;       /* 滤波系数 (0.0~1.0)，越小越平滑 */
    float output;      /* 上次输出值 */
    hz_u8  initialized;
} hz_lowpass_filter_t;

void hz_lowpass_filter_init(hz_lowpass_filter_t *f, float alpha);
float hz_lowpass_filter_process(hz_lowpass_filter_t *f, float input);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_SENSOR */
#endif /* HZ_SENSOR_FILTER_H */
