/*
 * HZCL - Horizon Zone Control Library
 * hz_platform.h - 平台抽象接口
 *
 * 每个平台需实现此接口：
 *   - 裸机：使用 SysTick + 关中断
 *   - FreeRTOS：使用 vTaskGetTickCount + taskENTER_CRITICAL
 *   - RT-Thread：使用 rt_tick_get + rt_enter_critical
 */

#ifndef HZ_PLATFORM_H
#define HZ_PLATFORM_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 平台初始化
 * ============================================================ */

hz_err_t hz_platform_init(void);

/* ============================================================
 * 时间服务
 * ============================================================ */

hz_u32 hz_platform_get_tick_ms(void);

/* ============================================================
 * 临界区保护
 * ============================================================ */

void hz_platform_critical_enter(void);
void hz_platform_critical_exit(void);

/* ============================================================
 * 阻塞延时（毫秒）
 * ============================================================ */

void hz_platform_delay_ms(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_PLATFORM_H */
