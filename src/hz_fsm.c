/*
 * HZCL - Horizon Zone Control Library
 * hz_fsm.c - 层级状态机实现
 */

#include "hz_fsm.h"

#if HZ_ENABLE_FSM

#include "hz_event.h"

/* ============================================================
 * 全局状态
 * ============================================================ */

static const hz_state_node_t *s_states = NULL;
static hz_u32                 s_state_count = 0;
static hz_state_t             s_current = HZ_STATE_NONE;
static hz_state_t             s_previous = HZ_STATE_NONE;
static hz_bool_t              s_inited = false;

/* ============================================================
 * 内部：查找状态节点
 * ============================================================ */

static const hz_state_node_t *find_state(hz_state_t id)
{
    hz_u32 i;
    for (i = 0; i < s_state_count; i++) {
        if (s_states[i].id == id) {
            return &s_states[i];
        }
    }
    return NULL;
}

/* ============================================================
 * 内部：从状态及其父状态中查找动作
 * ============================================================ */

static const hz_action_t *find_action(hz_state_t state_id, hz_action_id_t action_id)
{
    const hz_state_node_t *node = find_state(state_id);
    const hz_action_t     *act;

    while (node != NULL) {
        act = node->allowed_actions;
        if (act != NULL) {
            while (act->handler != NULL || act->name != NULL) {
                if (act->id == action_id) {
                    return act;
                }
                act++;
            }
        }
        /* 向父状态查找（状态继承） */
        if (node->parent != HZ_STATE_NONE) {
            node = find_state(node->parent);
        } else {
            break;
        }
    }
    return NULL;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_fsm_init(const hz_state_node_t *states, hz_u32 count)
{
    if (!states || count == 0) {
        return HZ_ERR_INVALID;
    }

    s_states       = states;
    s_state_count  = count;
    s_current      = HZ_STATE_NONE;
    s_previous     = HZ_STATE_NONE;
    s_inited       = true;

    return HZ_OK;
}

/* ============================================================
 * 状态切换
 * ============================================================ */

hz_err_t hz_fsm_transition(hz_state_t target)
{
    const hz_state_node_t *target_node;

    if (!s_inited) return HZ_ERR_INVALID;

    target_node = find_state(target);
    if (!target_node) {
        return HZ_ERR_NOT_FOUND;
    }

    /* 退出当前状态 */
    if (s_current != HZ_STATE_NONE) {
        const hz_state_node_t *cur = find_state(s_current);
        if (cur && cur->on_exit) {
            cur->on_exit(target);
        }
    }

    /* 记录切换 */
    s_previous = s_current;
    s_current  = target;

    /* 进入新状态 */
    if (target_node->on_enter) {
        target_node->on_enter(s_previous);
    }

    /* 发布事件 */
    hz_event_publish(EV_FSM_STATE_CHANGED, (void *)(hz_u32)s_current);

    return HZ_OK;
}

/* ============================================================
 * 执行动作
 * ============================================================ */

hz_err_t hz_fsm_execute_action(hz_action_id_t action_id, void *arg)
{
    const hz_action_t *act;

    if (!s_inited) return HZ_ERR_INVALID;
    if (s_current == HZ_STATE_NONE) return HZ_ERR_INVALID;

    act = find_action(s_current, action_id);
    if (!act) {
        return HZ_ERR_NOT_FOUND;  /* 当前状态下不允许此动作 */
    }

    if (!act->handler) {
        return HZ_ERR_INVALID;
    }

    /* 执行动作 */
    hz_err_t err = act->handler(arg);

    if (err == HZ_OK) {
        hz_event_publish(EV_FSM_ACTION_EXECUTED, (void *)(hz_u32)action_id);

        /* 如果动作定义了目标状态，自动切换 */
        if (act->target_state != HZ_STATE_NONE) {
            hz_fsm_transition(act->target_state);
        }
    }

    return err;
}

/* ============================================================
 * 查询当前状态是否允许某动作
 * ============================================================ */

hz_bool_t hz_fsm_is_action_allowed(hz_action_id_t action_id)
{
    if (!s_inited || s_current == HZ_STATE_NONE) return false;
    return (find_action(s_current, action_id) != NULL);
}

/* ============================================================
 * 获取当前状态
 * ============================================================ */

hz_state_t hz_fsm_current(void)
{
    return s_current;
}

const char *hz_fsm_current_name(void)
{
    const hz_state_node_t *node;
    if (!s_inited || s_current == HZ_STATE_NONE) return "NONE";
    node = find_state(s_current);
    return node ? node->name : "UNKNOWN";
}

/* ============================================================
 * 周期性处理
 * ============================================================ */

void hz_fsm_tick(hz_u32 ms)
{
    const hz_state_node_t *node;

    if (!s_inited || s_current == HZ_STATE_NONE) return;

    node = find_state(s_current);
    if (node && node->on_tick) {
        node->on_tick(ms);
    }
}

#endif /* HZ_ENABLE_FSM */
