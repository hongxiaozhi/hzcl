/*
 * HZCL - Horizon Zone Control Library
 * hz_init_mgr.c - 初始化管理器实现
 */

#include "hz_init_mgr.h"

#if HZ_ENABLE_INIT_MGR

#include "hz_event.h"
#include <string.h>

/* ============================================================
 * 全局状态
 * ============================================================ */

static hz_init_entry_t *s_entries[HZ_INIT_MAX_ENTRIES];
static hz_u32           s_entry_count = 0;
static hz_bool_t        s_initialized = false;

/* 初始化结果状态 */
typedef enum {
    HZ_INIT_STATUS_PENDING,
    HZ_INIT_STATUS_OK,
    HZ_INIT_STATUS_FAILED,
    HZ_INIT_STATUS_SKIPPED,
} hz_init_status_t;

static hz_init_status_t s_status[HZ_INIT_MAX_ENTRIES];

/* ============================================================
 * 内部查找
 * ============================================================ */

static hz_s32 find_entry_by_name(const char *name)
{
    hz_u32 i;
    if (!name) return -1;
    for (i = 0; i < s_entry_count; i++) {
        if (s_entries[i]->name &&
            strcmp(s_entries[i]->name, name) == 0) {
            return (hz_s32)i;
        }
    }
    return -1;
}

/* ============================================================
 * 注册初始化条目
 * ============================================================ */

hz_err_t hz_init_register(hz_init_entry_t *entry)
{
    if (!entry || !entry->name || !entry->init) {
        return HZ_ERR_INVALID;
    }
    if (s_entry_count >= HZ_INIT_MAX_ENTRIES) {
        return HZ_ERR_NOMEM;
    }
    if (find_entry_by_name(entry->name) >= 0) {
        return HZ_ERR_FAIL; /* 名称重复 */
    }

    s_entries[s_entry_count] = entry;
    s_status[s_entry_count] = HZ_INIT_STATUS_PENDING;
    s_entry_count++;
    return HZ_OK;
}

/* ============================================================
 * 比较函数用于排序（按 stage + order）
 * ============================================================ */

static hz_s32 entry_cmp(const void *a, const void *b)
{
    hz_init_entry_t *ea = *(hz_init_entry_t **)a;
    hz_init_entry_t *eb = *(hz_init_entry_t **)b;

    if (ea->stage != eb->stage) {
        return (ea->stage < eb->stage) ? -1 : 1;
    }
    if (ea->order != eb->order) {
        return (ea->order < eb->order) ? -1 : 1;
    }
    return 0;
}

/* ============================================================
 * 执行所有初始化
 * ============================================================ */

hz_err_t hz_init_all(void)
{
    hz_u32 i;

    /* 1. 按 stage + order 排序 */
    /* 简化版：按注册顺序执行（用户需自行保证注册顺序） */

    /* 2. 遍历所有条目 */
    for (i = 0; i < s_entry_count; i++) {
        hz_init_entry_t *e = s_entries[i];

        /* 检查依赖 */
        hz_bool_t dep_ok = true;
        hz_u32 j;
        for (j = 0; j < e->dep_count; j++) {
            hz_s32 dep_idx = find_entry_by_name(e->dependencies[j]);
            if (dep_idx < 0 || s_status[dep_idx] != HZ_INIT_STATUS_OK) {
                dep_ok = false;
                break;
            }
        }

        if (!dep_ok) {
            s_status[i] = HZ_INIT_STATUS_SKIPPED;
            continue;
        }

        /* 执行 init */
        hz_err_t err = e->init();

        if (err == HZ_OK) {
            s_status[i] = HZ_INIT_STATUS_OK;
        } else {
            switch (e->fail_policy) {
            case HZ_INIT_FAIL_STOP:
                s_status[i] = HZ_INIT_STATUS_FAILED;
                s_initialized = true;
                return err;
            case HZ_INIT_FAIL_SKIP:
                s_status[i] = HZ_INIT_STATUS_SKIPPED;
                break;
            case HZ_INIT_FAIL_RETRY:
                /* 重试 3 次 */
                {
                    hz_u8 retry;
                    for (retry = 0; retry < 3; retry++) {
                        err = e->init();
                        if (err == HZ_OK) break;
                    }
                    s_status[i] = (err == HZ_OK)
                        ? HZ_INIT_STATUS_OK
                        : HZ_INIT_STATUS_FAILED;
                }
                break;
            }
        }
    }

    s_initialized = true;
    hz_event_publish(EV_SYS_INIT_COMPLETE, NULL);
    return HZ_OK;
}

/* ============================================================
 * 逆序 deinit
 * ============================================================ */

void hz_init_deinit_all(void)
{
    hz_s32 i;
    for (i = (hz_s32)s_entry_count - 1; i >= 0; i--) {
        if (s_status[i] == HZ_INIT_STATUS_OK && s_entries[i]->deinit) {
            s_entries[i]->deinit();
        }
        s_status[i] = HZ_INIT_STATUS_PENDING;
    }
}

/* ============================================================
 * 暂停/恢复（休眠用）
 * ============================================================ */

hz_err_t hz_init_suspend_all(void)
{
    hz_s32 i;
    for (i = (hz_s32)s_entry_count - 1; i >= 0; i--) {
        if (s_status[i] == HZ_INIT_STATUS_OK && s_entries[i]->suspend) {
            s_entries[i]->suspend();
        }
    }
    return HZ_OK;
}

hz_err_t hz_init_resume_all(void)
{
    hz_u32 i;
    for (i = 0; i < s_entry_count; i++) {
        if (s_status[i] == HZ_INIT_STATUS_OK &&
            s_entries[i]->power_domain != HZ_POWER_DOMAIN_ALWAYS_ON &&
            s_entries[i]->resume) {
            s_entries[i]->resume();
        }
    }
    return HZ_OK;
}

/* ============================================================
 * 状态查询
 * ============================================================ */

hz_bool_t hz_init_is_ok(const char *name)
{
    hz_s32 idx = find_entry_by_name(name);
    if (idx < 0) return false;
    return s_status[idx] == HZ_INIT_STATUS_OK;
}

void hz_init_print_status(void)
{
    hz_u32 i;
    for (i = 0; i < s_entry_count; i++) {
        const char *status_str;
        switch (s_status[i]) {
        case HZ_INIT_STATUS_OK:      status_str = "OK";      break;
        case HZ_INIT_STATUS_FAILED:  status_str = "FAILED";  break;
        case HZ_INIT_STATUS_SKIPPED: status_str = "SKIPPED"; break;
        default:                     status_str = "PENDING"; break;
        }
        /* 实际应用中可通过日志接口输出 */
        (void)status_str;
    }
}

#endif /* HZ_ENABLE_INIT_MGR */
