/*
 * HZCL - Horizon Zone Control Library
 * hz_fsm.h - 层级状态机
 *
 * 解决痛点：状态管理混乱、全局变量散落、非法操作难拦截
 *
 * 核心思想：
 * - 状态表集中定义所有状态及每个状态下允许的动作
 * - 动作执行前自动检查是否允许，非法操作被拦截
 * - parent 字段支持状态继承（子状态继承父状态的允许动作）
 */

#ifndef HZ_FSM_H
#define HZ_FSM_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_FSM

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 状态ID（用户自定义，从 0 开始）
 * ============================================================ */

typedef hz_u8 hz_state_t;

/* ============================================================
 * 动作ID（用户自定义）
 * ============================================================ */

typedef hz_u16 hz_action_id_t;

/* ============================================================
 * 动作描述符
 * ============================================================ */

typedef struct hz_action {
    hz_action_id_t  id;             /* 动作ID */
    const char     *name;           /* 动作名称（调试用） */
    hz_err_t      (*handler)(void *arg);  /* 动作执行函数 */
    hz_state_t      target_state;   /* 执行后自动切换到的状态（HZ_STATE_NONE=不切换） */
} hz_action_t;

/* 空状态/无状态切换标记 */
#define HZ_STATE_NONE    ((hz_state_t)0xFF)

/* ============================================================
 * 状态节点
 * ============================================================ */

typedef struct hz_state_node {
    hz_state_t          id;             /* 状态ID */
    hz_state_t          parent;         /* 父状态ID（0xFF=顶层） */
    const char         *name;           /* 状态名称（调试用） */

    /* 生命周期回调 */
    void (*on_enter)(hz_state_t from);  /* 进入此状态 */
    void (*on_exit)(hz_state_t to);     /* 离开此状态 */
    void (*on_tick)(hz_u32 ms);        /* 此状态下的周期性处理 */

    /* 此状态下允许执行的动作列表（NULL结尾） */
    const hz_action_t  *allowed_actions;
} hz_state_node_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t  hz_fsm_init(const hz_state_node_t *states, hz_u32 count);
hz_err_t  hz_fsm_transition(hz_state_t target);
hz_err_t  hz_fsm_execute_action(hz_action_id_t action_id, void *arg);
hz_bool_t hz_fsm_is_action_allowed(hz_action_id_t action_id);
hz_state_t hz_fsm_current(void);
const char *hz_fsm_current_name(void);
void      hz_fsm_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_FSM */
#endif /* HZ_FSM_H */
