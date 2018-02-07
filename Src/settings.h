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

#define SETTINGSVERSION 2 /*Change this if you change the struct below to prevent people getting out of sync*/

struct systemSettings {
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	ironBoost_t boost;
	ironSleep_t sleep;
	tipData ironTips[10];
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t setTemperature;
} systemSettings;

void saveSettings();
void restoreSettings();
void resetSettings();

#endif /* SETTINGS_H_ */
