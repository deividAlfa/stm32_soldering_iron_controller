/*
 * addon_switch_off_reminder.c
 *
 *  Created on: 2022. ápr. 20.
 *      Author: KocsisV
 */

#include "addon_switch_off_reminder.h"
#include "settings.h"
#include "buzzer.h"
#include "iron.h"

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER

#define MSEC_IN_SEC 60000

void handleAddonSwitchOffReminder()
{
  static uint32_t lastActiveTime = 0u;
  static uint32_t lastBeepTime   = 0u;

  uint32_t const currentTimeStamp = HAL_GetTick();
  uint32_t const beepPeriodInMsec = systemSettings.addonSettings.swOffReminderPeriod * MSEC_IN_SEC;

  if((systemSettings.addonSettings.swOffReminderEnabled) &&
     (getCurrentMode() == mode_sleep))
  {
    uint32_t const elapsedSinceActive                 = currentTimeStamp - lastActiveTime;
    uint32_t const swOffReminderInactivityDelayInMsec = systemSettings.addonSettings.swOffReminderInactivityDelay * MSEC_IN_SEC;

    if(elapsedSinceActive > swOffReminderInactivityDelayInMsec)
    {
      // station is in sleep mode longer than the configured time

      if((lastBeepTime + beepPeriodInMsec) <= currentTimeStamp)
      {
        lastBeepTime = currentTimeStamp;

        switch (systemSettings.addonSettings.swOffReminderBeepType) {
          case switch_off_reminder_short_beep:
          {
            buzzer_short_beep();
            break;
          }
          case switch_off_reminder_medium_beep:
          {
            buzzer_long_beep();
            break;
          }
          case switch_off_reminder_long_beep:
          default:
          {
            buzzer_fatal_beep();
            break;
          }
        }
      }
    }
  }
  else
  {
    lastActiveTime = currentTimeStamp;
    lastBeepTime   = currentTimeStamp - beepPeriodInMsec;
  }
}

#endif
