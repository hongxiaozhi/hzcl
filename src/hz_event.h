/*
 * HZCL - Horizon Zone Control Library
 * hz_event.h - 事件总线（发布/订阅）
 */

#ifndef HZ_EVENT_H
#define HZ_EVENT_H

#include "hz_types.h"
#include "hz_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 事件ID定义
 * ============================================================ */

typedef hz_u16 hz_event_id_t;

/* 系统内置事件 */
enum {
    /* 系统级 */
    EV_SYS_INIT_COMPLETE    = 0x0001,   /* 系统初始化完成 */
    EV_SYS_ERROR            = 0x0002,   /* 系统错误 */
    EV_SYS_TICK_1S          = 0x0003,   /* 每秒心跳 */

    /* FSM 状态机 */
    EV_FSM_STATE_CHANGED    = 0x0010,   /* 状态切换完成 */
    EV_FSM_ACTION_EXECUTED  = 0x0011,   /* 动作执行完成 */

    /* Screen 屏幕导航 */
    EV_SCREEN_PUSHED        = 0x0020,   /* 页面入栈 */
    EV_SCREEN_POPPED        = 0x0021,   /* 页面出栈 */
    EV_SCREEN_REFRESH       = 0x0022,   /* 请求刷新 */

    /* Sensor 传感器 */
    EV_SENSOR_DATA_READY    = 0x0030,   /* 某传感器数据就绪 */
    EV_SENSOR_THRESHOLD     = 0x0031,   /* 阈值触发（上限/下限） */
    EV_SENSOR_ERROR         = 0x0032,   /* 传感器错误 */

    /* Mode 模式 */
    EV_MODE_CHANGED         = 0x0040,   /* 模式切换完成 */
    EV_MODE_TRANSITION      = 0x0041,   /* 模式正在过渡 */

    /* Alarm 报警 */
    EV_ALARM_TRIGGERED      = 0x0050,   /* 报警触发 */
    EV_ALARM_CLEARED        = 0x0051,   /* 报警解除 */

    /* Power 电源 */
    EV_POWER_SLEEP          = 0x0060,   /* 进入休眠 */
    EV_POWER_WAKEUP         = 0x0061,   /* 唤醒 */
    EV_POWER_BATTERY_LOW    = 0x0062,   /* 电量低 */
    EV_POWER_BATTERY_OK     = 0x0063,   /* 电量恢复正常 */

    /* Config 参数 */
    EV_CONFIG_CHANGED       = 0x0070,   /* 参数值变更 */
    EV_CONFIG_RESET         = 0x0071,   /* 参数恢复出厂 */

    /* 用户自定义事件起始 */
    EV_USER_BASE            = 0x0100,
};

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_event_init(void);
hz_err_t hz_event_subscribe(hz_event_id_t id, void (*cb)(void *data, void *user), void *user);
hz_err_t hz_event_publish(hz_event_id_t id, void *data);
void     hz_event_unsubscribe(hz_event_id_t id, void (*cb)(void *data, void *user));

#ifdef __cplusplus
}
#endif

#endif /* HZ_EVENT_H */
