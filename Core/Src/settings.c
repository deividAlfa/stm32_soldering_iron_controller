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

	CheckReset();

	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
	}
	if (systemSettings.version != SETTINGSVERSION) {
		resetSettings();
		saveSettings();
	}
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
	setupPIDFromStruct();
}


void resetSettings() {
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0xFF;
	systemSettings.boost.Time = 30;
	systemSettings.boost.Temperature = 400;
	systemSettings.sleep.Time = 120;
	systemSettings.sleep.Temperature = 200;
	systemSettings.standby.Time = 300;
	systemSettings.UserTemperature = 320;

#ifdef JBC
	for(uint8_t x = 0; x < 10; ++x) {
		systemSettings.ironTips[x].calADC_At_200 = 390;
		systemSettings.ironTips[x].calADC_At_300 = 625;
		systemSettings.ironTips[x].calADC_At_400 = 883;
		systemSettings.ironTips[x].PID.Kp = 0.0028;
		systemSettings.ironTips[x].PID.Ki = 0.0018;
		systemSettings.ironTips[x].PID.Kd = 0.00007;
		systemSettings.ironTips[x].PID.min = 0;
		systemSettings.ironTips[x].PID.max = 1;
		systemSettings.ironTips[x].PID.maxI = 200;
		systemSettings.ironTips[x].PID.minI = -50;
	}
	systemSettings.currentNumberOfTips = 2;
	systemSettings.currentTip = 1;
	systemSettings.sleep.sleepTemperature = 150;
	strcpy(systemSettings.ironTips[1].name, "010");
#else

	strcpy(systemSettings.ironTips[0].name, "DFLT");

	for(uint8_t x = 0; x < 10; ++x) {
		systemSettings.ironTips[x].calADC_At_200 = 1300;
		systemSettings.ironTips[x].calADC_At_300 = 2000;
		systemSettings.ironTips[x].calADC_At_400 = 3000;
		systemSettings.ironTips[x].PID.Kp = 0.0003;
		systemSettings.ironTips[x].PID.Kd = 0;
		systemSettings.ironTips[x].PID.Ki = 0.0025;
		systemSettings.ironTips[x].PID.min = 0;
		systemSettings.ironTips[x].PID.max = 1;
		systemSettings.ironTips[x].PID.maxI = 200;
		systemSettings.ironTips[x].PID.minI = -50;
	}
	systemSettings.currentNumberOfTips = 4;
	systemSettings.currentTip = 1;
	systemSettings.ironTips[1].calADC_At_200 = 1221;
	systemSettings.ironTips[1].calADC_At_300 = 1904;
	systemSettings.ironTips[1].calADC_At_400 = 2586;
	strcpy(systemSettings.ironTips[1].name, "B  ");
	systemSettings.ironTips[1].PID.max = 1;
	systemSettings.ironTips[1].PID.min = 0;
	systemSettings.ironTips[1].PID.Kp = 0.0040069999999999999;
	systemSettings.ironTips[1].PID.Ki = 0.003106000000000002;
	systemSettings.ironTips[1].PID.Kd = 0.00007;
	systemSettings.ironTips[1].PID.maxI = 200;
	systemSettings.ironTips[1].PID.minI = -50;
	systemSettings.ironTips[2].calADC_At_200 = 1463;
	systemSettings.ironTips[2].calADC_At_300 = 2313;
	systemSettings.ironTips[2].calADC_At_400 = 3162;
	strcpy(systemSettings.ironTips[2].name, "BC2");
	systemSettings.ironTips[2].PID.max = 1;
	systemSettings.ironTips[2].PID.min = 0;
	systemSettings.ironTips[2].PID.Kp = 0.003056;
	systemSettings.ironTips[2].PID.Ki = 0.0025000000000000001;
	systemSettings.ironTips[2].PID.Kd = 0;
	systemSettings.ironTips[2].PID.maxI = 200;
	systemSettings.ironTips[2].PID.minI = -50;
	systemSettings.ironTips[3].calADC_At_200 = 900;
	systemSettings.ironTips[3].calADC_At_300 = 1932;
	systemSettings.ironTips[3].calADC_At_400 = 3598;
	strcpy(systemSettings.ironTips[3].name, "D24");
	systemSettings.ironTips[3].PID.max = 1;
	systemSettings.ironTips[3].PID.min = 0;
	systemSettings.ironTips[3].PID.Kp = 0.02;
	systemSettings.ironTips[3].PID.Ki = 0.0025000000000000001;
	systemSettings.ironTips[3].PID.Kd = 0;
	systemSettings.ironTips[3].PID.maxI = 200;
	systemSettings.ironTips[3].PID.minI = -50;

#endif
}

void CheckReset(void){
	if(!BUTTON_input){
	 setContrast(255);
	 UG_FillScreen(C_BLACK);
	 UG_FontSelect(&FONT_8X14_reduced);
	 UG_SetForecolor(C_WHITE);
	 UG_SetBackcolor(C_BLACK);
	 UG_PutString(10,20,"HOLD BUTTON");
	 UG_PutString(20,32,"TO RESET");
	 UG_PutString(18,44,"DEFAULTS!!");
	 update_display();

	 uint16_t ResetTimer= HAL_GetTick();
	 while(!BUTTON_input){
		 if((HAL_GetTick()-ResetTimer)>5000){
			 resetSettings();
			 saveSettings();
			 UG_FillScreen(C_BLACK);
			 UG_PutString(40,15,"RESET!");
			 UG_PutString(0,40,"RELEASE BUTTON");
			 UG_Update();
			 update_display();
			 while(!BUTTON_input){;}
			 ResetTimer= HAL_GetTick();
			 while((HAL_GetTick()-ResetTimer)<1000){;}
			 NVIC_SystemReset();
		 }
	  }
   }
}
