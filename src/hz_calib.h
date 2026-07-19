/*
 * HZCL - Horizon Zone Control Library
 * hz_calib.h - 出厂校准数据管理
 *
 * 解决痛点：校准数据管理混乱、易被误改
 *
 * 与 hz_config_mgr 的区别：
 *   - hz_config_mgr：用户可修改的参数（亮度、音量等）
 *   - hz_calib：出厂标定数据（传感器零偏、系数等），运行时只读
 */

#ifndef HZ_CALIB_H
#define HZ_CALIB_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_CALIB

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 校准项ID
 * ============================================================ */

typedef hz_u8 hz_calib_id_t;

/* ============================================================
 * 校准项定义
 * ============================================================ */

typedef struct {
    hz_calib_id_t  id;
    const char    *name;
    hz_u32         size;           /* 数据大小（字节） */
    void          *default_val;    /* 默认校准值 */
} hz_calib_item_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_calib_init(const hz_calib_item_t *items, hz_u32 count);
hz_err_t hz_calib_read(hz_calib_id_t id, void *value);
hz_err_t hz_calib_write(hz_calib_id_t id, const void *value);
hz_err_t hz_calib_verify(void);
hz_err_t hz_calib_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_CALIB */
#endif /* HZ_CALIB_H */
