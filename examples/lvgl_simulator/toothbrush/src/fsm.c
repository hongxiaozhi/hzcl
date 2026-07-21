/**
 * @file fsm.c
 * @brief Toothbrush FSM — state table and action handlers
 */
#include "fsm.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_screen.h"

/*===========================================================================
 * Application globals
 *===========================================================================*/
app_data_t g_app = {
    .mode = MOTOR_CLEAN, .last_mode = MOTOR_CLEAN,
    .work_sec = 0, .total_sec = 0, .score = 0,
    .in_settings = 0, .idle_sec = 0,
};

/*===========================================================================
 * Screen descriptors (defined in screen_*.c)
 *===========================================================================*/
extern hz_screen_t g_scr_ready;
extern hz_screen_t g_scr_working;
extern hz_screen_t g_scr_stats;
extern hz_screen_t g_scr_sleep;

/*===========================================================================
 * Action handlers
 *===========================================================================*/
static hz_err_t act_start(void *arg)
{
    (void)arg;
    /* Only reset work timer on fresh start from READY, not resume from STATS/CANCEL */
    if (hz_fsm_current() == STATE_READY) {
        g_app.work_sec = 0;
    }
    g_app.idle_sec = 0;
    return HZ_OK;
}

static hz_err_t act_stop(void *arg)
{
    (void)arg;
    return HZ_OK;   /* transition decided by caller check in handle_click */
}

static hz_err_t act_mode_next(void *arg)
{
    (void)arg;
    g_app.mode = (g_app.mode + 1) % MOTOR_COUNT;
    return HZ_OK;
}

static hz_err_t act_enter_setting(void *arg)
{
    (void)arg;
    g_app.in_settings = 1;
    return HZ_OK;
}

static hz_err_t act_exit_setting(void *arg)
{
    (void)arg;
    g_app.in_settings = 0;
    return HZ_OK;
}

static hz_err_t act_cover_done(void *arg)
{
    (void)arg;
    return HZ_OK;
}

static hz_err_t act_idle_timeout(void *arg)
{
    (void)arg;
    g_app.last_mode = g_app.mode;
    return HZ_OK;
}

static hz_err_t act_wake(void *arg)
{
    (void)arg;
    g_app.mode = g_app.last_mode;
    g_app.idle_sec = 0;
    return HZ_OK;
}

/*===========================================================================
 * Per-state action tables (target_state=HZ_STATE_NONE means no auto-transition)
 *===========================================================================*/
static const hz_action_t s_actions_ready[] = {
    { ACT_START,         "start",          act_start,          STATE_WORKING  },
    { ACT_ENTER_SETTING, "enter_setting",  act_enter_setting,  STATE_SETTINGS },
    { ACT_IDLE_TIMEOUT,  "idle_timeout",   act_idle_timeout,   STATE_SLEEP    },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_working[] = {
    { ACT_STOP,          "stop",           act_stop,           HZ_STATE_NONE  },
    { ACT_MODE_NEXT,     "mode_next",      act_mode_next,      HZ_STATE_NONE  },
    { ACT_IDLE_TIMEOUT,  "idle_timeout",   act_idle_timeout,   STATE_SLEEP    },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_cover[] = {
    { ACT_COVER_DONE,    "cover_done",     act_cover_done,     STATE_WORKING  },
    { ACT_STOP,          "stop",           act_stop,           HZ_STATE_NONE  },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_stats[] = {
    { ACT_START,         "start",          act_start,          STATE_WORKING  },
    { ACT_IDLE_TIMEOUT,  "idle_timeout",   act_idle_timeout,   STATE_SLEEP    },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_cancel[] = {
    { ACT_START,         "start",          act_start,          STATE_WORKING  },
    { ACT_IDLE_TIMEOUT,  "idle_timeout",   act_idle_timeout,   STATE_SLEEP    },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_settings[] = {
    { ACT_EXIT_SETTING,  "exit_setting",   act_exit_setting,   STATE_READY    },
    { ACT_IDLE_TIMEOUT,  "idle_timeout",   act_idle_timeout,   STATE_SLEEP    },
    { 0, NULL, NULL, 0 },
};

static const hz_action_t s_actions_sleep[] = {
    { ACT_WAKE,          "wake",           act_wake,           STATE_READY    },
    { 0, NULL, NULL, 0 },
};

/*===========================================================================
 * Helper: push a screen (pop previous first if it's the same type)
 *===========================================================================*/
static void screen_switch(hz_screen_t *next)
{
    hz_screen_t *cur = hz_screen_current();
    /* Pop current before push to keep stack shallow */
    if (cur) hz_screen_pop();
    hz_screen_push(next, NULL);
}

/*===========================================================================
 * State entry callbacks
 *===========================================================================*/
static void on_ready_enter(hz_state_t from)    { (void)from; screen_switch(&g_scr_ready); }
static void on_working_enter(hz_state_t from)  { (void)from; screen_switch(&g_scr_working); }
static void on_cover_enter(hz_state_t from)    { (void)from; /* same screen as WORKING, nothing to switch */ }
static void on_stats_enter(hz_state_t from)    { (void)from; screen_switch(&g_scr_stats); }
static void on_cancel_enter(hz_state_t from)   { (void)from; screen_switch(&g_scr_stats); }
static void on_settings_enter(hz_state_t from) { (void)from; screen_switch(&g_scr_ready); }
static void on_sleep_enter(hz_state_t from)    { (void)from; screen_switch(&g_scr_sleep); }

/*===========================================================================
 * FSM state table
 *===========================================================================*/
static const hz_state_node_t s_states[] = {
    { .id = STATE_READY,    .parent = 0xFF, .name = "ready",    .on_enter = on_ready_enter,    .allowed_actions = s_actions_ready    },
    { .id = STATE_WORKING,  .parent = 0xFF, .name = "working",  .on_enter = on_working_enter,  .allowed_actions = s_actions_working  },
    { .id = STATE_COVER,    .parent = 0xFF, .name = "cover",    .on_enter = on_cover_enter,    .allowed_actions = s_actions_cover    },
    { .id = STATE_STATS,    .parent = 0xFF, .name = "stats",    .on_enter = on_stats_enter,    .allowed_actions = s_actions_stats    },
    { .id = STATE_CANCEL,   .parent = 0xFF, .name = "cancel",   .on_enter = on_cancel_enter,   .allowed_actions = s_actions_cancel   },
    { .id = STATE_SETTINGS, .parent = 0xFF, .name = "settings", .on_enter = on_settings_enter, .allowed_actions = s_actions_settings },
    { .id = STATE_SLEEP,    .parent = 0xFF, .name = "sleep",    .on_enter = on_sleep_enter,    .allowed_actions = s_actions_sleep    },
};

/*===========================================================================
 * Init
 *===========================================================================*/
void fsm_init(void)
{
    hz_fsm_init(s_states, 7);
    hz_fsm_transition(STATE_READY);
}

void fsm_tick_1s(void)
{
    hz_state_t cur = hz_fsm_current();

    /* Idle countdown (skip during WORKING — motor is running) */
    if (cur != STATE_WORKING) {
        g_app.idle_sec++;
        if (g_app.idle_sec >= 8 && cur != STATE_SLEEP) {
            g_app.idle_sec = 0;
            hz_fsm_transition(STATE_SLEEP);
            return;
        }
    } else {
        g_app.idle_sec = 0; /* Reset idle while working */
    }

    /* Work timer */
    if (cur == STATE_WORKING) {
        g_app.work_sec++;
        g_app.total_sec++;
        /* Save mode as last_mode once used > 10 s (requirement G3) */
        if (g_app.work_sec == 11) {
            g_app.last_mode = g_app.mode;
        }
        if (g_app.work_sec >= 120) {
            hz_fsm_transition(STATE_COVER);
        }
    }
}

void fsm_reset_idle(void)
{
    g_app.idle_sec = 0;
}
