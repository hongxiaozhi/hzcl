#include "init/timers.h"
#include "app.h"
#include "hz_timer.h"
#include "hz_fsm.h"
#include "hz_config_mgr.h"

static hz_timer_t s_pet_timer;
static hz_timer_t s_auto_save_timer;

static void pet_timer_cb(void *arg)
{
    (void)arg;
    hz_fsm_execute_action(ACT_TICK, NULL);
}

static void auto_save_cb(void *arg)
{
    (void)arg;
    hz_config_mgr_save();
}

void init_timers(void)
{
    s_pet_timer.interval_ms = 1000; s_pet_timer.remaining_ms = 0;
    s_pet_timer.callback = pet_timer_cb; s_pet_timer.arg = NULL;
    s_pet_timer.repeat = 1; s_pet_timer.active = 0; s_pet_timer.next = NULL;
    hz_timer_start(&s_pet_timer);

    s_auto_save_timer.interval_ms = g_app_cfg.auto_save_ms; s_auto_save_timer.remaining_ms = 0;
    s_auto_save_timer.callback = auto_save_cb; s_auto_save_timer.arg = NULL;
    s_auto_save_timer.repeat = 1; s_auto_save_timer.active = 0; s_auto_save_timer.next = NULL;
    hz_timer_start(&s_auto_save_timer);
}
