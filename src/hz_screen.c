/*
 * HZCL - Horizon Zone Control Library
 * hz_screen.c - 屏幕导航管理器实现
 */

#include "hz_screen.h"

#if HZ_ENABLE_SCREEN

#include <string.h>

/* ============================================================
 * 屏幕栈
 * ============================================================ */

typedef struct {
    hz_screen_t *screens[HZ_SCREEN_STACK_DEPTH];  /* 栈数组 */
    hz_u8        depth;                            /* 当前深度 */
    hz_screen_t *pending;                          /* 等待切换的屏幕 */
    hz_u8        pending_op;                       /* 0=无 1=push 2=replace 3=pop */
    void        *pending_ctx;                      /* 等待传入的参数 */
    void        *result;                           /* 返回结果 */
} hz_screen_stack_t;

static hz_screen_stack_t s_stack;
static hz_bool_t         s_inited = false;

/* ============================================================
 * 内部：实际执行屏幕切换
 * ============================================================ */

static void execute_switch(void)
{
    hz_screen_t *current;
    hz_screen_t *target;
    hz_u8        op = s_stack.pending_op;

    if (op == 0) return;

    current = (s_stack.depth > 0) ? s_stack.screens[s_stack.depth - 1] : NULL;

    switch (op) {
    case 1: /* push */
        if (s_stack.depth >= HZ_SCREEN_STACK_DEPTH) return;
        target = s_stack.pending;
        if (current) {
            if (current->on_exit) current->on_exit();
            current->on_unload = NULL; /* push 时不卸载，保留在栈中 */
        }
        s_stack.screens[s_stack.depth++] = target;
        if (target->on_load) target->on_load(s_stack.pending_ctx);
        if (target->on_enter) target->on_enter();
        hz_event_publish(EV_SCREEN_PUSHED, (void *)target->name);
        break;

    case 2: /* replace */
        if (s_stack.depth == 0) return;
        target = s_stack.pending;
        current = s_stack.screens[s_stack.depth - 1];
        if (current->on_exit) current->on_exit();
        if (current->on_unload) current->on_unload();
        s_stack.screens[s_stack.depth - 1] = target;
        if (target->on_load) target->on_load(s_stack.pending_ctx);
        if (target->on_enter) target->on_enter();
        break;

    case 3: /* pop */
        if (s_stack.depth == 0) return;
        current = s_stack.screens[s_stack.depth - 1];
        if (current->on_exit) current->on_exit();
        if (current->on_unload) current->on_unload();
        s_stack.depth--;
        if (s_stack.depth > 0) {
            target = s_stack.screens[s_stack.depth - 1];
            if (target->on_enter) target->on_enter();
        }
        hz_event_publish(EV_SCREEN_POPPED, NULL);
        break;
    }

    s_stack.pending_op = 0;
    s_stack.pending = NULL;
    s_stack.pending_ctx = NULL;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_screen_init(void)
{
    s_stack.depth = 0;
    s_stack.pending_op = 0;
    s_stack.pending = NULL;
    s_stack.result = NULL;
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 推入屏幕（入栈）
 * ============================================================ */

hz_err_t hz_screen_push(hz_screen_t *screen, void *ctx)
{
    if (!s_inited || !screen || !screen->name) return HZ_ERR_INVALID;

    s_stack.pending    = screen;
    s_stack.pending_op = 1;
    s_stack.pending_ctx = ctx;
    execute_switch();
    return HZ_OK;
}

/* ============================================================
 * 替换当前屏幕
 * ============================================================ */

hz_err_t hz_screen_replace(hz_screen_t *screen, void *ctx)
{
    if (!s_inited || !screen || !screen->name) return HZ_ERR_INVALID;
    if (s_stack.depth == 0) return HZ_ERR_FAIL;

    s_stack.pending    = screen;
    s_stack.pending_op = 2;
    s_stack.pending_ctx = ctx;
    execute_switch();
    return HZ_OK;
}

/* ============================================================
 * 返回上层屏幕
 * ============================================================ */

hz_err_t hz_screen_pop(void)
{
    hz_screen_t *cur;

    if (!s_inited) return HZ_ERR_INVALID;
    if (s_stack.depth <= 1) return HZ_ERR_FAIL; /* 至少保留主页 */

    /* 检查当前屏幕是否禁止返回 */
    cur = s_stack.screens[s_stack.depth - 1];
    if (cur && (cur->cfg_mask & HZ_SCREEN_CFG_NO_BACK)) return HZ_ERR_FAIL;

    s_stack.pending_op = 3;
    execute_switch();
    return HZ_OK;
}

/* ============================================================
 * 返回到指定屏幕
 * ============================================================ */

hz_err_t hz_screen_pop_to(const char *name)
{
    hz_s8 i;

    if (!s_inited || !name) return HZ_ERR_INVALID;

    for (i = (hz_s8)s_stack.depth - 1; i >= 0; i--) {
        /* 只是查找，不实际pop */
    }

    /* 实际执行pop到目标 */
    while (s_stack.depth > 0) {
        hz_screen_t *cur = s_stack.screens[s_stack.depth - 1];
        if (cur->name && strcmp(cur->name, name) == 0) break;
        hz_screen_pop();
    }
    return HZ_OK;
}

/* ============================================================
 * 返回主页（弹出除主页外的所有屏幕）
 * ============================================================ */

hz_err_t hz_screen_pop_all(void)
{
    if (!s_inited) return HZ_ERR_INVALID;
    while (s_stack.depth > 1) {
        hz_screen_pop();
    }
    return HZ_OK;
}

/* ============================================================
 * 获取当前屏幕
 * ============================================================ */

hz_screen_t *hz_screen_current(void)
{
    if (!s_inited || s_stack.depth == 0) return NULL;
    return s_stack.screens[s_stack.depth - 1];
}

hz_u8 hz_screen_stack_depth(void)
{
    return s_stack.depth;
}

/* ============================================================
 * 页面间传参
 * ============================================================ */

void hz_screen_set_result(void *result)
{
    s_stack.result = result;
}

void *hz_screen_get_result(void)
{
    void *r = s_stack.result;
    s_stack.result = NULL;
    return r;
}

/* ============================================================
 * 输入分发
 * ============================================================ */

hz_bool_t hz_screen_handle_input(hz_input_event_t *evt)
{
    hz_screen_t *cur;

    if (!s_inited || !evt) return false;

    cur = hz_screen_current();
    if (!cur) return false;

    if (evt->type == 0 && evt->code == 0) {
        /* 按键事件 */
        if (cur->on_key) {
            return cur->on_key(evt->code);
        }
    }
    return false;
}

/* ============================================================
 * 周期性刷新
 * ============================================================ */

void hz_screen_tick(hz_u32 ms)
{
    hz_screen_t *cur;

    if (!s_inited) return;

    cur = hz_screen_current();
    if (cur && cur->on_tick) {
        cur->on_tick(ms);
    }
}

#endif /* HZ_ENABLE_SCREEN */
