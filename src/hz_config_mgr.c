/*
 * HZCL - Horizon Zone Control Library
 * hz_config_mgr.c - 参数配置管理实现
 */

#include "hz_config_mgr.h"

#if HZ_ENABLE_CONFIG_MGR

#include <string.h>

/* ============================================================
 * 全局状态
 * ============================================================ */

static const hz_config_item_t *s_items = NULL;
static hz_u32                  s_item_count = 0;
static hz_u8                  *s_values = NULL;     /* 运行时值缓冲区 */
static hz_config_storage_t     s_storage;
static hz_bool_t               s_storage_set = false;
static hz_bool_t               s_inited = false;

/* ============================================================
 * 内部：查找参数项
 * ============================================================ */

static const hz_config_item_t *find_item(hz_config_id_t id)
{
    hz_u32 i;
    for (i = 0; i < s_item_count; i++) {
        if (s_items[i].id == id) return &s_items[i];
    }
    return NULL;
}

/* 获取参数数据类型大小 */
static hz_u32 type_size(hz_config_type_t t)
{
    switch (t) {
    case HZ_CONFIG_TYPE_U8:   return 1;
    case HZ_CONFIG_TYPE_U16:  return 2;
    case HZ_CONFIG_TYPE_U32:  return 4;
    case HZ_CONFIG_TYPE_FLOAT: return 4;
    case HZ_CONFIG_TYPE_BOOL: return 1;
    default: return 0;
    }
}

/* ============================================================
 * 简单 CRC 校验
 * ============================================================ */

static hz_u16 calc_crc16(const hz_u8 *data, hz_u32 len)
{
    hz_u16 crc = 0xFFFF;
    hz_u32 i, j;
    for (i = 0; i < len; i++) {
        crc ^= (hz_u16)data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else         crc >>= 1;
        }
    }
    return crc;
}

/* ============================================================
 * 初始化
 * ============================================================ */

hz_err_t hz_config_mgr_init(const hz_config_item_t *items, hz_u32 count)
{
    if (!items || count == 0 || count > HZ_CONFIG_MAX_ITEMS) {
        return HZ_ERR_INVALID;
    }
    s_items = items;
    s_item_count = count;
    s_inited = true;
    return HZ_OK;
}

void hz_config_mgr_set_storage(const hz_config_storage_t *storage)
{
    if (storage) {
        s_storage = *storage;
        s_storage_set = true;
    }
}

/* ============================================================
 * 从存储加载参数
 * ============================================================ */

hz_err_t hz_config_mgr_load(void)
{
    /* 简化版：从存储读取时直接使用默认值 */
    /* 实际应用中需实现 Flash 读取 + CRC 校验 */
    return hz_config_mgr_reset();
}

/* ============================================================
 * 保存参数到存储
 * ============================================================ */

hz_err_t hz_config_mgr_save(void)
{
    if (!s_inited || !s_storage_set) return HZ_ERR_INVALID;
    /* TODO: 实现持久化存储 */
    return HZ_OK;
}

/* ============================================================
 * 获取参数值
 * ============================================================ */

hz_err_t hz_config_mgr_get(hz_config_id_t id, void *value)
{
    const hz_config_item_t *item;
    if (!s_inited || !value) return HZ_ERR_INVALID;
    item = find_item(id);
    if (!item) return HZ_ERR_NOT_FOUND;
    memcpy(value, item->default_val, type_size(item->type));
    return HZ_OK;
}

/* ============================================================
 * 设置参数值
 * ============================================================ */

hz_err_t hz_config_mgr_set(hz_config_id_t id, const void *value)
{
    const hz_config_item_t *item;
    if (!s_inited || !value) return HZ_ERR_INVALID;
    item = find_item(id);
    if (!item) return HZ_ERR_NOT_FOUND;

    /* 合法性校验 */
    if (item->validator && !item->validator(value)) {
        return HZ_ERR_INVALID;
    }

    /* 写入运行时值 */
    /* TODO: 实现运行时值存储 */
    return HZ_OK;
}

/* ============================================================
 * 恢复出厂设置
 * ============================================================ */

hz_err_t hz_config_mgr_reset(void)
{
    /* 默认值即为出厂值 */
    return HZ_OK;
}

#endif /* HZ_ENABLE_CONFIG_MGR */
