cmake_minimum_required(VERSION 3.20)
project(toothbrush LANGUAGES C)

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

# Disable SDL features we don't need
set(SDL_SHARED            ON  CACHE BOOL "" FORCE)
set(SDL_STATIC            OFF CACHE BOOL "" FORCE)
set(SDL_TEST              OFF CACHE BOOL "" FORCE)
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
# HZCL Library (only enable what we use)
# ============================================================
set(HZ_ENABLE_FSM        ON  CACHE BOOL "" FORCE)
set(HZ_ENABLE_SCREEN     ON  CACHE BOOL "" FORCE)
set(HZ_ENABLE_SENSOR     OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_MODE       OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_CONFIG_MGR OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_ALARM      OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_POWER      OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_CALIB      OFF CACHE BOOL "" FORCE)
set(HZ_ENABLE_INIT_MGR   OFF CACHE BOOL "" FORCE)

add_subdirectory(${HZCL_ROOT} hzcl_build)

# ============================================================
# Toothbrush Executable
# ============================================================
add_executable(toothbrush
    src/main.c
    src/hz_platform_sdl2.c
    src/fsm.c
    src/screens/screen_ready.c
    src/screens/screen_working.c
    src/screens/screen_stats.c
    src/screens/screen_sleep.c
)

target_include_directories(toothbrush PRIVATE
    "${HZCL_ROOT}"                          # hz_config.h
    "${HZCL_ROOT}/src"                      # hz_types.h, hz_fsm.h, ...
    "${CMAKE_CURRENT_SOURCE_DIR}/src"       # fsm.h, app.h
)

target_compile_definitions(toothbrush PRIVATE SDL_MAIN_HANDLED)
target_compile_options(toothbrush PRIVATE -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-deprecated-declarations)

target_link_libraries(toothbrush PRIVATE
    lvgl
    hzcl
    SDL2
)

# Copy SDL2.dll to output directory
if(WIN32)
    add_custom_command(TARGET toothbrush POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL2>"
            "$<TARGET_FILE_DIR:toothbrush>"
        COMMENT "Copying SDL2.dll to output directory..."
    )
endif()
