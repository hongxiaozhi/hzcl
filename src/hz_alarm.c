/*
 * HZCL - Horizon Zone Control Library
 * hz_alarm.c - 报警系统实现
 */

#include "hz_alarm.h"

#if HZ_ENABLE_ALARM

#include "hz_event.h"
#include <string.h>

/* ============================================================
 * 全局状态
 * ============================================================ */

typedef struct {
    hz_bool_t               active;     /* 是否正在报警 */
    hz_u32                  elapsed_ms; /* 已持续的时间 */
} hz_alarm_state_t;

static const hz_alarm_cfg_t *s_alarms = NULL;
static hz_u32                s_alarm_count = 0;
static hz_alarm_state_t      s_states[HZ_ALARM_MAX_COUNT];
static hz_bool_t             s_inited = false;

/* ============================================================
 * 内部查找
 * ============================================================ */

static const hz_alarm_cfg_t *find_cfg(hz_alarm_id_t id)
{
    hz_u32 i;
    for (i = 0; i < s_alarm_count; i++) {
        if (s_alarms[i].id == id) return &s_alarms[i];
    }
    return NULL;
}

static hz_alarm_state_t *find_state(hz_alarm_id_t id)
{
    hz_u32 i;
    for (i = 0; i < s_alarm_count; i++) {
        if (s_alarms[i].id == id) return &s_states[i];
    }
    return NULL;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_alarm_init(const hz_alarm_cfg_t *alarms, hz_u32 count)
{
    if (!alarms || count == 0 || count > HZ_ALARM_MAX_COUNT) {
        return HZ_ERR_INVALID;
    }

    s_alarms = alarms;
    s_alarm_count = count;
    memset(s_states, 0, sizeof(s_states));
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 触发报警
 * ============================================================ */

hz_err_t hz_alarm_trigger(hz_alarm_id_t id)
{
    const hz_alarm_cfg_t *cfg = find_cfg(id);
    hz_alarm_state_t     *st  = find_state(id);

    if (!s_inited || !cfg || !st) return HZ_ERR_NOT_FOUND;

    st->active = true;
    st->elapsed_ms = 0;

    hz_event_publish(EV_ALARM_TRIGGERED, (void *)(hz_u32)id);
    return HZ_OK;
}

/* ============================================================
 * 解除报警
 * ============================================================ */

hz_err_t hz_alarm_clear(hz_alarm_id_t id)
{
    hz_alarm_state_t *st = find_state(id);
    if (!s_inited || !st) return HZ_ERR_NOT_FOUND;

    st->active = false;
    hz_event_publish(EV_ALARM_CLEARED, (void *)(hz_u32)id);
    return HZ_OK;
}

hz_err_t hz_alarm_clear_all(void)
{
    hz_u32 i;
    for (i = 0; i < s_alarm_count; i++) {
        if (s_states[i].active) {
            s_states[i].active = false;
            hz_event_publish(EV_ALARM_CLEARED, (void *)(hz_u32)s_alarms[i].id);
        }
    }
    return HZ_OK;
}

hz_bool_t hz_alarm_is_active(hz_alarm_id_t id)
{
    hz_alarm_state_t *st = find_state(id);
    return st ? st->active : false;
}

/* ============================================================
 * 周期性处理（自动恢复检测）
 * ============================================================ */

void hz_alarm_tick(hz_u32 ms)
{
    hz_u32 i;

    if (!s_inited) return;

    for (i = 0; i < s_alarm_count; i++) {
        if (!s_states[i].active) continue;
        if (!s_alarms[i].auto_recover) continue;

        s_states[i].elapsed_ms += ms;
        if (s_states[i].elapsed_ms >= s_alarms[i].recover_delay_ms) {
            hz_alarm_clear(s_alarms[i].id);
        }
    }
}

#endif /* HZ_ENABLE_ALARM */
