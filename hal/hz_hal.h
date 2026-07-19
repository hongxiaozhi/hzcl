/*
 * HZCL - Horizon Zone Control Library
 * hz_hal.h - 硬件抽象层：基础驱动接口
 *
 * 框架定义接口规范，用户根据此接口实现具体硬件驱动。
 */

#ifndef HZ_HAL_H
#define HZ_HAL_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 基础驱动操作集
 * ============================================================ */

typedef struct hz_driver_ops {
    const char  *name;
    hz_err_t (*init)(void *config);
    hz_err_t (*deinit)(void);
    hz_err_t (*self_test)(void);
} hz_driver_ops_t;

#ifdef __cplusplus
}
#endif

#endif /* HZ_HAL_H */
