/*
 * HZCL - Horizon Zone Control Library
 * hz_types.h - 公共类型定义
 *
 * 版权所有 (c) 2026 HongXiaozhi
 */

#ifndef HZ_TYPES_H
#define HZ_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 整数类型简写
 * ============================================================ */

typedef uint8_t   hz_u8;
typedef uint16_t  hz_u16;
typedef uint32_t  hz_u32;
typedef uint64_t  hz_u64;
typedef int8_t    hz_s8;
typedef int16_t   hz_s16;
typedef int32_t   hz_s32;
typedef int64_t   hz_s64;

/* ============================================================
 * 错误码定义
 * ============================================================ */

typedef int32_t hz_err_t;

#define HZ_OK               0
#define HZ_ERR_FAIL        (-1)
#define HZ_ERR_TIMEOUT     (-2)
#define HZ_ERR_NOT_FOUND   (-3)
#define HZ_ERR_BUSY        (-4)
#define HZ_ERR_INVALID     (-5)
#define HZ_ERR_NOMEM       (-6)
#define HZ_ERR_DEPENDENCY  (-7)
#define HZ_ERR_UNSUPPORTED (-8)
#define HZ_ERR_AGAIN       (-9)

/* ============================================================
 * 布尔类型
 * ============================================================ */

#ifndef hz_bool_t
typedef bool hz_bool_t;
#endif

/* ============================================================
 * 通用回调类型
 * ============================================================ */

typedef void (*hz_callback_t)(void *arg);

/* ============================================================
 * 通用声明宏
 * ============================================================ */

#define HZ_UNUSED(x)         ((void)(x))
#define HZ_ARRAY_SIZE(arr)   (sizeof(arr) / sizeof((arr)[0]))
#define HZ_MIN(a, b)         (((a) < (b)) ? (a) : (b))
#define HZ_MAX(a, b)         (((a) > (b)) ? (a) : (b))
#define HZ_CLAMP(val, lo, hi) (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))

#ifdef __cplusplus
}
#endif

#endif /* HZ_TYPES_H */
