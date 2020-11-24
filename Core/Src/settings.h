/*
 * settings.h
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"
#include "pid.h"
#include "iron.h"
#include "ugui.h"
#include "ssd1306.h"
#include "tempsensors.h"
#include <stdint.h>

#define SETTINGSVERSION 2 /*Change this if you change the struct below to prevent people getting out of sync*/

struct systemSettings {
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	ironSettings_t boost;
	ironSettings_t sleep;
	ironSettings_t standby;
	tipData ironTips[10];
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t UserTemperature;
} systemSettings;

void saveSettings();
void restoreSettings();
void resetSettings();
void CheckReset(void);

#endif /* SETTINGS_H_ */
