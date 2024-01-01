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

const settings_t defaultSystemSettings = {
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

__attribute__((section(".globalSettings"))) flashSettingsSettings_t flashGlobalSettings;
__attribute__((section(".profileSettings"))) flashSettingsProfiles_t flashProfilesSettings;
__attribute__((section(".tempSettings"))) temp_settings_t temp_settings;
#ifdef ENABLE_ADDONS
__attribute__((section(".addonSettings"))) flashSettingsAddons_t flashAddonSettings;
#endif

static uint16_t flashTempIndex;
static uint16_t flashTemp;
static uint8_t flashTipIndex[NUM_PROFILES];
static uint8_t flashTip[NUM_PROFILES];
static uint8_t flashProfileIndex;
static uint8_t flashProfile;
static uint8_t prevTip[NUM_PROFILES];
static uint8_t prevProfile;
static uint16_t prevTemp;
static uint32_t lastTempCheckTime[3];


systemSettings_t systemSettings;

static void storeSettings(uint8_t mode);
static void checksumError(uint8_t mode);
static void Flash_error(void);
static void Button_reset(void);
static void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);
static uint32_t ChecksumSystemSettings(settings_t* settings);
static uint32_t ChecksumProfileSettings(profile_settings_t* profile);
static uint32_t ChecksumProfile(profile_t* profile);
static ErrorStatus checkFlashProfiles(void);
static void resetSystemSettings(settings_t * data) ;
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

  if(getIronCalibrationMode() || systemSettings.setupMode || scr_index == screen_debug)                                 // Don't update while calibration is in progress or in the debug screen
    return;
                                                                                                                        // Always update the data, whether the batery option is enabled or not
  if(systemSettings.settings.hasBattery == true){
    bkpRamData.values.lastSelTip[systemSettings.currentProfile] = systemSettings.currentTip;
    bkpRamData.values.lastProfile = systemSettings.currentProfile;
    bkpRamData.values.lastTipTemp[systemSettings.currentProfile] = getUserTemperature();
    writeBackupRam();
  }
  else if ((systemSettings.settings.hasBattery == false) || force){                                                      // No battery or forced
    uint16_t currentTemp = getUserTemperature();
    uint8_t currentTip = systemSettings.currentTip;
    uint8_t currentProfile = systemSettings.currentProfile;

    if(force){
      prevTemp = currentTemp;
      prevTip[currentProfile] = currentTip;
      prevProfile = currentProfile;
    }

    if(flashTemp != currentTemp) {                                       // Compare with stored in flash
      if(prevTemp != currentTemp) {                                      // Store if different to last check
        prevTemp = currentTemp;
        lastTempCheckTime[0] = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-lastTempCheckTime[0])>4999)) {                           // Different than flash and timeout is over
        flashTemp = currentTemp;
        flashTempWrite();                                                           // Update temperature in flash
      }
    }
    if(flashTip[currentProfile] != currentTip) {
      if(prevTip[currentProfile] != currentTip) {                        // Store if different to last check
        prevTip[currentProfile] = currentTip;
        lastTempCheckTime[1] = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-lastTempCheckTime[1])>4999)) {                           // Different than flash and timeout is over
        for(uint8_t i=0;i<NUM_PROFILES;i++){                                            // Update all tips
          flashTip[i] = prevTip[i];
        }
        flashTipWrite();                                                            // Update tip data in flash
      }
    }
    if(flashProfile != currentProfile) {
      if(prevProfile != currentProfile) {                                // Store if different to last check
        prevProfile = currentProfile;
        lastTempCheckTime[2] = CurrentTime;                                            // Start timeout
      }
      if(force || ((CurrentTime-lastTempCheckTime[2])>4999)) {                           // Different than flash and timeout is over
        flashProfile = currentProfile;
        flashProfileWrite();                                                        // Update tip data in flash
      }
    }
  }
}

//This is done to avoid huge stack build up. Trigger saving using checkSettings with a flag instead direct call from menu.
// Thus, the flags stays after the screen exits. The code handling settings saving decides when it's ok to store them.
void saveSettings(uint8_t save_mode, uint8_t reboot_mode){
  if(reboot_mode  == do_reboot)                                        // Force safe mode (disable iron power) if rebooting.
    setSafeMode(enable);

  storeSettings(save_mode);

  if(reboot_mode  == do_reboot)
    NVIC_SystemReset();
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
    Flash_error();
  }
  HAL_FLASH_Lock();

  // Ensure flash was erased
  for (uint32_t i = 0u; i < (numPages * FLASH_PAGE_SIZE / sizeof(int32_t)); i++) {
    if( *((uint32_t*)pageAddress+i) != 0xFFFFFFFF){
      Flash_error();
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
      Flash_error();
    }
    dstAddr += sizeof(uint32_t); // increase pointers
    srcData++;
  }
  HAL_FLASH_Lock();
}

static void storeSettings(uint8_t mode){
#ifndef DEBUG
  struct mallinfo mi = mallinfo();
#endif
  if(mi.uordblks > 128)                   // Check that heap usage is low
    Error_Handler();                      // We got there from a working screen (Heap must be free)
  uint8_t needs_saving = mode;
  uint32_t _irq = __get_PRIMASK();

  if( ((mode & save_Settings) && (mode & reset_Settings)) ||        // Sanity check
      ((mode & save_Profile) && (mode & reset_Profile))   ||
      ((mode & save_Profile) && (mode & reset_Profiles))  ||
      ((mode & save_Addons) && (mode & reset_Addons))     ){

    Error_Handler();
  }

  uint8_t profile = systemSettings.currentProfile;

  if((profile>profile_C210) || (systemSettings.Profile.ID != profile ))                                   // Sanity check
      Error_Handler();


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

  while(ADC_Status != ADC_Idle);
  __disable_irq();
  systemSettings.isSaving = 1;
  configurePWMpin(output_Low);
  __set_PRIMASK(_irq);

#ifdef ENABLE_ADDONS                                                                                      //                                                            ADDONS
  if((mode&(save_Addons | reset_Addons)) == 0){                                                           // No addon mode specified
    if(ChecksumAddons(&flashAddonSettings.addonSettings) != flashAddonSettings.addonSettingsChecksum){    // Check existing flash data is valid
      resetAddonSettings(&flashBufferAddons->addonSettings);                                              // Load defaults if wrong
      needs_saving=1;
    }
    else
      *flashBufferAddons = flashAddonSettings;                                                            // Keep existing addons
  }
  if(mode & reset_Addons)                                                                                 // Reset Addons
    resetAddonSettings(&flashBufferAddons->addonSettings);
  else if(mode & save_Addons)                                                                             // Save Addons
    flashBufferAddons->addonSettings = systemSettings.addonSettings;
  flashBufferAddons->addonSettingsChecksum = ChecksumAddons(&flashBufferAddons->addonSettings);;            // Update addon checksum

#endif
                                                                                                          //                                                            SETTINGS
  if((mode&(save_Settings | reset_Settings)) == 0){                                                       // No settings mode specified
    if( (ChecksumSystemSettings(&flashGlobalSettings.settings) != flashGlobalSettings.settingsChecksum) ||     // Check existing flash data is valid
        (flashGlobalSettings.settings.version) != SYSTEM_SETTINGS_VERSION ){
      resetSystemSettings(&flashBufferSettings->settings);
      needs_saving=1;
    }                                                // Load defaults if wrong
    else                                                                                                  // Keep existing settings
      flashBufferSettings->settings = flashGlobalSettings.settings;                                       // Keep existing data if ok
  }
  if(mode & reset_Settings){                                                                              // Reset settings
    resetSystemSettings(&flashBufferSettings->settings);                                                  // Load defaults
    if(mode == reset_All){
      flashBufferSettings->settings.version = 0xFF;                                                       // To trigger setup screen
    }
  }
  else if(mode & save_Settings){                                                                          // Save current settings
    flashBufferSettings->settings = systemSettings.settings;
  }
  flashBufferSettings->settingsChecksum = ChecksumSystemSettings(&flashBufferSettings->settings);         // Update checksum

  for(uint8_t i=0;i<NUM_PROFILES;i++){                                                                    //                                                            PROFILES
    if( (ChecksumProfile(&flashProfilesSettings.Profile[i]) != flashProfilesSettings.ProfileChecksum[i]) || // Check existing flash data is valid
        (flashProfilesSettings.Profile[i].settings.version != PROFILE_SETTINGS_VERSION) ||
        ((mode & reset_Profile) && i == profile) ||
         (mode & reset_Profiles) ){

      resetProfile(&flashBufferProfiles->Profile[i], i);                                                  //Reset if wrong or reset flag set
      needs_saving=1;
    }
    else
      flashBufferProfiles->Profile[i] = flashProfilesSettings.Profile[i];                                 // OK, backup flash profile
  }

  if(mode & (save_Tip | save_Profile)){                                                                   // Save current profile or tip
    if(mode & save_Tip){
      if(systemSettings.tipUpdateMode == mode_SaveTip)
        flashBufferProfiles->Profile[profile].tip[systemSettings.tipUpdateIndex] =  systemSettings.currentTipData;

      else if(systemSettings.tipUpdateMode == mode_AddTip){
        flashBufferProfiles->Profile[profile].tip[systemSettings.Profile.currentNumberOfTips++] =  systemSettings.currentTipData;       // Add new tip, increase count
        sortTips(&flashBufferProfiles->Profile[profile], profile);                                          // Sort tips
      }

      else if(systemSettings.tipUpdateMode == mode_DeleteTip){                                              // Delete tip
        for(uint8_t i=systemSettings.tipUpdateIndex; i<systemSettings.Profile.currentNumberOfTips-1;i++)    // Overwrite selected tip and move the rest one position backwards
          flashBufferProfiles->Profile[profile].tip[i] = flashBufferProfiles->Profile[profile].tip[i+1];

        systemSettings.Profile.currentNumberOfTips--;                                                       // Decrease the number of tips in the system
        if(systemSettings.tipUpdateIndex<=systemSettings.currentTip){                                       // If the deleted tip is lower or equal than current used tip
          if(systemSettings.currentTip)                                                                     // Move one position back if possible
            systemSettings.currentTip--;
        }

        for(uint8_t x = systemSettings.Profile.currentNumberOfTips; x < NUM_TIPS;x++) {                     // Fill the unused tips with blank names
          strcpy(flashBufferProfiles->Profile[profile].tip[x].name, _BLANK_TIP);
        }
        sortTips(&flashBufferProfiles->Profile[profile], profile);                                          // Sort tips
      }
    }
    flashBufferProfiles->Profile[profile].settings = systemSettings.Profile;                                // Copy profile settings from system
  }
  for(uint8_t i=0;i<NUM_PROFILES;i++){                                                                      // Generate profile checksums
    flashBufferProfiles->ProfileChecksum[i] = ChecksumProfile(&flashBufferProfiles->Profile[i]);
    flashBufferProfiles->ProfileSettingsChecksum[i] = ChecksumProfileSettings(&flashBufferProfiles->Profile[i].settings);
  }

  if(!needs_saving){                                                                                        // Nothing to save, return
    systemSettings.isSaving = 0;
#ifdef ENABLE_ADDONS
  _free(flashBufferAddons);
#endif
  _free(flashBufferSettings);
  _free(flashBufferProfiles);
    return;
  }

  eraseFlashPages(SETTINGS_SECTION_START, (SETTINGS_SECTION_LENGTH+FLASH_PAGE_SIZE-1) / FLASH_PAGE_SIZE);
  writeFlash((uint32_t*)flashBufferProfiles, sizeof(flashSettingsProfiles_t), (uint32_t)&flashProfilesSettings);
  writeFlash((uint32_t*)flashBufferSettings, sizeof(flashSettingsSettings_t), (uint32_t)&flashGlobalSettings);
#ifdef ENABLE_ADDONS
  writeFlash((uint32_t*)flashBufferAddons, sizeof(flashSettingsAddons_t), (uint32_t)&flashAddonSettings);
#endif
  uint32_t flashChecksum, ramChecksum;

  // Check flash and profile have same checksum
  for(uint8_t x=0;x<NUM_PROFILES;x++){
    flashChecksum = ChecksumProfile(&flashProfilesSettings.Profile[x]);
    ramChecksum   = ChecksumProfile(&flashBufferProfiles->Profile[x]);
    if(flashChecksum != ramChecksum)
      Flash_error();
  }

  // Check flash and settings buffer have same checksum
  flashChecksum   = ChecksumSystemSettings(&flashGlobalSettings.settings);
  ramChecksum     = ChecksumSystemSettings(&flashBufferSettings->settings);
  if(flashChecksum != ramChecksum)
    Flash_error();

#ifdef ENABLE_ADDONS
  // Verify addon crc
  flashChecksum   = ChecksumAddons(&flashAddonSettings.addonSettings);
  ramChecksum     = ChecksumAddons(&flashBufferAddons->addonSettings);
  if(flashChecksum != ramChecksum)
    Flash_error();
#endif

#ifdef ENABLE_ADDONS
  _free(flashBufferAddons);
#endif
  _free(flashBufferSettings);
  _free(flashBufferProfiles);
  if(systemSettings.tipUpdateMode){                   // If tip was deleted / updated / added
    systemSettings.tipUpdateMode = 0;
    setCurrentTip(systemSettings.currentTip);        // Reload tip, sortTips will have updated the number to keep the same tip, or the new one
  }
  systemSettings.isSaving = 0;
}

void restoreSettings(void) {
  bool setup = (flashGlobalSettings.settings.version == 0xFF);     // 0xFF = erased flash, trigger setup mode
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

  if(setup){                                                                                              // Setup mode, so flash data is not initialized
    systemSettings.setupMode = enable;                                                                    // Load setup state for boot screen
    setSafeMode(enable);                                                                                  // Safe mode just in case
    resetSystemSettings(&systemSettings.settings);                                                        // Load default system settings
    resetAddonSettings(&systemSettings.addonSettings);                                                    // Load default addon settings
    resetProfileSettings(&systemSettings.Profile, profile_T12);                                           // Load default profile settings
    flashTempSettingsInit();                                                                              // Always init this to set default/safe values at boot. Initializes flashTemp index
    loadSettingsFromBackupRam();                                                                          // Also read backup ram. loadProfile wil take the correct values depending on hasBattery variable.
    systemSettings.currentProfile = profile_T12;                                                          // Force profile T12, we can't call loadProfile, there's nothing in flash, let boot screen finish it
  }
  else{
    Button_reset();
    if(flashGlobalSettings.settings.version != SYSTEM_SETTINGS_VERSION)                                   // System settings version mismatch
      reset |= reset_Settings;                                                                            // Reset silently
    else if(ChecksumSystemSettings(&flashGlobalSettings.settings) != flashGlobalSettings.settingsChecksum){   // Show error message if bad checksum
      checksumError(reset_Settings);
      reset |= reset_Settings;
    }
    else
      systemSettings.settings = flashGlobalSettings.settings;                                             // Load settings

#ifdef ENABLE_ADDONS
    reset |= loadAddonSettings();
#endif
    if(checkFlashProfiles()==ERROR){                                                                      // Some profile is wrong
      for(uint8_t i=0;i<NUM_PROFILES;i++){
        if(flashProfilesSettings.Profile[i].settings.version==PROFILE_SETTINGS_VERSION){                  // If profile version is correct,
          checksumError(reset_Profile);                                                                   // Show checksum error, otherwise clear silently
          break;
        }
      }
      reset |= reset_Profile;
    }
    else{
      flashTempSettingsInit();                              // These initialize systemSettings.currentProfile, depending on the variable hasBattery
      loadSettingsFromBackupRam();                          //
      loadProfile(systemSettings.currentProfile);           // loadProfile will restore lastTemp and lastTip from the correct source (Flash or backup SRAM).
    }
  }
  if(!setup && reset){                                              // If not in setup mode and we have bad data
    saveSettings(perform_scanFix, do_reboot);                       // Store settings with no arguments, this will check and reset any bad setting
  }
}                                                                   // Otherwise, everything be resetted when exiting the boot screen

void loadSettingsFromBackupRam(void) {
  if(systemSettings.settings.hasBattery == 0)
    return;

  // assert on backup ram size
  if(sizeof(backupRamValues_t) + sizeof(uint32_t) > BACKUP_RAM_SIZE_IN_BYTES)
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
  systemSettings.currentProfile = bkpRamData.values.lastProfile;
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
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)bkpRamData.bytes, sizeof(backupRamValues_t)/sizeof(uint32_t) );
  return checksum;
}

void flashTempSettingsInitialSetup(void){                                     // To be called after initial setup screen
  flashTempSettingsErase();
  for(uint8_t i=0;i<NUM_PROFILES;i++)
    flashTip[i]= 0;

  flashTemp = defaultProfileSettings.defaultTemperature;
  flashProfile=profile_T12;
  flashTempWrite();
  flashTipWrite();
  flashProfileWrite();

  systemSettings.currentProfile = profile_T12;                                // Just in case
  systemSettings.currentTip = 0;
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
      flashTempIndex = i;
      flashTemp = temp_settings.temperature[i];
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
        flashProfile=temp_settings.profile[i];
        flashProfileIndex = i;
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
          flashTipIndex[n] = i;
          flashTip[n] = temp_settings.tip[n][i];
        }
      }
      else
        setDefault=1;
    }
  }

  if(setDefault)
    flashTempSettingsInitialSetup();

  else{
    systemSettings.currentProfile = flashProfile;
    if(temp_settings.temperature[flashTempIndex] != UINT16_MAX)
      flashTempIndex++;                                             // Increase index if current slot is not empty (Only required when flash is completely erased)

    if(temp_settings.profile[flashProfileIndex] != UINT16_MAX)
      flashProfileIndex++;

    for(uint8_t n=0; n<NUM_PROFILES; n++)
      if(temp_settings.tip[n][flashTipIndex[n]] != UINT16_MAX)
        flashTipIndex[n]++;
  }
}

void flashTempWrite(void)
{
  if(flashTempIndex>=(sizeof(temp_settings.temperature)/2)-1)                // All positions used
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
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.temperature[flashTempIndex], flashTemp) != HAL_OK) {    // Store new temp
    Flash_error();
  }
  HAL_FLASH_Lock();
  flashTempIndex++;                                                                                                         // Increase index for next time
}

void flashProfileWrite(void)
{
  if(flashProfileIndex>=(sizeof(temp_settings.profile)/2)-1)                // All positions used
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
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.profile[flashProfileIndex], flashProfile) != HAL_OK) {        // Store new tip/profile data
    Flash_error();
  }
  HAL_FLASH_Lock();
  flashProfileIndex++;                                                                                                         // Increase index for next time
}

void flashTipWrite(void)
{
  uint8_t p = systemSettings.currentProfile;
  if(flashTipIndex[p]>=(sizeof(temp_settings.tip[0])/2)-1)                // All positions used
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
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&temp_settings.tip[p][flashTipIndex[p]], flashTip[p]) != HAL_OK) {        // Store new tip/profile data
    Flash_error();
  }
  HAL_FLASH_Lock();
  flashTipIndex[p]++;                                                                                                         // Increase index for next time
}

void flashTempSettingsErase(void){
  eraseFlashPages(TEMP_SETTINGS_SECTION_START, (TEMP_SETTINGS_SECTION_LENGTH+FLASH_PAGE_SIZE-1) / FLASH_PAGE_SIZE);
  flashTempIndex = 0;                                             // Reset index
  flashProfileIndex = 0;
  for(uint8_t i=0; i<NUM_PROFILES;i++)
      flashTipIndex[i] = 0;

  flashTemp=0xFFFF;
  flashProfile=0xFF;
}

static uint32_t ChecksumSystemSettings(settings_t* settings){                                                       // Check system settings block
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)settings, sizeof(settings_t)/sizeof(uint32_t) );
  return checksum;
}

static uint32_t ChecksumProfile(profile_t* profile){                                                                // Check whole profile block (Including tips)
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile, sizeof(profile_t)/sizeof(uint32_t));
  return checksum;
}

static uint32_t ChecksumProfileSettings(profile_settings_t* profile){                                               // Check only profile settings (Not tips)
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)profile, sizeof(profile_settings_t)/sizeof(uint32_t));
  return checksum;
}

static ErrorStatus checkFlashProfiles(void){
  ErrorStatus status=SUCCESS;
  for(uint8_t i=0; i<NUM_PROFILES; i++){           // Reset buffered values to trigger flash udpate
    if(ChecksumProfile(&flashProfilesSettings.Profile[i]) != flashProfilesSettings.ProfileChecksum[i]){
      status=ERROR;
      break;
    }
  }
  return status;
}

#ifdef ENABLE_ADDONS
static uint8_t loadAddonSettings(void) {
  if((flashAddonSettings.addonSettings.enabledAddons == 0xffffffffffffffffU) && (flashAddonSettings.addonSettingsChecksum == 0xffffffffU)){     // Assume flash is erased, restore defaults silently
    return reset_Addons;
  }
  if(ChecksumAddons(&flashAddonSettings.addonSettings) != flashAddonSettings.addonSettingsChecksum){       // crc mismatch
    checksumError(reset_Addons);
    return reset_Addons;
  }
  else
    systemSettings.addonSettings = flashAddonSettings.addonSettings;

  return SUCCESS;
}

static void resetAddonSettings(addonSettings_t *addons){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  *addons = defaultAddonSettings;
  __set_PRIMASK(_irq);
}

static uint32_t ChecksumAddons(addonSettings_t* addonSettings){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)addonSettings, sizeof(addonSettings_t)/sizeof(uint32_t));
  return checksum;
}
#endif

static void resetSystemSettings(settings_t * data) {
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  *data = defaultSystemSettings;
  flashTempSettingsInitialSetup();
  __set_PRIMASK(_irq);
}

static void resetProfile(profile_t * data, uint8_t profile){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  resetProfileSettings(&data->settings, profile);
  if(profile==profile_T12){
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      data->tip[x].calADC_At_250   = T12_Cal250;
      data->tip[x].calADC_At_400   = T12_Cal400;     // These values are way lower, but better to be safe than sorry
      data->tip[x].PID.Kp          = 4000;           // val = /1.000.000
      data->tip[x].PID.Ki          = 5500;           // val = /1.000.000
      data->tip[x].PID.Kd          = 700;           // val = /1.000.000
      data->tip[x].PID.maxI        = 70;             // val = /100
      data->tip[x].PID.minI        = 0;              // val = /100
      strcpy(data->tip[x].name, _BLANK_TIP);         // Empty name
    }
    strcpy(data->tip[0].name, "T12-BC3");               // Put some generic name.
  }

  else if(profile==profile_C245){
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      data->tip[x].calADC_At_250   = C245_Cal250;
      data->tip[x].calADC_At_400   = C245_Cal400;
      data->tip[x].PID.Kp          = 4000;           // val = /1.000.000
      data->tip[x].PID.Ki          = 5500;           // val = /1.000.000
      data->tip[x].PID.Kd          = 700;           // val = /1.000.000
      data->tip[x].PID.maxI        = 70;             // val = /100
      data->tip[x].PID.minI        = 0;
      strcpy(data->tip[x].name, _BLANK_TIP);
    }
    strcpy(data->tip[0].name, "C245-963");
  }

  else if(profile==profile_C210){
    for(uint8_t x = 0; x < NUM_TIPS; x++) {
      data->tip[x].calADC_At_250   = C210_Cal250;
      data->tip[x].calADC_At_400   = C210_Cal400;
      data->tip[x].PID.Kp          = 4000;           // val = /1.000.000
      data->tip[x].PID.Ki          = 5500;           // val = /1.000.000
      data->tip[x].PID.Kd          = 700;           // val = /1.000.000
      data->tip[x].PID.maxI        = 70;             // val = /100
      data->tip[x].PID.minI        = 0;
      strcpy(data->tip[x].name, _BLANK_TIP);
    }
    strcpy(data->tip[0].name, "C210-018");
  }
  else{
    Error_Handler();  // We shouldn't get here!
  }
  __set_PRIMASK(_irq);
}


static void resetProfileSettings(profile_settings_t * data, uint8_t profile){
  uint32_t _irq = __get_PRIMASK();
  *data = defaultProfileSettings;

  __disable_irq();
    if(profile==profile_T12){
    data->ID = profile_T12;
    data->currentNumberOfTips      = 1;
    data->impedance                = 80;             // 8.0 Ohms
    data->power                    = 80;             // 80W
    data->noIronValue              = 4000;
    data->Cal250_default           = T12_Cal250;
    data->Cal400_default           = T12_Cal400;
  }

  else if(profile==profile_C245){
    data->ID = profile_C245;
    data->currentNumberOfTips      = 1;
    data->impedance                = 26;
    data->power                    = 150;
    data->noIronValue              = 4000;
    data->Cal250_default           = C245_Cal250;
    data->Cal400_default           = C245_Cal400;
  }

  else if(profile==profile_C210){
    data->ID = profile_C210;
    data->currentNumberOfTips    = 1;
    data->power                  = 80;
    data->impedance              = 21;
    data->noIronValue            = 1200;
    data->Cal250_default         = C210_Cal250;
    data->Cal400_default         = C210_Cal400;
  }
  else{
    Error_Handler();  // We shouldn't get here!
  }

  __set_PRIMASK(_irq);
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
    if(flashProfilesSettings.Profile[profile].settings.version != PROFILE_SETTINGS_VERSION){          // If flash profile not initialized or version mismatch
      return reset_Profile;                                                                           // Silent reset
    }
    else{
      if(ChecksumProfile(&flashProfilesSettings.Profile[profile]) != flashProfilesSettings.ProfileChecksum[profile]){
        checksumError(reset_Profile);
        return reset_Profile;
      }
      else{
        __disable_irq();
        systemSettings.Profile = flashProfilesSettings.Profile[profile].settings;
        systemSettings.currentProfile=profile;
      }
    }
  }
  else{
    Error_Handler();
  }
  setSystemTempUnit(getSystemTempUnit());                        // Ensure the profile uses the same temperature unit as the system

  if(systemSettings.settings.hasBattery){
    if((bkpRamData.values.lastTipTemp[profile] < systemSettings.Profile.MinSetTemperature) || (bkpRamData.values.lastTipTemp[profile] > systemSettings.Profile.MaxSetTemperature)){
      bkpRamData.values.lastTipTemp[profile] = systemSettings.Profile.defaultTemperature;
    }
    setUserTemperature(bkpRamData.values.lastTipTemp[profile]);

    if(bkpRamData.values.lastSelTip[profile] >= systemSettings.Profile.currentNumberOfTips)
      setCurrentTip(0);
    else
      setCurrentTip(bkpRamData.values.lastSelTip[profile]);
    writeBackupRam();
  }
  else{
    if((flashProfile==0xFF) || (flashTemp==0xFFFF)){                              // flashTemp not initialized, load defaults now the profile was loaded
      setUserTemperature(systemSettings.Profile.defaultTemperature);
      setCurrentTip(flashTip[profile]);
      updateTempData(force_update);                                               // Force save now
    }
    else{
      if((flashTemp < systemSettings.Profile.MinSetTemperature) || (flashTemp > systemSettings.Profile.MaxSetTemperature))
        setUserTemperature(systemSettings.Profile.defaultTemperature);            // Stored value out of limits, set default
      else
        setUserTemperature(flashTemp);                                            // Load stored value

      if(flashTip[profile] >= systemSettings.Profile.currentNumberOfTips)         // Bad tip index, load first tip
        setCurrentTip(0);
      else
        setCurrentTip(flashTip[profile]);
    }
  }
  systemSettings.currentProfile = profile;
  TIP.filter=systemSettings.Profile.tipFilter;
  ironSchedulePwmUpdate();
  __set_PRIMASK(_irq);
  return SUCCESS;
}

static void sortTips(profile_t * data, uint8_t profile){
  uint8_t min;
  char current[TIP_LEN+1];
  tipData_t backupTip;
  strcpy (current, systemSettings.currentTipData.name);                                           // Copy tip name being used in the system
  for(uint8_t j=0; j<systemSettings.Profile.currentNumberOfTips; j++){                            // Sort alphabetically
    min = j;                                                                                      // Min is start position
    for(uint8_t i=j+1; i<systemSettings.Profile.currentNumberOfTips; i++){
      if(strcmp(data->tip[min].name, data->tip[i].name) > 0)                                      // If Tip[Min] > Tip[i]
        min = i;                                                                                  // Update min
    }
    if(min!=j){                                                                                   // If j is not the min
      backupTip = data->tip[j];                                                                   // Swap j and min
      data->tip[j] = data->tip[min];
      data->tip[min] = backupTip;
    }
  }
  systemSettings.currentTip = 0;                                                                  // Just in case it's not found
  for(uint8_t i=0; i<systemSettings.Profile.currentNumberOfTips; i++){                            // Find the same tip after sorting
    if(strcmp(current, data->tip[i].name) == 0){                                                  // If matching name
      systemSettings.currentTip = i;                                                              // Update the tip to be reloaded later
      break;
    }
  }
}

void setCurrentTip(uint8_t tip) {
  if(tip >= systemSettings.Profile.currentNumberOfTips) // sanity check
    tip = 0u;
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  systemSettings.currentTip = tip;                                                                            // Set current tip number
  systemSettings.currentTipData = flashProfilesSettings.Profile[systemSettings.currentProfile].tip[tip];      // Copy data from flash
  setupPID(&systemSettings.currentTipData.PID);
  __set_PRIMASK(_irq);
}

tipData_t *getCurrentTip(void) {
  return &systemSettings.currentTipData;
}

static void Flash_error(void){
  HAL_FLASH_Lock();
  fatalError(error_FLASH);
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
        saveSettings(reset_All, do_reboot);
      }
    }
  }
}

bool isCurrentProfileChanged(void) {
  return ChecksumProfileSettings(&systemSettings.Profile) != flashProfilesSettings.ProfileSettingsChecksum[systemSettings.currentProfile];
}
bool isSystemSettingsChanged(void) {
  return ChecksumSystemSettings(&systemSettings.settings) != flashGlobalSettings.settingsChecksum;
}
bool isAddonSettingsChanged(void) {
  return ChecksumAddons(&systemSettings.addonSettings) != flashAddonSettings.addonSettingsChecksum;
}

void copy_bkp_data(uint8_t mode){
  if(mode==flash_to_ram){
    bkpRamData.values.lastProfile = flashProfile;
    for(uint8_t i=0;i<NUM_PROFILES;i++){
      bkpRamData.values.lastSelTip[i] = flashTip[i];
      bkpRamData.values.lastTipTemp[i] = flashTemp;
    }
    writeBackupRam();
  }
  else if(mode==ram_to_flash){
    flashProfile = bkpRamData.values.lastProfile;
    flashTemp = bkpRamData.values.lastTipTemp[0];                            // Flash temp only stores one temperature. Choose first profile.
    for(uint8_t i=0;i<NUM_PROFILES;i++){
      flashTip[i] = bkpRamData.values.lastSelTip[i];
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
