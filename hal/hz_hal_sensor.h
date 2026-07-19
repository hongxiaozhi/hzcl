/*
 * HZCL - Horizon Zone Control Library
 * hz_hal_sensor.h - 传感器 HAL 接口
 *
 * 具体传感器驱动需实现此接口中的函数指针。
 */

#ifndef HZ_HAL_SENSOR_H
#define HZ_HAL_SENSOR_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 传感器驱动接口
 *
 * 用户实现这个结构体，然后注册到 hz_sensor 模块。
 * ============================================================ */

typedef struct hz_sensor_hal {
    const char  *name;          /* 传感器型号，如 "MSP6002" */
    hz_u8        type;          /* 传感器类型（参考 hz_sensor_type_t） */

    hz_err_t (*init)(void *param);          /* 初始化传感器 */
    hz_err_t (*deinit)(void);              /* 去初始化 */
    hz_err_t (*read_raw)(hz_s32 *raw);     /* 读取原始值 */
    float    (*raw_to_physical)(hz_s32 raw); /* 原始值转物理值 */
    hz_err_t (*self_test)(void);           /* 自检 */
} hz_sensor_hal_t;

#ifdef __cplusplus
}
#endif

#endif /* HZ_HAL_SENSOR_H */
