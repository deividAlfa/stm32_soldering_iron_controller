/*
 * addon_fume_extractor.c
 *
 *  Created on: Apr 18, 2022
 *      Author: KocsisV
 */

#include "main.h"
#include "addon_fume_extractor.h"
#include "iron.h"

#ifdef ENABLE_ADDON_FUME_EXTRACTOR

#define AFTERRUN_IN_SEC 10
#define AFTERRUN (AFTERRUN_IN_SEC * 1000u)

void handleAddonFumeExtractor()
{
  static uint32_t lastActiveTime   = 0u;
  static bool     afterrunRequired = false;

  bool     extractorRequired = false;
  uint32_t currentTimeStamp  = HAL_GetTick();
  uint8_t  currentMode       = getCurrentMode();

  if((currentMode == mode_run) ||
     (currentMode == mode_boost))
  {
    lastActiveTime = currentTimeStamp;
    extractorRequired = true;
    afterrunRequired = true;
  }

  if(afterrunRequired &&
     ((lastActiveTime  + AFTERRUN) < currentTimeStamp))
  {
    // iron has not been in the active state for AFTERRUN_IN_SEC of time
    afterrunRequired = false;
  }

  HAL_GPIO_WritePin(EXTRACTOR_GPIO_Port, EXTRACTOR_Pin, (extractorRequired || afterrunRequired) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

#endif
