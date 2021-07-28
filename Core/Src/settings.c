/*
 * settings.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
 */

#include "settings.h"
#include "pid.h"
#include "iron.h"
#include "gui.h"
#include "ssd1306.h"
#include "tempsensors.h"

systemSettings_t systemSettings;
flashSettings_t* flashSettings = (flashSettings_t*)FLASH_ADDR;
void settingsChkErr(void);
void ProfileChkErr(void);
void Flash_error(void);
void Button_reset(void);
void Diag_init(void);
void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);



void checkSettings(void){

  static uint32_t prevSysChecksum=0, newSysChecksum=0, prevTipChecksum=0, newTipChecksum=0, lastCheckTime=0, lastChangeTime=0;
  uint32_t CurrentTime = HAL_GetTick();

  // Save from menu
  if(systemSettings.save_Flag){
    switch(systemSettings.save_Flag){
      case save_Settings:
        saveSettings(keepProfiles);
        break;
      case reset_Settings:
      {
        uint8_t currentProfile=systemSettings.settings.currentProfile;
        resetSystemSettings();
        systemSettings.settings.currentProfile=currentProfile;
        saveSettings(keepProfiles);
        break;
      }
      case reset_Profile:
        resetCurrentProfile();
        saveSettings(keepProfiles);
        break;

      case reset_Profiles:
        systemSettings.settings.currentProfile=profile_None;
        saveSettings(wipeProfiles);
        break;
      case reset_All:
        resetSystemSettings();
        saveSettings(wipeProfiles);
        break;

      default:
        Error_Handler();
    }
    if(systemSettings.save_Flag>=reset_Profile){
      NVIC_SystemReset();
    }
    systemSettings.save_Flag=0;
    return;
  }

  // Auto save on content change
  if( (systemSettings.setupMode==setup_On) || (Iron.calibrating==calibration_On) || (systemSettings.settings.saveSettingsDelay==0) || (Iron.Error.safeMode==enable) || (CurrentTime-lastCheckTime<999)){
    return;
  }

  lastCheckTime = CurrentTime;                                                                                              // Store current time
  newSysChecksum = ChecksumSettings(&systemSettings.settings);                                                              // Calculate system checksum
  newTipChecksum = ChecksumProfile(&systemSettings.Profile);                                                                // Calculate tip profile checksum

  if((systemSettings.settingsChecksum != newSysChecksum) || (systemSettings.ProfileChecksum != newTipChecksum)){            // If anything was changed (Checksum mismatch)

    if((prevSysChecksum != newSysChecksum) || (prevTipChecksum != newTipChecksum)){                                         // If different from the previous calculated checksum (settings are being changed quickly, don't save every time).
      prevSysChecksum = newSysChecksum;                                                                                     // Store last computed checksum
      prevTipChecksum = newTipChecksum;
      lastChangeTime = CurrentTime;                                                                                         // Reset timer (we don't save anything until we pass a certain time without changes)
    }

    else if((CurrentTime-lastChangeTime)>((uint32_t)systemSettings.settings.saveSettingsDelay*1000)){                       // If different from the previous calculated checksum, and timer expired (No changes for enough time)
      saveSettings(save_Settings);                                                                                          // Data was saved (so any pending interrupt knows this)
    }
  }
}


//This is done to avoid huge stack build up. Trigger a save using checkSettings with a flag instead direct call from menu.
void saveSettingsFromMenu(uint8_t mode){
  systemSettings.save_Flag=mode;
  if(mode>=reset_Profile){
    setSafeMode(enable);
  }
}

void saveSettings(uint8_t mode){

  #ifdef NOSAVESETTINGS
    return;
  #endif

  uint32_t error=0;
  uint8_t profile = systemSettings.settings.currentProfile;

  flashSettings_t flashBuffer=*flashSettings;

  while(ADC_Status != ADC_Idle);
  __disable_irq();
  systemSettings.isSaving = 1;
  configurePWMpin(output_Low);
  __enable_irq();

  if( (systemSettings.settings.NotInitialized!=initialized) || (systemSettings.Profile.NotInitialized!=initialized) ){
    Error_Handler();
  }

  systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);
  flashBuffer.settingsChecksum = systemSettings.settingsChecksum;
  flashBuffer.settings = systemSettings.settings;

  if(mode==keepProfiles){
    if((systemSettings.settings.currentProfile<=profile_C210) &&
       (systemSettings.Profile.ID == profile )){

      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
      flashBuffer.ProfileChecksum[profile] = systemSettings.ProfileChecksum;
      flashBuffer.Profile[profile] = systemSettings.Profile;
    }
    else{
      Error_Handler();
    }
  }
  else{
    for(uint8_t x=0;x<ProfileSize;x++){
      flashBuffer.Profile[x].NotInitialized = 0xFF;
      flashBuffer.ProfileChecksum[x] = 0xFFFFFFFF;
      memset(&flashBuffer.Profile[x],0xFF,sizeof(profile_t));
    }
  }
  __disable_irq();
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase;
  erase.NbPages = (1024*StoreSize)/FLASH_PAGE_SIZE;
  erase.PageAddress = FLASH_ADDR;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;

  if((HAL_FLASHEx_Erase(&erase, &error)!=HAL_OK) || (error!=0xFFFFFFFF)){
    Flash_error();
  }
  HAL_FLASH_Lock();
  __enable_irq();

  // Ensure flash was erased
  for (uint16_t i = 0; i < sizeof(flashSettings_t)/2; i++) {
    if( *(uint16_t*)(FLASH_ADDR+(i*2)) != 0xFFFF){
      Flash_error();
    }
  }

    __disable_irq();
  HAL_FLASH_Unlock();
  __enable_irq();

  uint32_t dest = (uint32_t)flashSettings;
  uint16_t *data = (uint16_t*)&flashBuffer;

  // Store settings
  // written = number of 16-bit values written
  for(uint16_t written=0; written < (sizeof(flashSettings_t)/2); written++){
    __disable_irq();
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, dest, *data ) != HAL_OK){
      Flash_error();
    }
    dest += 2;                // address +2 because we write 16 bit data
    data++;                   // +1 because it's 16Bit pointer
    __enable_irq();
  }

  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();

  if(mode==keepProfiles){
    uint32_t ProfileFlash  = ChecksumProfile(&flashSettings->Profile[profile]);
    uint32_t ProfileRam    = ChecksumProfile(&systemSettings.Profile);

    if((ProfileFlash != ProfileRam) ||  (flashSettings->settings.currentProfile != profile)){
      Flash_error();
    }
  }

  // Check flash and system settings have same checksum
  uint32_t SettingsFlash  = ChecksumSettings(&flashSettings->settings);
  uint32_t SettingsRam  = ChecksumSettings(&systemSettings.settings);
  if(SettingsFlash != SettingsRam){
    Flash_error();
  }

  __disable_irq();
  systemSettings.isSaving = 0;
  __enable_irq();
}

void restoreSettings() {
#ifdef NOSAVESETTINGS                                                 // Stop erasing the flash while in debug mode
  resetSystemSettings();                                              // TODO not tested with the new profile system
  systemSettings.settings.currentProfile = profile_T12;
  resetCurrentProfile();
  setupPID(systemSettings.Profile.tip[0].PID;);
  return;
#endif

  if(flashSettings->settings.NotInitialized != initialized){
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  else{
    Button_reset();
  }

  systemSettings.settings = flashSettings->settings;
  systemSettings.settingsChecksum = flashSettings->settingsChecksum;
  loadProfile(systemSettings.settings.currentProfile);

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
  __disable_irq();
  systemSettings.settings.version           = SETTINGS_VERSION;
  systemSettings.settings.contrast          = 255;
  systemSettings.settings.screenDimming     = true;
  systemSettings.settings.OledOffset        = 2;
  systemSettings.settings.errorDelay        = 100;
  systemSettings.settings.guiUpdateDelay    = 200;
  systemSettings.settings.tempUnit          = mode_Celsius;
  systemSettings.settings.tempStep          = 10;                   // 10º steps
  systemSettings.settings.activeDetection   = true;
  systemSettings.settings.saveSettingsDelay = 5;                    // 5s
  systemSettings.settings.lvp               = 110;                  // 11.0V Low voltage
  systemSettings.settings.currentProfile    = profile_None;
  systemSettings.settings.initMode          = mode_run;
  systemSettings.settings.buzzerMode        = buzzer_Off;
  systemSettings.settings.wakeOnButton      = wakeButton_On;
  systemSettings.settings.wakeOnShake       = wakeShake_On;
  systemSettings.settings.WakeInputMode     = wakeInputmode_shake;
  systemSettings.settings.StandMode         = mode_sleep;
  systemSettings.settings.EncoderMode       = RE_Mode_One;
  systemSettings.settings.NotInitialized    = initialized;
  __enable_irq();
}


void resetCurrentProfile(void){
#ifdef NOSAVESETTINGS
  systemSettings.settings.currentProfile=profile_T12; /// Force T12 when debugging. TODO this is not tested with the profiles update!
#endif
  __disable_irq();
    if(systemSettings.settings.currentProfile==profile_T12){
    systemSettings.Profile.ID = profile_T12;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = T12_Cal250;
      systemSettings.Profile.tip[x].calADC_At_350   = T12_Cal350;     // These values are way lower, but better to be safe than sorry
      systemSettings.Profile.tip[x].calADC_At_450   = T12_Cal450;     // User needs to calibrate its station
      systemSettings.Profile.tip[x].PID.Kp          = 11000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki          = 8000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd          = 2500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI        = 50;             // val = /100
      systemSettings.Profile.tip[x].PID.minI        = 0;              // val = /100
      systemSettings.Profile.tip[x].PID.tau         = 10;             // val = /100
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);         // Empty name
    }
    strcpy(systemSettings.Profile.tip[0].name, "BC3 ");               // Put some generic name
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip               = 0;
    systemSettings.Profile.impedance                = 80;             // 8.0 Ohms
    systemSettings.Profile.power                    = 80;             // 80W
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = T12_Cal250;
    systemSettings.Profile.Cal350_default           = T12_Cal350;
    systemSettings.Profile.Cal450_default           = T12_Cal450;

  }

  else if(systemSettings.settings.currentProfile==profile_C245){
    systemSettings.Profile.ID = profile_C245;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C245_Cal250;
      systemSettings.Profile.tip[x].calADC_At_350   = C245_Cal350;
      systemSettings.Profile.tip[x].calADC_At_450   = C245_Cal450;
      systemSettings.Profile.tip[x].PID.Kp          = 1800;
      systemSettings.Profile.tip[x].PID.Ki          = 500;
      systemSettings.Profile.tip[x].PID.Kd          = 200;
      systemSettings.Profile.tip[x].PID.maxI        = 10;
      systemSettings.Profile.tip[x].PID.minI        = 0;
      systemSettings.Profile.tip[x].PID.tau         = 10;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C245");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip               = 0;
    systemSettings.Profile.impedance                = 26;
    systemSettings.Profile.power                    = 150;
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = C245_Cal250;
    systemSettings.Profile.Cal350_default           = C245_Cal350;
    systemSettings.Profile.Cal450_default           = C245_Cal450;
  }

  else if(systemSettings.settings.currentProfile==profile_C210){
    systemSettings.Profile.ID = profile_C210;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C210_Cal250;
      systemSettings.Profile.tip[x].calADC_At_350   = C210_Cal350;
      systemSettings.Profile.tip[x].calADC_At_450   = C210_Cal450;
      systemSettings.Profile.tip[x].PID.Kp          = 1800;
      systemSettings.Profile.tip[x].PID.Ki          = 500;
      systemSettings.Profile.tip[x].PID.Kd          = 200;
      systemSettings.Profile.tip[x].PID.maxI        = 10;
      systemSettings.Profile.tip[x].PID.minI        = 0;
      systemSettings.Profile.tip[x].PID.tau         = 10;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C210");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip             = 0;
    systemSettings.Profile.power                  = 80;
    systemSettings.Profile.impedance              = 21;
    systemSettings.Profile.noIronValue            = 1200;
    systemSettings.Profile.Cal250_default         = C210_Cal250;
    systemSettings.Profile.Cal350_default         = C210_Cal350;
    systemSettings.Profile.Cal450_default         = C210_Cal450;
  }
  else{
    Error_Handler();  // We shouldn't get here!
  }
  systemSettings.Profile.CalNTC                   = 25;
  systemSettings.Profile.sleepTimeout             = 5;
  systemSettings.Profile.standbyTimeout           = 5;
  systemSettings.Profile.standbyTemperature       = 180;
  systemSettings.Profile.UserSetTemperature       = 180;
  systemSettings.Profile.MaxSetTemperature        = 450;
  systemSettings.Profile.MinSetTemperature        = 180;
  systemSettings.Profile.pwmMul                   = 1;
  systemSettings.Profile.readPeriod               = (200*200)-1;             // Because we have a 5uS timer clock
  systemSettings.Profile.readDelay                = (20*200)-1;
  systemSettings.Profile.filterFactor             = 3;
  systemSettings.Profile.tempUnit                 = mode_Celsius;
  systemSettings.Profile.NotInitialized           = initialized;
  __enable_irq();
}

void loadProfile(uint8_t profile){
  while(ADC_Status!=ADC_Idle);
  __disable_irq();
  HAL_IWDG_Refresh(&hiwdg);
  systemSettings.settings.currentProfile=profile;
  if(profile==profile_None){                                                    // If profile nos initialized yet, use C245 values until the system is configured
    systemSettings.settings.currentProfile=profile_C245;                        // Force C245 profile
    resetCurrentProfile();                                                      // Load data
    systemSettings.settings.currentProfile=profile_None;                        // Revert to none
  }
  else if(profile<=profile_C210){
    systemSettings.Profile = flashSettings->Profile[profile];
    systemSettings.ProfileChecksum = flashSettings->ProfileChecksum[profile];
    if(systemSettings.Profile.NotInitialized!=initialized){
      resetCurrentProfile();
      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
    }

    // Calculate data checksum and compare with stored checksum, also ensure the stored ID is the same as the requested profile

    if( (profile!=systemSettings.Profile.ID) || (systemSettings.ProfileChecksum != ChecksumProfile(&systemSettings.Profile)) ){
      ProfileChkErr();
    }
    setUserTemperature(systemSettings.Profile.UserSetTemperature);
    setCurrentTip(systemSettings.Profile.currentTip);
    TIP.filter_normal=systemSettings.Profile.filterFactor;
    Iron.updatePwm=1;
  }
  else{
    Error_Handler();
  }
  if(systemSettings.settings.tempUnit != systemSettings.Profile.tempUnit){
    setSystemTempUnit(systemSettings.settings.tempUnit);
    systemSettings.Profile.tempUnit = systemSettings.settings.tempUnit;
  }
  __enable_irq();
}

void Diag_init(void){
  setContrast(255);
  FillBuffer(BLACK,fill_soft);
  u8g2_SetFont(&u8g2,default_font );
  u8g2_SetDrawColor(&u8g2, WHITE);
}

void Flash_error(void){

  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();

  Diag_init();
  putStrAligned("FLASH ERROR!", 16, align_center);
  putStrAligned("HALTING SYSTEM", 32, align_center);
  update_display();
  while(1){
    HAL_IWDG_Refresh(&hiwdg);
  }
}
void settingsChkErr(void){
  Diag_init();
  systemSettings.settings.OledOffset = 2;
  putStrAligned("SETTING ERR!", 10, align_center);
  putStrAligned("RESTORING", 26, align_center);
  putStrAligned("DEFAULTS...", 42, align_center);
  update_display();
  ErrCountDown(3,117,50);

  if(systemSettings.settings.currentProfile<=profile_C210){
    if(systemSettings.ProfileChecksum==ChecksumProfile(&systemSettings.Profile)){   // If current profile checksum is correct
      uint8_t tip = systemSettings.settings.currentProfile;                         // save current tip
      resetSystemSettings();                                                        // reset settings
      systemSettings.settings.currentProfile=tip;                                   // Restore tip type
      saveSettings(save_Settings);                                                  // Save settings preserving tip data
    }
    else{                                                                           // If checksum wrong
      resetSystemSettings();                                                        // reset settings
      saveSettings(wipeProfiles);                                                   // Save settings erasing tip data
    }
  }
  else{                                                                             // If current profile not valid
    resetSystemSettings();                                                          // Assume something went wrong, reset settings
    saveSettings(save_Settings);                                                    // Save keeping tip data, if it's corrupted it will be detected when loading the profile data.
  }
}

void ProfileChkErr(void){
  Diag_init();
  putStrAligned("PROFILE ERR!", 10, align_center);
  putStrAligned("RESTORING", 26, align_center);
  putStrAligned("DEFAULTS...", 42, align_center);
  update_display();
  ErrCountDown(3,117,50);
  resetCurrentProfile();
}

void Button_reset(void){
  uint16_t ResetTimer= HAL_GetTick();
  if(!BUTTON_input()){
    Diag_init();
    putStrAligned("HOLD BUTTON", 10, align_center);
    putStrAligned("TO RESTORE", 26, align_center);
    putStrAligned("DEFAULTS", 42, align_center);
    update_display();
    while(!BUTTON_input()){
      HAL_IWDG_Refresh(&hiwdg);
      if((HAL_GetTick()-ResetTimer)>5000){
        FillBuffer(BLACK,fill_dma);
        putStrAligned("RELEASE", 16, align_center);
        putStrAligned("BUTTON NOW", 32, align_center);
        update_display();
        while(!BUTTON_input()){
          HAL_IWDG_Refresh(&hiwdg);
        }
        resetSystemSettings();
        saveSettings(wipeProfiles);
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
  while(oled.status!=oled_idle);

  while(Start){
    timErr=HAL_GetTick();
    u8g2_SetDrawColor(&u8g2, BLACK);
    u8g2_DrawBox(&u8g2,xpos,ypos,u8g2_GetStrWidth(&u8g2,str),u8g2_GetMaxCharHeight(&u8g2));
    u8g2_SetDrawColor(&u8g2, WHITE);
    sprintf(&str[0],"%*u",length-1,Start--);
    u8g2_DrawStr(&u8g2,xpos,ypos,str);
    update_display();
    while( (HAL_GetTick()-timErr)<999 ){
      HAL_IWDG_Refresh(&hiwdg);
    }
  }
}
