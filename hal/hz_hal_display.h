/*
 * HZCL - Horizon Zone Control Library
 * hz_hal_display.h - 显示 HAL 接口
 *
 * 具体显示驱动需实现此接口。
 * 支持两种模式：
 *   - 帧缓冲模式：由 flush 将帧缓冲区内容刷新到屏幕
 *   - 命令模式：提供 draw_pixel / draw_rect 等基本绘图操作
 */

#ifndef HZ_HAL_DISPLAY_H
#define HZ_HAL_DISPLAY_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 显示配置
 * ============================================================ */

typedef struct {
    hz_u16 width;           /* 屏幕宽度（像素） */
    hz_u16 height;          /* 屏幕高度（像素） */
    hz_u8  color_depth;     /* 色彩深度（位） */
    hz_u8  orientation;     /* 显示方向 0=正常 1=90° 2=180° 3=270° */
} hz_display_config_t;

/* ============================================================
 * 显示驱动接口
 * ============================================================ */

typedef struct hz_display_hal {
    const char *name;

    hz_err_t (*init)(const hz_display_config_t *config);
    hz_err_t (*deinit)(void);

    /* 帧缓冲刷新：将指定区域的像素数据刷新到屏幕 */
    hz_err_t (*flush)(hz_u16 x, hz_u16 y, hz_u16 w, hz_u16 h,
                      const hz_u8 *pixels);

    /* 亮度控制 */
    hz_err_t (*set_brightness)(hz_u8 level);

    /* 休眠/唤醒 */
    hz_err_t (*sleep)(void);
    hz_err_t (*wakeup)(void);
} hz_display_hal_t;

#ifdef __cplusplus
}
#endif

#endif /* HZ_HAL_DISPLAY_H */
