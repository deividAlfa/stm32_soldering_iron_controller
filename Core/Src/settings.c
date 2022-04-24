/*
 * settings.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "settings.h"
#include "pid.h"
#include "iron.h"
#include "gui.h"
#include "lcd.h"
#include "tempsensors.h"
#include "main.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "settings.c"
#endif

const settings_t defaultSettings = {
  .version            = (~((uint32_t)SETTINGS_VERSION<<16)&0xFFFF0000) | SETTINGS_VERSION,  // Higher 16bit is 1s complement to make detection stronger against padding/endianness
#ifdef ST7565
  .contrast           = 0x16,
#else
  .contrast           = 255,
#endif
  .dim_mode           = dim_sleep,
  .dim_Timeout        = 10000,                // ms
  .dim_inSleep        = enable,
  .OledOffset         = OLED_OFFSET,
  .guiUpdateDelay     = 200,                  //ms
  .guiTempDenoise     = 5,                    // ±5°C
  .tempUnit           = mode_Celsius,
  .tempStep           = 5,                    // 5º steps
  .tempBigStep        = 20,                   // 20º big steps
  .activeDetection    = true,
  .saveSettingsDelay  = 5,                    // 5s
  .lvp                = 110,                  // 11.0V Low voltage
  .currentProfile     = profile_None,
  .initMode           = mode_run,
  .buzzerMode         = disable,
  .buttonWakeMode     = wake_all,
  .shakeWakeMode      = wake_all,
  .EncoderMode        = RE_Mode_Forward,
  .debugEnabled       = disable,
  .language           = lang_english,
  .state              = initialized,
};

__attribute__((section(".settings"))) flashSettings_t flashSettings;
systemSettings_t systemSettings;

void checksumError(uint8_t mode);
void Flash_error(void);
void Button_reset(void);
void Oled_error_init(void);
void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);



void checkSettings(void){

  #ifdef NOSAVESETTINGS
  return;
  #endif

  static uint32_t prevSysChecksum=0, newSysChecksum=0, prevTipChecksum=0, newTipChecksum=0, lastCheckTime=0, lastChangeTime=0;
  uint32_t CurrentTime = HAL_GetTick();
  uint8_t scr_index=current_screen->index;

  // Disable saving when screens that use a lot of ram are active.
  // Change detection will be active, but saving will postponed until exiting the screen. This is done to ensure compatibility with 10KB RAM devices
  uint8_t noSave = (scr_index==screen_iron || scr_index==screen_system || scr_index==screen_tip_settings || scr_index==screen_debug );



  // Save from menu
  if(systemSettings.save_Flag && !noSave){
    switch(systemSettings.save_Flag){
      case save_Settings:
        saveSettings(keepProfiles);
        break;
      case reset_Profiles:
        systemSettings.settings.currentProfile=profile_None;
        saveSettings(wipeProfiles);
        break;
      case reset_Profile:
        __disable_irq();
        resetCurrentProfile();
        __enable_irq();
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
      case reset_All:
        resetSystemSettings();
        saveSettings(wipeProfiles);
        break;

      default:
        Error_Handler();
    }
    if(systemSettings.save_Flag>=reset_Profiles){
      NVIC_SystemReset();
    }
    systemSettings.save_Flag=0;
    return;
  }

  // Auto save on content change
  if( (systemSettings.setupMode==enable) || (Iron.calibrating==enable) || (systemSettings.settings.saveSettingsDelay==0) || (Iron.Error.safeMode==enable) || (CurrentTime-lastCheckTime<999)){
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

    else if( !noSave && (CurrentTime-lastChangeTime)>((uint32_t)systemSettings.settings.saveSettingsDelay*1000)){           // If different from the previous calculated checksum, and timer expired (No changes for enough time)
      saveSettings(save_Settings);                                                                                          // Data was saved (so any pending interrupt knows this)
    }
  }
}


//This is done to avoid huge stack build up. Trigger a save using checkSettings with a flag instead direct call from menu.
void saveSettingsFromMenu(uint8_t mode){
  systemSettings.save_Flag=mode;
  if(mode>=reset_Profiles){
    setSafeMode(enable);
  }
}

void saveSettings(uint8_t mode){
  #ifdef NOSAVESETTINGS
    return;
  #endif

  uint32_t error=0;
  uint32_t *dest = (uint32_t*)&flashSettings;
  uint8_t profile = systemSettings.settings.currentProfile;
  flashSettings_t *flashBuffer=_malloc(sizeof(flashSettings_t));
  uint32_t *src = &*(uint32_t*)flashBuffer;

  if(!flashBuffer){ Error_Handler(); }

  *flashBuffer = flashSettings;                                     // Backup current flash data before erasing

  while(ADC_Status != ADC_Idle);
  __disable_irq();
  systemSettings.isSaving = 1;
  configurePWMpin(output_Low);
  __enable_irq();

  if( (systemSettings.settings.state!=initialized) || (systemSettings.Profile.state!=initialized) ){
    Error_Handler();
  }

  systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);
  flashBuffer->settingsChecksum = systemSettings.settingsChecksum;
  flashBuffer->settings = systemSettings.settings;

  if((mode==keepProfiles) && (systemSettings.settings.currentProfile<=profile_C210) && (systemSettings.Profile.ID == profile )){
    systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
    flashBuffer->ProfileChecksum[profile] = systemSettings.ProfileChecksum;
    flashBuffer->Profile[profile] = systemSettings.Profile;
  }
  else{
    mode = wipeProfiles;
    for(uint8_t x=0;x<ProfileSize;x++){
      flashBuffer->Profile[x].state = 0xFF;
      flashBuffer->ProfileChecksum[x] = 0xFFFFFFFF;
      memset(&flashBuffer->Profile[x],0xFF,sizeof(profile_t));
    }
  }
  __disable_irq();
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase;
  erase.NbPages = (sizeof(flashSettings_t)+FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE;
  erase.PageAddress = (uint32_t)dest;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;

  HAL_IWDG_Refresh(&hiwdg);
  if((HAL_FLASHEx_Erase(&erase, &error)!=HAL_OK) || (error!=0xFFFFFFFF)){
    Flash_error();
  }
  HAL_FLASH_Lock();
  __enable_irq();

  // Ensure flash was erased
  for (uint16_t i = 0; i < sizeof(flashSettings_t)/(sizeof(uint32_t)); i++) {
    if( *dest++ != 0xFFFFFFFF){
      Flash_error();
    }
  }
  dest = (uint32_t*)&flashSettings;

  __disable_irq();
  HAL_FLASH_Unlock();
  __enable_irq();

  // Store settings
  // written = number of 16-bit values written
  for(uint16_t written=0; written < (sizeof(flashSettings_t)/4); written++){
    __disable_irq();
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)dest, *src ) != HAL_OK){
      Flash_error();
    }
    dest++;                // increase pointers
    src++;
    __enable_irq();
  }

  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();

  if(mode==keepProfiles){
    uint32_t ProfileFlash  = ChecksumProfile(&flashSettings.Profile[profile]);
    uint32_t ProfileRam    = ChecksumProfile(&systemSettings.Profile);

    if((ProfileFlash != ProfileRam) ||  (flashSettings.settings.currentProfile != profile)){
      Flash_error();
    }
  }

  // Check flash and system settings have same checksum
  uint32_t SettingsFlash  = ChecksumSettings(&flashSettings.settings);
  uint32_t SettingsRam  = ChecksumSettings(&systemSettings.settings);
  if(SettingsFlash != SettingsRam){
    Flash_error();
  }
  _free(flashBuffer);
  __disable_irq();
  systemSettings.isSaving = 0;
  __enable_irq();
}

void restoreSettings() {
#ifdef NOSAVESETTINGS                                                 // Stop erasing the flash while in debug mode
  __disable_irq();
  resetSystemSettings();                                              // TODO not tested with the new profile system
  systemSettings.settings.currentProfile = profile_T12;
  resetCurrentProfile();
  loadProfile(systemSettings.settings.currentProfile);
  __enable_irq();
  return;
#endif

  if(flashSettings.settings.state != initialized){
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  else{
    Button_reset();
  }

  if(flashSettings.settings.version!=defaultSettings.version){    // Silent reset if version mismatch
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  else{
    systemSettings.settings = flashSettings.settings;
    systemSettings.settingsChecksum = flashSettings.settingsChecksum;
  }


  if(ChecksumSettings(&systemSettings.settings)!=systemSettings.settingsChecksum){   // Show error message if bad checksum
    checksumError(reset_All);
  }

  loadProfile(systemSettings.settings.currentProfile);

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
  systemSettings.settings = defaultSettings;
  __enable_irq();
}


void resetCurrentProfile(void){
    if(systemSettings.settings.currentProfile==profile_T12){
    systemSettings.Profile.ID = profile_T12;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = T12_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = T12_Cal400;     // These values are way lower, but better to be safe than sorry
      systemSettings.Profile.tip[x].PID.Kp          = 7500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki          = 4800;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd          = 1200;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI        = 85;             // val = /100
      systemSettings.Profile.tip[x].PID.minI        = 0;              // val = /100
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);         // Empty name
    }
    strcpy(systemSettings.Profile.tip[0].name, "BC3 ");               // Put some generic name
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip               = 0;
    systemSettings.Profile.impedance                = 80;             // 8.0 Ohms
    systemSettings.Profile.power                    = 80;             // 80W
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = T12_Cal250;
    systemSettings.Profile.Cal400_default           = T12_Cal400;
  }

  else if(systemSettings.settings.currentProfile==profile_C245){
    systemSettings.Profile.ID = profile_C245;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C245_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = C245_Cal400;
      systemSettings.Profile.tip[x].PID.Kp          = 1800;
      systemSettings.Profile.tip[x].PID.Ki          = 500;
      systemSettings.Profile.tip[x].PID.Kd          = 200;
      systemSettings.Profile.tip[x].PID.maxI        = 85;
      systemSettings.Profile.tip[x].PID.minI        = 0;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C245");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip               = 0;
    systemSettings.Profile.impedance                = 26;
    systemSettings.Profile.power                    = 150;
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = C245_Cal250;
    systemSettings.Profile.Cal400_default           = C245_Cal400;
  }

  else if(systemSettings.settings.currentProfile==profile_C210){
    systemSettings.Profile.ID = profile_C210;
    for(uint8_t x = 0; x < TipSize; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C210_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = C210_Cal400;
      systemSettings.Profile.tip[x].PID.Kp          = 1800;
      systemSettings.Profile.tip[x].PID.Ki          = 500;
      systemSettings.Profile.tip[x].PID.Kd          = 200;
      systemSettings.Profile.tip[x].PID.maxI        = 85;
      systemSettings.Profile.tip[x].PID.minI        = 0;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C210");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.currentTip             = 0;
    systemSettings.Profile.power                  = 80;
    systemSettings.Profile.impedance              = 21;
    systemSettings.Profile.noIronValue            = 1200;
    systemSettings.Profile.Cal250_default         = C210_Cal250;
    systemSettings.Profile.Cal400_default         = C210_Cal400;
  }
  else{
    Error_Handler();  // We shouldn't get here!
  }
  systemSettings.Profile.calADC_At_0                = 0;
  systemSettings.Profile.tipFilter.coefficient      = 90;   // % of old data (more %, more filtering)
  systemSettings.Profile.tipFilter.threshold        = 50;
  systemSettings.Profile.tipFilter.min              = 65;   // Don't go below x% when decreasing after exceeding threshold limits
  systemSettings.Profile.tipFilter.count_limit      = 0;
  systemSettings.Profile.tipFilter.step             = -3;   // -5% less everytime the reading diff exceeds threshold_limit and the counter is greater than count_limit
  systemSettings.Profile.tipFilter.reset_threshold  = 600;  // Any diff over 500 reset the filter (Tip removed or connected)

  #ifdef USE_NTC
  systemSettings.Profile.ntc.enabled                = enable;
  #ifdef PULLUP
  systemSettings.Profile.ntc.pullup                 = 1;
  #elif defined PULLDOWN
  systemSettings.Profile.ntc.pullup                 = 0;
  #else
  #error NO PULL MODE DEFINED
  #endif
  systemSettings.Profile.ntc.detection              = 0;
  systemSettings.Profile.ntc.NTC_res                = NTC_RES/100;
  systemSettings.Profile.ntc.NTC_beta               = NTC_BETA;
  systemSettings.Profile.ntc.high_NTC_res           = 1000;           // 100.0K
  systemSettings.Profile.ntc.low_NTC_res            = 100;            // 10.0K
  systemSettings.Profile.ntc.high_NTC_beta          = NTC_BETA;
  systemSettings.Profile.ntc.low_NTC_beta           = NTC_BETA;
  systemSettings.Profile.ntc.pull_res               = PULL_RES/100;
  #else
  systemSettings.Profile.ntc.enabled                = 0;
  #endif

  systemSettings.Profile.errorTimeout               = 100;                    // ms
  systemSettings.Profile.errorResumeMode            = error_resume;
  systemSettings.Profile.sleepTimeout               = (uint32_t)5*60000;      // ms
  systemSettings.Profile.standbyTimeout             = (uint32_t)5*60000;
  systemSettings.Profile.standbyTemperature         = 180;
  systemSettings.Profile.UserSetTemperature         = 180;
  systemSettings.Profile.MaxSetTemperature          = 450;
  systemSettings.Profile.MinSetTemperature          = 180;
  systemSettings.Profile.boostTimeout               = 60000;                  // ms
  systemSettings.Profile.boostTemperature           = 50;
  systemSettings.Profile.pwmMul                     = 1;
  systemSettings.Profile.readPeriod                 = (200*200)-1;             // 200ms * 200  because timer period is 5us
  systemSettings.Profile.readDelay                  = (20*200)-1;              // 20ms (Also uses 5us clock)
  systemSettings.Profile.tempUnit                   = mode_Celsius;
  systemSettings.Profile.shakeFiltering             = disable;
  systemSettings.Profile.WakeInputMode              = mode_shake;
  systemSettings.Profile.StandMode                  = mode_sleep;
  systemSettings.Profile.state                      = initialized;
}

void loadProfile(uint8_t profile){
  while(ADC_Status!=ADC_Idle);
  __disable_irq();
  HAL_IWDG_Refresh(&hiwdg);
  systemSettings.settings.currentProfile=profile;

#ifdef NOSAVESETTINGS
  resetCurrentProfile();                                                        // Load default data
#else
  if(profile==profile_None){                                                    // If profile not initialized yet, use T12 values until the system is configured
    systemSettings.settings.currentProfile=profile_T12;                         // Force T12 profile
    __disable_irq();
    resetCurrentProfile();                                                      // Load default data
    __enable_irq();
    systemSettings.settings.currentProfile=profile_None;                        // Revert to none to trigger setup screen
  }
  else if(profile<=profile_C210){                                               // If valid profile
    if(flashSettings.Profile[profile].state!=initialized){                      // If flash profile not initialized
      __disable_irq();
      resetCurrentProfile();                                                    // Load defaults
      __enable_irq();
      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
    }
    else{
      systemSettings.Profile = flashSettings.Profile[profile];
      systemSettings.ProfileChecksum = flashSettings.ProfileChecksum[profile];
    }
    // Calculate data checksum and compare with stored checksum, also ensure the stored ID is the same as the requested profile
    if( (profile!=systemSettings.Profile.ID) || (systemSettings.ProfileChecksum != ChecksumProfile(&systemSettings.Profile)) ){
      __enable_irq();
      checksumError(reset_Profile);
      __disable_irq();
    }
#endif
#ifndef NOSAVESETTINGS
  }
  else{
    Error_Handler();
  }
#endif
  setSystemTempUnit(systemSettings.settings.tempUnit);                        // Ensure the profile uses the same temperature unit as the system
  setUserTemperature(systemSettings.Profile.UserSetTemperature);
  setCurrentTip(systemSettings.Profile.currentTip);
  TIP.filter=systemSettings.Profile.tipFilter;
  Iron.updatePwm=1;
  __enable_irq();
}

void Oled_error_init(void){
  setContrast(255);
  fillBuffer(BLACK,fill_soft);
  u8g2_SetFont(&u8g2,default_font );
  u8g2_SetDrawColor(&u8g2, WHITE);
  u8g2_SetMaxClipWindow(&u8g2);
  systemSettings.settings.OledOffset = OLED_OFFSET;
}

void Flash_error(void){
  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();
  fatalError(error_FLASH);
}

void checksumError(uint8_t mode){
  Oled_error_init();
  putStrAligned("BAD CHECKSUM!", 0, align_center);
  putStrAligned("RESTORING THE", 20, align_center);
  if(mode==reset_Profile){
    putStrAligned("PROFILE", 36, align_center);
  }
  else{
    putStrAligned("SYSTEM", 36, align_center);
  }
  update_display();
  ErrCountDown(3,117,50);
  if(mode==reset_Profile){
    __disable_irq();
    resetCurrentProfile();
    __enable_irq();
    saveSettings(keepProfiles);
  }
  else{
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  NVIC_SystemReset();
}

void Button_reset(void){
  uint16_t ResetTimer= HAL_GetTick();
  if(!BUTTON_input()){
    Oled_error_init();
    putStrAligned("HOLD BUTTON", 10, align_center);
    putStrAligned("TO RESTORE", 26, align_center);
    putStrAligned("DEFAULTS", 42, align_center);
    update_display();
    while(!BUTTON_input()){
      HAL_IWDG_Refresh(&hiwdg);
      if((HAL_GetTick()-ResetTimer)>5000){
        fillBuffer(BLACK,fill_dma);
        putStrAligned("RELEASE", 16, align_center);
        putStrAligned("BUTTON NOW", 32, align_center);
        update_display();
        while(!BUTTON_input()){
          HAL_IWDG_Refresh(&hiwdg);
        }
        resetSystemSettings();
        saveSettings(wipeProfiles);
        NVIC_SystemReset();
      }
    }
  }
}

//Max 99 seconds countdown.
void ErrCountDown(uint8_t Start,uint8_t  xpos, uint8_t ypos){
  uint32_t timErr = 0;
  char str[5];
  uint8_t length;
  if(Start>99){Start=99;}
  if(Start>9){
    length=2;
  }
  else{
    length=1;
  }
  do{
    HAL_IWDG_Refresh(&hiwdg);
  }while(oled.status!=oled_idle);

  while(Start){
    timErr=HAL_GetTick();
    u8g2_SetDrawColor(&u8g2, BLACK);
    u8g2_DrawBox(&u8g2,xpos,ypos,u8g2_GetUTF8Width(&u8g2,str),u8g2_GetMaxCharHeight(&u8g2));
    u8g2_SetDrawColor(&u8g2, WHITE);
    sprintf(&str[0],"%*u",length-1,Start);
    u8g2_DrawUTF8(&u8g2,xpos,ypos,str);
    update_display();
    while( (HAL_GetTick()-timErr)<999 ){
      HAL_IWDG_Refresh(&hiwdg);
    }
    Start--;
  }
}
