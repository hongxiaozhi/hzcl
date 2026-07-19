/*
 * HZCL - Horizon Zone Control Library
 * hz_mode.h - 模式引擎
 *
 * 解决痛点：工作模式切换不平滑、多执行器协同复杂
 *
 * 核心思想：
 *   - 每种模式定义参数表（各执行器目标值）
 *   - 切换时自动渐变过渡（线性插值）
 *   - 发布事件通知其他模块
 */

#ifndef HZ_MODE_H
#define HZ_MODE_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_MODE

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 模式ID（用户自定义）
 * ============================================================ */

typedef hz_u8 hz_mode_id_t;

/* ============================================================
 * 执行器参数项
 * ============================================================ */

typedef struct {
    hz_u8   actuator_id;    /* 执行器ID */
    float   target;         /* 目标值（0.0~1.0 或 物理值） */
} hz_mode_param_t;

/* ============================================================
 * 模式描述符
 * ============================================================ */

typedef struct hz_mode {
    hz_mode_id_t        id;
    const char         *name;
    hz_u32              transition_ms;      /* 切换过渡时间（毫秒） */

    void (*on_enter)(hz_mode_id_t prev);    /* 进入模式 */
    void (*on_exit)(hz_mode_id_t next);     /* 退出模式 */
    void (*on_tick)(hz_u32 ms);             /* 模式运行控制循环 */

    const hz_mode_param_t *params;          /* 参数表 */
    hz_u8                   param_count;
} hz_mode_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t  hz_mode_init(void);
hz_err_t  hz_mode_register(const hz_mode_t *mode);
hz_err_t  hz_mode_switch(hz_mode_id_t id);
hz_mode_id_t hz_mode_current(void);
const char *hz_mode_current_name(void);
hz_bool_t hz_mode_is_transitioning(void);
void      hz_mode_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_MODE */
#endif /* HZ_MODE_H */
