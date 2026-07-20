#include "init/fsm.h"
#include "app.h"
#include "hz_fsm.h"
#include "hz_event.h"
#include "hz_alarm.h"
#include <string.h>

/*===========================================================================
 * Global data (extern'd in app.h)
 *===========================================================================*/
pet_status_t          g_pet          = { 30, 80, 60, 100 };
pet_state_counters_t  g_counters     = { 0 };

/*===========================================================================
 * Action Handlers
 *===========================================================================*/
static hz_err_t act_feed_handler(void *arg)
{
    (void)arg;
    g_pet.hunger = 0;
    g_pet.mood += 10;
    if (g_pet.mood > 100) g_pet.mood = 100;
    g_counters.eating_duration = 0;
    app_notify_activity();
    app_log_action("Fed the pet");
    return HZ_OK;
}

static hz_err_t act_pet_handler(void *arg)
{
    (void)arg;
    g_pet.mood = 100;
    g_counters.happy_duration = 0;
    app_notify_activity();
    app_log_action("Petted the pet");
    return HZ_OK;
}

static hz_err_t act_sleep_handler(void *arg)   { (void)arg; app_log_action("Fell asleep"); return HZ_OK; }
static hz_err_t act_wake_handler(void *arg)    { (void)arg; app_log_action("Woke up"); return HZ_OK; }
static hz_err_t act_clean_handler(void *arg)
{
    (void)arg;
    g_counters.cleaning_duration = 0;
    app_notify_activity();
    app_log_action("Cleaned the pet");
    return HZ_OK;
}
static hz_err_t act_train_handler(void *arg)
{
    (void)arg;
    g_counters.training_duration = 0;
    app_notify_activity();
    app_log_action("Started training");
    return HZ_OK;
}
static hz_err_t act_stop_handler(void *arg)
{
    (void)arg;
    app_log_action("Stopped activity");
    return HZ_OK;
}
static hz_err_t act_tick_handler(void *arg) { (void)arg; pet_status_tick(); return HZ_OK; }

/*===========================================================================
 * Per-State Action Tables
 *===========================================================================*/
static const hz_action_t s_actions_idle[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING   },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_GO_SLEEP, "go_sleep", act_sleep_handler, PET_STATE_SLEEPING },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_happy[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HYPER },
    { ACT_GO_SLEEP, "go_sleep", act_sleep_handler, PET_STATE_SLEEPING },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_eating[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sleeping[] = {
    { ACT_PET, "pet", act_pet_handler, PET_STATE_IDLE },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sad[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_sick[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_CLEAN, "clean", act_clean_handler, PET_STATE_CLEANING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_hyper[] = {
    { ACT_FEED, "feed", act_feed_handler, PET_STATE_EATING },
    { ACT_PET, "pet", act_pet_handler, PET_STATE_HAPPY },
    { ACT_TRAIN, "train", act_train_handler, PET_STATE_TRAINING },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_training[] = {
    { ACT_STOP, "stop", act_stop_handler, PET_STATE_IDLE },
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};
static const hz_action_t s_actions_cleaning[] = {
    { ACT_TICK, "tick", act_tick_handler, HZ_STATE_NONE },
    { 0, NULL, NULL, 0 },
};

/*===========================================================================
 * State Entry Callbacks
 *===========================================================================*/
static void on_idle_enter(hz_state_t from)   { (void)from; }
static void on_happy_enter(hz_state_t from)  { (void)from; g_counters.happy_duration = 0; }
static void on_eating_enter(hz_state_t from) { (void)from; g_counters.eating_duration = 0; }
static void on_sleep_enter(hz_state_t from)  { (void)from; }
static void on_sad_enter(hz_state_t from)    { (void)from; }
static void on_sick_enter(hz_state_t from)   { (void)from; }
static void on_hyper_enter(hz_state_t from)  { (void)from; }
static void on_train_enter(hz_state_t from)  { (void)from; g_counters.training_duration = 0; }
static void on_clean_enter(hz_state_t from)  { (void)from; g_counters.cleaning_duration = 0; }

/*===========================================================================
 * FSM State Table
 *===========================================================================*/
static const hz_state_node_t s_states[] = {
    { .id = PET_STATE_IDLE,     .parent = 0xFF, .name = "idle",     .on_enter = on_idle_enter,  .allowed_actions = s_actions_idle     },
    { .id = PET_STATE_HAPPY,    .parent = 0xFF, .name = "happy",    .on_enter = on_happy_enter, .allowed_actions = s_actions_happy    },
    { .id = PET_STATE_EATING,   .parent = 0xFF, .name = "eating",   .on_enter = on_eating_enter,.allowed_actions = s_actions_eating   },
    { .id = PET_STATE_SLEEPING, .parent = 0xFF, .name = "sleeping", .on_enter = on_sleep_enter, .allowed_actions = s_actions_sleeping },
    { .id = PET_STATE_SAD,      .parent = 0xFF, .name = "sad",      .on_enter = on_sad_enter,   .allowed_actions = s_actions_sad      },
    { .id = PET_STATE_SICK,     .parent = 0xFF, .name = "sick",     .on_enter = on_sick_enter,  .allowed_actions = s_actions_sick     },
    { .id = PET_STATE_HYPER,    .parent = 0xFF, .name = "hyper",    .on_enter = on_hyper_enter, .allowed_actions = s_actions_hyper    },
    { .id = PET_STATE_TRAINING, .parent = 0xFF, .name = "training", .on_enter = on_train_enter, .allowed_actions = s_actions_training },
    { .id = PET_STATE_CLEANING, .parent = 0xFF, .name = "cleaning", .on_enter = on_clean_enter, .allowed_actions = s_actions_cleaning },
};

void init_fsm(void)
{
    hz_fsm_init(s_states, HZ_ARRAY_SIZE(s_states));
}
