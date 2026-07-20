/*
 * HZCL - Horizon Zone Control Library
 * hz_config_mgr.c - Parameter configuration manager implementation
 */

#include "hz_config_mgr.h"

#if HZ_ENABLE_CONFIG_MGR

#include <string.h>

/* ============================================================
 * Global state
 * ============================================================ */

static const hz_config_item_t *s_items = NULL;
static hz_u32                  s_item_count = 0;
static hz_u8                  *s_values = NULL;      /* Runtime value buffer */
static hz_config_storage_t     s_storage;
static hz_bool_t               s_storage_set = false;
static hz_bool_t               s_inited = false;

/* ============================================================
 * Internal: find item by ID
 * ============================================================ */

static const hz_config_item_t *find_item(hz_config_id_t id)
{
    hz_u32 i;
    for (i = 0; i < s_item_count; i++) {
        if (s_items[i].id == id) return &s_items[i];
    }
    return NULL;
}

/* Get data type size in bytes */
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
 * Simple CRC16 checksum
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
 * Initialize
 * ============================================================ */

hz_err_t hz_config_mgr_init(const hz_config_item_t *items, hz_u32 count)
{
    hz_u32 i;

    if (!items || count == 0 || count > HZ_CONFIG_MAX_ITEMS) {
        return HZ_ERR_INVALID;
    }

    s_items = items;
    s_item_count = count;
    s_inited = true;

    /* Allocate runtime value buffer and initialize from defaults */
    s_values = (hz_u8 *)items; /* Will set up properly below */

    /* We need a private buffer for runtime values.
     * This implementation uses a statically-sized internal array
     * sized to accommodate all registered items. */
    static hz_u8 s_value_buf[HZ_CONFIG_MAX_ITEMS * 4]; /* max 4 bytes per item */
    s_values = s_value_buf;

    for (i = 0; i < s_item_count; i++) {
        hz_u32 sz = type_size(s_items[i].type);
        hz_u32 offset = i * 4; /* 4-byte aligned slots */
        memcpy(&s_values[offset], s_items[i].default_val, sz);
    }

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
 * Load parameters from storage
 * ============================================================ */

hz_err_t hz_config_mgr_load(void)
{
    /* Simplified: reset to defaults */
    /* Real implementation would read from flash + CRC verify */
    return hz_config_mgr_reset();
}

/* ============================================================
 * Save parameters to storage
 * ============================================================ */

hz_err_t hz_config_mgr_save(void)
{
    hz_u32 i, total_size = 0;

    if (!s_inited || !s_storage_set) return HZ_ERR_INVALID;

    /* Calculate total data size (4-byte slots) */
    for (i = 0; i < s_item_count; i++) {
        hz_u32 offset = i * 4;
        hz_u32 end = offset + type_size(s_items[i].type);
        if (end > total_size) total_size = end;
    }

    /* Write data + CRC */
    if (s_storage.write) {
        hz_err_t err = s_storage.write(0, s_values, total_size);
        if (err != HZ_OK) return err;

        hz_u16 crc = calc_crc16(s_values, total_size);
        return s_storage.write(total_size, (const hz_u8 *)&crc, sizeof(crc));
    }

    return HZ_OK;
}

/* ============================================================
 * Get parameter value
 * ============================================================ */

hz_err_t hz_config_mgr_get(hz_config_id_t id, void *value)
{
    const hz_config_item_t *item;
    if (!s_inited || !value) return HZ_ERR_INVALID;
    item = find_item(id);
    if (!item) return HZ_ERR_NOT_FOUND;

    /* Return runtime value from buffer */
    hz_u32 offset = (hz_u32)(item - s_items) * 4;
    hz_u32 sz = type_size(item->type);
    memcpy(value, &s_values[offset], sz);
    return HZ_OK;
}

/* ============================================================
 * Set parameter value
 * ============================================================ */

hz_err_t hz_config_mgr_set(hz_config_id_t id, const void *value)
{
    const hz_config_item_t *item;
    if (!s_inited || !value) return HZ_ERR_INVALID;
    item = find_item(id);
    if (!item) return HZ_ERR_NOT_FOUND;

    /* Validate */
    if (item->validator && !item->validator(value)) {
        return HZ_ERR_INVALID;
    }

    /* Store runtime value */
    hz_u32 offset = (hz_u32)(item - s_items) * 4;
    hz_u32 sz = type_size(item->type);
    memcpy(&s_values[offset], value, sz);

    return HZ_OK;
}

/* ============================================================
 * Factory reset
 * ============================================================ */

hz_err_t hz_config_mgr_reset(void)
{
    hz_u32 i;
    if (!s_inited) return HZ_ERR_INVALID;

    for (i = 0; i < s_item_count; i++) {
        hz_u32 offset = i * 4;
        hz_u32 sz = type_size(s_items[i].type);
        memcpy(&s_values[offset], s_items[i].default_val, sz);
    }
    return HZ_OK;
}

#endif /* HZ_ENABLE_CONFIG_MGR */
