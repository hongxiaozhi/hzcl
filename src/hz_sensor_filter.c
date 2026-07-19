/*
 * HZCL - Horizon Zone Control Library
 * hz_sensor_filter.c - 传感器数据滤波算法实现
 */

#include "hz_sensor_filter.h"

#if HZ_ENABLE_SENSOR

/* ============================================================
 * 滑动平均滤波
 * ============================================================ */

void hz_moving_avg_filter_init(hz_moving_avg_filter_t *f, hz_u8 window)
{
    hz_u8 i;
    f->window = (window > HZ_SENSOR_FILTER_WINDOW_MAX)
                ? HZ_SENSOR_FILTER_WINDOW_MAX : window;
    f->index  = 0;
    f->count  = 0;
    f->sum    = 0.0f;
    for (i = 0; i < f->window; i++) {
        f->buffer[i] = 0.0f;
    }
}

float hz_moving_avg_filter_process(hz_moving_avg_filter_t *f, float value)
{
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = value;
    f->sum += value;
    f->index = (f->index + 1) % f->window;
    if (f->count < f->window) {
        f->count++;
        return value;
    }
    return f->sum / (float)f->window;
}

/* ============================================================
 * 一阶低通滤波
 * ============================================================ */

void hz_lowpass_filter_init(hz_lowpass_filter_t *f, float alpha)
{
    f->alpha = alpha;
    f->output = 0.0f;
    f->initialized = 0;
}

float hz_lowpass_filter_process(hz_lowpass_filter_t *f, float input)
{
    if (!f->initialized) {
        f->output = input;
        f->initialized = 1;
        return input;
    }
    f->output = f->output + f->alpha * (input - f->output);
    return f->output;
}

#endif /* HZ_ENABLE_SENSOR */
