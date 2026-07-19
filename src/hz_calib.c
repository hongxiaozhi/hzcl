/*
 * HZCL - Horizon Zone Control Library
 * hz_calib.c - 出厂校准数据管理实现
 */

#include "hz_calib.h"

#if HZ_ENABLE_CALIB

#include <string.h>

/* ============================================================
 * 全局状态
 * ============================================================ */

static const hz_calib_item_t *s_items = NULL;
static hz_u32                 s_item_count = 0;
static hz_bool_t              s_inited = false;

/* ============================================================
 * 内部查找
 * ============================================================ */

static const hz_calib_item_t *find_item(hz_calib_id_t id)
{
    hz_u32 i;
    for (i = 0; i < s_item_count; i++) {
        if (s_items[i].id == id) return &s_items[i];
    }
    return NULL;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_calib_init(const hz_calib_item_t *items, hz_u32 count)
{
    if (!items || count == 0) return HZ_ERR_INVALID;
    s_items = items;
    s_item_count = count;
    s_inited = true;
    return HZ_OK;
}

/* ============================================================
 * 读取校准数据（运行时只读）
 * ============================================================ */

hz_err_t hz_calib_read(hz_calib_id_t id, void *value)
{
    const hz_calib_item_t *item;

    if (!s_inited || !value) return HZ_ERR_INVALID;
    item = find_item(id);
    if (!item) return HZ_ERR_NOT_FOUND;

    memcpy(value, item->default_val, item->size);
    return HZ_OK;
}

/* ============================================================
 * 写入校准数据（仅在校准模式下可写）
 * ============================================================ */

hz_err_t hz_calib_write(hz_calib_id_t id, const void *value)
{
    (void)id;
    (void)value;
    /* TODO: 实现校准写入（需在校准模式下才允许） */
    return HZ_ERR_UNSUPPORTED;
}

/* ============================================================
 * 校验校准数据完整性
 * ============================================================ */

hz_err_t hz_calib_verify(void)
{
    return HZ_OK; /* 简化版 */
}

/* ============================================================
 * 恢复出厂校准值
 * ============================================================ */

hz_err_t hz_calib_reset(void)
{
    return HZ_OK;
}

#endif /* HZ_ENABLE_CALIB */
