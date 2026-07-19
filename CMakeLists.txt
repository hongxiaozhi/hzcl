# HZCL - Horizon Zone Control Library
cmake_minimum_required(VERSION 3.10)
project(hzcl VERSION 1.0.0 LANGUAGES C)

# 指定 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 模块配置（可覆盖）
option(HZ_ENABLE_FSM          "Enable FSM module"          ON)
option(HZ_ENABLE_SCREEN       "Enable Screen module"       ON)
option(HZ_ENABLE_SENSOR       "Enable Sensor module"       ON)
option(HZ_ENABLE_MODE         "Enable Mode module"         ON)
option(HZ_ENABLE_CONFIG_MGR   "Enable Config module"       ON)
option(HZ_ENABLE_ALARM        "Enable Alarm module"        ON)
option(HZ_ENABLE_POWER        "Enable Power module"        ON)
option(HZ_ENABLE_CALIB        "Enable Calib module"        ON)
option(HZ_ENABLE_INIT_MGR     "Enable Init Manager module" ON)

# 头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 源文件列表
set(HZCL_SOURCES
    src/hz_event.c
    src/hz_timer.c
)

# 根据选项添加模块
if(HZ_ENABLE_FSM)
    list(APPEND HZCL_SOURCES src/hz_fsm.c)
endif()
if(HZ_ENABLE_SCREEN)
    list(APPEND HZCL_SOURCES src/hz_screen.c)
endif()
if(HZ_ENABLE_SENSOR)
    list(APPEND HZCL_SOURCES src/hz_sensor.c src/hz_sensor_filter.c)
endif()
if(HZ_ENABLE_MODE)
    list(APPEND HZCL_SOURCES src/hz_mode.c)
endif()
if(HZ_ENABLE_CONFIG_MGR)
    list(APPEND HZCL_SOURCES src/hz_config_mgr.c)
endif()
if(HZ_ENABLE_ALARM)
    list(APPEND HZCL_SOURCES src/hz_alarm.c)
endif()
if(HZ_ENABLE_POWER)
    list(APPEND HZCL_SOURCES src/hz_power.c)
endif()
if(HZ_ENABLE_CALIB)
    list(APPEND HZCL_SOURCES src/hz_calib.c)
endif()
if(HZ_ENABLE_INIT_MGR)
    list(APPEND HZCL_SOURCES src/hz_init_mgr.c)
endif()

# 添加平台文件（用户需自行提供）
# list(APPEND HZCL_SOURCES src/hz_platform_xxx.c)

# 创建库
add_library(hzcl STATIC ${HZCL_SOURCES})

# 安装
install(TARGETS hzcl DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/hzcl FILES_MATCHING PATTERN "*.h")
install(DIRECTORY hal/ DESTINATION include/hzcl FILES_MATCHING PATTERN "*.h")
install(FILES hz_config.h DESTINATION include/hzcl)
