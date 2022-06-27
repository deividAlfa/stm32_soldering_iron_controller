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
#include "display.h"
#include "tempsensors.h"
#include "main.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "settings.c"
#endif

const settings_t defaultSettings = {
  .version              = (~((uint32_t)SETTINGS_VERSION<<16)&0xFFFF0000) | SETTINGS_VERSION,  // Higher 16bit is 1s complement to make detection stronger
#ifdef ST7565
  .contrastOrBrightness = 34,
#else
  .contrastOrBrightness = 255,
#endif
  .dim_mode             = dim_sleep,
  .dim_Timeout          = 10000,                // ms
  .dim_inSleep          = enable,
  .displayOffset        = DISPLAY_OFFSET,
  .displayXflip         = 1,
#ifdef SSD1306
  .displayYflip         = 1,
#elif defined ST7565
  .displayYflip         = 0,
  .displayResRatio      = 5,                    // For ST7565 only
#endif
  .guiUpdateDelay       = 200,                  // ms
  .guiTempDenoise       = 5,                    // ±5°C
  .tempUnit             = mode_Celsius,
  .tempStep             = 5,                    // 5º steps
  .tempBigStep          = 20,                   // 20º big steps
  .activeDetection      = true,
  .rememberLastProfile  = true,
  .rememberLastTemp     = false,
  .rememberLastTip      = true,
  .lvp                  = 110,                  // 11.0V Low voltage
  .bootProfile          = profile_None,
  .initMode             = mode_sleep,           // Safer to boot in sleep mode by default!
  .buzzerMode           = disable,
  .buttonWakeMode       = wake_all,
  .shakeWakeMode        = wake_all,
  .EncoderMode          = RE_Mode_Forward,
  .debugEnabled         = disable,
  .language             = lang_english,
  .state                = initialized,
};

#ifdef ENABLE_ADDONS
const addonSettings_t defaultAddonSettings = {
    .enabledAddons    = 0
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
                          + 0b1
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
                          + 0b10
#endif
  ,
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  .fumeExtractorMode     = fume_extractor_mode_auto,
  .fumeExtractorAfterrun = 2,
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
  .swOffReminderEnabled          = disable,
  .swOffReminderInactivityDelay  = 30,
  .swOffReminderBeepType         = switch_off_reminder_short_beep,
  .swOffReminderPeriod           = 5,
#endif
};
#endif

__attribute__((aligned(4))) typedef struct{
  settings_t      settings;
  uint32_t        settingsChecksum;
} flashSettingsSettings_t;
__attribute__((section(".globalSettings"))) flashSettingsSettings_t flashGlobalSettings;

__attribute__((aligned(4))) typedef struct{
  profile_t       Profile[NUM_PROFILES];
  uint32_t        ProfileChecksum[NUM_PROFILES];
} flashSettingsProfiles_t;
__attribute__((section(".profileSettings"))) flashSettingsProfiles_t flashProfilesSettings;

#ifdef ENABLE_ADDONS
__attribute__((aligned(4))) typedef struct{
  addonSettings_t addonSettings;
  uint32_t        addonSettingsChecksum;
} flashSettingsAddons_t;
__attribute__((section(".addonSettings"))) flashSettingsAddons_t flashAddonSettings;
#endif

systemSettings_t systemSettings;

static void saveSettings(uint8_t mode);
static void checksumError(uint8_t mode);
static void Flash_error(void);
static void Button_reset(void);
static void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);
static uint32_t ChecksumSettings(settings_t* settings);
static uint32_t ChecksumProfile(profile_t* profile);
static void resetSystemSettings(void);
static void resetCurrentProfile(void);

#ifdef ENABLE_ADDONS
static void loadAddonSettings(void);
static void resetAddonSettings();
static uint32_t ChecksumAddons(addonSettings_t* addonSettings);
#endif

#ifdef HAS_BATTERY

#define BACKUP_RAM_SIZE_IN_BYTES 20u
#define NUM_BACKUP_RAM_REGISTERS (BACKUP_RAM_SIZE_IN_BYTES / 2u) // each register holds 2 byte of data in the lower nibble

typedef struct
{
  // max (BACKUP_RAM_SIZE_IN_BYTES - 4) byte of data in total
  uint16_t lastTipTemp[NUM_PROFILES];
  uint8_t  lastSelTip[NUM_PROFILES];
  uint8_t  lastProfile;
} backupRamValues_t;

typedef union
{
  __attribute__((aligned(4))) struct
  {
    backupRamValues_t values;
    uint32_t crc;
  };
  uint8_t bytes[BACKUP_RAM_SIZE_IN_BYTES];
} backupRamData_t;

static void loadSettingsFromBackupRam(void);
static uint32_t ChecksumBackupRam();
static void readBackupRam();
static void writeBackupRam();

static backupRamData_t bkpRamData;

#endif

extern int  __SETTINGS_SECTION_START; /* defined by the linker, only its address which is the target value */
#define SETTINGS_SECTION_START ((uint32_t)&__SETTINGS_SECTION_START)
extern int  __SETTINGS_SECTION_LENGTH; /* defined by the linker, only its address which is the target value */
#define SETTINGS_SECTION_LENGTH ((uint32_t)&__SETTINGS_SECTION_LENGTH)

void checkSettings(void){

  #ifdef NOSAVESETTINGS
  return;
  #endif

  static uint32_t prevSysChecksum    = 0u;
  static uint32_t newSysChecksum     = 0u;
  static uint32_t prevTipChecksum    = 0u;
  static uint32_t newProfileChecksum = 0u;
#ifdef ENABLE_ADDONS
  static uint32_t prevAddonChecksum  = 0u;
  static uint32_t newAddonChecksum   = 0u;
#endif
  static uint32_t lastCheckTime=0;
  static uint32_t lastChangeTime=0;
  uint32_t CurrentTime = HAL_GetTick();
  uint8_t scr_index=current_screen->index;

  // To reduce heap usage, only allow saving in smaller screens.
  // Content change detection will be still active, but saving will postponed upon returning to a smaller screen.
  // This is done to ensure compatibility with 10KB RAM devices yet allowing the firmware to grow unconstrained by ram usage
  uint8_t allowSave = (scr_index == screen_boot              ||
                       scr_index == screen_main              ||
                       scr_index == screen_settings          ||
                       scr_index == screen_calibration       ||
                       scr_index == screen_reset_confirmation);

#ifndef HAS_BATTERY
  if(systemSettings.settings.rememberLastProfile)
  {
    systemSettings.settings.bootProfile = systemSettings.currentProfile;
  }
  if(systemSettings.settings.rememberLastTip)
  {
    systemSettings.Profile.defaultTip = systemSettings.currentTip;
  }
#endif

  // Save from menu
  if(systemSettings.save_Flag && allowSave){
    switch(systemSettings.save_Flag){
      case save_Settings:
        saveSettings(keepProfiles);
        break;
      case reset_Profiles:
        systemSettings.currentProfile=profile_None;
        saveSettings(wipeProfiles);
        break;
      case reset_Profile:
        resetCurrentProfile();
        saveSettings(keepProfiles);
        break;
      case reset_Settings:
      {
        uint8_t currentProfile=systemSettings.currentProfile;
        resetSystemSettings();
        systemSettings.currentProfile=currentProfile;
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
    if(systemSettings.save_Flag>=reset_Profiles){       // If save flag indicates any resetting mode, reboot
      NVIC_SystemReset();
    }
    systemSettings.save_Flag=0;
    return;
  }

  // Auto save on content change
  if( (systemSettings.setupMode==enable) || (isIronInCalibrationMode()) || (getIronErrorFlags().safeMode) || ((CurrentTime-lastCheckTime)<999)){
    return;
  }

  lastCheckTime = CurrentTime;                                          // Store current time
  newSysChecksum     = ChecksumSettings(&systemSettings.settings);      // Calculate system checksum
  newProfileChecksum = ChecksumProfile(&systemSettings.Profile);        // Calculate profile checksum
#ifdef ENABLE_ADDONS
  newAddonChecksum   = ChecksumAddons(&(systemSettings.addonSettings)); // Calculate the addon checksum
#endif

  // If anything was changed (Checksum mismatch)
  if(   (systemSettings.settingsChecksum      != newSysChecksum    )
     || (systemSettings.ProfileChecksum       != newProfileChecksum)
#ifdef ENABLE_ADDONS
     || (systemSettings.addonSettingsChecksum != newAddonChecksum  )
#endif
  ){

    if(    (prevSysChecksum != newSysChecksum)
        || (prevTipChecksum != newProfileChecksum)
#ifdef ENABLE_ADDONS
        || (prevAddonChecksum != newAddonChecksum)
#endif
    ){
      // If different from the previous calculated checksum (settings are being changed quickly, don't save every time).
      prevSysChecksum = newSysChecksum;                       // Store last computed checksum
      prevTipChecksum = newProfileChecksum;
#ifdef ENABLE_ADDONS
      prevAddonChecksum = newAddonChecksum;
#endif
      lastChangeTime = CurrentTime;                           // Reset timer (we don't save anything until we pass a certain time without changes)
    }

    else if(allowSave && ((CurrentTime-lastChangeTime)>5000)){ // If different from the previous calculated checksum, and timer expired (No changes in the last 5 sec)
      saveSettings(save_Settings);                             // Data was saved (so any pending interrupt knows this)
    }
  }

  #ifdef HAS_BATTERY
  bkpRamData.values.lastProfile = systemSettings.currentProfile;
  if(!isIronInCalibrationMode() && scr_index != screen_debug) // don't persist the temperature while calibration is in progress or in the debug screen
  {
    bkpRamData.values.lastTipTemp[systemSettings.currentProfile] = getUserSetTemperature();
  }
  bkpRamData.values.lastSelTip[systemSettings.currentProfile] = systemSettings.currentTip;
  writeBackupRam();
  #endif
}


//This is done to avoid huge stack build up. Trigger saving using checkSettings with a flag instead direct call from menu.
// Thus, the flags stays after the screen exits. The code handling settings saving decides when it's ok to store them.
void saveSettingsFromMenu(uint8_t mode){
  systemSettings.save_Flag=mode;
  if(mode>=reset_Profiles){           // Force safe mode (disable iron power) in any resetting mode, the station will reboot when done.
    setSafeMode(enable);
  }
}

static void eraseFlashPages(uint32_t pageAddress, uint32_t numPages)
{
  uint32_t error = 0;
  FLASH_EraseInitTypeDef erase = {0};

  __disable_irq();
  configurePWMpin(output_Low);

  HAL_FLASH_Unlock();

  erase.NbPages     = numPages;
  erase.PageAddress = pageAddress;
  erase.TypeErase   = FLASH_TYPEERASE_PAGES;

  HAL_IWDG_Refresh(&hiwdg);
  if((HAL_FLASHEx_Erase(&erase, &error)!=HAL_OK) || (error!=0xFFFFFFFF)){
    Flash_error();
  }
  HAL_FLASH_Lock();
  __enable_irq();

  // Ensure flash was erased
  for (uint32_t i = 0u; i < (numPages * FLASH_PAGE_SIZE / sizeof(int32_t)); i++) {
    if( *((uint32_t*)(pageAddress + i)) != 0xFFFFFFFF){
      Flash_error();
    }
  }
}

static void writeFlash(uint32_t* src, uint32_t len, uint32_t dstAddr)
{
  uint32_t const numWordsToWrite = (len + sizeof(uint32_t) - 1u) / sizeof(uint32_t);
  uint32_t* srcData = src;

  __disable_irq();
  HAL_FLASH_Unlock();
  __enable_irq();

  // written = number of 32-bit values written
  for(uint32_t written=0; written < numWordsToWrite; written++){
    __disable_irq();
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dstAddr, *srcData ) != HAL_OK)
    {
      Flash_error();
    }
    dstAddr += sizeof(uint32_t); // increase pointers
    srcData++;
    __enable_irq();
  }

  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();
}

static void saveSettings(uint8_t mode){
#ifdef NOSAVESETTINGS
  return;
#else

  uint8_t const profile = systemSettings.currentProfile;

  flashSettingsProfiles_t* flashBufferProfiles = _malloc(sizeof(flashSettingsProfiles_t));
  flashSettingsSettings_t* flashBufferSettings = _malloc(sizeof(flashSettingsSettings_t));
#ifdef ENABLE_ADDONS
  flashSettingsAddons_t*   flashBufferAddons   = _malloc(sizeof(flashSettingsAddons_t));
#endif

  // check if malloc succeeded or not
  if(    (flashBufferProfiles == NULL)
      || (flashBufferSettings == NULL)
#ifdef ENABLE_ADDONS
      || (flashBufferAddons   == NULL)
#endif
  )
  { Error_Handler(); }

  *flashBufferProfiles = flashProfilesSettings; // Save current profiles into the temp buffer

  while(ADC_Status != ADC_Idle);
  __disable_irq();
  systemSettings.isSaving = 1;
  configurePWMpin(output_Low);
  __enable_irq();

  if( (systemSettings.settings.state!=initialized) || (systemSettings.Profile.state!=initialized) ){
    Error_Handler();
  }

  systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);
  flashBufferSettings->settingsChecksum = systemSettings.settingsChecksum;
  flashBufferSettings->settings = systemSettings.settings;

#ifdef ENABLE_ADDONS
  systemSettings.addonSettingsChecksum = ChecksumAddons(&(systemSettings.addonSettings));
  flashBufferAddons->addonSettingsChecksum = systemSettings.addonSettingsChecksum;
  flashBufferAddons->addonSettings = systemSettings.addonSettings;
#endif

  if((mode==keepProfiles) && (profile<=profile_C210) && (systemSettings.Profile.ID == profile )){
    systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
    flashBufferProfiles->ProfileChecksum[profile] = systemSettings.ProfileChecksum;
    flashBufferProfiles->Profile[profile] = systemSettings.Profile;
  }
  else{
    mode = wipeProfiles;
    for(uint8_t x=0;x<NUM_PROFILES;x++){
      flashBufferProfiles->Profile[x].state = 0xFF;
      flashBufferProfiles->ProfileChecksum[x] = 0xFFFFFFFF;
      memset(&flashBufferProfiles->Profile[x],0xFF,sizeof(profile_t));
    }
  }

  eraseFlashPages(SETTINGS_SECTION_START, (SETTINGS_SECTION_LENGTH+FLASH_PAGE_SIZE-1) / FLASH_PAGE_SIZE);

  writeFlash((uint32_t*)flashBufferProfiles, sizeof(flashSettingsProfiles_t), (uint32_t)&flashProfilesSettings);
  writeFlash((uint32_t*)flashBufferSettings, sizeof(flashSettingsSettings_t), (uint32_t)&flashGlobalSettings);
#ifdef ENABLE_ADDONS
  writeFlash((uint32_t*)flashBufferAddons, sizeof(flashSettingsAddons_t), (uint32_t)&flashAddonSettings);
#endif

  if(mode==keepProfiles){
    uint32_t ProfileFlash  = ChecksumProfile(&flashProfilesSettings.Profile[profile]);
    uint32_t ProfileRam    = ChecksumProfile(&systemSettings.Profile);

    if(ProfileFlash != ProfileRam){
      Flash_error();
    }
  }

  // Check flash and system settings have same checksum
  uint32_t SettingsFlash  = ChecksumSettings(&flashGlobalSettings.settings);
  uint32_t SettingsRam  = ChecksumSettings(&systemSettings.settings);
  if(SettingsFlash != SettingsRam){
    Flash_error();
  }

#ifdef ENABLE_ADDONS
  // Verify addon crc
  uint32_t addonsFlashCrc  = ChecksumAddons(&flashAddonSettings.addonSettings);
  uint32_t addonsRamCrc  = ChecksumAddons(&systemSettings.addonSettings);
  if(addonsFlashCrc != addonsRamCrc){
    Flash_error();
  }
#endif

#ifdef ENABLE_ADDONS
  _free(flashBufferAddons);
#endif
  _free(flashBufferSettings);
  _free(flashBufferProfiles);
  __disable_irq();
  systemSettings.isSaving = 0;
  __enable_irq();

#endif
}

void restoreSettings() {
#ifdef NOSAVESETTINGS                                                 // Stop erasing the flash while in debug mode
  __disable_irq();
  resetSystemSettings();
  systemSettings.settings.currentProfile = profile_T12;
  resetCurrentProfile();
  loadProfile(systemSettings.settings.currentProfile);
  __enable_irq();
  return;
#endif

  if(flashGlobalSettings.settings.state != initialized){
    resetSystemSettings();
#ifdef ENABLE_ADDONS
    resetAddonSettings();
#endif
    saveSettings(wipeProfiles);
  }
  else{
    Button_reset();
  }

  if(flashGlobalSettings.settings.version!=defaultSettings.version){    // Silent reset if version mismatch
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  else{
    systemSettings.settings = flashGlobalSettings.settings;
    systemSettings.settingsChecksum = flashGlobalSettings.settingsChecksum;
  }


  if(ChecksumSettings(&systemSettings.settings)!=systemSettings.settingsChecksum){   // Show error message if bad checksum
    checksumError(reset_All);
  }

  loadProfile(systemSettings.settings.bootProfile); // assume the boot profile

#ifdef ENABLE_ADDONS
  loadAddonSettings();
#endif

#ifdef HAS_BATTERY
  loadSettingsFromBackupRam();
  if(systemSettings.settings.rememberLastProfile)
  {
    loadProfile(bkpRamData.values.lastProfile);
  }
  if(systemSettings.settings.rememberLastTip)
  {
    setCurrentTip(bkpRamData.values.lastSelTip[systemSettings.currentProfile]);
    ironSchedulePwmUpdate();
  }
#endif
}

#ifdef HAS_BATTERY

void loadSettingsFromBackupRam(void)
{
  // assert on backup ram size
  if(sizeof(backupRamValues_t) + sizeof(uint32_t) > BACKUP_RAM_SIZE_IN_BYTES)
  {
    Error_Handler(); // can't put this much data into the backup ram
  }

  readBackupRam();

  // check crc
  if(bkpRamData.crc != ChecksumBackupRam())
  {
    // restore defaults, show error
    memset((void*)&bkpRamData,0, sizeof(backupRamData_t));
    for(uint8_t i = 0; i < NUM_PROFILES; i++)
    {
      bkpRamData.values.lastTipTemp[i] = flashProfilesSettings.Profile[i].defaultTemperature;
      if(bkpRamData.values.lastTipTemp[i] == UINT16_MAX) // just a sanity check to handle uninitialized data
      {
        bkpRamData.values.lastTipTemp[i] = 0u;
      }
      bkpRamData.values.lastSelTip[i] = 0u;
    }
    bkpRamData.values.lastProfile = systemSettings.settings.bootProfile;
    writeBackupRam();

    Oled_error_init();
    putStrAligned("New/low batt?", 0, align_center);
    putStrAligned("Forgot last", 16, align_center);
    putStrAligned("used settings.", 32, align_center);
    putStrAligned("Restored dflt.", 48, align_left);
    update_display();
    ErrCountDown(3,117,50);
    NVIC_SystemReset();
  }
}

void restoreLastSessionSettings(void)
{
  if(systemSettings.settings.rememberLastTemp)
  {
    setUserTemperature(bkpRamData.values.lastTipTemp[systemSettings.currentProfile]);
  }
}

static void readBackupRam()
{
  for(uint8_t i = 0; i < NUM_BACKUP_RAM_REGISTERS; i++)
  {
    uint16_t const data = (uint16_t)*(&(BKP->DR1) + i);
    bkpRamData.bytes[i*2  ] = data;
    bkpRamData.bytes[i*2+1] = data >> 8u;
  }
}

static void writeBackupRam()
{
  bkpRamData.crc = ChecksumBackupRam(bkpRamData);
  for(uint8_t i = 0; i < NUM_BACKUP_RAM_REGISTERS; i++)
  {
    *(&(BKP->DR1) + i) = bkpRamData.bytes[i*2] + ((uint32_t)bkpRamData.bytes[i*2+1] << 8u);
  }
}

static uint32_t ChecksumBackupRam(){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)bkpRamData.bytes, sizeof(backupRamValues_t)/sizeof(uint32_t) );
  return checksum;
}

#endif

static uint32_t ChecksumSettings(settings_t* settings){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)settings, sizeof(settings_t)/sizeof(uint32_t) );
  return checksum;
}

static uint32_t ChecksumProfile(profile_t* profile){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile, sizeof(profile_t)/sizeof(uint32_t));
  return checksum;
}

#ifdef ENABLE_ADDONS
static void loadAddonSettings(void)
{
  systemSettings.addonSettings = flashAddonSettings.addonSettings;
  systemSettings.addonSettingsChecksum = ChecksumAddons(&(systemSettings.addonSettings));
  if((systemSettings.addonSettings.enabledAddons != defaultAddonSettings.enabledAddons) || // list of addons changed
      (systemSettings.addonSettingsChecksum != flashAddonSettings.addonSettingsChecksum))       // crc mismatch
  {
    checksumError(reset_Addons);
  }
}

static void resetAddonSettings()
{
  __disable_irq();
  systemSettings.addonSettings = defaultAddonSettings;
  systemSettings.addonSettingsChecksum = 0u;
  __enable_irq();
}

static uint32_t ChecksumAddons(addonSettings_t* addonSettings){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)addonSettings, sizeof(addonSettings_t)/sizeof(uint32_t));
  return checksum;
}
#endif

static void resetSystemSettings(void) {
  __disable_irq();
  systemSettings.settings = defaultSettings;
  __enable_irq();
}


static void resetCurrentProfile(void){
  __disable_irq();
    if(systemSettings.currentProfile==profile_T12){
    systemSettings.Profile.ID = profile_T12;
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = T12_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = T12_Cal400;     // These values are way lower, but better to be safe than sorry
      systemSettings.Profile.tip[x].PID.Kp          = 4000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki          = 5500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd          = 700;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI        = 70;             // val = /100
      systemSettings.Profile.tip[x].PID.minI        = 0;              // val = /100
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);         // Empty name
    }
    strcpy(systemSettings.Profile.tip[0].name, "BC3 ");               // Put some generic name
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.defaultTip               = 0;
    systemSettings.Profile.impedance                = 80;             // 8.0 Ohms
    systemSettings.Profile.power                    = 80;             // 80W
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = T12_Cal250;
    systemSettings.Profile.Cal400_default           = T12_Cal400;
  }

  else if(systemSettings.currentProfile==profile_C245){
    systemSettings.Profile.ID = profile_C245;
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C245_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = C245_Cal400;
      systemSettings.Profile.tip[x].PID.Kp          = 4000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki          = 5500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd          = 700;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI        = 70;             // val = /100
      systemSettings.Profile.tip[x].PID.minI        = 0;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C245");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.defaultTip               = 0;
    systemSettings.Profile.impedance                = 26;
    systemSettings.Profile.power                    = 150;
    systemSettings.Profile.noIronValue              = 4000;
    systemSettings.Profile.Cal250_default           = C245_Cal250;
    systemSettings.Profile.Cal400_default           = C245_Cal400;
  }

  else if(systemSettings.currentProfile==profile_C210){
    systemSettings.Profile.ID = profile_C210;
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      systemSettings.Profile.tip[x].calADC_At_250   = C210_Cal250;
      systemSettings.Profile.tip[x].calADC_At_400   = C210_Cal400;
      systemSettings.Profile.tip[x].PID.Kp          = 4000;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Ki          = 5500;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.Kd          = 700;           // val = /1.000.000
      systemSettings.Profile.tip[x].PID.maxI        = 70;             // val = /100
      systemSettings.Profile.tip[x].PID.minI        = 0;
      strcpy(systemSettings.Profile.tip[x].name, _BLANK_TIP);
    }
    strcpy(systemSettings.Profile.tip[0].name, "C210");
    systemSettings.Profile.currentNumberOfTips      = 1;
    systemSettings.Profile.defaultTip             = 0;
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
  systemSettings.Profile.tipFilter.coefficient      = 75;   // % of old data (more %, more filtering)
  systemSettings.Profile.tipFilter.threshold        = 50;
  systemSettings.Profile.tipFilter.min              = 50;   // Don't go below x% when decreasing after exceeding threshold limits
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
  systemSettings.Profile.defaultTemperature         = 320;
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
  systemSettings.currentProfile=profile;
  if(profile==profile_None){                                                    // If profile not initialized yet, use T12 values until the system is configured
    systemSettings.currentProfile=profile_T12;                         // Force T12 profile
    resetCurrentProfile();                                                      // Load default data
    systemSettings.currentProfile=profile_None;                        // Revert to none to trigger setup screen
  }
  else if(profile<=profile_C210){                                               // If valid profile
    if(flashProfilesSettings.Profile[profile].state!=initialized){                      // If flash profile not initialized
      resetCurrentProfile();                                                    // Load defaults
      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
    }
    else{
      systemSettings.Profile = flashProfilesSettings.Profile[profile];
      systemSettings.ProfileChecksum = flashProfilesSettings.ProfileChecksum[profile];
    }
    // Calculate data checksum and compare with stored checksum, also ensure the stored ID is the same as the requested profile
    if( (profile!=systemSettings.Profile.ID) || (systemSettings.ProfileChecksum != ChecksumProfile(&systemSettings.Profile)) ){
      checksumError(reset_Profile);
    }
  }
  else{
    Error_Handler();
  }
  setSystemTempUnit(getSystemTempUnit());                        // Ensure the profile uses the same temperature unit as the system
  setUserTemperature(systemSettings.Profile.defaultTemperature);
  setCurrentTip(systemSettings.Profile.defaultTip);
  TIP.filter=systemSettings.Profile.tipFilter;
  ironSchedulePwmUpdate();
  __enable_irq();
}

static void Flash_error(void){
  __disable_irq();
  HAL_FLASH_Lock();
  __enable_irq();
  fatalError(error_FLASH);
}

static void checksumError(uint8_t mode){
  Oled_error_init();
  putStrAligned("BAD CHECKSUM!", 0, align_center);
  putStrAligned("RESTORING THE", 20, align_center);
  if(mode==reset_Profile){
    putStrAligned("PROFILE", 36, align_center);
  }
#ifdef ENABLE_ADDONS
  else if(mode == reset_Addons)
  {
    putStrAligned("ADDONS", 36, align_center);
  }
#endif
  else{
    putStrAligned("SYSTEM", 36, align_center);
  }
  update_display();
  ErrCountDown(3,117,50);
  if(mode == reset_Profile){
    resetCurrentProfile();
    saveSettings(keepProfiles);
  }
#ifdef ENABLE_ADDONS
  else if(mode == reset_Addons)
  {
    resetAddonSettings();
    saveSettings(keepProfiles);
  }
#endif
  else{
    resetSystemSettings();
    saveSettings(wipeProfiles);
  }
  NVIC_SystemReset();
}

static void Button_reset(void){
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
#ifdef ENABLE_ADDONS
        resetAddonSettings();
#endif
        saveSettings(wipeProfiles);
        NVIC_SystemReset();
      }
    }
  }
}

bool isCurrentProfileChanged(void)
{
  return ChecksumProfile(&systemSettings.Profile) != systemSettings.ProfileChecksum;
}

//Max 99 seconds countdown.
static void ErrCountDown(uint8_t Start,uint8_t  xpos, uint8_t ypos){
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
