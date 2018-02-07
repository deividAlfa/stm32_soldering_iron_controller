/*
 * settings.c
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#include "settings.h"
#include <string.h>
#define FLASH_ADDR (0x8000000|64512)/*Flash start OR'ed with the maximum amount of flash - 256 bytes*/

void saveSettings() {
	HAL_FLASH_Unlock(); //unlock flash writing
	FLASH_EraseInitTypeDef erase;
	erase.NbPages = 1;
	erase.PageAddress = FLASH_ADDR;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t error;
	HAL_FLASHEx_Erase(&erase, &error);
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_ADDR + (i * 2), data[i]);
	}
	HAL_FLASH_Lock();
}

void restoreSettings() {
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
	}
	if (systemSettings.version != SETTINGSVERSION) {
		resetSettings();
		saveSettings();
	}

}

void resetSettings() {
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0x7F;
	systemSettings.boost.temperature = 400;
	systemSettings.boost.time = 60;
	systemSettings.sleep.sleepTime = 0;
	systemSettings.sleep.standbyTime = 5;
	systemSettings.sleep.sleepTemperature = 100;
	systemSettings.currentTip = 0;
	systemSettings.currentNumberOfTips = 1;
	strcpy(systemSettings.ironTips[0].name, "DFLT");
	for(uint8_t x = 0; x < 10; ++x) {
		systemSettings.ironTips[x].calADC_At_200 = 976;//793;
		systemSettings.ironTips[x].calADC_At_300 = 1536;//1323;
		systemSettings.ironTips[x].calADC_At_400 = 1966;//1900;
		systemSettings.ironTips[x].PID.Kp = 0.02;
		systemSettings.ironTips[x].PID.Kd = 0;//0.00025182;
		systemSettings.ironTips[x].PID.Ki = 0.0025;//0.1;//.0038516;
		systemSettings.ironTips[x].PID.min = 0;
		systemSettings.ironTips[x].PID.max = 1;
		systemSettings.ironTips[x].PID.maxI = 200;
		systemSettings.ironTips[x].PID.minI = -50;
	}
	systemSettings.setTemperature = 300;
}
