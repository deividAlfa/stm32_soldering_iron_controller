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


typedef struct{
  uint16_t profile[128];
  uint16_t tip[NUM_PROFILES][128];
  uint16_t temperature[512];
}temp_settings_t;

__attribute__((section(".tempSettings"))) temp_settings_t temp_settings;

static uint16_t flashTempIndex;
static uint16_t flashTemp;
static uint8_t flashTipIndex[NUM_PROFILES];
static uint8_t flashTip[NUM_PROFILES];
static uint8_t flashProfileIndex;
static uint8_t flashProfile;
static tipData_t *currentTipData;

systemSettings_t systemSettings;

static void storeSettings(uint8_t mode);
static void checksumError(uint8_t mode);
static void Flash_error(void);
static void Button_reset(void);
static void ErrCountDown(uint8_t Start,uint8_t xpos, uint8_t ypos);
static uint32_t ChecksumSettings(settings_t* settings);
static uint32_t ChecksumProfile(profile_t* profile);
static void resetSystemSettings(void);
static void resetCurrentProfile(void);
static void flashTempSettingsErase(void);
static void flashTempSettingsReset(void);
static void flashTempSettingsInit(void);
static void flashTempWrite(void);
static void flashTipWrite(void);
static void flashProfileWrite(void);

#ifdef ENABLE_ADDONS
static void loadAddonSettings(void);
static void resetAddonSettings();
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
static uint32_t ChecksumBackupRam();
static void readBackupRam();
static void writeBackupRam();

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
  static uint8_t prevTip[NUM_PROFILES];
  static uint8_t prevProfile;
  static uint16_t prevTemp;
  static uint32_t lastTempCheckTime;

  uint32_t CurrentTime = HAL_GetTick();
  uint8_t scr_index=current_screen->index;

  if(!getIronCalibrationMode() && scr_index != screen_debug){                               // Don't update while calibration is in progress or in the debug screen
                                                                                            // Always update the data, whether the batery option is enabled or not
    bkpRamData.values.lastSelTip[systemSettings.currentProfile] = systemSettings.currentTip;
    bkpRamData.values.lastProfile = systemSettings.currentProfile;
    bkpRamData.values.lastTipTemp[systemSettings.currentProfile] = getUserTemperature();

    if(systemSettings.settings.hasBattery == true){
      flashTip[systemSettings.currentProfile] = systemSettings.currentTip;                // This will keep track of tips in the flash variables just in case the battery option was enabled in the menu and later disabled.
      writeBackupRam();
    }
    else {
      uint16_t currentTemp = getUserTemperature();
      uint8_t currentTip = systemSettings.currentTip;
      uint8_t currentProfile = systemSettings.currentProfile;

      if(flashTemp != currentTemp) {                                                      // Compare with stored in flash
        if(prevTemp != currentTemp) {                                                     // Store if different to last check
          prevTemp = currentTemp;
          lastTempCheckTime = CurrentTime;                                                // Start timeout
        }
        else if(force || (CurrentTime-lastTempCheckTime)>4999) {                          // Different than flash and timeout is over
          flashTemp = currentTemp;
          flashTempWrite();                                                               // Update temperature in flash
        }
      }
      if(flashTip[currentProfile] != currentTip) {
        if(prevTip[currentProfile] != currentTip) {                                       // Store if different to last check
          prevTip[currentProfile] = currentTip;
          lastTempCheckTime = CurrentTime;                                                // Start timeout
        }
        else if(force || (CurrentTime-lastTempCheckTime)>4999) {                          // Different than flash and timeout is over
          flashTip[currentProfile] = currentTip;
          flashTipWrite();                                                                // Update tip data in flash
        }
      }
      if(flashProfile != currentProfile) {
        if(prevProfile != currentProfile) {                                               // Store if different to last check
          prevProfile = currentProfile;
          lastTempCheckTime = CurrentTime;                                                // Start timeout
        }
        else if(force || (CurrentTime-lastTempCheckTime)>4999) {                          // Different than flash and timeout is over
          flashProfile = currentProfile;
          flashProfileWrite();                                                            // Update tip data in flash
        }
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
  if(mi.uordblks > 128)                   // Check that the heap usage is low
    Error_Handler();                      // We got there from a working screen (Heap must be free)

  uint32_t _irq = __get_PRIMASK();

  if( ((mode & save_Settings) && (mode & reset_Settings)) ||        // Sanity check
      ((mode & save_Profile) && (mode & reset_Profile))   ||
      ((mode & save_Profile) && (mode & reset_Profiles))  ||
      ((mode & save_Addons) && (mode & reset_Addons))     ){

    Error_Handler();
  }

  uint8_t profile = systemSettings.currentProfile;

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

#ifdef ENABLE_ADDONS
  if(mode & reset_Addons){
    resetAddonSettings();
  }
  if(mode & (save_Addons | reset_Addons)){                                                                         // Save current addons
    systemSettings.addonSettingsChecksum = ChecksumAddons(&(systemSettings.addonSettings));
    flashBufferAddons->addonSettingsChecksum = systemSettings.addonSettingsChecksum;
    flashBufferAddons->addonSettings = systemSettings.addonSettings;
  }
  else{
    *flashBufferAddons = flashAddonSettings;                                                      // Keep current addons
  }
#endif
  if(mode & reset_Settings){                                                                      // Reset settings
    resetSystemSettings();
    if(mode == reset_All)
      systemSettings.settings.version = 0xFF;                                                     // To trigger setup screen
  }
  if(mode & (save_Settings | reset_Settings)){                                                    // Save current settings
    systemSettings.settingsChecksum = ChecksumSettings(&systemSettings.settings);
    flashBufferSettings->settingsChecksum = systemSettings.settingsChecksum;
    flashBufferSettings->settings = systemSettings.settings;
  }
  else{                                                                                           // Keep existing settings
    *flashBufferSettings = flashGlobalSettings;
  }

  if(mode & reset_Profiles){                                                                      // Reset all profiles to default
    for(uint8_t i=profile_T12; i<=profile_C210; i++){
      systemSettings.currentProfile = i;
      resetCurrentProfile();
      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
      flashBufferProfiles->ProfileChecksum[i] = systemSettings.ProfileChecksum;
      flashBufferProfiles->Profile[i] = systemSettings.Profile;
    }
    systemSettings.currentProfile = profile;
  }
  else{
    *flashBufferProfiles = flashProfilesSettings;                                                 // Save flash profiles into temp buffer

    if(mode & reset_Profile){                                                                     // Reset current profile
      resetCurrentProfile();
    }
    if(mode & (save_Profile | reset_Profile)){                                                    // Save current profile
      if((profile<=profile_C210) && (systemSettings.Profile.ID == profile )){
        systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);
        flashBufferProfiles->ProfileChecksum[profile] = systemSettings.ProfileChecksum;
        flashBufferProfiles->Profile[profile] = systemSettings.Profile;
      }
      else
        Error_Handler();
    }
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
  flashChecksum   = ChecksumSettings(&flashGlobalSettings.settings);
  ramChecksum     = ChecksumSettings(&flashBufferSettings->settings);
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
  systemSettings.isSaving = 0;

#endif
}

void restoreSettings() {
  bool setup=0;
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

  flashTempSettingsInit();                                  // Always init this, no matter if battery is enabled or not

  if(systemSettings.settings.hasBattery)
    loadSettingsFromBackupRam();

  loadProfile(systemSettings.currentProfile);

#if (SYSTEM_SETTINGS_VERSION == 27)             // Future version
#warning Remove this workaround!
#endif

  if(flashGlobalSettings.settings.version == 25){                                 // Upgrade settings version from 25 to 26 without erasing everything.
    systemSettings.settings.coldBoost = true;                                     // TODO: Remove this when settings version is updated
    flashGlobalSettings.settings.version = SYSTEM_SETTINGS_VERSION;               // Update version
  }

  if(flashGlobalSettings.settings.version != SYSTEM_SETTINGS_VERSION){            // System settings version mismatch
    if(flashGlobalSettings.settings.version == 0xFF)                              // 0xFF = erased flash, trigger setup mode
      setup=1;
    resetSystemSettings();                                                        // Reset current settings
#ifdef ENABLE_ADDONS
    resetAddonSettings();
#endif
  }
  else{
    Button_reset();
    systemSettings.settings = flashGlobalSettings.settings;
    systemSettings.settingsChecksum = flashGlobalSettings.settingsChecksum;
    if(ChecksumSettings(&systemSettings.settings)!=systemSettings.settingsChecksum){   // Show error message if bad checksum
      checksumError(reset_Settings);
    }
  }

#ifdef ENABLE_ADDONS
  loadAddonSettings();
#endif

  if(setup){
    systemSettings.setupMode = enable;                      // Load setup state for boot screen
    setSafeMode(enable);
  }
}

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
  ironSchedulePwmUpdate();
  setUserTemperature(bkpRamData.values.lastTipTemp[systemSettings.currentProfile]);
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
  bkpRamData.crc = ChecksumBackupRam(bkpRamData);
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
        flashProfileIndex = i;
        flashProfile=temp_settings.profile[i];
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

  if(setDefault){
    flashTempSettingsErase();

    for(uint8_t n=0; n<NUM_PROFILES; n++)           // Reset buffered values to trigger flash udpate
      flashTipIndex[n] =  0xFF;
    flashTemp = 0xFFFF;
    flashProfile = 0xFF;
    systemSettings.currentProfile = profile_T12;
    systemSettings.currentTip = 0;
    setUserTemperature(systemSettings.Profile.defaultTemperature);
  }
  else{
    uint8_t profile = temp_settings.profile[flashProfileIndex];
    uint16_t temp = temp_settings.temperature[flashTempIndex];
    if(profile > profile_C210)
      profile = profile_T12;

    systemSettings.currentProfile = profile;

    /* Load temperature */
    if( temp > systemSettings.Profile.MaxSetTemperature || temp < systemSettings.Profile.MinSetTemperature)   // Check limits
      setUserTemperature(systemSettings.Profile.defaultTemperature);
    else
      setUserTemperature(temp);                                                                   // Load temperature from slot


    if(temp_settings.temperature[flashTempIndex] != UINT16_MAX) flashTempIndex++;                                             // Increase index if current slot is not empty (Only required when flash is completely erased)
    if(temp_settings.profile[flashProfileIndex] != UINT16_MAX) flashProfileIndex++;
    for(uint8_t n=0; n<NUM_PROFILES; n++)
      if(temp_settings.tip[n][flashTipIndex[n]] != UINT16_MAX) flashTipIndex[n]++;
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
}

void flashTempSettingsReset(void){
  flashTempSettingsErase();
  flashTemp=0;
}


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
  if((flashAddonSettings.addonSettings.enabledAddons == 0xffffffffffffffffU) && (flashAddonSettings.addonSettingsChecksum == 0xffffffffU)){     // Assume flash is erased, restore defaults silently
    resetAddonSettings();
    return;
  }
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
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  systemSettings.addonSettings = defaultAddonSettings;
  systemSettings.addonSettingsChecksum = 0u;
  __set_PRIMASK(_irq);
}

static uint32_t ChecksumAddons(addonSettings_t* addonSettings){
  uint32_t checksum;
  checksum = HAL_CRC_Calculate(&hcrc, (uint32_t*)addonSettings, sizeof(addonSettings_t)/sizeof(uint32_t));
  return checksum;
}
#endif

static void resetSystemSettings(void) {
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  systemSettings.settings = defaultSettings;
  flashTempSettingsReset();
  __set_PRIMASK(_irq);
}


static void resetCurrentProfile(void){
  uint32_t _irq = __get_PRIMASK();
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
    strcpy(systemSettings.Profile.tip[0].name, "T12-BC3");               // Put some generic name.
    systemSettings.Profile.currentNumberOfTips      = 1;
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
    strcpy(systemSettings.Profile.tip[0].name, "C245-963");
    systemSettings.Profile.currentNumberOfTips      = 1;
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
    strcpy(systemSettings.Profile.tip[0].name, "C210-018");
    systemSettings.Profile.currentNumberOfTips      = 1;
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

  systemSettings.Profile.errorTimeout               = 500;                    // ms
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
  systemSettings.Profile.smartActiveEnabled         = disable;
  systemSettings.Profile.smartActiveLoad            = 30;
  systemSettings.Profile.standDelay                 = 0;
  systemSettings.Profile.StandMode                  = mode_sleep;
  systemSettings.Profile.version                    = PROFILE_SETTINGS_VERSION;
  __set_PRIMASK(_irq);
}

void loadProfile(uint8_t profile){
  uint8_t p = profile;
  if(profile>profile_C210){
    profile = profile_T12;                            // If profile not valid, set T12 by default
  }

  while(ADC_Status!=ADC_Idle);
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  HAL_IWDG_Refresh(&hiwdg);
  systemSettings.currentProfile=profile;
  if(profile<=profile_C210){                                                                  // If valid profile
    if(flashProfilesSettings.Profile[profile].version != PROFILE_SETTINGS_VERSION){           // If flash profile not initialized or version mismatch
      resetCurrentProfile();                                                                  // Load defaults silently
      systemSettings.ProfileChecksum = ChecksumProfile(&systemSettings.Profile);              // Compute checksum, this avoids data saving from trigger.
    }                                                                                         // There's no need to store defaults. Will be updated once the user makes any change.
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

  if(systemSettings.settings.hasBattery)
    systemSettings.currentTip = bkpRamData.values.lastSelTip[profile];
  else
    systemSettings.currentTip = flashTip[profile];

  setCurrentTip(systemSettings.currentTip);
  TIP.filter=systemSettings.Profile.tipFilter;
  ironSchedulePwmUpdate();
  if(p==profile_None){                                     // If profile not set, revert to profile_none to trigger setup screen
    systemSettings.currentProfile = p;
  }
  __set_PRIMASK(_irq);
}

void setCurrentTip(uint8_t tip) {
  if(tip >= systemSettings.Profile.currentNumberOfTips) // sanity check
    tip = 0u;
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  systemSettings.currentTip = tip;
  currentTipData = &systemSettings.Profile.tip[tip];
  setupPID(&currentTipData->PID);
  __set_PRIMASK(_irq);
}

tipData_t *getCurrentTip(void) {
  return currentTipData;
}


static void Flash_error(void){
  HAL_FLASH_Lock();
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
  saveSettings(mode, do_reboot);
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
        saveSettings(reset_All, do_reboot);
      }
    }
  }
}

bool isCurrentProfileChanged(void) {
  return ChecksumProfile(&systemSettings.Profile) != systemSettings.ProfileChecksum;
}
bool isSystemSettingsChanged(void) {
  return ChecksumSettings(&systemSettings.settings) != systemSettings.settingsChecksum;
}
bool isAddonSettingsChanged(void) {
  return ChecksumAddons(&systemSettings.addonSettings) != systemSettings.addonSettingsChecksum;
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
