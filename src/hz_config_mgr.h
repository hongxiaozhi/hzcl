/*
 * HZCL - Horizon Zone Control Library
 * hz_config_mgr.h - 参数配置管理
 *
 * 解决痛点：参数存取/校验/掉电保护逻辑分散
 *
 * 核心思想：
 *   - KV 存储模式，通过 ID 读写参数
 *   - set() 时自动校验合法性
 *   - 支持 CRC 校验，检测数据损坏
 *   - 注意：此文件是运行时参数管理模块，顶层 hz_config.h 是编译时配置
 */

#ifndef HZ_CONFIG_MGR_H
#define HZ_CONFIG_MGR_H

#include "hz_types.h"
#include "hz_config.h"

#if HZ_ENABLE_CONFIG_MGR

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 参数ID（用户自定义）
 * ============================================================ */

typedef hz_u16 hz_config_id_t;

/* ============================================================
 * 参数数据类型
 * ============================================================ */

typedef enum {
    HZ_CONFIG_TYPE_U8,
    HZ_CONFIG_TYPE_U16,
    HZ_CONFIG_TYPE_U32,
    HZ_CONFIG_TYPE_FLOAT,
    HZ_CONFIG_TYPE_BOOL,
} hz_config_type_t;

/* ============================================================
 * 参数项定义
 * ============================================================ */

typedef struct {
    hz_config_id_t   id;
    const char      *name;
    hz_config_type_t type;
    void            *default_val;    /* 默认值指针 */
    hz_bool_t      (*validator)(const void *value); /* 校验函数 */
} hz_config_item_t;

/* ============================================================
 * 存储后端接口（由平台实现，如 Flash / EEPROM）
 * ============================================================ */

typedef struct {
    hz_err_t (*read)(hz_u32 addr, void *buf, hz_u32 size);
    hz_err_t (*write)(hz_u32 addr, const void *buf, hz_u32 size);
    hz_err_t (*erase)(hz_u32 addr, hz_u32 size);
} hz_config_storage_t;

/* ============================================================
 * API 声明
 * ============================================================ */

hz_err_t hz_config_mgr_init(const hz_config_item_t *items, hz_u32 count);
void     hz_config_mgr_set_storage(const hz_config_storage_t *storage);
hz_err_t hz_config_mgr_load(void);                      /* 从存储加载 */
hz_err_t hz_config_mgr_save(void);                      /* 保存到存储 */
hz_err_t hz_config_mgr_get(hz_config_id_t id, void *value);
hz_err_t hz_config_mgr_set(hz_config_id_t id, const void *value);
hz_err_t hz_config_mgr_reset(void);                     /* 恢复出厂 */

#ifdef __cplusplus
}
#endif

#endif /* HZ_ENABLE_CONFIG_MGR */
#endif /* HZ_CONFIG_MGR_H */
