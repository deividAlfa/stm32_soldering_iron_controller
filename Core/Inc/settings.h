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

#define SETTINGSVERSION 10 /*Change this if you change the struct below to prevent people getting out of sync*/

struct systemSettings {		//Size: 480 Bytes (Reserved in flash: 512)
	uint16_t checksum;
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	ironSettings_t boost;
	ironSettings_t sleep;
	ironSettings_t standby;
	tipData ironTips[7];
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t UserSetTemperature;
	uint16_t pwmPeriod;
	uint16_t pwmDelay;
	uint16_t noIronValue;
	uint16_t noIronDelay;
	uint16_t guiUpdateDelay;
	uint8_t tempUnit;
	uint8_t saveSettingsDelay;
} systemSettings;

void saveSettings();
void restoreSettings();
uint16_t ChecksumSettings(void);
void resetSettings();

#endif /* SETTINGS_H_ */
