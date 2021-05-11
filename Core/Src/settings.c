/*
 * settings.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "settings.h"
#include "pid.h"
#include "iron.h"
#include "gui.h"
#include "ssd1306.h"
#include "tempsensors.h"

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

	// Check init flags
	if( (systemSettings.settings.NotInitialized!=initialized) || (systemSettings.Profile.NotInitialized!=initialized) ){
		Error_Handler();
	}

	// Compute checksum
	systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);

	// Transfer system settings to flash buffer
	flashBuffer.settingsChecksum=systemSettings.settingsChecksum;
	flashBuffer.settings=systemSettings.settings;

	if(!wipeAllProfileData){																				                                      // If wipe all tips is not set
		if(systemSettings.settings.currentProfile!=profile_None){										                        // If current tip is initialized
			if((systemSettings.settings.currentProfile<=profile_C210) &&                                      // Check valid Profile
				 (systemSettings.Profile.ID == systemSettings.settings.currentProfile )){		                    // Ensure profile ID is correct

				systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);										  // Compute checksum
				flashBuffer.ProfileChecksum[systemSettings.settings.currentProfile]=systemSettings.ProfileChecksum;
				memcpy(&flashBuffer.Profile[systemSettings.settings.currentProfile],&systemSettings.Profile,sizeof(profile_t));	// Transfer system profile to flash buffer
			}
			else{
				Error_Handler(); // Invalid tip (uncontrolled state)
			}
		}
	}
	else{																								                                                  // Wipe all tip data
		for(uint8_t x=0;x<ProfileSize;x++){
			flashBuffer.Profile[x].NotInitialized=0xFF;													                              // Set all profile data to "1"
			flashBuffer.ProfileChecksum[x]=0xFF;
			memset(&flashBuffer.Profile[x],0xFF,sizeof(profile_t));
		}
	}

	//Clear watchdog before unlocking flash
	HAL_IWDG_Refresh(&hiwdg);

	HAL_FLASH_Unlock();                                                                                   //unlock flash writing
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
	HAL_IWDG_Refresh(&hiwdg);

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
	if(SettingsFlash != SettingsRam){			                                      // Check flash and system settings have same checksum
		Flash_error();							                                              // Error if data mismatch
	}

}

void restoreSettings() {
#ifdef NOSAVESETTINGS				// Stop erasing the flash every time while in debug mode
	resetSystemSettings();			// TODO not tested with the new profile system
	setupPID(systemSettings.Profile.tip[systemSettings.settings.currentTip[systemSettings.settings.currentProfile]].PID);
	return;
#endif

	if(flashSettings->settings.NotInitialized!=initialized){								  // If flash not initialized (Erased flash is always read "1")
		resetSystemSettings();
		saveSettings(saveWipingProfiles);
	}
	else{
		Button_reset();																	                        // Check for button reset
	}

	systemSettings.settings=flashSettings->settings;									        // Load system settings from flash
	systemSettings.settingsChecksum=flashSettings->settingsChecksum;					// Load stored checksum
	loadProfile(systemSettings.settings.currentProfile);								      // Load current tip data into system memory

	// Compare loaded checksum with calculated checksum
	if( (systemSettings.settings.version != SETTINGS_VERSION) || (ChecksumSettings(&systemSettings.settings)!=systemSettings.settingsChecksum) ){
		settingsChkErr();
	}

	setContrast(systemSettings.settings.contrast);
}

uint32_t ChecksumSettings(settings_t* settings){
	uint32_t checksum;
	checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)settings, sizeof(settings_t)/sizeof(uint32_t) );
	return checksum;
}

uint32_t ChecksumProfile(profile_t* profile){
	uint32_t checksum;
	checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile, sizeof(profile_t)/sizeof(uint32_t));
	return checksum;
}

void resetSystemSettings(void) {
	systemSettings.settings.version 		      = SETTINGS_VERSION;
	systemSettings.settings.contrast 			    = 255;
	systemSettings.settings.screenDimming 	  = true;
	systemSettings.settings.OledOffset 			  = 2;
	systemSettings.settings.errorDelay			  = 500;
	systemSettings.settings.guiUpdateDelay	  = 200;
	systemSettings.settings.tempUnit			    = mode_Celsius;
	systemSettings.settings.tempStep			    = 5;
	systemSettings.settings.activeDetection		= 1;
	systemSettings.settings.saveSettingsDelay	= 5;
	systemSettings.settings.currentProfile		= profile_None;
	systemSettings.settings.initMode			    = mode_run;
	systemSettings.settings.buzzerMode			  = buzzer_Off;
	systemSettings.settings.wakeOnButton		  = wakeButton_On;
	systemSettings.settings.wakeOnShake		    = wakeShake_On;
	systemSettings.settings.WakeInputMode		  = wakeInputmode_shake;
	systemSettings.settings.EncoderMode			  = RE_Mode_One;
	systemSettings.settings.NotInitialized		= initialized;
}


void resetCurrentProfile(void){
#ifdef NOSAVESETTINGS
	systemSettings.settings.currentProfile=profile_T12; /// Force T12 when debugging. TODO this is not tested with the profiles update!
#endif
	char str[TipCharSize];
	for(uint8_t x=0;x<TipCharSize;x++){
	  str[x] = ' ';
	}
	str[TipCharSize-1] = 0;

	if(systemSettings.settings.currentProfile==profile_T12){
		systemSettings.Profile.ID = profile_T12;
		for(uint8_t x = 0; x < TipSize; x++) {
			systemSettings.Profile.tip[x].calADC_At_250	= T12_Cal250;
			systemSettings.Profile.tip[x].calADC_At_350	= T12_Cal350;			// These values are way lower, but better to be safe than sorry
			systemSettings.Profile.tip[x].calADC_At_450	= T12_Cal450;			// User needs to calibrate its station
			systemSettings.Profile.tip[x].PID.Kp 		    = 6500;           // val = /1.000.000
			systemSettings.Profile.tip[x].PID.Ki 		    = 2500;           // val = /1.000.000
			systemSettings.Profile.tip[x].PID.Kd 		    = 3000;           // val = /1.000.000
			systemSettings.Profile.tip[x].PID.maxI	    = 30;             // val = /100
			systemSettings.Profile.tip[x].PID.minI 	    = 0;              // val = /100
			strcpy(systemSettings.Profile.tip[x].name, str);              // Empty name
		}
		strcpy(systemSettings.Profile.tip[0].name, "T12 ");
		systemSettings.Profile.currentNumberOfTips  = 1;
		systemSettings.Profile.currentTip           = 0;
		systemSettings.Profile.impedance            = 80;					      // 8.0 Ohms
		systemSettings.Profile.power                = 80;					      // 80W
		systemSettings.Profile.noIronValue				  = 4000;
		systemSettings.Profile.Cal250_default			  = T12_Cal250;
		systemSettings.Profile.Cal350_default			  = T12_Cal350;
		systemSettings.Profile.Cal450_default			  = T12_Cal450;

	}

	else if(systemSettings.settings.currentProfile==profile_C245){
		systemSettings.Profile.ID = profile_C245;
		for(uint8_t x = 0; x < TipSize; x++) {
			systemSettings.Profile.tip[x].calADC_At_250 = C245_Cal250;
			systemSettings.Profile.tip[x].calADC_At_350 = C245_Cal350;
			systemSettings.Profile.tip[x].calADC_At_450 = C245_Cal450;
      systemSettings.Profile.tip[x].PID.Kp        = 6500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki        = 2500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd        = 3000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI      = 30;             // val = /100
      systemSettings.Profile.tip[x].PID.minI      = 0;              // val = /100
      strcpy(systemSettings.Profile.tip[x].name, str);              // Empty name
		}
		strcpy(systemSettings.Profile.tip[0].name, "C245");
		systemSettings.Profile.currentNumberOfTips	= 1;
		systemSettings.Profile.currentTip 				  = 0;
		systemSettings.Profile.impedance				    = 26;
		systemSettings.Profile.power					      = 150;
		systemSettings.Profile.noIronValue				  = 4000;
		systemSettings.Profile.Cal250_default			  = C245_Cal250;
		systemSettings.Profile.Cal350_default			  = C245_Cal350;
		systemSettings.Profile.Cal450_default			  = C245_Cal450;
	}

	else if(systemSettings.settings.currentProfile==profile_C210){
		systemSettings.Profile.ID = profile_C210;
		for(uint8_t x = 0; x < TipSize; x++) {
			systemSettings.Profile.tip[x].calADC_At_250 = C210_Cal250;
			systemSettings.Profile.tip[x].calADC_At_350 = C210_Cal350;
			systemSettings.Profile.tip[x].calADC_At_450 = C210_Cal450;
      systemSettings.Profile.tip[x].PID.Kp        = 6500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki        = 2500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd        = 3000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI      = 30;             // val = /100
      systemSettings.Profile.tip[x].PID.minI      = 0;              // val = /100
      strcpy(systemSettings.Profile.tip[x].name, str);              // Empty name
		}
		strcpy(systemSettings.Profile.tip[0].name, "C210");
		systemSettings.Profile.currentNumberOfTips  = 1;
		systemSettings.Profile.currentTip           = 0;
		systemSettings.Profile.power					      = 80;
		systemSettings.Profile.impedance			      = 21;
		systemSettings.Profile.noIronValue		      = 1200;
		systemSettings.Profile.Cal250_default	      = C210_Cal250;
		systemSettings.Profile.Cal350_default	      = C210_Cal350;
		systemSettings.Profile.Cal450_default	      = C210_Cal450;
	}
	else{
		Error_Handler();  // We shouldn't get here!
	}
	systemSettings.Profile.CalNTC				        = 25;
	systemSettings.Profile.sleepTimeout 		    = 10;
  systemSettings.Profile.UserSetTemperature   = 320;
  systemSettings.Profile.MaxSetTemperature    = 450;
  systemSettings.Profile.MinSetTemperature    = 180;
	systemSettings.Profile.pwmPeriod			      = 19999;
	systemSettings.Profile.pwmDelay				      = 1999;
	systemSettings.Profile.filterFactor		    	= 2;
	systemSettings.Profile.filterMode			      = filter_ema;
	systemSettings.Profile.tempUnit				      = mode_Celsius;
	systemSettings.Profile.NotInitialized		    = initialized;
}

void loadProfile(uint8_t profile){
  systemSettings.settings.currentProfile=profile;									                      // Update system profile
	if(profile<=profile_C210){                                                            // If current tip type is valid
		systemSettings.Profile = flashSettings->Profile[profile];						                // Load stored tip data
		systemSettings.ProfileChecksum = flashSettings->ProfileChecksum[profile];		        // Load stored checksum
		if(systemSettings.Profile.NotInitialized!=initialized){							                // Check if initialized
			resetCurrentProfile();														                                // Load defaults if not
			systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);	      // Compute checksum
		}
		// Calculate data checksum and compare with stored checksum, also ensure the stored ID is the same as the requested profile
		if( (profile!=systemSettings.Profile.ID) || (systemSettings.ProfileChecksum != ChecksumProfile(&systemSettings.Profile)) ){
			ProfileChkErr();															                                    // Checksum mismatch, reset current tip data
		}
		setSetTemperature(systemSettings.Profile.UserSetTemperature);				                // Load user set temperature
		setCurrentTip(systemSettings.Profile.currentTip);								                    // Load TIP data
	}
	else if(profile==profile_None){
		return;																			                                        // Profiles not initialized, load nothing
	}
	else{                                                                                 // Unknown profile
		Error_Handler();
	}
	if(systemSettings.settings.tempUnit != systemSettings.Profile.tempUnit){			        // If stored temps are in different units
		setSystemTempUnit(systemSettings.settings.tempUnit);							                  // Convert temperatures
		systemSettings.Profile.tempUnit = systemSettings.settings.tempUnit;				          // Store unit in profile
	}
}

void Diag_init(void){
	setContrast(255);
	FillBuffer(BLACK,fill_soft);
	u8g2_SetFont(&u8g2,default_font );
	u8g2_SetDrawColor(&u8g2, WHITE);
}

void Flash_error(void){
	Diag_init();
	putStrAligned("ERROR WHILE", 0, align_center);
	putStrAligned("WRITING TO", 16, align_center);
	putStrAligned("FLASH!", 32, align_center);
	update_display();
	while(1){
		HAL_IWDG_Refresh(&hiwdg);
	}
}
void settingsChkErr(void){
	Diag_init();
	systemSettings.settings.OledOffset = 2;		// Set default value
	putStrAligned("SETTING ERR!", 0, align_center);
	putStrAligned("RESTORING", 16, align_center);
	putStrAligned("DEFAULTS...", 32, align_center);
	update_display();
	ErrCountDown(3,117,50);

	if(systemSettings.settings.currentProfile<=profile_C210){                         // If current tip type is valid

		if(systemSettings.ProfileChecksum==ChecksumProfile(&systemSettings.Profile)){	  // If current profile checksum is correct
			uint8_t tip = systemSettings.settings.currentProfile;				                  // save current tip
			resetSystemSettings();												                                // reset settings
			systemSettings.settings.currentProfile=tip;							                      // Restore tip type
			saveSettings(saveKeepingProfiles);									                          // Save settings preserving tip data
		}
		else{                                                                           // If checksum wrong
			resetSystemSettings();									                                      // reset settings
			saveSettings(saveWipingProfiles);						                                  // Save settings erasing tip data
		}
	}
	else{                                                                             // If current tip not valid
		resetSystemSettings();									                                        // Assume something went wrong, reset settings
		saveSettings(saveKeepingProfiles);									                            // Save keeping tip data
	}
}

void ProfileChkErr(void){
	Diag_init();
	putStrAligned("PROFILE ERR!", 0, align_center);
	putStrAligned("RESTORING", 16, align_center);
	putStrAligned("DEFAULTS...", 32, align_center);
	update_display();
	ErrCountDown(3,117,50);
	resetCurrentProfile();							// Reset current tip type data only
}

void Button_reset(void){
	uint16_t ResetTimer= HAL_GetTick();
	if(!BUTTON_input()){
		Diag_init();
		putStrAligned("HOLD BUTTON", 0, align_center);
		putStrAligned("TO RESTORE", 16, align_center);
		putStrAligned("DEFAULTS", 32, align_center);
		update_display();
		while(!BUTTON_input()){
			HAL_IWDG_Refresh(&hiwdg);
			if((HAL_GetTick()-ResetTimer)>5000){
				FillBuffer(BLACK,fill_dma);
				putStrAligned("RELEASE", 12, align_center);
				putStrAligned("BUTTON NOW", 28, align_center);
				update_display();
				while(!BUTTON_input()){
					HAL_IWDG_Refresh(&hiwdg);
				}
				resetSystemSettings();
				saveSettings(saveWipingProfiles);
			}
		}
	}
}
//Max 99 seconds countdown.
void ErrCountDown(uint8_t Start,uint8_t  xpos, uint8_t ypos){
	uint32_t timErr = 0;
	char str[4];
	uint8_t length;
	if(Start>99){Start=99;}
	if(Start>9){
		length=2;
	}
	else{
		length=1;
	}
	HAL_IWDG_Refresh(&hiwdg);
	while(oled.status!=oled_idle);  // Wait for the srceen to be idle (few mS at most). Hanging here will cause a watchdog reset.
	//HAL_Delay(20);				        // (Old) Dirty fix to ensure Oled DMA transfer has ended before writing to the buffer

	while(Start){
		timErr=HAL_GetTick();
		u8g2_SetDrawColor(&u8g2, BLACK);
		u8g2_DrawBox(&u8g2,xpos,ypos,u8g2_GetStrWidth(&u8g2,str),u8g2_GetMaxCharHeight(&u8g2));
		u8g2_SetDrawColor(&u8g2, WHITE);
		sprintf(&str[0],"%*u",length-1,Start--);
		u8g2_DrawStr(&u8g2,xpos,ypos,str);
		update_display();
		while( (HAL_GetTick()-timErr)<999 ){
			HAL_IWDG_Refresh(&hiwdg);			// Clear watchdog
		}
	}
}
