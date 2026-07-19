/*
 * HZCL - Horizon Zone Control Library
 * hz_platform_template.c - 平台移植模板
 *
 * 将此文件复制为 hz_platform_<平台>.c，根据实际平台实现。
 */

#include "hz_platform.h"
#include "hz_config.h"

static volatile hz_u32 s_tick_counter = 0;

/* ============================================================
 * 平台初始化
 * ============================================================ */

hz_err_t hz_platform_init(void)
{
    s_tick_counter = 0;
    /* TODO: 初始化系统滴答定时器，配置为 HZ_PLATFORM_TICK_HZ 频率中断 */
    return HZ_OK;
}

/* 滴答定时器中断服务函数中调用此函数 */
void hz_platform_tick_handler(void)
{
    s_tick_counter++;
}

/* ============================================================
 * 获取系统滴答计数值（毫秒）
 * ============================================================ */

hz_u32 hz_platform_get_tick_ms(void)
{
    hz_u32 tick;
    hz_platform_critical_enter();
    tick = s_tick_counter;
    hz_platform_critical_exit();
    return tick;
}

/* ============================================================
 * 临界区保护
 * ============================================================ */

void hz_platform_critical_enter(void)
{
    /* TODO: 关中断 */
    /* 裸机: __disable_irq() */
    /* FreeRTOS: taskENTER_CRITICAL() */
}

void hz_platform_critical_exit(void)
{
    /* TODO: 开中断 */
    /* 裸机: __enable_irq() */
    /* FreeRTOS: taskEXIT_CRITICAL() */
}

/* ============================================================
 * 阻塞延时
 * ============================================================ */

void hz_platform_delay_ms(hz_u32 ms)
{
    hz_u32 start = hz_platform_get_tick_ms();
    while ((hz_platform_get_tick_ms() - start) < ms) {
        /* 忙等待 */
    }
}
