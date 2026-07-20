cmake_minimum_required(VERSION 3.20)
project(desk_pet LANGUAGES C)

# ============================================================
# Paths
# ============================================================
get_filename_component(HZCL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
get_filename_component(LVGL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../lvgl" ABSOLUTE)
set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/SDL2-2.30.9")

message(STATUS "HZCL_ROOT: ${HZCL_ROOT}")
message(STATUS "LVGL_ROOT: ${LVGL_ROOT}")
message(STATUS "SDL2_DIR:  ${SDL2_DIR}")

# ============================================================
# LVGL Library
# ============================================================
set(LV_BUILD_CONF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc" CACHE PATH "LVGL config dir" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "Build LVGL examples" FORCE)
set(CONFIG_LV_BUILD_DEMOS    OFF CACHE BOOL "Build LVGL demos"    FORCE)

add_subdirectory(${LVGL_ROOT} lvgl_build)

# LVGL SDL driver needs SDL2 headers (uses LV_SDL_INCLUDE_PATH "SDL.h")
target_include_directories(lvgl PRIVATE "${SDL2_DIR}/include")

# ============================================================
# HZCL Library
# ============================================================
# Override module options for simulator (all enabled)
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
# Bundled SDL2 (prebuilt libraries at sdl2/SDL2-2.30.9/lib/x64/)
# ============================================================
add_library(sdl2_bundled INTERFACE)
target_include_directories(sdl2_bundled INTERFACE "${SDL2_DIR}/include")
target_link_directories(sdl2_bundled INTERFACE "${SDL2_DIR}/lib/x64")
target_link_libraries(sdl2_bundled INTERFACE SDL2.lib SDL2main.lib)

# ============================================================
# Desk Pet Executable
# ============================================================
add_executable(desk_pet WIN32
    src/main.c
    src/hz_platform_sdl2.c
)

target_include_directories(desk_pet PRIVATE
    "${HZCL_ROOT}"       # hz_config.h
    "${HZCL_ROOT}/src"   # hz_types.h, hz_fsm.h, ...
)

target_link_libraries(desk_pet PRIVATE
    lvgl
    hzcl
    sdl2_bundled
)

# Copy SDL2.dll to output directory
add_custom_command(TARGET desk_pet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DIR}/lib/x64/SDL2.dll"
        "$<TARGET_FILE_DIR:desk_pet>"
    COMMENT "Copying SDL2.dll to output directory..."
)
