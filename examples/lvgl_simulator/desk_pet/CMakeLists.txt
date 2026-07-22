cmake_minimum_required(VERSION 3.20)
project(desk_pet LANGUAGES C)

# ============================================================
# Paths
# ============================================================
get_filename_component(HZCL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
get_filename_component(LVGL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lvgl" ABSOLUTE)
get_filename_component(SDL_ROOT  "${CMAKE_CURRENT_SOURCE_DIR}/../sdl" ABSOLUTE)

message(STATUS "HZCL_ROOT: ${HZCL_ROOT}")
message(STATUS "LVGL_ROOT: ${LVGL_ROOT}")
message(STATUS "SDL_ROOT:  ${SDL_ROOT}")

# ============================================================
# SDL — git submodule (shared at ../sdl)
# ============================================================
if(NOT EXISTS "${SDL_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "SDL submodule not found at ${SDL_ROOT}\n"
        "Run: git submodule update --init examples/lvgl_simulator/sdl"
    )
endif()

# Disable SDL features we don't need (faster build)
set(SDL_SHARED           ON  CACHE BOOL "" FORCE)
set(SDL_STATIC           OFF CACHE BOOL "" FORCE)
set(SDL_TEST             OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_SDL2MAIN OFF CACHE BOOL "" FORCE)

add_subdirectory(${SDL_ROOT} sdl_build)

# ============================================================
# LVGL Library
# ============================================================
set(LV_BUILD_CONF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc" CACHE PATH "LVGL config dir" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build LVGL examples" FORCE)
set(CONFIG_LV_BUILD_DEMOS    OFF CACHE BOOL "Build LVGL demos"    FORCE)

add_subdirectory(${LVGL_ROOT} lvgl_build)

# LVGL SDL driver needs SDL2 headers
target_include_directories(lvgl PRIVATE "${SDL_ROOT}/include")

# ============================================================
# HZCL Library
# ============================================================
set(HZ_ENABLE_FSM        ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_SCREEN     ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_SENSOR     ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_MODE       ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_CONFIG_MGR ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_ALARM      ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_POWER      ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_CALIB      ON CACHE BOOL "" FORCE)
set(HZ_ENABLE_INIT_MGR   ON CACHE BOOL "" FORCE)

add_subdirectory(${HZCL_ROOT} hzcl_build)

# ============================================================
# Desk Pet Executable
# ============================================================
add_executable(desk_pet
    src/main.c
    src/hz_platform_sdl2.c
    src/pet_face.c
    src/screens/screen_home.c
    src/screens/screen_stats.c
    src/screens/screen_settings.c
    src/screens/screen_about.c
    src/screens/screen_hmi.c
    src/init/fsm.c
    src/init/modes.c
    src/init/sensors.c
    src/init/alarms.c
    src/init/config.c
    src/init/timers.c
    src/init/events.c
    src/init/lvgl.c
    res/fonts/mdi_icons_20.c
)

target_include_directories(desk_pet PRIVATE
    "${HZCL_ROOT}"              # hz_config.h
    "${HZCL_ROOT}/src"          # hz_types.h, hz_fsm.h, ...
    "${CMAKE_CURRENT_SOURCE_DIR}/src"  # app.h
    "${CMAKE_CURRENT_SOURCE_DIR}/res/fonts"  # mdi_icons.h
)

target_compile_definitions(desk_pet PRIVATE SDL_MAIN_HANDLED)

# Suppress pointer-to-int cast warnings (config_id passed via void* on 64-bit)
if(MSVC)
    target_compile_options(desk_pet PRIVATE /utf-8 /wd4311 /wd4312 /wd4996)
else()
    target_compile_options(desk_pet PRIVATE -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast)
    target_compile_options(desk_pet PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(desk_pet PRIVATE
    lvgl
    hzcl
    SDL2
)

# Copy SDL2.dll to output directory
if(WIN32)
    add_custom_command(TARGET desk_pet POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL2>"
            "$<TARGET_FILE_DIR:desk_pet>"
        COMMENT "Copying SDL2.dll to output directory..."
    )
endif()
