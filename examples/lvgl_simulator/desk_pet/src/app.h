/**
 * @file app.h
 * @brief Pro Desk Pet — shared types, enums, and data structures
 *
 * Single source of shared definitions for all demo modules.
 * Follows the HZCL convention: app-level enums, structs, and extern globals.
 */

#ifndef APP_H
#define APP_H

#include "hz_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
 * FSM — State IDs
 *===========================================================================*/
enum {
    PET_STATE_IDLE       = 0,
    PET_STATE_HAPPY      = 1,
    PET_STATE_EATING     = 2,
    PET_STATE_SLEEPING   = 3,
    PET_STATE_SAD        = 4,
    PET_STATE_SICK       = 5,
    PET_STATE_HYPER      = 6,
    PET_STATE_TRAINING   = 7,
    PET_STATE_CLEANING   = 8,
    PET_STATE_COUNT      = 9,
};

/*===========================================================================
 * FSM — Action IDs
 *===========================================================================*/
enum {
    ACT_FEED     = 1,
    ACT_PET      = 2,
    ACT_GO_SLEEP = 3,
    ACT_TICK     = 4,
    ACT_WAKE     = 5,
    ACT_CLEAN    = 6,
    ACT_TRAIN    = 7,
    ACT_STOP     = 8,
};

/*===========================================================================
 * Custom Event IDs (EV_USER_BASE + N)
 *===========================================================================*/
enum {
    EV_PET_FEED     = 0x0100,
    EV_PET_PET      = 0x0101,
    EV_PET_SLEEP    = 0x0102,
    EV_PET_CLEAN    = 0x0103,
    EV_PET_TRAIN    = 0x0104,
    EV_PET_WAKE     = 0x0105,
    EV_PET_MODE     = 0x0106,
};

/*===========================================================================
 * Mode IDs
 *===========================================================================*/
enum {
    MODE_NORMAL = 0,
    MODE_FOCUS  = 1,
    MODE_PLAY   = 2,
    MODE_NIGHT  = 3,
    MODE_COUNT  = 4,
};

/*===========================================================================
 * Sensor Channel IDs
 *===========================================================================*/
enum {
    SENSOR_CH_WELLNESS = 0,
    SENSOR_CH_ACTIVITY = 1,
    SENSOR_CH_COUNT    = 2,
};

/*===========================================================================
 * Alarm IDs
 *===========================================================================*/
enum {
    ALARM_HUNGER_HIGH        = 0,
    ALARM_ENERGY_LOW         = 1,
    ALARM_SICK_PROLONGED     = 2,
    ALARM_ACHIEVEMENT_UNLOCK = 3,
    ALARM_MODE_AUTO_SWITCH   = 4,
    ALARM_LOW_ACTIVITY       = 5,
    ALARM_COUNT              = 6,
};

/*===========================================================================
 * Config Parameter IDs
 *===========================================================================*/
enum {
    CFG_HUNGER_DECAY   = 1,
    CFG_ENERGY_DECAY   = 2,
    CFG_MOOD_DECAY     = 3,
    CFG_HUNGER_THRESH  = 4,
    CFG_ENERGY_THRESH  = 5,
    CFG_THEME_COLOR    = 6,
    CFG_AUTO_SAVE_MS   = 7,
    CFG_COUNT          = 7,
};

/*===========================================================================
 * Pet Status Data
 *===========================================================================*/
typedef struct {
    int32_t hunger;    /**< 0-100, higher = hungrier */
    int32_t energy;    /**< 0-100, higher = more energetic */
    int32_t mood;      /**< 0-100, higher = happier */
    int32_t hygiene;   /**< 0-100, higher = cleaner */
} pet_status_t;

/*===========================================================================
 * Mode Factors (decay rate multipliers)
 *===========================================================================*/
typedef struct {
    float hunger_decay;  /**< Multiplier for hunger increase */
    float energy_decay;  /**< Multiplier for energy decrease */
    float mood_decay;    /**< Multiplier for mood change */
} mode_factors_t;

/*===========================================================================
 * Theme Colors
 *===========================================================================*/
typedef struct {
    hz_u32 bg_color;     /**< Background color (0xRRGGBB) */
    hz_u32 accent_color; /**< Accent color for buttons / highlights */
} theme_colors_t;

/*===========================================================================
 * Config Cache (runtime mirror of hz_config_mgr values)
 *===========================================================================*/
typedef struct {
    float  hunger_decay;
    float  energy_decay;
    float  mood_decay;
    hz_u8  hunger_thresh;
    hz_u8  energy_thresh;
    hz_u32 theme_color;
    hz_u16 auto_save_ms;
} app_config_t;

/*===========================================================================
 * State Counters (track duration in auto-transition states)
 *===========================================================================*/
typedef struct {
    hz_u8 happy_duration;
    hz_u8 eating_duration;
    hz_u8 training_duration;
    hz_u8 cleaning_duration;
    hz_u8 sad_duration;
    hz_u8 sick_duration;
} pet_state_counters_t;

/*===========================================================================
 * Activity Log
 *===========================================================================*/
#define ACTIVITY_LOG_SIZE 5

typedef struct {
    char entries[ACTIVITY_LOG_SIZE][32];
    hz_u8 head;
    hz_u8 count;
} activity_log_t;

/*===========================================================================
 * Chart History (for Stats screen)
 *===========================================================================*/
#define CHART_DATA_POINTS 10

typedef struct {
    int32_t mood[CHART_DATA_POINTS];
    int32_t energy[CHART_DATA_POINTS];
    int32_t fullness[CHART_DATA_POINTS]; /**< 100 - hunger */
    hz_u8 index;
} chart_history_t;

/*===========================================================================
 * Achievement Flags
 *===========================================================================*/
typedef struct {
    hz_u8 fed_10_times        : 1;  /**< Fed the pet 10 times */
    hz_u8 mood_80_for_10      : 1;  /**< Mood > 80 for 10+ cumulative ticks */
    hz_u8 played_at_night     : 1;  /**< Interacted during NIGHT mode */
    hz_u8 reached_hyper       : 1;  /**< Reached HYPER state */
    hz_u8 trained_5_times     : 1;  /**< Trained 5 times */
    hz_u8 all_states_seen     : 1;  /**< Visited every FSM state */
} achievement_flags_t;

/*===========================================================================
 * Extern Globals (owned by main.c, readable by screens)
 *===========================================================================*/
extern pet_status_t          g_pet;
extern pet_state_counters_t  g_counters;
extern mode_factors_t        g_mode_factors;
extern theme_colors_t        g_theme;
extern app_config_t          g_app_cfg;
extern activity_log_t        g_activity_log;
extern chart_history_t       g_chart_history;
extern achievement_flags_t   g_achievements;
extern const char           *g_state_emojis[PET_STATE_COUNT];
extern const char           *g_state_moods[PET_STATE_COUNT];

/*===========================================================================
 * Public Functions
 *===========================================================================*/

/** Notify the activity sensor that a user interaction occurred */
void app_notify_activity(void);

/** Log an action to the activity log ring buffer */
void app_log_action(const char *action);

/** Pet status simulation (called each second via ACT_TICK) */
void pet_status_tick(void);

/** Check and unlock achievements */
void app_check_achievements(void);

/** Get the mood label string for a given mood value */
const char *app_mood_label(int32_t mood);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
