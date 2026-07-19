/*
 * HZCL - Horizon Zone Control Library
 * hz_power.c - 电源管理实现
 */

#include "hz_power.h"

#if HZ_ENABLE_POWER

#include "hz_event.h"
#include "hz_platform.h"

/* ============================================================
 * 全局状态
 * ============================================================ */

static hz_power_config_t  s_config;
static hz_power_state_t   s_state = HZ_POWER_ACTIVE;
static hz_u32             s_idle_elapsed = 0;
static hz_bool_t          s_inited = false;

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_power_init(const hz_power_config_t *cfg)
{
    if (!cfg) return HZ_ERR_INVALID;

    s_config = *cfg;
    s_state = HZ_POWER_ACTIVE;
    s_idle_elapsed = 0;
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 手动设置电源状态
 * ============================================================ */

hz_err_t hz_power_set_state(hz_power_state_t state)
{
    if (!s_inited) return HZ_ERR_INVALID;
    if (state == s_state) return HZ_OK;

    hz_power_state_t prev = s_state;
    s_state = state;
    s_idle_elapsed = 0;

    if (state == HZ_POWER_SLEEP || state == HZ_POWER_DEEP_SLEEP) {
        hz_event_publish(EV_POWER_SLEEP, (void *)(hz_u32)state);
    } else if (prev == HZ_POWER_SLEEP || prev == HZ_POWER_DEEP_SLEEP) {
        hz_event_publish(EV_POWER_WAKEUP, NULL);
    }

    return HZ_OK;
}

hz_power_state_t hz_power_get_state(void) { return s_state; }

/* ============================================================
 * 重置空闲计时器（用户操作时调用）
 * ============================================================ */

void hz_power_reset_idle_timer(void)
{
    s_idle_elapsed = 0;
    if (s_state != HZ_POWER_ACTIVE) {
        hz_power_set_state(HZ_POWER_ACTIVE);
    }
}

/* ============================================================
 * 获取电量（需用户实现底层）
 * ============================================================ */

hz_u8 hz_power_get_battery_level(void)
{
    /* TODO: 读取 ADC 电量检测 */
    return 100;
}

/* ============================================================
 * 周期性处理（超时检测）
 * ============================================================ */

void hz_power_tick(hz_u32 ms)
{
    if (!s_inited) return;

    s_idle_elapsed += ms;

    switch (s_state) {
    case HZ_POWER_ACTIVE:
        if (s_config.idle_timeout_ms > 0 &&
            s_idle_elapsed >= s_config.idle_timeout_ms) {
            hz_power_set_state(HZ_POWER_IDLE);
        }
        break;
    case HZ_POWER_IDLE:
        if (s_config.sleep_timeout_ms > 0 &&
            s_idle_elapsed >= s_config.idle_timeout_ms + s_config.sleep_timeout_ms) {
            hz_power_set_state(HZ_POWER_SLEEP);
        }
        break;
    default:
        break;
    }
}

#endif /* HZ_ENABLE_POWER */
