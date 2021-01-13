/*
 * settings.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "settings.h"
#include "pid.h"
#include "iron.h"
#include "ugui.h"
#include "ssd1306.h"
#include "tempsensors.h"

char* profileStr[ProfileSize] = {" T12", "C245", "C210" };

__attribute__((aligned(4))) systemSettings_t systemSettings;
flashSettings_t* flashSettings = (flashSettings_t*)FLASH_ADDR;
void settingsChkErr(void);
void ProfileChkErr(void);
void Flash_error(void);
void Button_reset(void);
void Diag_init(void);
void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);

void saveSettings(bool wipeAllProfileData) {
	uint32_t error=0;
	flashSettings_t flashBuffer=*flashSettings;	//Read stored data, as everything will be erased and we don't store data for all iron tips in ram (only the current tip type)

	// Reset flag
	systemSettings.settings.notInitialized=0;

	// Compute checksum
	systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);

	// Transfer system settings to flash buffer
	flashBuffer.settingsChecksum=systemSettings.settingsChecksum;
	flashBuffer.settings=systemSettings.settings;

	if(!wipeAllProfileData){																				// If wipe all tips is not set
		if(systemSettings.settings.currentProfile!=Profile_None){											// If current tip is initialized
			if( 	(systemSettings.settings.currentProfile==Profile_T12)	||								// Check valid Profile
					(systemSettings.settings.currentProfile==Profile_C210)	||
					(systemSettings.settings.currentProfile==Profile_C245)	){

				systemSettings.Profile.notInitialized=0;																		// Reset flag
				systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);										// Compute checksum
				flashBuffer.ProfileChecksum[systemSettings.settings.currentProfile]=systemSettings.ProfileChecksum;
				memcpy(&flashBuffer.Profile[systemSettings.settings.currentProfile],&systemSettings.Profile,sizeof(Profile_t));	// Transfer system profile to flash buffer
			}
			else{
				Error_Handler();																								// Invalid tip (uncontrolled state)
			}
		}
	}
	else{																								// Wipe all tip data
		for(uint8_t x=0;x<ProfileSize;x++){
			flashBuffer.Profile[x].notInitialized=1;													// Set all profile data to "1"
			flashBuffer.ProfileChecksum[x]=0xFF;
			memset(&flashBuffer.Profile[x],0xFF,sizeof(Profile_t));
		}
	}

	//Clear watchdog before unlocking flash
	HAL_IWDG_Refresh(&HIWDG);

	HAL_FLASH_Unlock(); //unlock flash writing
	FLASH_EraseInitTypeDef erase;
	erase.NbPages = (1024*StoreSize)/FLASH_PAGE_SIZE;
	erase.PageAddress = FLASH_ADDR;
	erase.TypeErase = FLASH_TYPEERASE_PAGES;

	//Erase flash page
	if(HAL_FLASHEx_Erase(&erase, &error)!=HAL_OK){
		Flash_error();
	}
	if(error!=0xFFFFFFFF){
		Flash_error();
		HAL_FLASH_Lock();
		return;
	}
	// Ensure that the flash was erased
	for (uint16_t i = 0; i < sizeof(flashSettings_t)/2; i++) {
			if( *(uint16_t*)(FLASH_ADDR+(i*2)) != 0xFFFF){
				Flash_error();
			}
		}

	//Clear watchdog before writing
	HAL_IWDG_Refresh(&HIWDG);

	//Store settings
	for (uint16_t i = 0; i < sizeof(flashSettings_t) / 2; i++) {
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (((uint32_t)flashSettings+i*2)), *(((uint16_t*)&flashBuffer)+i) ) != HAL_OK){
			Flash_error();
		}
	}
	HAL_FLASH_Lock();

	if(!wipeAllProfileData){
		uint32_t ProfileFlash	= ChecksumProfile(&flashSettings->Profile[systemSettings.settings.currentProfile]);
		uint32_t ProfileRam		= ChecksumProfile(&systemSettings.Profile);

		// Check flash and system profiles have same checksum and type
		if((ProfileFlash != ProfileRam) ||	(flashSettings->settings.currentProfile != systemSettings.settings.currentProfile)){
			Flash_error();						// Error if data mismatch
		}
	}

	// Check flash and system settings have same checksum

	uint32_t SettingsFlash	= ChecksumSettings(&flashSettings->settings);
	uint32_t SettingsRam	= ChecksumSettings(&systemSettings.settings);
	if(SettingsFlash != SettingsRam){			// Check flash and system settings have same checksum
		Flash_error();							// Error if data mismatch
	}

}

void restoreSettings() {
#ifdef NOSAVESETTINGS				// Stop erasing the flash every time while in debug mode
	resetSystemSettings();			// TODO not tested with the new profile system
	currentPID = systemSettings.Profile.tip[systemSettings.settings.currentTip[systemSettings.settings.currentProfile]].PID;
	setupPIDFromStruct();
	return;
#endif

	if(flashSettings->settings.notInitialized){											// If flash not initialized (Erased flash is always read "1")
		resetSystemSettings();
		saveSettings(1);
	}
	else{
		Button_reset();																	// Check for button reset
	}

	systemSettings.settings=flashSettings->settings;									// Load system settings from flash
	systemSettings.settingsChecksum=flashSettings->settingsChecksum;					// Load stored checksum

	// Compare loaded checksum with calculated checksum
	if( (systemSettings.settings.version != FW_Version) || (ChecksumSettings(&systemSettings.settings)!=systemSettings.settingsChecksum) ){
		settingsChkErr();
	}

	loadProfile(systemSettings.settings.currentProfile);								// Load current tip data into system memory

	setContrast(systemSettings.settings.contrast);
}

uint32_t ChecksumSettings(settings_t* settings){
	uint32_t checksum;
	checksum = HAL_CRC_Calculate(&HCRC, (uint32_t*)settings, sizeof(settings_t)/sizeof(uint32_t) );
	return checksum;
}

uint32_t ChecksumProfile(Profile_t* profile){
	uint32_t checksum;
	checksum = HAL_CRC_Calculate(&HCRC, (uint32_t*)profile, sizeof(Profile_t)/sizeof(uint32_t));
	return checksum;
}

void resetSystemSettings(void) {
	systemSettings.settings.version = FW_Version;
	systemSettings.settings.contrast = 255;
	systemSettings.settings.OledOffset = 2;
	systemSettings.settings.noIronDelay=500;
	systemSettings.settings.guiUpdateDelay=200;
	systemSettings.settings.tempUnit=Unit_Celsius;
	systemSettings.settings.tempStep=5;
	systemSettings.settings.saveSettingsDelay=5;
	systemSettings.settings.currentProfile=Profile_None;
	systemSettings.settings.initMode=mode_normal;
	systemSettings.settings.buzzEnable=0;
	systemSettings.settings.wakeOnButton=0;
}


void resetCurrentProfile(void){
#ifdef NOSAVESETTINGS
	systemSettings.settings.currentProfile=Profile_T12;										// Force T12 when debugging
																							// TODO this is not tested with the profiles update!
#endif
	if(systemSettings.settings.currentProfile==Profile_T12){
		for(uint8_t x = 0; x < TipSize; x++) {			//TODO ++x?
			systemSettings.Profile.tip[x].calADC_At_200 = 900;
			systemSettings.Profile.tip[x].calADC_At_300 = 1500;			// These values are way lower, but better to be safe than sorry
			systemSettings.Profile.tip[x].calADC_At_400 = 2000;			// User needs to calibrate its station
			systemSettings.Profile.tip[x].PID.Kp = 0.0045;
			systemSettings.Profile.tip[x].PID.Ki = 0.0030;
			systemSettings.Profile.tip[x].PID.Kd = 0.0015;
			systemSettings.Profile.tip[x].PID.min = 0;
			systemSettings.Profile.tip[x].PID.max = 1;
			systemSettings.Profile.tip[x].PID.maxI = 200;
			systemSettings.Profile.tip[x].PID.minI = -50;
		}
		systemSettings.Profile.currentNumberOfTips= 5;
		systemSettings.Profile.currentTip = 0;
		strcpy(systemSettings.Profile.tip[0].name, "DFLT");
		strcpy(systemSettings.Profile.tip[1].name, "B   ");
		strcpy(systemSettings.Profile.tip[2].name, "BC2 ");
		strcpy(systemSettings.Profile.tip[3].name, "D24 ");
		strcpy(systemSettings.Profile.tip[4].name, "C245");				// Fake JBC tip with similar T12 sensor but gives little higher output
		systemSettings.Profile.tip[4].calADC_At_200 = 1400;
		systemSettings.Profile.tip[4].calADC_At_300 = 2300;
		systemSettings.Profile.tip[4].calADC_At_400 = 3150;


		systemSettings.Profile.boost.Time = 1;
		systemSettings.Profile.boost.Temperature = 400;
		systemSettings.Profile.sleep.Time = 5;
		systemSettings.Profile.sleep.Temperature = 150;
		systemSettings.Profile.standby.Time = 10;
		systemSettings.Profile.UserSetTemperature = 320;
		systemSettings.Profile.pwmPeriod=19999;
		systemSettings.Profile.pwmDelay=1999;
		systemSettings.Profile.noIronValue=4000;
		//saveSettings(0);
	}

	else if(systemSettings.settings.currentProfile==Profile_C245){
		for(uint8_t x = 0; x < TipSize; x++) {			//TODO ++x?
			systemSettings.Profile.tip[x].calADC_At_200 = 390;
			systemSettings.Profile.tip[x].calADC_At_300 = 625;
			systemSettings.Profile.tip[x].calADC_At_400 = 883;
			systemSettings.Profile.tip[x].PID.Kp = 0.0028;
			systemSettings.Profile.tip[x].PID.Ki = 0.0018;
			systemSettings.Profile.tip[x].PID.Kd = 0.00007;
			systemSettings.Profile.tip[x].PID.min = 0;
			systemSettings.Profile.tip[x].PID.max = 1;
			systemSettings.Profile.tip[x].PID.maxI = 200;
			systemSettings.Profile.tip[x].PID.minI = -50;
		}
		systemSettings.Profile.currentNumberOfTips = 1;
		systemSettings.Profile.currentTip = 0;
		strcpy(systemSettings.Profile.tip[0].name, "245");

		systemSettings.Profile.boost.Time = 1;
		systemSettings.Profile.boost.Temperature = 400;
		systemSettings.Profile.sleep.Time = 5;
		systemSettings.Profile.sleep.Temperature = 150;
		systemSettings.Profile.standby.Time = 10;
		systemSettings.Profile.UserSetTemperature = 320;
		systemSettings.Profile.pwmPeriod=19999;
		systemSettings.Profile.pwmDelay=1999;
		systemSettings.Profile.noIronValue=4000;
		//saveSettings(0);
	}

	else if(systemSettings.settings.currentProfile==Profile_C210){
		for(uint8_t x = 0; x < TipSize; x++) {			//TODO ++x?
			systemSettings.Profile.tip[x].calADC_At_200 = 390;
			systemSettings.Profile.tip[x].calADC_At_300 = 625;
			systemSettings.Profile.tip[x].calADC_At_400 = 883;
			systemSettings.Profile.tip[x].PID.Kp = 0.0028;
			systemSettings.Profile.tip[x].PID.Ki = 0.0018;
			systemSettings.Profile.tip[x].PID.Kd = 0.00007;
			systemSettings.Profile.tip[x].PID.min = 0;
			systemSettings.Profile.tip[x].PID.max = 1;
			systemSettings.Profile.tip[x].PID.maxI = 200;
			systemSettings.Profile.tip[x].PID.minI = -50;
		}
		systemSettings.Profile.currentNumberOfTips = 1;
		systemSettings.Profile.currentTip = 0;
		strcpy(systemSettings.Profile.tip[0].name, "210");

		systemSettings.Profile.boost.Time = 1;
		systemSettings.Profile.boost.Temperature = 400;
		systemSettings.Profile.sleep.Time = 5;
		systemSettings.Profile.sleep.Temperature = 150;
		systemSettings.Profile.standby.Time = 10;
		systemSettings.Profile.UserSetTemperature = 320;
		systemSettings.Profile.pwmPeriod=19999;
		systemSettings.Profile.pwmDelay=1999;
		systemSettings.Profile.noIronValue=1200;

		//saveSettings(0);
	}
	else if(systemSettings.settings.currentProfile==Profile_None){
		asm("nop");																		// We shouldn't get here
		return;
	}
	else{
		Error_Handler();
	}
}

void loadProfile(uint8_t profile){
	if(	(profile==Profile_T12)	||														// If current tip type is valid
		(profile==Profile_C210)	||
		(profile==Profile_C245)	){

		systemSettings.settings.currentProfile=profile;									// Set system profile

		systemSettings.Profile = flashSettings->Profile[profile];						// Load stored tip data
		systemSettings.ProfileChecksum = flashSettings->ProfileChecksum[profile];		// Load stored checksum

		if(systemSettings.Profile.notInitialized){										// Check if initialized
			resetCurrentProfile();														// Load defaults if not
			systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);	// Compute checksum
		}

		if(systemSettings.ProfileChecksum != ChecksumProfile(&systemSettings.Profile)){	// Calculate data checksum and compare with stored checksum
			ProfileChkErr();															// Checksum mismatch, reset current tip data
		}
		setSetTemperature(systemSettings.Profile.UserSetTemperature);					// Load user set temperature
		setCurrentTip(systemSettings.Profile.currentTip);								// Load TIP data
	}
	else if(profile==Profile_None){
		systemSettings.settings.currentProfile=profile;									// Set system profile
		return;																			// Profiles not initialized, load nothing
	}
	else{
		Error_Handler();
	}
}

void Diag_init(void){
	setContrast(255);
	FillBuffer(C_BLACK,fill_soft);
	UG_FontSelect(&FONT_10X16_reduced);
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
}

void Flash_error(void){
	Diag_init();
	UG_PutString(8,0,"ERROR WHILE");//11
	UG_PutString(13,16,"WRITING TO");//10
	UG_PutString(33,32,"FLASH!");//6
	update_display();
	while(1){
		HAL_IWDG_Refresh(&HIWDG);
	}
}
void settingsChkErr(void){
	Diag_init();
	systemSettings.settings.OledOffset = 2;		// Set known value
	UG_PutString(3,0,"SETTING ERR!");//12
	UG_PutString(18,16,"RESTORING");//9
	UG_PutString(8,32,"DEFAULTS...");//11
	update_display();
	ErrCountDown(3,117,50);

	if(	(systemSettings.settings.currentProfile==Profile_T12)		||		// I current tip type is valid
		(systemSettings.settings.currentProfile==Profile_C210)	||
		(systemSettings.settings.currentProfile==Profile_C245)	){

		uint8_t tip = systemSettings.settings.currentProfile;		// save current tip
		resetSystemSettings();									// reset settings
		systemSettings.settings.currentProfile=tip;				// Restore tip type
		saveSettings(0);									// Save settings preserving tip data
	}
	else{
		resetSystemSettings();									// Reset settings
		saveSettings(1);									// Save wiping tip data
	}
}

void ProfileChkErr(void){
	Diag_init();
	UG_PutString(3,0,"PROFILE ERR!");//12
	UG_PutString(18,16,"RESTORING");//9
	UG_PutString(8,32,"DEFAULTS...");//11
	update_display();
	ErrCountDown(3,117,50);
	resetCurrentProfile();							// Reset current tip type data only
}

void Button_reset(void){
	uint16_t ResetTimer= HAL_GetTick();
	if(!BUTTON_input){
		Diag_init();
		UG_PutString(8,10,"HOLD BUTTON");//11
		UG_PutString(13,26,"TO RESTORE");//10
		UG_PutString(23,42,"DEFAULTS");//8
		update_display();
		while(!BUTTON_input){
			HAL_IWDG_Refresh(&HIWDG);
			if((HAL_GetTick()-ResetTimer)>5000){
				FillBuffer(C_BLACK,fill_dma);
				UG_PutString(28,15,"RELEASE");//7
				UG_PutString(13,31,"BUTTON NOW");// 10
				update_display();
				while(!BUTTON_input){
					HAL_IWDG_Refresh(&HIWDG);
				}
				resetSystemSettings();
				saveSettings(1);
			}
		}
	}
}
//Max 99 seconds countdown.
void ErrCountDown(uint8_t Start,uint8_t  xpos, uint8_t ypos){
	uint32_t timErr = 0;
	char str[3];
	char cleanStr[3]= "  ";
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
		sprintf(&str[0],"%*u",length-1,Start--);
		UG_PutString(xpos,ypos,&str[0]);
		update_display();
		while( (HAL_GetTick()-timErr)<999 ){
			HAL_IWDG_Refresh(&HIWDG);			// Clear watchdog
		}
	}
}
