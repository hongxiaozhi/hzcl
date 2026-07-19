/*
 * HZCL - Horizon Zone Control Library
 * hz_alarm.h - 报警系统
 *
 * 解决痛点：报警逻辑与业务代码耦合
 *
 * 核心思想：
 *   - 事件驱动：传感器超阈值 → 发布事件 → 报警系统自动响应
 *   - 四种报警级别，联动声/光/震动
 *   - 支持自动恢复
 */

#ifndef HZ_ALARM_H
#define HZ_ALARM_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_ALARM

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 报警级别
 * ============================================================ */

typedef enum {
    HZ_ALARM_INFO       = 0,    /* 提示信息 */
    HZ_ALARM_WARNING    = 1,    /* 警告 */
    HZ_ALARM_CRITICAL   = 2,    /* 严重 */
    HZ_ALARM_EMERGENCY  = 3,    /* 紧急 */
} hz_alarm_level_t;

/* ============================================================
 * 报警ID（用户自定义）
 * ============================================================ */

typedef hz_u8 hz_alarm_id_t;

/* ============================================================
 * 报警配置
 * ============================================================ */

typedef struct {
    hz_alarm_id_t     id;
    const char       *name;
    hz_alarm_level_t  level;
    hz_u32            display_timeout_ms;  /* 显示自动消失时间 */

    /* 联动动作 */
    hz_bool_t action_buzzer;    /* 蜂鸣器 */
    hz_bool_t action_vibrator;  /* 震动 */
    hz_u32    action_led_mask;  /* LED 闪烁模式位掩码 */

    /* 自动恢复 */
    hz_bool_t auto_recover;
    hz_u32    recover_delay_ms;
} hz_alarm_cfg_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_alarm_init(const hz_alarm_cfg_t *alarms, hz_u32 count);
hz_err_t hz_alarm_trigger(hz_alarm_id_t id);
hz_err_t hz_alarm_clear(hz_alarm_id_t id);
hz_err_t hz_alarm_clear_all(void);
hz_bool_t hz_alarm_is_active(hz_alarm_id_t id);
void      hz_alarm_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_ALARM */
#endif /* HZ_ALARM_H */
