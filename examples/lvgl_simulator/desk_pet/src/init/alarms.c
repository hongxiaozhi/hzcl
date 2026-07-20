#include "init/alarms.h"
#include "app.h"
#include "hz_alarm.h"

static const hz_alarm_cfg_t s_alarm_defs[] = {
    { .id = ALARM_HUNGER_HIGH,        .name = "HUNGER_HIGH",        .level = HZ_ALARM_WARNING,  .display_timeout_ms = 5000,  .auto_recover = true,  .recover_delay_ms = 5000  },
    { .id = ALARM_ENERGY_LOW,         .name = "ENERGY_LOW",         .level = HZ_ALARM_WARNING,  .display_timeout_ms = 5000,  .auto_recover = true,  .recover_delay_ms = 5000  },
    { .id = ALARM_SICK_PROLONGED,     .name = "SICK_PROLONGED",     .level = HZ_ALARM_CRITICAL, .display_timeout_ms = 0,     .auto_recover = false, .recover_delay_ms = 0     },
    { .id = ALARM_ACHIEVEMENT_UNLOCK, .name = "ACHIEVEMENT_UNLOCK", .level = HZ_ALARM_INFO,     .display_timeout_ms = 3000,  .auto_recover = true,  .recover_delay_ms = 3000  },
    { .id = ALARM_MODE_AUTO_SWITCH,   .name = "MODE_AUTO_SWITCH",   .level = HZ_ALARM_INFO,     .display_timeout_ms = 2000,  .auto_recover = true,  .recover_delay_ms = 2000  },
    { .id = ALARM_LOW_ACTIVITY,       .name = "LOW_ACTIVITY",       .level = HZ_ALARM_INFO,     .display_timeout_ms = 4000,  .auto_recover = true,  .recover_delay_ms = 4000  },
};

void init_alarms(void)
{
    hz_alarm_init(s_alarm_defs, ALARM_COUNT);
}
