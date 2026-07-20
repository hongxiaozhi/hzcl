#include "init/config.h"
#include "app.h"
#include "hz_types.h"
#include "hz_config_mgr.h"

app_config_t g_app_cfg;

/*===========================================================================
 * Default values
 *===========================================================================*/
static float   s_def_hunger_decay  = 1.0f;
static float   s_def_energy_decay  = 1.0f;
static float   s_def_mood_decay    = 1.0f;
static hz_u8   s_def_hunger_thresh = 75;
static hz_u8   s_def_energy_thresh = 20;
static hz_u32  s_def_theme_color   = 0x00D2FF;
static hz_u16  s_def_auto_save_ms  = 30000;

static hz_bool_t validate_decay(const void *val) {
    float f = *(const float *)val; return (f >= 0.1f && f <= 5.0f) ? 1 : 0;
}
static hz_bool_t validate_thresh_u8(const void *val) {
    hz_u8 v = *(const hz_u8 *)val; return (v <= 100) ? 1 : 0;
}

static const hz_config_item_t s_config_items[] = {
    { .id = CFG_HUNGER_DECAY,  .name = "hunger_decay",  .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_hunger_decay,  .validator = validate_decay     },
    { .id = CFG_ENERGY_DECAY,  .name = "energy_decay",  .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_energy_decay,  .validator = validate_decay     },
    { .id = CFG_MOOD_DECAY,    .name = "mood_decay",    .type = HZ_CONFIG_TYPE_FLOAT, .default_val = &s_def_mood_decay,    .validator = validate_decay     },
    { .id = CFG_HUNGER_THRESH, .name = "hunger_thresh", .type = HZ_CONFIG_TYPE_U8,    .default_val = &s_def_hunger_thresh, .validator = validate_thresh_u8 },
    { .id = CFG_ENERGY_THRESH, .name = "energy_thresh", .type = HZ_CONFIG_TYPE_U8,    .default_val = &s_def_energy_thresh, .validator = validate_thresh_u8 },
    { .id = CFG_THEME_COLOR,   .name = "theme_color",   .type = HZ_CONFIG_TYPE_U32,   .default_val = &s_def_theme_color,   .validator = NULL               },
    { .id = CFG_AUTO_SAVE_MS,  .name = "auto_save_ms",  .type = HZ_CONFIG_TYPE_U16,   .default_val = &s_def_auto_save_ms,  .validator = NULL               },
};

static void config_cache_init(void)
{
    g_app_cfg.hunger_decay  = s_def_hunger_decay;
    g_app_cfg.energy_decay  = s_def_energy_decay;
    g_app_cfg.mood_decay    = s_def_mood_decay;
    g_app_cfg.hunger_thresh = s_def_hunger_thresh;
    g_app_cfg.energy_thresh = s_def_energy_thresh;
    g_app_cfg.theme_color   = s_def_theme_color;
    g_app_cfg.auto_save_ms  = s_def_auto_save_ms;
}

void init_config(void)
{
    hz_config_mgr_init(s_config_items, CFG_COUNT);
    config_cache_init();
}
