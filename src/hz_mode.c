/*
 * HZCL - Horizon Zone Control Library
 * hz_mode.c - 模式引擎实现
 */

#include "hz_mode.h"

#if HZ_ENABLE_MODE

#include "hz_event.h"

/* ============================================================
 * 模式表
 * ============================================================ */

#define HZ_MODE_MAX 16

static const hz_mode_t *s_modes[HZ_MODE_MAX];
static hz_u8            s_mode_count = 0;
static hz_mode_id_t     s_current = 0;
static hz_bool_t        s_inited = false;

/* 渐变过渡状态 */
static hz_bool_t s_transitioning = false;
static hz_u32    s_transition_elapsed = 0;
static hz_u32    s_transition_duration = 0;

/* ============================================================
 * 内部查找
 * ============================================================ */

static const hz_mode_t *find_mode(hz_mode_id_t id)
{
    hz_u8 i;
    for (i = 0; i < s_mode_count; i++) {
        if (s_modes[i]->id == id) return s_modes[i];
    }
    return NULL;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_mode_init(void)
{
    s_mode_count = 0;
    s_current = 0;
    s_transitioning = false;
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 注册模式
 * ============================================================ */

hz_err_t hz_mode_register(const hz_mode_t *mode)
{
    if (!s_inited || !mode || !mode->name) return HZ_ERR_INVALID;
    if (s_mode_count >= HZ_MODE_MAX) return HZ_ERR_NOMEM;
    if (find_mode(mode->id)) return HZ_ERR_FAIL; /* 重复注册 */

    s_modes[s_mode_count++] = mode;
    return HZ_OK;
}

/* ============================================================
 * 切换模式
 * ============================================================ */

hz_err_t hz_mode_switch(hz_mode_id_t id)
{
    const hz_mode_t *target;

    if (!s_inited) return HZ_ERR_INVALID;

    target = find_mode(id);
    if (!target) return HZ_ERR_NOT_FOUND;

    hz_mode_id_t prev = s_current;

    /* 退出当前模式 */
    const hz_mode_t *cur = find_mode(s_current);
    if (cur && cur->on_exit) {
        cur->on_exit(id);
    }

    /* 设置目标模式 */
    s_current = id;

    /* 进入新模式 */
    if (target->on_enter) {
        target->on_enter(prev);
    }

    /* 启动渐变过渡 */
    if (target->transition_ms > 0) {
        s_transitioning = true;
        s_transition_elapsed = 0;
        s_transition_duration = target->transition_ms;
    }

    hz_event_publish(EV_MODE_CHANGED, (void *)(hz_u32)id);
    return HZ_OK;
}

/* ============================================================
 * 查询
 * ============================================================ */

hz_mode_id_t hz_mode_current(void) { return s_current; }

const char *hz_mode_current_name(void)
{
    const hz_mode_t *m = find_mode(s_current);
    return m ? m->name : "NONE";
}

hz_bool_t hz_mode_is_transitioning(void) { return s_transitioning; }

/* ============================================================
 * 周期性处理
 * ============================================================ */

void hz_mode_tick(hz_u32 ms)
{
    const hz_mode_t *cur;

    if (!s_inited) return;

    /* 推进渐变过渡 */
    if (s_transitioning) {
        s_transition_elapsed += ms;
        if (s_transition_elapsed >= s_transition_duration) {
            s_transitioning = false;
        }
        hz_event_publish(EV_MODE_TRANSITION, NULL);
    }

    /* 调用当前模式的 on_tick */
    cur = find_mode(s_current);
    if (cur && cur->on_tick) {
        cur->on_tick(ms);
    }
}

#endif /* HZ_ENABLE_MODE */
