/*
 * HZCL - Horizon Zone Control Library
 * hz_hal_actuator.h - 执行器 HAL 接口
 *
 * 具体执行器驱动需实现此接口。
 * 如：电机、蜂鸣器、加热器、振动马达等。
 */

#ifndef HZ_HAL_ACTUATOR_H
#define HZ_HAL_ACTUATOR_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 执行器驱动接口
 * ============================================================ */

typedef struct hz_actuator_hal {
    const char *name;

    hz_err_t (*init)(void *param);
    hz_err_t (*deinit)(void);
    hz_err_t (*set_output)(float value);    /* 0.0~1.0 归一化输出 */
    hz_err_t (*get_status)(hz_u8 *status);  /* 获取执行器状态 */
    hz_err_t (*self_test)(void);
} hz_actuator_hal_t;

#ifdef __cplusplus
}
#endif

#endif /* HZ_HAL_ACTUATOR_H */
