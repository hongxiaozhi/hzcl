/*
 * HZCL - Horizon Zone Control Library
 * hz_event.c - 事件总线实现
 *
 * 使用静态数组管理订阅者，无动态内存分配。
 * 适用于裸机和RTOS环境。
 */

#include "hz_event.h"

/* ============================================================
 * 订阅者节点
 * ============================================================ */

typedef struct {
    hz_event_id_t          id;         /* 订阅的事件ID */
    void (*cb)(void *data, void *user); /* 回调函数 */
    void                  *user;       /* 用户数据 */
} hz_subscriber_t;

/* ============================================================
 * 全局状态
 * ============================================================ */

static hz_subscriber_t s_sub_pool[HZ_EVENT_MAX_SUBSCRIBERS];
static hz_u8           s_sub_count = 0;
static hz_bool_t       s_inited    = false;

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_event_init(void)
{
    s_sub_count = 0;
    s_inited    = true;
    return HZ_OK;
}

/* ============================================================
 * 订阅事件
 * ============================================================ */

hz_err_t hz_event_subscribe(hz_event_id_t id,
                            void (*cb)(void *data, void *user),
                            void *user)
{
    hz_u8 i;

    if (!s_inited || !cb) {
        return HZ_ERR_INVALID;
    }

    /* 查重 */
    for (i = 0; i < s_sub_count; i++) {
        if (s_sub_pool[i].id == id &&
            s_sub_pool[i].cb == cb &&
            s_sub_pool[i].user == user) {
            return HZ_OK;
        }
    }

    if (s_sub_count >= HZ_EVENT_MAX_SUBSCRIBERS) {
        return HZ_ERR_NOMEM;
    }

    s_sub_pool[s_sub_count].id   = id;
    s_sub_pool[s_sub_count].cb   = cb;
    s_sub_pool[s_sub_count].user = user;
    s_sub_count++;
    return HZ_OK;
}

/* ============================================================
 * 发布事件
 * ============================================================ */

hz_err_t hz_event_publish(hz_event_id_t id, void *data)
{
    hz_u8 i;

    if (!s_inited) {
        return HZ_ERR_INVALID;
    }

    for (i = 0; i < s_sub_count; i++) {
        if (s_sub_pool[i].id == id) {
            s_sub_pool[i].cb(data, s_sub_pool[i].user);
        }
    }
    return HZ_OK;
}

/* ============================================================
 * 取消订阅
 * ============================================================ */

void hz_event_unsubscribe(hz_event_id_t id,
                          void (*cb)(void *data, void *user))
{
    if (!s_inited || !cb) return;

    for (hz_u8 i = 0; i < s_sub_count; i++) {
        if (s_sub_pool[i].id == id &&
            s_sub_pool[i].cb == cb) {
            /* 用最后一个元素覆盖，减少移动 */
            s_sub_count--;
            if (i < s_sub_count) {
                s_sub_pool[i] = s_sub_pool[s_sub_count];
            }
            return;
        }
    }
}
