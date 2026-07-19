/*
 * HZCL - Horizon Zone Control Library
 * hz_screen.h - 屏幕导航管理器
 *
 * 解决痛点：页面跳转混乱、传参靠全局变量、返回结果无标准机制
 *
 * 核心思想：参考 Android Activity 栈管理
 *   - Push: 新页面入栈（记住上层，可返回）
 *   - Replace: 替换当前页面（不保留历史）
 *   - Pop: 返回上层
 *   - 传参: on_load(ctx) 接收参数，screen_set_result() 返回结果
 */

#ifndef HZ_SCREEN_H
#define HZ_SCREEN_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_SCREEN

#include "hz_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 输入事件类型
 * ============================================================ */

typedef struct hz_input_event {
    hz_u8  type;        /* 0=按键 1=触摸 2=编码器 */
    hz_u8  code;        /* 按键码/触摸手势 */
    hz_u16 value;       /* 附加数值 */
} hz_input_event_t;

/* ============================================================
 * 屏幕描述符
 *
 * 每个 UI 页面实现此结构体中的回调。
 * 不需要全部实现，用 NULL 跳过即可。
 * ============================================================ */

typedef struct hz_screen {
    const char *name;   /* 屏幕名称（必须唯一，用于调试和 pop_to） */

    /* 生命周期回调 */
    void (*on_create)(void);                    /* 创建（首次） */
    void (*on_load)(void *ctx);                 /* 加载（每次入栈，带参数） */
    void (*on_enter)(void);                     /* 成为当前屏幕 */
    void (*on_exit)(void);                      /* 离开当前屏幕 */
    void (*on_unload)(void);                    /* 卸载（从栈中移除） */
    void (*on_destroy)(void);                   /* 销毁 */

    /* 运行回调 */
    void (*on_draw)(void);                      /* 绘制 */
    void (*on_tick)(hz_u32 ms);                 /* 周期刷新 */
    void (*on_event)(hz_event_id_t evt, void *data); /* 接收系统事件 */

    /* 输入处理（返回 true 表示已消费） */
    hz_bool_t (*on_key)(hz_u8 key_code);

    void *priv;         /* 屏幕私有数据 */
} hz_screen_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t  hz_screen_init(void);
hz_err_t  hz_screen_push(hz_screen_t *screen, void *ctx);
hz_err_t  hz_screen_replace(hz_screen_t *screen, void *ctx);
hz_err_t  hz_screen_pop(void);
hz_err_t  hz_screen_pop_to(const char *name);
hz_err_t  hz_screen_pop_all(void);
hz_screen_t *hz_screen_current(void);
hz_u8     hz_screen_stack_depth(void);

/* 页面间传参 */
void      hz_screen_set_result(void *result);
void     *hz_screen_get_result(void);

/* 输入分发（由底层驱动调用） */
hz_bool_t hz_screen_handle_input(hz_input_event_t *evt);

/* 刷新 */
void      hz_screen_tick(hz_u32 ms);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_SCREEN */
#endif /* HZ_SCREEN_H */
