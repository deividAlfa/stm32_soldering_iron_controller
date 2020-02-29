/*
 * settings.c
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 *      Modified on: Sep 13, 2019
 *      Author: jose
 */

#include "settings.h"
#include <string.h>
#include "crc16.h"
#include "assert.h"
//				.calADC_At_200 = 1300	,
//				.calADC_At_300 = 2000	,
//				.calADC_At_400 = 3000	,
//				.PID.Kp = 0.0003		,
//				.PID.Kd = 0				,
//				.PID.Ki = 0.0025		,
//				.PID.min = 0		,
//				.PID.max = 1			,
//				.PID.maxI = 200			,
//				.PID.minI = -50			,
//				.name =  "DFLT"
#define DF_LIMS 1 , 0, 200, 0
#define DFTL_PARAMS 1300 , 2000	,3000 , { 0.003	, 0.025, 0,  DF_LIMS }
//systemSettings_t systemSettings = {
//		.ironTips[0 ... 9] = {
//				"DFLT", DFTL_PARAMS
//			},
//};

systemSettings_t systemSettings = { .ironTips[0 ... MAX_IRON_TIPS - 1] = { "DFLT", DFTL_PARAMS }, };

#define FLASH_ADDR ( (FLASH_BANK1_END+1)-FLASH_PAGE_SIZE)/*Flash start OR'ed with the maximum amount of flash - 256 bytes*/

_Static_assert( (sizeof(systemSettings_t)<=FLASH_PAGE_SIZE) , "systemSettings_t dont fit in FLASH_PAGE_SIZE");

void saveSettings() {
	HAL_FLASH_Unlock(); //unlock flash writing
	FLASH_EraseInitTypeDef erase;
	erase.NbPages = 1;
	erase.PageAddress = FLASH_ADDR;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;
	uint32_t error;

	uint16_t calc_crc16 = crc16_compute((uint8_t*) &systemSettings, offsetof(systemSettings_t, crc16), NULL);
	systemSettings.crc16 = calc_crc16;
	HAL_FLASHEx_Erase(&erase, &error);
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_ADDR + (i * 2), data[i]);
	}
	HAL_FLASH_Lock();
}

void restoreSettings() {
	//memcpy(&systemSettings, (void*) FLASH_ADDR, sizeof(systemSettings));
	uint16_t *data = (uint16_t*) &systemSettings;
	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
	}
	uint16_t calc_crc16 = crc16_compute((uint8_t*) &systemSettings, offsetof(systemSettings_t, crc16), NULL);
	//resetSettings();
	if (systemSettings.version != SETTINGSVERSION || calc_crc16 != systemSettings.crc16) {
		resetSettings();
		saveSettings();
	}

}

void resetSettings() {
	memset(&systemSettings, 0, sizeof(systemSettings_t));
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0x7F;
	systemSettings.boost.temperature = 400;
	systemSettings.boost.time = 60;
	systemSettings.sleep.sleepTime = 120;
	systemSettings.sleep.standbyTime = 55;
	systemSettings.sleep.sleepTemperature = 100;
	systemSettings.setTemperature = 320;
	systemSettings.currentTip = 2;
	systemSettings.sleep.sleepTemperature = 100;

//	for(uint8_t x = 0; x < 10; ++x) {
//		tipData a = {
//				.calADC_At_200 = 1300	,
//				.calADC_At_300 = 2000	,
//				.calADC_At_400 = 3000	,
//				.PID.Kp = 0.0003		,
//				.PID.Kd = 0				,
//				.PID.Ki = 0.0025		,
//				.PID.min = 0		,
//				.PID.max = 1			,
//				.PID.maxI = 200			,
//				.PID.minI = -50			,
//				.name =  "DFLT"
//		};
//		systemSettings.ironTips[x] = a;
//	}
	int idx = 0;
	systemSettings.ironTips[idx++] = (tipData ) { "B", 1300, 2100, 2600, { 0.0015, 0.002, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "BC2", 1200, 2000, 3100, { 0.003, 0.0025, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "BL", 1100, 1800, 2800, { 0.0015, 0.002, 0, DF_LIMS } };
	//systemSettings.ironTips[idx++] = (tipData){"BCF1"	, 1100 , 1800	,2800 , { 0.001 , 0.0005 	,   	0, DF_LIMS }};
	//systemSettings.ironTips[idx++] = (tipData){"C1"		, 1450 , 2300	,2800 , { 0.0015 , 0.003 	,   	0, DF_LIMS }};
	//systemSettings.ironTips[idx++] = (tipData){"C2"		, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData ) { "C4", 1200, 2600, 2900, { 0.002, 0.003, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "D16", 1200, 2000, 2600, { 0.003, 0.003 * 2, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "D24", 1300, 2300, 3000, { 0.003, 0.003 * 2, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "ILS", 1400, 2150, 2800, { 0.001, 0.003, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "406", 1000, 2000, 3100, { 0.001, 0.001, 0, DF_LIMS } };
	systemSettings.ironTips[idx++] = (tipData ) { "403", 1000, 2000, 3100, { 0.001, 0.001, 0, DF_LIMS } };

	systemSettings.currentNumberOfTips = idx;

}
