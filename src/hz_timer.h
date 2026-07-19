/*
 * HZCL - Horizon Zone Control Library
 * hz_timer.h - 软定时器
 *
 * 基于 hz_platform_get_tick_ms() 实现，不依赖硬件定时器。
 * 支持周期定时和单次定时两种模式。
 */

#ifndef HZ_TIMER_H
#define HZ_TIMER_H

#include "hz_types.h"
#include "hz_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 定时器句柄
 * ============================================================ */

typedef struct hz_timer {
    hz_u32          interval_ms;    /* 定时周期（毫秒） */
    hz_u32          remaining_ms;   /* 剩余时间（毫秒） */
    hz_callback_t   callback;       /* 超时回调函数 */
    void           *arg;            /* 回调参数 */
    hz_u8           repeat : 1;     /* 1=周期 0=单次 */
    hz_u8           active : 1;     /* 1=运行中 0=已停止 */
    struct hz_timer *next;          /* 链表指针 */
} hz_timer_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_timer_init(void);
hz_err_t hz_timer_start(hz_timer_t *timer);
void     hz_timer_stop(hz_timer_t *timer);
void     hz_timer_tick(hz_u32 elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_TIMER_H */
