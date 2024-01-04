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
#endif
#define __BASE_FILE__ "settings.c"

const systemSettings_t defaultSystemSettings = {
  .version              =  SYSTEM_SETTINGS_VERSION,
#ifdef ST7565
  .contrastOrBrightness = 34,
#else
  .contrastOrBrightness = 255,
#endif
  .dim_mode             = dim_always,
  .dim_Timeout          = 10000,                // ms
  .dim_inSleep          = enable,
  .displayStartColumn   = DISPLAY_START_COLUMN,
  .displayStartLine     = DISPLAY_START_LINE,
  .displayXflip         = 1,
#ifdef SSD1306
  .displayClk           = 0xF0,
  .displayVcom          = 0x44,
  .displayPrecharge     = 0x3C,
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
  .hasBattery           = false,
  .coldBoost            = true,
  .lvp                  = 110,                  // 11.0V Low voltage
  .initMode             = mode_sleep,           // Safer to boot in sleep mode by default!
  .buzzerMode           = disable,
  .buttonWakeMode       = wake_all,
  .shakeWakeMode        = wake_all,
  .EncoderMode          = RE_Mode_Forward,
  .debugEnabled         = disable,
  .language             = lang_english,
  .clone_fix            = disable,
};

#ifdef ENABLE_ADDONS
const addonSettings_t defaultAddons = {
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

const profile_settings_t defaultProfileSettings = {
  .calADC_At_0                = 0,
  .tipFilter.coefficient      = 75,   // % of old data (more %, more filtering)
  .tipFilter.threshold        = 50,
  .tipFilter.min              = 50,   // Don't go below x% when decreasing after exceeding threshold limits
  .tipFilter.count_limit      = 0,
  .tipFilter.step             = -3,   // -5% less everytime the reading diff exceeds threshold_limit and the counter is greater than count_limit
  .tipFilter.reset_threshold  = 600,  // Any diff over 500 reset the filter (Tip removed or connected)

  #ifdef USE_NTC
  .ntc.enabled                = enable,
  #ifdef PULLUP
  .ntc.pullup                 = 1,
  #elif defined PULLDOWN
  .ntc.pullup                 = 0,
  #else
  #error NO PULL MODE DEFINED
  #endif
  .ntc.detection              = 0,
  .ntc.NTC_res                = NTC_RES/100,
  .ntc.NTC_beta               = NTC_BETA,
  .ntc.high_NTC_res           = 1000,           // 100.0K
  .ntc.low_NTC_res            = 100,            // 10.0K
  .ntc.high_NTC_beta          = NTC_BETA,
  .ntc.low_NTC_beta           = NTC_BETA,
  .ntc.pull_res               = PULL_RES/100,
  #else
  .ntc.enabled                = 0,
  #endif

  .errorTimeout               = 500,                    // ms
  .errorResumeMode            = error_resume,
  .sleepTimeout               = (uint32_t)5*60000,      // ms
  .standbyTimeout             = (uint32_t)5*60000,
  .standbyTemperature         = 180,
  .defaultTemperature         = 320,
  .MaxSetTemperature          = 450,
  .MinSetTemperature          = 180,
  .boostTimeout               = 60000,                  // ms
  .boostTemperature           = 50,
  .pwmMul                     = 1,
  .readPeriod                 = (200*200)-1,             // 200ms * 200  because timer period is 5us
  .readDelay                  = (20*200)-1,              // 20ms (Also uses 5us clock)
  .tempUnit                   = mode_Celsius,
  .shakeFiltering             = disable,
  .WakeInputMode              = mode_shake,
  .smartActiveEnabled         = disable,
  .smartActiveLoad            = 30,
  .standDelay                 = 0,
  .StandMode                  = mode_sleep,
  .version                    = PROFILE_SETTINGS_VERSION,
};


const tipData_t defaultTipData[NUM_PROFILES] = {
  [profile_T12] = {
    .calADC_At_250   = T12_Cal250,
    .calADC_At_400   = T12_Cal400,     // These values are way lower, but better to be safe than sorry
    .PID.Kp          = 4000,           // val = /1.000.000
    .PID.Ki          = 5500,           // val = /1.000.000
    .PID.Kd          = 700,           // val = /1.000.000
    .PID.maxI        = 70,             // val = /100
    .PID.minI        = 0,              // val = /100
    .name            = "T12-",               // Put some generic name
  },
  [profile_C245] = {
    .calADC_At_250   = C245_Cal250,
    .calADC_At_400   = C245_Cal400,
    .PID.Kp          = 4000,           // val = /1.000.000
    .PID.Ki          = 5500,           // val = /1.000.000
    .PID.Kd          = 700,           // val = /1.000.000
    .PID.maxI        = 70,             // val = /100
    .PID.minI        = 0,
    .name            = "C245-",
  },
  [profile_C210] = {
    .calADC_At_250   = C210_Cal250,
    .calADC_At_400   = C210_Cal400,
    .PID.Kp          = 4000,           // val = /1.000.000
    .PID.Ki          = 5500,           // val = /1.000.000
    .PID.Kd          = 700,           // val = /1.000.000
    .PID.maxI        = 70,             // val = /100
    .PID.minI        = 0,
    .name            =  "C210-",
  },
};


__attribute__((section(".globalSettings"))) flashSettings_t flashSettings;
__attribute__((section(".profileSettings"))) flashProfiles_t flashProfiles;
__attribute__((section(".tempSettings"))) temp_settings_t temp_settings;
#ifdef ENABLE_ADDONS
__attribute__((section(".addonSettings"))) flashAddons_t flashAddons;
#endif

static flashTemp_t flashTemp;

#if (__CORTEX_M == 3)                 // STM32F1xx flash page size are 1K or 2K depending on the flash size
static uint8_t flashPages_GlobalSettings, flashPages_TempSettings;
static uint16_t flashPageSize;
#elif (__CORTEX_M == 0)               // All STM32F07x have 2KB flash pages
  #if (FLASH_PAGE_SIZE != 2048)
    #error Wrong flash page size??
  #endif
#define flashPageSize                 FLASH_PAGE_SIZE
#define flashPages_Global     ((SETTINGS_SECTION_LENGTH + flashPageSize - 1) / flashPageSize)
#define flashPages_TempSettings       ((TEMP_SETTINGS_SECTION_LENGTH + flashPageSize - 1) / flashPageSize)
#endif


settings_t settings;

static void checksumError(uint8_t mode);
static void Button_reset(void);
static void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);
static uint32_t ChecksumSystemSettings(systemSettings_t* system);
static uint32_t ChecksumProfileSettings(profile_settings_t* profile_settings);
static uint32_t ChecksumProfile(profile_t* profile);
static ErrorStatus checkFlashProfiles(void);
static void resetSystemSettings(systemSettings_t * system) ;
static void resetProfile(profile_t * data, uint8_t profile);
static void resetProfileSettings(profile_settings_t * data, uint8_t profile);
static void sortTips(profile_t * data, uint8_t profile);
static void flashTempSettingsErase(void);
static void flashTempSettingsInit(void);
static void flashTempWrite(void);
static void flashTipWrite(void);
static void flashProfileWrite(void);

#ifdef ENABLE_ADDONS
static uint8_t loadAddonSettings(void);
static void resetAddonSettings(addonSettings_t *addons);
static uint32_t ChecksumAddons(addonSettings_t* addonSettings);
#endif

static void loadSettingsFromBackupRam(void);
static uint32_t ChecksumBackupRam(void);
static void readBackupRam(void);
static void writeBackupRam(void);

static backupRamData_t bkpRamData;


extern int  __SETTINGS_SECTION_START; /* defined by the linker, only its address which is the target value */
#define SETTINGS_SECTION_START ((uint32_t)&__SETTINGS_SECTION_START)
extern int  __SETTINGS_SECTION_LENGTH; /* defined by the linker, only its address which is the target value */
#define SETTINGS_SECTION_LENGTH ((uint32_t)&__SETTINGS_SECTION_LENGTH)

extern int  __TEMP_SETTINGS_SECTION_START; /* defined by the linker, only its address which is the target value */
#define TEMP_SETTINGS_SECTION_START ((uint32_t)&__TEMP_SETTINGS_SECTION_START)
extern int  __TEMP_SETTINGS_SECTION_LENGTH; /* defined by the linker, only its address which is the target value */
#define TEMP_SETTINGS_SECTION_LENGTH ((uint32_t)&__TEMP_SETTINGS_SECTION_LENGTH)


void updateTempData(bool force){
  uint32_t CurrentTime = HAL_GetTick();
  uint8_t scr_index=current_screen->index;

  if(getIronCalibrationMode() || getSettings()->setupMode || scr_index == screen_debug)                                 // Don't update while calibration is in progress or in the debug screen
    return;
                                                                                                                        // Always update the data, whether the batery option is enabled or not
  if(getSystemSettings()->hasBattery == true){
    bkpRamData.values.lastSelTip[getCurrentProfile()] = getCurrentTip();
    bkpRamData.values.lastProfile = getCurrentProfile();
    bkpRamData.values.lastTipTemp[getCurrentProfile()] = getUserTemperature();
    writeBackupRam();
  }
  else if ((getSystemSettings()->hasBattery == false) || force){                                                      // No battery or forced
    uint16_t currentTemp = getUserTemperature();
    uint8_t currentTip = getCurrentTip();
    uint8_t currentProfile = getCurrentProfile();

    if(force){
      flashTemp.prevTemp = currentTemp;
      flashTemp.prevTip[currentProfile] = currentTip;
      flashTemp.prevProfile = currentProfile;
    }

    if(flashTemp.temp != currentTemp) {                                       // Compare with stored in flash
      if(flashTemp.prevTemp != currentTemp) {                                      // Store if different to last check
        flashTemp.prevTemp = currentTemp;
        flashTemp.tempCheckTime = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-flashTemp.tempCheckTime)>4999)) {                           // Different than flash and timeout is over
        flashTemp.temp = currentTemp;
        flashTempWrite();                                                           // Update temperature in flash
      }
    }
    if(flashTemp.tip[currentProfile] != currentTip) {
      if(flashTemp.prevTip[currentProfile] != currentTip) {                        // Store if different to last check
        flashTemp.prevTip[currentProfile] = currentTip;
        flashTemp.tipCheckTime = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-flashTemp.tipCheckTime)>4999)) {                           // Different than flash and timeout is over
        for(uint8_t i=0;i<NUM_PROFILES;i++){                                            // Update all tips
          flashTemp.tip[i] = flashTemp.prevTip[i];
        }
        flashTipWrite();                                                            // Update tip data in flash
      }
    }
    if(flashTemp.profile != currentProfile) {
      if(flashTemp.prevProfile != currentProfile) {                                // Store if different to last check
        flashTemp.prevProfile = currentProfile;
        flashTemp.profileCheckTime = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-flashTemp.profileCheckTime)>4999)) {                           // Different than flash and timeout is over
        flashTemp.profile = currentProfile;
        flashProfileWrite();                                                        // Update tip data in flash
      }
    }
  }
}

static void eraseFlashPages(uint32_t pageAddress, uint32_t numPages)
{
  uint32_t error = 0;
  FLASH_EraseInitTypeDef erase = {0};

  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  configurePWMpin(output_Low);

  HAL_FLASH_Unlock();
  __set_PRIMASK(_irq);

  erase.NbPages     = numPages;
  erase.PageAddress = pageAddress;
  erase.TypeErase   = FLASH_TYPEERASE_PAGES;

  HAL_IWDG_Refresh(&hiwdg);
  if((HAL_FLASHEx_Erase(&erase, &error)!=HAL_OK) || (error!=0xFFFFFFFF)){
     Error_Handler();
  }
  HAL_FLASH_Lock();

  // Ensure flash was erased
  for (uint32_t i = 0u; i < (numPages * flashPageSize / sizeof(int32_t)); i++) {
    if( *((uint32_t*)pageAddress+i) != 0xFFFFFFFF){
      Error_Handler();
    }
  }
}

static void writeFlash(uint32_t* src, uint32_t len, uint32_t dstAddr)
{
  uint32_t numWordsToWrite = (len + sizeof(uint32_t) - 1u) / sizeof(uint32_t);
  uint32_t* srcData = src;

  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  HAL_FLASH_Unlock();
  __set_PRIMASK(_irq);

  // written = number of 32-bit values written
  for(uint32_t written=0; written < numWordsToWrite; written++){
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dstAddr, *srcData ) != HAL_OK)
    {
       Error_Handler();
    }
    dstAddr += sizeof(uint32_t); // increase pointers
    srcData++;
  }
  HAL_FLASH_Lock();
}

void saveSettings(uint8_t save_mode, uint8_t tip_mode, uint8_t tip_index, uint8_t reboot_mode){
  if(reboot_mode  == do_reboot)                                        // Force safe save_mode (disable iron power) if rebooting.
    setSafeMode(enable);

  #ifndef DEBUG
  struct mallinfo mi = mallinfo();
#endif
  if(mi.uordblks > 128)                   // Check that heap usage is low
    Error_Handler();                      // We got there from a working screen (Heap must be free)
  uint8_t needs_saving = save_mode;
  uint32_t _irq = __get_PRIMASK();

  if( ((save_mode & save_Settings) && (save_mode & reset_Settings)) ||        // Sanity check
      ((save_mode & save_Profile) && (save_mode & reset_Profile))   ||
      ((save_mode & save_Profile) && (save_mode & reset_Profiles))  ||
      ((save_mode & save_Addons) && (save_mode & reset_Addons))     ){

    Error_Handler();
  }

  uint8_t profile = getCurrentProfile();

  if((profile>profile_C210) || (getProfileSettings()->ID != profile ))                                   // Sanity check
      Error_Handler();


  flashProfiles_t* flashBufferProfiles = _malloc(sizeof(flashProfiles_t));
  flashSettings_t* flashBufferSettings = _malloc(sizeof(flashSettings_t));
#ifdef ENABLE_ADDONS
  flashAddons_t*   flashBufferAddons   = _malloc(sizeof(flashAddons_t));
#endif

  // check if malloc succeeded or not
  if(    (flashBufferProfiles == NULL)
      || (flashBufferSettings == NULL)
#ifdef ENABLE_ADDONS
      || (flashBufferAddons   == NULL)
#endif
  )
  { Error_Handler(); }

  while(ADC_Status != ADC_Idle);
  __disable_irq();
  getSettings()->isSaving = 1;
  configurePWMpin(output_Low);
  __set_PRIMASK(_irq);

#ifdef ENABLE_ADDONS                                                                                      //                                                            ADDONS
  if((save_mode&(save_Addons | reset_Addons)) == 0){                                                           // No addon save_mode specified
    if(ChecksumAddons(&flashAddons.addons) != flashAddons.addonsChecksum){    // Check existing flash data is valid
      resetAddonSettings(&flashBufferAddons->addons);                                              // Load defaults if wrong
      needs_saving=1;
    }
    else
      *flashBufferAddons = flashAddons;                                                            // Keep existing addons
  }
  if(save_mode & reset_Addons)                                                                            // Reset Addons
    resetAddonSettings(&flashBufferAddons->addons);
  else if(save_mode & save_Addons)                                                                        // Save Addons
    flashBufferAddons->addons = *getAddons();
  flashBufferAddons->addonsChecksum = ChecksumAddons(&flashBufferAddons->addons);          // Update addon checksum

#endif
                                                                                                          //                                                            SETTINGS
  if((save_mode&(save_Settings | reset_Settings)) == 0){                                                  // No settings save_mode specified
    if( (ChecksumSystemSettings(&flashSettings.system) != flashSettings.systemChecksum) ||// Check existing flash data is valid
        (flashSettings.system.version) != SYSTEM_SETTINGS_VERSION ){
      resetSystemSettings(&flashBufferSettings->system);
      needs_saving=1;
    }                                                // Load defaults if wrong
    else                                                                                                  // Keep existing settings
      flashBufferSettings->system = flashSettings.system;                                       // Keep existing data if ok
  }
  if(save_mode & reset_Settings){                                                                         // Reset settings
    resetSystemSettings(&flashBufferSettings->system);                                                  // Load defaults
    if(save_mode == reset_All){
      flashBufferSettings->system.version = 0xFF;                                                       // To trigger setup screen
    }
  }
  else if(save_mode & save_Settings){                                                                     // Save current settings
    flashBufferSettings->system = *getSystemSettings();
  }
  flashBufferSettings->systemChecksum = ChecksumSystemSettings(&flashBufferSettings->system);         // Update checksum

  for(uint8_t i=0;i<NUM_PROFILES;i++){                                                                    //                                                            PROFILES
    if( (ChecksumProfile(&flashProfiles.profile[i]) != flashProfiles.profileChecksum[i]) || // Check existing flash data is valid
        (flashProfiles.profile[i].settings.version != PROFILE_SETTINGS_VERSION) ||
        ((save_mode & reset_Profile) && i == profile) ||
         (save_mode & reset_Profiles) ){

      resetProfile(&flashBufferProfiles->profile[i], i);                                                  //Reset if wrong or reset flag set
      needs_saving=1;
    }
    else
      flashBufferProfiles->profile[i] = flashProfiles.profile[i];                                 // OK, backup flash profile
  }

  if(save_mode & (save_Tip | save_Profile)){                                                              // Save current profile or tip
    if(save_mode & save_Tip){
      if(tip_mode == mode_SaveTip)
        flashBufferProfiles->profile[profile].tip[tip_index] =  *getCurrentTipData();

      else if(tip_mode == mode_AddTip){
        flashBufferProfiles->profile[profile].tip[getProfileSettings()->currentNumberOfTips++] =  *getCurrentTipData();       // Add new tip, increase count
      }

      else if(tip_mode == mode_DeleteTip){                                                                // Delete tip
        for(uint8_t i=tip_index; i<getProfileSettings()->currentNumberOfTips-1;i++)                      // Overwrite selected tip and move the rest one position backwards
          flashBufferProfiles->profile[profile].tip[i] = flashBufferProfiles->profile[profile].tip[i+1];

        getProfileSettings()->currentNumberOfTips--;                                                     // Decrease the number of tips in the system
        uint8_t t = getCurrentTip();
        if(t && tip_index<=t)                                                                           // If the deleted tip is lower or equal than current used tip
          setCurrentTip(--t);                                                                             // Move one position back if possible

        for(uint8_t x = getProfileSettings()->currentNumberOfTips; x < NUM_TIPS;x++) {                   // Fill the unused tips with blank names
          strcpy(flashBufferProfiles->profile[profile].tip[x].name, _BLANK_TIP);
        }
      }
      else
        Error_Handler();                                                                                  // Save mode = Save_Tip, but tip_modenot defined, something went wrong

      sortTips(&flashBufferProfiles->profile[profile], profile);                                          // Sort tips
    }
    flashBufferProfiles->profile[profile].settings = *getProfileSettings();                              // Copy profile settings from system
  }
  for(uint8_t i=0;i<NUM_PROFILES;i++){                                                                    // Generate profile checksums
    flashBufferProfiles->profileChecksum[i] = ChecksumProfile(&flashBufferProfiles->profile[i]);
    flashBufferProfiles->profileSettingsChecksum[i] = ChecksumProfileSettings(&flashBufferProfiles->profile[i].settings);
  }

  if(!needs_saving){                                                                                      // Nothing to save, return
    getSettings()->isSaving = 0;
#ifdef ENABLE_ADDONS
  _free(flashBufferAddons);
#endif
  _free(flashBufferSettings);
  _free(flashBufferProfiles);
    return;
  }

  eraseFlashPages(SETTINGS_SECTION_START, flashPages_GlobalSettings);
  writeFlash((uint32_t*)flashBufferProfiles, sizeof(flashProfiles_t), (uint32_t)&flashProfiles);
  writeFlash((uint32_t*)flashBufferSettings, sizeof(flashSettings_t), (uint32_t)&flashSettings);
#ifdef ENABLE_ADDONS
  writeFlash((uint32_t*)flashBufferAddons, sizeof(flashAddons_t), (uint32_t)&flashAddons);
#endif
  uint32_t flashChecksum, ramChecksum;

  // Check flash and profile have same checksum
  for(uint8_t x=0;x<NUM_PROFILES;x++){
    flashChecksum = ChecksumProfile(&flashProfiles.profile[x]);
    ramChecksum   = ChecksumProfile(&flashBufferProfiles->profile[x]);
    if(flashChecksum != ramChecksum)
       Error_Handler();
  }

  // Check flash and settings buffer have same checksum
  flashChecksum   = ChecksumSystemSettings(&flashSettings.system);
  ramChecksum     = ChecksumSystemSettings(&flashBufferSettings->system);
  if(flashChecksum != ramChecksum)
     Error_Handler();

#ifdef ENABLE_ADDONS
  // Verify addon crc
  flashChecksum   = ChecksumAddons(&flashAddons.addons);
  ramChecksum     = ChecksumAddons(&flashBufferAddons->addons);
  if(flashChecksum != ramChecksum)
     Error_Handler();
#endif

#ifdef ENABLE_ADDONS
  _free(flashBufferAddons);
#endif
  _free(flashBufferSettings);
  _free(flashBufferProfiles);
  if(tip_mode){                                                                                   // If tip was deleted / updated / added
    tip_mode = 0;
    loadTipDataFromFlash(getCurrentTip());                                                                     // Reload tip, sortTips will have updated the number to keep the same tip, or the new one
  }
  getSettings()->isSaving = 0;

  if(reboot_mode  == do_reboot)
    NVIC_SystemReset();
}

void restoreSettings(void) {
  bool setup = (flashSettings.system.version == 0xFF);     // 0xFF = erased flash, trigger setup mode
  uint8_t reset = 0;
#ifndef STM32F072xB
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;    // power the BKP peripheral
  PWR->CR      |= PWR_CR_DBP;                               // enable access to the BKP registers
  BKP->CR      = 0u;                                        // disable tamper pin, just to be sure
#else
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;                        // power the BKP peripheral
  RCC->BDCR    |= RCC_BDCR_RTCEN;
  PWR->CR      |= PWR_CR_DBP;                               // enable access to the BKP registers
  RTC->TAFCR   = 0u;                                        // disable tamper pin, just to be sure
#endif

#if (__CORTEX_M == 3)
  uint32_t flashSize = 0xFFFF & *(uint32_t*)FLASHSIZE_BASE;   // Some clones report no flash size (0xFFFF), assume these use 1KB sectors.


  if( (flashSize>128) && (flashSize<=1024) )      // Between 256K and 1M (High Density / XL devices), 2KB sectors
    flashPageSize = 2048;

  else                                            //  Everything else, assume 1KB sectors.
    flashPageSize = 1024;

  flashPages_GlobalSettings = ((SETTINGS_SECTION_LENGTH + flashPageSize - 1) / flashPageSize);
  flashPages_TempSettings   = ((TEMP_SETTINGS_SECTION_LENGTH + flashPageSize - 1) / flashPageSize);
#endif

  if(setup){                                                                                              // Setup mode, so flash data is not initialized
    getSettings()->setupMode = enable;                                                                    // Load setup state for boot screen
    setSafeMode(enable);                                                                                  // Safe mode just in case
    resetSystemSettings(getSystemSettings());                                                        // Load default system settings
    resetAddonSettings(getAddons());                                                    // Load default addon settings
    resetProfileSettings(getProfileSettings(), profile_T12);                                           // Load default profile settings
    flashTempSettingsInit();                                                                              // Always init this to set default/safe values at boot. Initializes flashTemp index
    loadSettingsFromBackupRam();                                                                          // Also read backup ram. loadProfile wil take the correct values depending on hasBattery variable.
    setCurrentProfile(profile_T12);                                                                       // Force profile T12, we can't call loadProfile, there's nothing in flash, let boot screen finish it
  }
  else{
    Button_reset();
    if(flashSettings.system.version != SYSTEM_SETTINGS_VERSION)                                   // System settings version mismatch
      reset |= reset_Settings;                                                                            // Reset silently
    else if(ChecksumSystemSettings(&flashSettings.system) != flashSettings.systemChecksum){   // Show error message if bad checksum
      checksumError(reset_Settings);
      reset |= reset_Settings;
    }
    else
      *getSystemSettings() = flashSettings.system;                                             // Load settings

#ifdef ENABLE_ADDONS
    reset |= loadAddonSettings();
#endif
    if(checkFlashProfiles()==ERROR){                                                                      // Some profile is wrong
      for(uint8_t i=0;i<NUM_PROFILES;i++){
        if(flashProfiles.profile[i].settings.version==PROFILE_SETTINGS_VERSION){                  // If profile version is correct,
          checksumError(reset_Profile);                                                                   // Show checksum error, otherwise clear silently
          break;
        }
      }
      reset |= reset_Profile;
    }
    else{
      flashTempSettingsInit();                              // These initialize getSettings()->currentProfile, depending on the variable hasBattery
      loadSettingsFromBackupRam();                          //
      loadProfile(getCurrentProfile());           // loadProfile will restore lastTemp and lastTip from the correct source (Flash or backup SRAM).
    }
  }
  if(!setup && reset){                                              // If not in setup mode and we have bad data
    saveSettings(perform_scanFix, no_mode, no_mode, do_reboot);                       // Store settings with no arguments, this will check and reset any bad setting
  }
}                                                                   // Otherwise, everything be resetted when exiting the boot screen

void loadSettingsFromBackupRam(void) {
  if(getSystemSettings()->hasBattery == 0)
    return;

  // assert on backup ram size
  if(sizeof(bkpRamData.values) + sizeof(uint32_t) > BACKUP_RAM_SIZE_IN_BYTES)
  {
    Error_Handler(); // can't put this much data into the backup ram
  }

  readBackupRam();
  // check crc
  if(bkpRamData.crc != ChecksumBackupRam() || bkpRamData.values.lastProfile > profile_C210  )
  {
    // restore defaults, show error
    memset((void*)&bkpRamData,0, sizeof(backupRamData_t));
    for(uint8_t i = 0; i < NUM_PROFILES; i++)
    {
      bkpRamData.values.lastTipTemp[i] = 0;               // To be resetted by loadProfile on loading
      if(bkpRamData.values.lastTipTemp[i] == UINT16_MAX) // just a sanity check to handle uninitialized data
      {
        bkpRamData.values.lastTipTemp[i] = 0u;
      }
      bkpRamData.values.lastSelTip[i] = 0u;
    }
    bkpRamData.values.lastProfile = profile_T12;
    writeBackupRam();

    Oled_error_init();
    putStrAligned("New/low batt?", 0, align_center);
    putStrAligned("Forgot last", 16, align_center);
    putStrAligned("used settings.", 32, align_center);
    putStrAligned("Restored dflt.", 48, align_left);
    update_display();
    ErrCountDown(3,117,50);
  }
  setCurrentProfile(bkpRamData.values.lastProfile);
}


static void readBackupRam()
{
#ifndef STM32F072xB
  for(uint8_t i = 0; i < NUM_BACKUP_RAM_REGISTERS; i++)                                   // STM32F10x has 16.bit backup registers
  {
    uint16_t data = (uint16_t)*(&(BKP->DR1) + i);
    bkpRamData.bytes[i*2  ] = data;
    bkpRamData.bytes[i*2+1] = data >> 8u;
  }
#else
  memcpy(bkpRamData.bytes, (uint8_t*)&RTC->BKP0R, BACKUP_RAM_SIZE_IN_BYTES);              // STM32F072 has 32bit backup registers
#endif
}

static void writeBackupRam()
{
  bkpRamData.crc = ChecksumBackupRam();
#ifndef STM32F072xB
  for(uint8_t i = 0; i < NUM_BACKUP_RAM_REGISTERS; i++)
  {
    *(&(BKP->DR1) + i) = bkpRamData.bytes[i*2] + ((uint32_t)bkpRamData.bytes[i*2+1] << 8u);
  }
#else
  memcpy((uint8_t*)&RTC->BKP0R, bkpRamData.bytes, BACKUP_RAM_SIZE_IN_BYTES);
#endif
}

static uint32_t ChecksumBackupRam(){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)bkpRamData.bytes, sizeof(bkpRamData.values)/sizeof(uint32_t) );
  return checksum;
}

void flashTempSettingsInitialSetup(void){                                     // To be called after initial setup screen
  flashTempSettingsErase();
  for(uint8_t i=0;i<NUM_PROFILES;i++)
    flashTemp.tip[i]= 0;

  flashTemp.temp = defaultProfileSettings.defaultTemperature;
  flashTemp.profile=profile_T12;
  flashTempWrite();
  flashTipWrite();
  flashProfileWrite();

  setCurrentProfile(profile_T12);                                // Just in case
  setCurrentTip(0);
  setUserTemperature( defaultProfileSettings.defaultTemperature);
}

void flashTempSettingsInit(void) //call it only once during init
{
  bool setDefault=0;
  uint16_t i;

  for(i=0; i<(sizeof(temp_settings.temperature)/2)-1 && temp_settings.temperature[i+1] != UINT16_MAX; i++);   // Seek through the array

  if(i<(sizeof(temp_settings.temperature)/2)-1){                                                              // Free slot found
    for(uint16_t j=i+1; j<(sizeof(temp_settings.temperature)/2)-1; j++) {                                     // Ensure rest of array is erased
      if(temp_settings.temperature[j] != UINT16_MAX){                                                         // Found unexpected data
        setDefault=1;                                                                                         // Reset
        break;
      }
    }
    if(!setDefault){
      flashTemp.tempIndex = i;
      flashTemp.temp = temp_settings.temperature[i];
    }
  }                                                                                                           // No free slot found, reset
  else
    setDefault=1;

  if(!setDefault){
    for(i=0; i<(sizeof(temp_settings.profile)/2)-1 && temp_settings.profile[i+1] != UINT16_MAX; i++);         // Same operation with profile
    if(i<(sizeof(temp_settings.profile)/2)-1){
      for(uint16_t j=i+1; j<(sizeof(temp_settings.profile)/2)-1; j++) {
        if(temp_settings.profile[j] != UINT16_MAX){
          setDefault=1;
          break;
        }
      }
      if(!setDefault){
        flashTemp.profile=temp_settings.profile[i];
        flashTemp.profileIndex = i;
      }
    }
    else
      setDefault=1;
  }

  if(!setDefault){
    for(uint8_t n=0; n<NUM_PROFILES; n++){
      for(i=0; i<(sizeof(temp_settings.tip[0])/2)-1 && temp_settings.tip[n][i+1] != UINT16_MAX; i++);         // Same operation with tips
      if(i<(sizeof(temp_settings.tip[0])/2)-1){
        for(uint16_t j=i+1; j<(sizeof(temp_settings.tip[0])/2)-1; j++) {
          if(temp_settings.tip[n][j] != UINT16_MAX){
            setDefault=1;
            break;
          }
        }
        if(!setDefault){
          flashTemp.tipIndex[n] = i;
          flashTemp.tip[n] = temp_settings.tip[n][i];
        }
      }
      else
        setDefault=1;
    }
  }

  if(setDefault)
    flashTempSettingsInitialSetup();

  else{
    setCurrentProfile(flashTemp.profile);
    if(temp_settings.temperature[flashTemp.tempIndex] != UINT16_MAX)
      flashTemp.tempIndex++;                                             // Increase index if current slot is not empty (Only required when flash is completely erased)

    if(temp_settings.profile[flashTemp.profileIndex] != UINT16_MAX)
      flashTemp.profileIndex++;

    for(uint8_t n=0; n<NUM_PROFILES; n++)
      if(temp_settings.tip[n][flashTemp.tipIndex[n]] != UINT16_MAX)
        flashTemp.tipIndex[n]++;
  }
}

void flashTempWrite(void)
{
  if(flashTemp.tempIndex>=(sizeof(temp_settings.temperature)/2)-1)                // All positions used
  {
    flashTempSettingsErase();
    flashTipWrite();
    flashProfileWrite();
  }
  HAL_IWDG_Refresh(&hiwdg);
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  HAL_FLASH_Unlock();
  __set_PRIMASK(_irq);
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.temperature[flashTemp.tempIndex], flashTemp.temp) != HAL_OK) {    // Store new temp
     Error_Handler();
  }
  HAL_FLASH_Lock();
  flashTemp.tempIndex++;                                                                                                         // Increase index for next time
}

void flashProfileWrite(void)
{
  if(flashTemp.profileIndex>=(sizeof(temp_settings.profile)/2)-1)                // All positions used
  {
    flashTempSettingsErase();
    flashTipWrite();
    flashTempWrite();
  }

  HAL_IWDG_Refresh(&hiwdg);
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  HAL_FLASH_Unlock();
  __set_PRIMASK(_irq);
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.profile[flashTemp.profileIndex], flashTemp.profile) != HAL_OK) {        // Store new tip/profile data
     Error_Handler();
  }
  HAL_FLASH_Lock();
  flashTemp.profileIndex++;                                                                                                         // Increase index for next time
}

void flashTipWrite(void)
{
  uint8_t p = getCurrentProfile();
  if(flashTemp.tipIndex[p]>=(sizeof(temp_settings.tip[0])/2)-1)                // All positions used
  {
    flashTempSettingsErase();
    flashTempWrite();
    flashProfileWrite();
  }
  HAL_IWDG_Refresh(&hiwdg);
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  HAL_FLASH_Unlock();
  __set_PRIMASK(_irq);
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.tip[p][flashTemp.tipIndex[p]], flashTemp.tip[p]) != HAL_OK) {        // Store new tip/profile data
     Error_Handler();
  }
  HAL_FLASH_Lock();
  flashTemp.tipIndex[p]++;                                                                                                         // Increase index for next time
}

void flashTempSettingsErase(void){
  eraseFlashPages(TEMP_SETTINGS_SECTION_START, flashPages_TempSettings);
  flashTemp.tempIndex = 0;                                             // Reset index
  flashTemp.profileIndex = 0;
  for(uint8_t i=0; i<NUM_PROFILES;i++)
    flashTemp.tipIndex[i] = 0;

  flashTemp.temp=0xFFFF;
  flashTemp.profile=0xFF;
}

static uint32_t ChecksumSystemSettings(systemSettings_t* system){                                                       // Check system settings block
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)system, sizeof(systemSettings_t)/sizeof(uint32_t) );
  return checksum;
}

static uint32_t ChecksumProfile(profile_t* profile){                                                                // Check whole profile block (Including tips)
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile, sizeof(profile_t)/sizeof(uint32_t));
  return checksum;
}

static uint32_t ChecksumProfileSettings(profile_settings_t* profile_settings){                                               // Check only profile settings (Not tips)
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile_settings, sizeof(profile_settings_t)/sizeof(uint32_t));
  return checksum;
}

static ErrorStatus checkFlashProfiles(void){
  ErrorStatus status=SUCCESS;
  for(uint8_t i=0; i<NUM_PROFILES; i++){           // Reset buffered values to trigger flash udpate
    if(ChecksumProfile(&flashProfiles.profile[i]) != flashProfiles.profileChecksum[i]){
      status=ERROR;
      break;
    }
  }
  return status;
}

#ifdef ENABLE_ADDONS
static uint8_t loadAddonSettings(void) {
  if((flashAddons.addons.enabledAddons == 0xffffffffffffffffU) && (flashAddons.addonsChecksum == 0xffffffffU)){     // Assume flash is erased, restore defaults silently
    return reset_Addons;
  }
  if(ChecksumAddons(&flashAddons.addons) != flashAddons.addonsChecksum){       // crc mismatch
    checksumError(reset_Addons);
    return reset_Addons;
  }
  else
    *getAddons() = flashAddons.addons;

  return SUCCESS;
}

static void resetAddonSettings(addonSettings_t *addons){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  *addons = defaultAddons;
  __set_PRIMASK(_irq);
}

static uint32_t ChecksumAddons(addonSettings_t* addonSettings){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)addonSettings, sizeof(addonSettings_t)/sizeof(uint32_t));
  return checksum;
}
addonSettings_t * getAddons(void){
  return &getSettings()->addons;
}
#endif

static void resetSystemSettings(systemSettings_t * system) {
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  *system = defaultSystemSettings;
  flashTempSettingsInitialSetup();
  __set_PRIMASK(_irq);
}

static void resetProfile(profile_t * p, uint8_t profile){
  uint32_t _irq = __get_PRIMASK();

  if(profile>profile_C210){
    Error_Handler();
  }
  __disable_irq();
  resetProfileSettings(&p->settings, profile);
  for(uint8_t x = 0; x < NUM_TIPS; x++){
    p->tip[x] = defaultTipData[profile];
  }
  if(profile==profile_T12){
    strcpy(p->tip[0].name, "T12-BC3");               // Put some generic name.
  }
  else if(profile==profile_C245){
    strcpy(p->tip[0].name, "C245-963");
  }
  else if(profile==profile_C210){
    strcpy(p->tip[0].name, "C210-018");
  }
  __set_PRIMASK(_irq);
}


static void resetProfileSettings(profile_settings_t * p, uint8_t profile){
  uint32_t _irq = __get_PRIMASK();
  *p = defaultProfileSettings;

  __disable_irq();
    if(profile==profile_T12){
      p->ID = profile_T12;
      p->currentNumberOfTips      = 1;
      p->impedance                = 80;             // 8.0 Ohms
      p->power                    = 80;             // 80W
      p->noIronValue              = 4000;
      p->Cal250_default           = T12_Cal250;
      p->Cal400_default           = T12_Cal400;
  }

  else if(profile==profile_C245){
      p->ID = profile_C245;
      p->currentNumberOfTips      = 1;
      p->impedance                = 26;
      p->power                    = 150;
      p->noIronValue              = 4000;
      p->Cal250_default           = C245_Cal250;
      p->Cal400_default           = C245_Cal400;
  }

  else if(profile==profile_C210){
      p->ID = profile_C210;
      p->currentNumberOfTips    = 1;
      p->power                  = 80;
      p->impedance              = 21;
      p->noIronValue            = 1200;
      p->Cal250_default         = C210_Cal250;
      p->Cal400_default         = C210_Cal400;
  }
  else{
    Error_Handler();  // We shouldn't get here!
  }

  __set_PRIMASK(_irq);
}

flashProfiles_t * getFlashProfiles(void){
  return &flashProfiles;
}

profile_settings_t * getFlashProfileSettings(void){
  return &getFlashProfiles()->profile[getCurrentProfile()].settings;
}

profile_settings_t * getProfileSettings(void){
  return &getSettings()->profile;
}
uint8_t getCurrentProfile(void){
  return getSettings()->currentProfile;
}

void setCurrentProfile(uint8_t profile){
  getSettings()->currentProfile = profile;
}

uint8_t loadProfile(uint8_t profile){
  while(ADC_Status!=ADC_Idle);

  if(profile==0xFF)                                   // Erased flash, load T12
    profile=profile_T12;
  else if(profile>profile_C210)                       // Sanity check
    Error_Handler();

  uint32_t _irq = __get_PRIMASK();

  HAL_IWDG_Refresh(&hiwdg);
  if(profile<=profile_C210){                                                                          // If valid profile
    if(flashProfiles.profile[profile].settings.version != PROFILE_SETTINGS_VERSION){          // If flash profile not initialized or version mismatch
      return reset_Profile;                                                                           // Silent reset
    }
    else{
      if(ChecksumProfile(&flashProfiles.profile[profile]) != flashProfiles.profileChecksum[profile]){
        checksumError(reset_Profile);
        return reset_Profile;
      }
      else{
        __disable_irq();
        setCurrentProfile(profile);
        *getProfileSettings() = *getFlashProfileSettings();
      }
    }
  }
  else{
    Error_Handler();
  }
  setSystemTempUnit(getSystemTempUnit());                        // Ensure the profile uses the same temperature unit as the system

  if(getSystemSettings()->hasBattery){
    if((bkpRamData.values.lastTipTemp[profile] < getProfileSettings()->MinSetTemperature) || (bkpRamData.values.lastTipTemp[profile] > getProfileSettings()->MaxSetTemperature)){
      bkpRamData.values.lastTipTemp[profile] = getProfileSettings()->defaultTemperature;
    }
    setUserTemperature(bkpRamData.values.lastTipTemp[profile]);

    if(bkpRamData.values.lastSelTip[profile] >= getProfileSettings()->currentNumberOfTips)
      loadTipDataFromFlash(0);
    else
      loadTipDataFromFlash(bkpRamData.values.lastSelTip[profile]);
    writeBackupRam();
  }
  else{
    if((flashTemp.profile==0xFF) || (flashTemp.temp==0xFFFF)){                              // flashTemp not initialized, load defaults now the profile was loaded
      setUserTemperature(getProfileSettings()->defaultTemperature);
      loadTipDataFromFlash(flashTemp.tip[profile]);
      updateTempData(force_update);                                               // Force save now
    }
    else{
      if((flashTemp.temp < getProfileSettings()->MinSetTemperature) || (flashTemp.temp > getProfileSettings()->MaxSetTemperature))
        setUserTemperature(getProfileSettings()->defaultTemperature);            // Stored value out of limits, set default
      else
        setUserTemperature(flashTemp.temp);                                            // Load stored value

      if(flashTemp.tip[profile] >= getProfileSettings()->currentNumberOfTips)         // Bad tip index, load first tip
        loadTipDataFromFlash(0);
      else
        loadTipDataFromFlash(flashTemp.tip[profile]);
    }
  }
  setCurrentProfile(profile);
  TIP.filter=getProfileSettings()->tipFilter;
  ironSchedulePwmUpdate();
  __set_PRIMASK(_irq);
  return SUCCESS;
}

static void sortTips(profile_t * data, uint8_t profile){
  uint8_t min;
  char current[TIP_LEN+1];
  tipData_t backupTip;
  strcpy (current, getCurrentTipData()->name);                                                    // Copy tip name being used in the system
  for(uint8_t j=0; j<getProfileSettings()->currentNumberOfTips; j++){                            // Sort alphabetically
    min = j;                                                                                      // Min is start position
    for(uint8_t i=j+1; i<getProfileSettings()->currentNumberOfTips; i++){
      if(strcmp(data->tip[min].name, data->tip[i].name) > 0)                                      // If Tip[Min] > Tip[i]
        min = i;                                                                                  // Update min
    }
    if(min!=j){                                                                                   // If j is not the min
      backupTip = data->tip[j];                                                                   // Swap j and min
      data->tip[j] = data->tip[min];
      data->tip[min] = backupTip;
    }
  }
  setCurrentTip(0);                                                                               // Just in case it's not found
  for(uint8_t i=0; i<getProfileSettings()->currentNumberOfTips; i++){                            // Find the same tip after sorting
    if(strcmp(current, data->tip[i].name) == 0){                                                  // If matching name
      setCurrentTip(i);                                                                           // Update the tip to be reloaded later
      break;
    }
  }
}

tipData_t * getFlashTipData(uint8_t tip){
  return &flashProfiles.profile[getCurrentProfile()].tip[tip];
}

void loadTipDataFromFlash(uint8_t tip) {
  if(tip >= getProfileSettings()->currentNumberOfTips) // sanity check
    tip = 0u;
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  setCurrentTip(tip);                                                                               // Set current tip number
  setCurrentTipData(getFlashTipData(tip));                                             // Copy data from flash
  setupPID(&getCurrentTipData()->PID);
  __set_PRIMASK(_irq);
}

void setCurrentTip(uint8_t tip){
  getSettings()->currentTip = tip;
}

uint8_t getCurrentTip(void){
  return getSettings()->currentTip;
}

void setCurrentTipData(tipData_t * tip) {
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  getSettings()->currentTipData = *tip;
  __set_PRIMASK(_irq);
}

tipData_t * getCurrentTipData(void){
  return &getSettings()->currentTipData;
}

systemSettings_t* getSystemSettings(void){
  return(&settings.system);
}

flashSettings_t * getFlashSettings(void){
  return &flashSettings;
}
systemSettings_t * getFlashSystemSettings(void){
  return &getFlashSettings()->system;
}
systemSettings_t * getDefaultSystemSettings(void){
  return (systemSettings_t*)&defaultSystemSettings;
}
settings_t * getSettings(void){
  return(&settings);
}

static void checksumError(uint8_t mode){
  Oled_error_init();
  putStrAligned("BAD CHECKSUM!", 0, align_center);
  putStrAligned("RESTORING THE", 20, align_center);
  if(mode==reset_Profile)
    putStrAligned("PROFILE", 36, align_center);
#ifdef ENABLE_ADDONS
  else if(mode == reset_Addons)
    putStrAligned("ADDONS", 36, align_center);
#endif
  else
    putStrAligned("SYSTEM", 36, align_center);
  update_display();
  ErrCountDown(3,117,50);
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
        while(!BUTTON_input())
          HAL_IWDG_Refresh(&hiwdg);
        saveSettings(reset_All, no_mode, no_mode, do_reboot);
      }
    }
  }
}

bool isCurrentProfileChanged(void) {
  return ChecksumProfileSettings(getProfileSettings()) != flashProfiles.profileSettingsChecksum[getCurrentProfile()];
}
bool isSystemSettingsChanged(void) {
  return ChecksumSystemSettings(getSystemSettings()) != flashSettings.systemChecksum;
}
bool isAddonSettingsChanged(void) {
  return ChecksumAddons(getAddons()) != flashAddons.addonsChecksum;
}

void copy_bkp_data(uint8_t mode){
  if(mode==flash_to_ram){
    bkpRamData.values.lastProfile = flashTemp.profile;
    for(uint8_t i=0;i<NUM_PROFILES;i++){
      bkpRamData.values.lastSelTip[i] = flashTemp.tip[i];
      bkpRamData.values.lastTipTemp[i] = flashTemp.temp;
    }
    writeBackupRam();
  }
  else if(mode==ram_to_flash){
    flashTemp.profile = bkpRamData.values.lastProfile;
    flashTemp.temp = bkpRamData.values.lastTipTemp[0];                            // Flash temp only stores one temperature. Choose first profile.
    for(uint8_t i=0;i<NUM_PROFILES;i++){
      flashTemp.tip[i] = bkpRamData.values.lastSelTip[i];
    }
    flashTempWrite();
    flashProfileWrite();
    flashTipWrite();
  }
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
