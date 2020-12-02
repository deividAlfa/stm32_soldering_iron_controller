/*
 * settings.c
 *
 *  Created on: Sep 13, 2017
 *      Author: jose
 */

#include "settings.h"
#include <string.h>
#define FLASH_ADDR (0x8000000|65024)/*Flash start OR'ed with the maximum amount of flash - 512 bytes*/
void Checksum_error(void);
void Flash_error(void);
void Button_reset(void);
void Diag_init(void);
void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);

void saveSettings() {
	uint16_t *data = (uint16_t*) &systemSettings;
	uint32_t error=0;
	// Calculate new checksum
	systemSettings.checksum=ChecksumSettings();

	HAL_FLASH_Unlock(); //unlock flash writing
	FLASH_EraseInitTypeDef erase;
	erase.NbPages = 1;
	erase.PageAddress = FLASH_ADDR;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;

	HAL_FLASHEx_Erase(&erase, &error);
	if(error!=0xFFFFFFFF){
		Flash_error();
		HAL_FLASH_Lock();
		return;
	}

	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_ADDR + (i * 2), data[i]);
	}
	HAL_FLASH_Lock();
}

void restoreSettings() {
	uint16_t *data = (uint16_t*) &systemSettings;
	volatile uint16_t checksum;
#ifdef NOSAVESETTINGS				// Stop erasing the flash every time while in debug mode
	resetSettings();
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
	setupPIDFromStruct();
	return;
#endif
	Button_reset();
	for (uint16_t i = 0; i < (sizeof(systemSettings) / 2); i++) {		// Load data from flash
		data[i] = *(uint16_t *) (FLASH_ADDR + (i * 2));
	}
	checksum=ChecksumSettings();										// Compare loaded checksum with calculated checksum
	if( (systemSettings.version != SETTINGSVERSION) || (checksum!=systemSettings.checksum) ){
		Checksum_error();
	}
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
	setupPIDFromStruct();
}
uint16_t ChecksumSettings(void){
	uint16_t *data = (uint16_t*) &systemSettings;
	volatile uint16_t i;
	volatile uint16_t checksum=0,oldchecksum;
	oldchecksum=systemSettings.checksum;
	systemSettings.checksum = 0;
	for (i= 0; i < (sizeof(systemSettings) / 2); i++) {
		checksum += data[i];
	}
	systemSettings.checksum=oldchecksum;
	return checksum;
}

void resetSettings() {
	systemSettings.version = SETTINGSVERSION;
	systemSettings.contrast = 0xFF;
	systemSettings.boost.Time = 30;
	systemSettings.boost.Temperature = 400;
	systemSettings.sleep.Time = 120;
	systemSettings.sleep.Temperature = 200;
	systemSettings.standby.Time = 300;
	systemSettings.UserSetTemperature = 320;
	systemSettings.pwmPeriod=19999;
	systemSettings.pwmDelay=99;
	systemSettings.noIronValue=3500;
	systemSettings.noIronDelay=500;
	systemSettings.guiUpdateDelay=200;
	systemSettings.tempUnit=Unit_Celsius;
	systemSettings.saveSettingsDelay=10;

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

	for(uint8_t x = 0; x < ( sizeof(systemSettings.ironTips)/sizeof(systemSettings.ironTips[0])); ++x) {
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



void Diag_init(void){
	setContrast(255);
	UG_FillScreen(C_BLACK);
	UG_FontSelect(&FONT_10X16_reduced);
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
}
void Flash_error(void){
	Diag_init();
	UG_PutString(2,0,"ERROR WHILE");//11
	UG_PutString(24,16,"WRITING");//10
	UG_PutString(19,32,"SETTINGS");//7
	update_display();

	// Long delay with countdown to show the error before resuming
	ErrCountDown(5,117,50);	// Not critical error. Continue
}

void Checksum_error(void){
	Diag_init();
	UG_PutString(8,0,"CHKSUM ERR");//10
	UG_PutString(13,16,"RESETTING");//9
	UG_PutString(19,32,"DEFAULTS");//10
	update_display();
	ErrCountDown(2,117,50);
	resetSettings();
	saveSettings();
}

void Button_reset(void){
	uint16_t ResetTimer= HAL_GetTick();
	if(!BUTTON_input){
		Diag_init();
		UG_PutString(2,10,"HOLD BUTTON");//11
		UG_PutString(19,26,"TO RESET");//8
		UG_PutString(8,42,"DEFAULTS!!");//10
		update_display();
		while(!BUTTON_input){
			HAL_IWDG_Refresh(&hiwdg);
			if((HAL_GetTick()-ResetTimer)>5000){
				resetSettings();
				saveSettings();
				UG_FillScreen(C_BLACK);
				UG_PutString(13,10,"RESETTED!");//9
				UG_PutString(24,26,"RELEASE");//7
				UG_PutString(8,42,"BUTTON NOW");// 10
				UG_Update();
				update_display();
				while(!BUTTON_input){
					HAL_IWDG_Refresh(&hiwdg);
				}
				ResetTimer= HAL_GetTick();
				while((HAL_GetTick()-ResetTimer)<1000){
					HAL_IWDG_Refresh(&hiwdg);

				}
				NVIC_SystemReset();
			}
		}
	}
}
//Simple function. Max 99 seconds countdown.
void ErrCountDown(uint8_t Start,uint8_t  xpos, uint8_t ypos){
	uint32_t timErr = 0;
	char str[3];
	char cleanStr[3]= {' ', ' ', 0};
	uint8_t length;
	if(Start>99){Start=99;}
	if(Start>9){
		length=2;
	}
	else{
		length=1;
		cleanStr[1]=0;
	}

	HAL_Delay(20);				// Dirty fix to ensure Oled DMA transfer has ended before writing to the buffer
	while(Start){
		timErr=HAL_GetTick();
		UG_PutString(xpos,ypos,&cleanStr[0]);
		sprintf(&str[0],"%*d",length-1,Start--);
		UG_PutString(xpos,ypos,&str[0]);
		update_display();
		while( (HAL_GetTick()-timErr)<999 ){
			HAL_IWDG_Refresh(&hiwdg);			// Clear watchdog
		}
	}
}
