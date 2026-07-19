# HZCL — Horizon Zone Control Library

通用嵌入式外设控制与显示系统框架。

## 简介

HZCL 是一套纯 C 语言编写的嵌入式框架，旨在解决个人健康护理设备（电动牙刷、呼吸机等）开发中的常见痛点：状态管理混乱、页面导航复杂、传感器数据分散处理、硬件初始化顺序维护等。

## 架构

```
┌────────────────────────────────────────────┐
│              Your Application               │
├────────────────────────────────────────────┤
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────────┐  │
│  │ FSM  │ │Screen│ │Sensor│ │ Mode     │  │
│  │状态机 │ │导航   │ │流水线 │ │ 模式引擎  │  │
│  ├──────┤ ├──────┤ ├──────┤ ├──────────┤  │
│  │Config│ │Alarm │ │Power │ │ Calib    │  │
│  │配置  │ │报警  │ │电源  │ │ 校准     │  │
│  ├──────┤                             │  │
│  │Init  │                              │  │
│  │初始化│                              │  │
│  ├──────┴──────────── Event Bus ───────┤  │
│  │       hz_event / hz_timer          │  │
├────────────────────────────────────────────┤
│                  HAL 硬件抽象               │
│   hal/hz_hal_sensor.h / actuator / display │
└────────────────────────────────────────────┘
```

## 模块

| 模块 | 文件 | 解决痛点 |
|------|------|---------|
| hz_fsm | 状态机 | 状态管理混乱，全局变量堆砌 |
| hz_screen | 屏幕导航 | 页面跳转混乱，传参靠全局变量 |
| hz_sensor | 传感器流水线 | 采集/滤波/阈值逻辑分散 |
| hz_mode | 模式引擎 | 模式切换不平滑 |
| hz_config_mgr | 参数配置 | 参数存取/校验分散 |
| hz_alarm | 报警系统 | 报警逻辑与业务耦合 |
| hz_power | 电源管理 | 休眠/唤醒散落各处 |
| hz_calib | 出厂校准 | 校准数据易丢失 |
| hz_init_mgr | 初始化管理器 | init 顺序混乱，依赖隐式 |

## 快速开始

```c
#include "hz_config.h"
#include "hz_fsm.h"
#include "hz_screen.h"

int main(void) {
    hz_platform_init();
    hz_event_init();
    hz_timer_init();

    hz_fsm_init(g_states, STATE_COUNT);
    hz_screen_init();

    hz_screen_push(&scr_home, NULL);
    hz_fsm_transition(STATE_STANDBY);

    while (1) {
        hz_u32 ms = hz_platform_get_tick_ms();
        hz_timer_tick(ms);
        hz_fsm_tick(ms);
        hz_screen_tick(ms);
        hz_platform_delay_ms(1);
    }
}
```

## 移植

将 HZCL 移植到新平台只需：

1. 实现 `hz_platform.c`（提供 get_tick_ms / critical_enter / critical_exit / delay_ms）
2. 按 `hal/` 接口实现具体硬件驱动
3. 修改 `hz_config.h` 裁剪需要的模块

## 许可

版权所有 (c) 2026 HongXiaozhi
