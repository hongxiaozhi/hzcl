/*
 * HZCL - Horizon Zone Control Library
 * hz_init_mgr.h - 初始化管理器
 *
 * 解决痛点：硬件 init/deinit 顺序混乱、依赖关系隐式、失败无策略
 *
 * 核心思想：
 *   - 每个硬件/模块注册一个初始化条目
 *   - 按阶段(stage) + 顺序(order) + 依赖(dependency) 自动编排
 *   - 支持 STOP / SKIP / RETRY 三种失败策略
 *   - 支持 suspend/resume 用于休眠唤醒
 */

#ifndef HZ_INIT_MGR_H
#define HZ_INIT_MGR_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_INIT_MGR

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 初始化阶段
 * ============================================================ */

typedef enum {
    HZ_INIT_STAGE_0_PLATFORM    = 0,    /* 平台层：时钟、GPIO */
    HZ_INIT_STAGE_1_BUS         = 1,    /* 总线层：I2C、SPI、UART */
    HZ_INIT_STAGE_2_DRIVERS     = 2,    /* 驱动层：传感器、执行器 */
    HZ_INIT_STAGE_3_SERVICES    = 3,    /* 服务层：Sensor Pipeline、Mode */
    HZ_INIT_STAGE_4_APPLICATION = 4,    /* 应用层：FSM、Screen */
    HZ_INIT_STAGE_5_READY       = 5,    /* 就绪 */
} hz_init_stage_t;

/* ============================================================
 * 失败策略
 * ============================================================ */

typedef enum {
    HZ_INIT_FAIL_STOP,      /* 失败则停止后续所有初始化 */
    HZ_INIT_FAIL_SKIP,      /* 失败则跳过当前项，继续后续 */
    HZ_INIT_FAIL_RETRY,     /* 失败后重试3次 */
} hz_init_fail_policy_t;

/* ============================================================
 * 电源域（用于休眠恢复）
 * ============================================================ */

typedef enum {
    HZ_POWER_DOMAIN_ALWAYS_ON,      /* 永不掉电 */
    HZ_POWER_DOMAIN_SLEEP_LOSS,     /* 休眠后会掉电 */
    HZ_POWER_DOMAIN_DEEP_SLEEP,     /* 深度睡眠后掉电 */
} hz_power_domain_t;

/* ============================================================
 * 初始化条目
 * ============================================================ */

typedef struct hz_init_entry {
    const char            *name;
    hz_init_stage_t        stage;
    hz_u32                 order;      /* 同一阶段内的顺序 */
    hz_err_t             (*init)(void);
    void                 (*deinit)(void);
    hz_err_t             (*suspend)(void);
    hz_err_t             (*resume)(void);
    const char *const     *dependencies;  /* 依赖项名称数组 */
    hz_u32                 dep_count;
    hz_init_fail_policy_t  fail_policy;
    hz_power_domain_t      power_domain;
} hz_init_entry_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_init_register(hz_init_entry_t *entry);
hz_err_t hz_init_all(void);
void     hz_init_deinit_all(void);
hz_err_t hz_init_suspend_all(void);
hz_err_t hz_init_resume_all(void);
hz_bool_t hz_init_is_ok(const char *name);
void     hz_init_print_status(void);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_INIT_MGR */
#endif /* HZ_INIT_MGR_H */
