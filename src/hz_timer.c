/*
 * HZCL - Horizon Zone Control Library
 * hz_timer.c - 软定时器实现
 */

#include "hz_timer.h"
#include "hz_platform.h"

/* ============================================================
 * 全局状态（静态链表管理所有定时器）
 * ============================================================ */

static hz_timer_t *s_timer_list = NULL;
static hz_bool_t   s_inited     = false;

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_timer_init(void)
{
    s_timer_list = NULL;
    s_inited     = true;
    return HZ_OK;
}

/* ============================================================
 * 启动定时器
 * ============================================================ */

hz_err_t hz_timer_start(hz_timer_t *timer)
{
    if (!s_inited || !timer || !timer->callback) {
        return HZ_ERR_INVALID;
    }

    if (timer->interval_ms == 0) {
        return HZ_ERR_INVALID;
    }

    hz_platform_critical_enter();

    timer->remaining_ms = timer->interval_ms;
    timer->active       = 1;

    /* 如果不在链表中，加入链表 */
    if (timer->next == NULL && s_timer_list != timer) {
        timer->next = s_timer_list;
        s_timer_list = timer;
    }

    hz_platform_critical_exit();
    return HZ_OK;
}

/* ============================================================
 * 停止定时器
 * ============================================================ */

void hz_timer_stop(hz_timer_t *timer)
{
    if (!s_inited || !timer) return;

    hz_platform_critical_enter();
    timer->active = 0;
    hz_platform_critical_exit();
}

/* ============================================================
 * 定时器心跳推进（由主循环或中断调用）
 *
 * 遍历所有活跃定时器，递减剩余时间。
 * 超时后调用回调，周期定时器自动重置，单次定时器自动停止。
 * ============================================================ */

void hz_timer_tick(hz_u32 elapsed_ms)
{
    hz_timer_t *t;

    if (!s_inited) return;

    hz_platform_critical_enter();

    for (t = s_timer_list; t != NULL; t = t->next) {
        if (!t->active) continue;

        if (t->remaining_ms > elapsed_ms) {
            t->remaining_ms -= elapsed_ms;
        } else {
            t->remaining_ms = 0;
            t->active = 0;

            /* 在回调前短暂退出临界区，避免死锁 */
            hz_platform_critical_exit();
            t->callback(t->arg);
            hz_platform_critical_enter();

            if (t->repeat) {
                /* 周期定时器：重新开始 */
                t->remaining_ms = t->interval_ms;
                t->active = 1;
            }
        }
    }

    hz_platform_critical_exit();
}
