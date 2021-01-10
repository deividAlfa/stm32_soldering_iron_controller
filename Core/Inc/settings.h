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

#define SETTINGSVERSION 11 /*Change this if you change the struct below to prevent people getting out of sync*/
#define FLASH_ADDR (0x8000000 + ((FLASH_SZ-1)*1024))	// Last 1KB flash (Minimum erase size, page size=1KB)

enum {Tip_T12=0,Tip_C245=1,Tip_C210=2,Tip_None=0xff};
typedef struct{
	uint32_t checksum;
	uint8_t version;				//Used to track if a reset is needed on firmware upgrade
	uint8_t contrast;
	uint8_t OledFix;
	ironSettings_t boost;
	ironSettings_t sleep;
	ironSettings_t standby;
	tipData ironTips[10];
	uint8_t currentNumberOfTips;
	uint8_t currentTip;
	uint16_t UserSetTemperature;
	uint16_t pwmPeriod;
	uint16_t pwmDelay;
	uint16_t noIronValue;
	uint16_t noIronDelay;
	uint16_t guiUpdateDelay;
	uint8_t TipType;
	uint8_t saveSettingsDelay;
	uint8_t initMode;
	bool tempUnit;
	bool buzzEnable;
	bool wakeEncoder;
}systemSettings_t;
extern systemSettings_t systemSettings;
extern systemSettings_t* flashSettings;
void saveSettings();
void restoreSettings();
uint32_t ChecksumSettings(systemSettings_t* Settings);
void resetSettings();
void resetTips(void);

#endif /* SETTINGS_H_ */
