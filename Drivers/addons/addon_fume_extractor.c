/*
 * addon_fume_extractor.c
 *
 *  Created on: Apr 18, 2022
 *      Author: KocsisV
 */

#include "main.h"
#include "addon_fume_extractor.h"
#include "iron.h"
#include "settings.h"

#ifdef ENABLE_ADDON_FUME_EXTRACTOR

void handleAddonFumeExtractor()
{
  static uint32_t lastActiveTime    = 0u;
  static bool     extractorRequired = false;

  switch (systemSettings.addonSettings.fumeExtractorMode)
  {
    case fume_extractor_mode_disabled:
    default:
    {
      // addon disabled
      extractorRequired = false;
      HAL_GPIO_WritePin(EXTRACTOR_GPIO_Port, EXTRACTOR_Pin, GPIO_PIN_RESET);
      break;
    }

    case fume_extractor_mode_auto:
    {
      uint32_t const currentTimeStamp = HAL_GetTick();

      if(getCurrentMode() >= mode_run)
      {
        // if in a mode where extraction is required
        lastActiveTime = currentTimeStamp;
        extractorRequired = true;
      }

      if(extractorRequired &&
         ((lastActiveTime + (5000u * systemSettings.addonSettings.fumeExtractorAfterrun)) < currentTimeStamp))
      {
        // iron has not been in the active state for the configured amount of time, turn it off
        extractorRequired = false;
      }

      HAL_GPIO_WritePin(EXTRACTOR_GPIO_Port, EXTRACTOR_Pin, extractorRequired ? GPIO_PIN_SET : GPIO_PIN_RESET);
      break;
    }

    case fume_extractor_mode_always_on:
    {
      // extractor is always on
      extractorRequired = false; // set to false to override after run
      HAL_GPIO_WritePin(EXTRACTOR_GPIO_Port, EXTRACTOR_Pin, GPIO_PIN_SET);
      break;
    }
  }

}

#endif
