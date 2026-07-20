/**
 * @file hz_platform_sdl2.c
 * @brief HZCL platform adaptation for SDL2 (PC simulator)
 *
 * Implements the platform abstraction layer using SDL2 functions.
 */

#include "hz_platform.h"
#include "SDL.h"

hz_err_t hz_platform_init(void)
{
    /* SDL is already initialized by LVGL */
    return HZ_OK;
}

hz_u32 hz_platform_get_tick_ms(void)
{
    return (hz_u32)SDL_GetTicks();
}

void hz_platform_critical_enter(void)
{
    /* No critical section needed on PC simulator */
}

void hz_platform_critical_exit(void)
{
    /* No critical section needed on PC simulator */
}

void hz_platform_delay_ms(hz_u32 ms)
{
    SDL_Delay(ms);
}
