/**
 * @file fsm.h
 * @brief Toothbrush FSM — state machine and application data
 *
 * Uses hz_fsm for state management. Only 3 HZCL modules: fsm, screen, timer.
 */

#ifndef FSM_H
#define FSM_H

#include "hz_types.h"

/*===========================================================================
 * Motor modes
 *===========================================================================*/
typedef enum {
    MOTOR_CARE  = 0,  /* Care / gentle */
    MOTOR_CLEAN = 1,  /* Clean */
    MOTOR_WHITE = 2,  /* White / whitening */
    MOTOR_COUNT = 3,
} motor_mode_t;

/*===========================================================================
 * FSM State IDs
 *===========================================================================*/
enum {
    STATE_READY    = 0,  /* Ready / standby */
    STATE_WORKING  = 1,  /* Motor running */
    STATE_COVER    = 2,  /* Coverage area reminder */
    STATE_STATS    = 3,  /* Brushing statistics */
    STATE_CANCEL   = 4,  /* Brushing cancelled (< 30s) */
    STATE_SETTINGS = 5,  /* Settings menu */
    STATE_SLEEP    = 6,  /* Sleep / display off */
    STATE_COUNT    = 7,
};

/*===========================================================================
 * FSM Action IDs
 *===========================================================================*/
enum {
    ACT_START        = 1,  /* Short press → start working */
    ACT_STOP         = 2,  /* Short press → stop (stats or cancel) */
    ACT_MODE_NEXT    = 3,  /* Short press → cycle motor mode */
    ACT_ENTER_SETTING= 4,  /* Long press → settings */
    ACT_EXIT_SETTING = 5,  /* Exit settings */
    ACT_COVER_DONE   = 6,  /* Coverage reviewed → continue */
    ACT_IDLE_TIMEOUT = 7,  /* 8s no operation → sleep */
    ACT_WAKE         = 8,  /* Wake from sleep */
};

/*===========================================================================
 * Application data (shared across screens)
 *===========================================================================*/
typedef struct {
    motor_mode_t mode;        /* Current motor mode */
    motor_mode_t last_mode;   /* Remembered mode across sleep */
    hz_u32       work_sec;    /* Elapsed work seconds (0-120) */
    hz_u32       total_sec;   /* Total brushing seconds (accumulated) */
    hz_u8        score;       /* Brushing score (0-100) */
    hz_bool_t    in_settings; /* Currently in settings sub-menu */
    hz_u8        idle_sec;    /* Idle countdown (8 → 0) */
} app_data_t;

/*===========================================================================
 * Globals
 *===========================================================================*/
extern app_data_t g_app;

/*===========================================================================
 * Init and tick
 *===========================================================================*/
void fsm_init(void);
void fsm_tick_1s(void);   /* Called every second while working */
void fsm_reset_idle(void); /* Reset idle timer on user interaction */

#endif /* FSM_H */
