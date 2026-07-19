/*
 * HZCL - Horizon Zone Control Library
 * hz_power.h - 电源管理
 *
 * 解决痛点：休眠/唤醒逻辑散落、超时时间各处维护
 *
 * 核心思想：
 *   - 统一电源状态机：ACTIVE → IDLE → SLEEP → DEEP_SLEEP
 *   - 用户操作时调用 hz_power_reset_idle_timer() 重置空闲计时
 *   - 超时自动进入下一级休眠
 */

#ifndef HZ_POWER_H
#define HZ_POWER_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_POWER

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 电源状态
 * ============================================================ */

typedef enum {
    HZ_POWER_ACTIVE,         /* 正常工作 */
    HZ_POWER_IDLE,           /* 空闲（屏幕调暗） */
    HZ_POWER_SLEEP,          /* 休眠（屏幕关闭，按键唤醒） */
    HZ_POWER_DEEP_SLEEP,     /* 深度睡眠（最低功耗） */
    HZ_POWER_OFF,            /* 关机 */
} hz_power_state_t;

/* ============================================================
 * 电源配置
 * ============================================================ */

typedef struct {
    hz_u32 idle_timeout_ms;         /* 无操作进入空闲（毫秒） */
    hz_u32 sleep_timeout_ms;        /* 无操作进入休眠 */
    hz_u32 deep_sleep_timeout_ms;   /* 无操作进入深度休眠 */
} hz_power_config_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_power_init(const hz_power_config_t *cfg);
hz_err_t hz_power_set_state(hz_power_state_t state);
hz_power_state_t hz_power_get_state(void);
void    hz_power_reset_idle_timer(void);
hz_u8   hz_power_get_battery_level(void);
void    hz_power_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_POWER */
#endif /* HZ_POWER_H */
