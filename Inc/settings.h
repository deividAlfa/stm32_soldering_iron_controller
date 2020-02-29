/*
 * settings.h
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "stm32f1xx_hal.h"
#include "pid.h"
#include "iron.h"
#include "tempsensors.h"
#include <stdint.h>
#include "stm32f1xx_hal_flash.h"

#define SETTINGSVERSION 66 /*Change this if you change the struct below to prevent people getting out of sync*/

#define MAX_IRON_TIPS 20
typedef struct {
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	ironBoost_t boost;
	ironSleep_t sleep;
	tipData ironTips[MAX_IRON_TIPS];
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t setTemperature;
	uint16_t crc16;
} systemSettings_t;

extern systemSettings_t systemSettings;

void saveSettings();
void restoreSettings();
void resetSettings();

#endif /* SETTINGS_H_ */
