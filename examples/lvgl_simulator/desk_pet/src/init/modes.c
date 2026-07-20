#include "init/modes.h"
#include "app.h"
#include "hz_mode.h"
#include "hz_event.h"

/*===========================================================================
 * Theme & Mode Factors (global, used by screens & pet simulation)
 *===========================================================================*/
theme_colors_t g_theme = { 0x1a1a2e, 0x00d2ff };
mode_factors_t g_mode_factors = { 1.0f, 1.0f, 1.0f };

/*===========================================================================
 * Mode Enter Callbacks
 *===========================================================================*/
static void mode_enter_normal(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 1.0f; g_mode_factors.energy_decay = 1.0f; g_mode_factors.mood_decay = 1.0f;
    g_theme.bg_color = 0x1a1a2e; g_theme.accent_color = 0x00d2ff;
}
static void mode_enter_focus(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 0.5f; g_mode_factors.energy_decay = 0.7f; g_mode_factors.mood_decay = 0.8f;
    g_theme.bg_color = 0x2d3436; g_theme.accent_color = 0x74b9ff;
}
static void mode_enter_play(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 1.5f; g_mode_factors.energy_decay = 1.5f; g_mode_factors.mood_decay = 1.2f;
    g_theme.bg_color = 0x2d1b1b; g_theme.accent_color = 0xe17055;
}
static void mode_enter_night(hz_mode_id_t prev)
{
    (void)prev;
    g_mode_factors.hunger_decay = 0.3f; g_mode_factors.energy_decay = 0.3f; g_mode_factors.mood_decay = 1.0f;
    g_theme.bg_color = 0x0a0a1a; g_theme.accent_color = 0x6c5ce7;
}

static const hz_mode_param_t s_params_normal[] = { { 0, 1.0f }, { 1, 1.0f }, { 2, 1.0f } };
static const hz_mode_param_t s_params_focus[]  = { { 0, 0.5f }, { 1, 0.7f }, { 2, 0.8f } };
static const hz_mode_param_t s_params_play[]   = { { 0, 1.5f }, { 1, 1.5f }, { 2, 1.2f } };
static const hz_mode_param_t s_params_night[]  = { { 0, 0.3f }, { 1, 0.3f }, { 2, 1.0f } };

static const hz_mode_t s_modes[] = {
    { .id = MODE_NORMAL, .name = "NORMAL", .transition_ms = 0, .on_enter = mode_enter_normal, .params = s_params_normal, .param_count = 3 },
    { .id = MODE_FOCUS,  .name = "FOCUS",  .transition_ms = 0, .on_enter = mode_enter_focus,  .params = s_params_focus,  .param_count = 3 },
    { .id = MODE_PLAY,   .name = "PLAY",   .transition_ms = 0, .on_enter = mode_enter_play,   .params = s_params_play,   .param_count = 3 },
    { .id = MODE_NIGHT,  .name = "NIGHT",  .transition_ms = 0, .on_enter = mode_enter_night,  .params = s_params_night,  .param_count = 3 },
};

void init_modes(void)
{
    for (int i = 0; i < MODE_COUNT; i++)
        hz_mode_register(&s_modes[i]);
}
