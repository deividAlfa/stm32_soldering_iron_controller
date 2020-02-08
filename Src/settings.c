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

#define DFTL_PARAMS 1300 , 2000	,3000 , { 0.003	, 0.025, 0 ,  1 , 0, 200, -50 }
systemSettings_t systemSettings = {
		.ironTips[0 ... 9] = {
				"DFLT", DFTL_PARAMS
			},
};



#define FLASH_ADDR (0x8000000|64512)/*Flash start OR'ed with the maximum amount of flash - 256 bytes*/
//#define JBC
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
//	uint16_t *data = (uint16_t*) &systemSettings;
//	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
//		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
//	}
		resetSettings();	
//	if (systemSettings.version != SETTINGSVERSION) {
//		resetSettings();
//		saveSettings();
//	}

}


void resetSettings() {
	memset( &systemSettings , 0, sizeof(systemSettings_t) );
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0x7F;
	systemSettings.boost.temperature = 400;
	systemSettings.boost.time = 60;
	systemSettings.sleep.sleepTime = 120;
	systemSettings.sleep.standbyTime = 5;
	systemSettings.sleep.sleepTemperature = 100;
	systemSettings.setTemperature = 320;
	systemSettings.currentTip = 0;
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
	systemSettings.ironTips[idx++] = (tipData){"B"		, 1300 , 2100	,2586 , { 0.003	, 0.003*6	 , 		 0,  1 , 0, 200, -50} };
	systemSettings.ironTips[idx++] = (tipData){"BL"		, 1100 , 1800	,2800 , { 0.0015 , 0.003*2	,   0.001 ,  1 , 0, 200, -50 }};
	systemSettings.ironTips[idx++] = (tipData){"BC2"	, 1200 , 2000	,3100 , { 0.003	, 0.0025*5	, 		0 ,  1 , 0, 200, -50 }};

	systemSettings.ironTips[idx++] = (tipData){"BCF1"	, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData){"D24"	, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData){"C1"		, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData){"C2"		, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData){"C4"		, DFTL_PARAMS };
	systemSettings.ironTips[idx++] = (tipData){"ILS"	, 1300 , 2100	,2800 , { 0.001	, 0.003		,   0 ,  1 , 0, 200, -50 }};
	systemSettings.ironTips[idx++] = (tipData){"D24"	, DFTL_PARAMS };


	systemSettings.currentNumberOfTips = idx;

}
