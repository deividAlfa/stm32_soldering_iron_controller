/*
 * settings.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"
#include "pid.h"
#include "board.h"

#define ENABLE_DBG_SAVE
#define SWSTRING          "SW: 1.13.4     "                         // Software version reported in settings screen
#define SYSTEM_SETTINGS_VERSION   28                                // Change this if you change the system settings struct to prevent getting out of sync
#define PROFILE_SETTINGS_VERSION  4                                 // Same, but for profile settings struct
#define TIP_SETTINGS_VERSION      1                                 // Same, but for tip settings struct
#define ADDONS_SETTINGS_VERSION   1                                 // Same, but for addons settings struct

#define LANGUAGE_COUNT    7                                         // Number of languages
#define NUM_PROFILES      3                                         // Number of profiles
#define NUM_TIPS          85                                        // Number of tips for each profile
#define TIP_LEN           8                                         // String size for each tip name (Including null termination)
#define _BLANK_TIP        "        "                                // Empty tip name, containing (TIP_LEN) spaces. Defined here for quick updating if TIP_LEN is modified.

#define TEMPSETTINGS_SAVE_TIME  3000                                // Last tip/profile/temperature in flash are updated after 3 seconds with no changes. This is not applied when battery option is enabled (Stored instantly).
#define DEFAULT_TEMPERATURE     320                                 // Temperature loaded by default
#ifndef PROFILE_VALUES

#define T12_Cal250        1100                                      // Default values to be used in the calibration if not adjusted
#define T12_Cal350        1200
#define T12_Cal450        1300

#define C210_Cal250       300
#define C210_Cal350       400
#define C210_Cal450       500

#define C245_Cal250       900
#define C245_Cal350       1000
#define C245_Cal450       1100

#endif


typedef enum{
  mode_shake              = 0,
  mode_stand              = 1,

  wake_off                = 0,
  wake_standby            = 1,
  wake_sleep              = 2,
  wake_all                = 3,

  no_update               = 0,
  needs_update            = 1,

  runaway_ok              = 0,
  runaway_25              = 1,
  runaway_50              = 2,
  runaway_75              = 3,
  runaway_100             = 4,
  runaway_500             = 5,

  runaway_triggered       = 1,

  disable                 = 0,
  enable                  = 1,

  old_reading             = 0,
  new_reading             = 1,

  read_average            = 0,
  read_unfiltered         = 1,

  encoder_normal          = 0,
  encoder_reverse         = 1,

  mode_Celsius            = 0,
  mode_Farenheit          = 1,

  mode_sleep              = 0,
  mode_standby            = 1,
  mode_run                = 2,
  mode_boost              = 3,
  mode_coldboost          = 4,

  initialized             = 0,

  profile_T12             = 0,
  profile_C245            = 1,
  profile_C210            = 2,
  profile_None            = 0xff,

  save_settings           = 1,
  save_profile            = 2,
  save_addons             = 8,
  save_all                = save_settings | save_profile | save_addons,

  reset_settings          = 0x10,
  reset_profile           = 0x20,
  reset_profiles          = 0x40,
  reset_addons            = 0x80,
  reset_all               = reset_settings | reset_profiles |  reset_addons,

  no_mode                 = 0,
  save_tip                = 1,
  add_tip                 = 2,
  delete_tip              = 3,
  reset_tips              = 4,
  reset_allTips           = 5,

  perform_scanFix         = 0,      // Perform check over existing flash, reset any wrong section

  no_reboot               = 0,
  do_reboot               = 1,

  ram_to_flash            = 0,
  flash_to_ram            = 1,

  output_PWM,
  output_Low,
  output_High,

  lang_english            = 0,
  lang_russian            = 1,
  lang_swedish            = 2,
  lang_german             = 3,
  lang_turkish            = 4,
  lang_tchinese           = 5,
  lang_bulgarian          = 6,

  dim_off                 = 0,
  dim_sleep               = 1,
  dim_always              = 2,

  error_sleep             = 0,
  error_run               = 1,
  error_resume            = 2,

  normal_update           = 0,
  force_update            = 1,
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  fume_extractor_mode_disabled  = 0,
  fume_extractor_mode_auto      = 1,
  fume_extractor_mode_always_on = 2,
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
  switch_off_reminder_short_beep  = 0,
  switch_off_reminder_medium_beep = 1,
  switch_off_reminder_long_beep   = 2,
#endif
}system_types;


__attribute__((aligned(4))) typedef struct{
  int8_t        coefficient;          // Filter normally applied
  int8_t        counter;              // Counter for threshold limit
  int8_t        min;                  // Minimum filtering when decreasing
  int8_t        step;                 // Start decreasing the filter coefficient, assume it's a fast temperature change, so provide faster response
  int8_t        count_limit;          // Count the spikes, if exceeding this limit, start reducing the filter coefficient.
  uint16_t      threshold;            // Base noise limit, if diff exceeds this limit, trigger threshold limit and start decreasing filtering
  uint16_t      reset_threshold;      // Threshold for completely resetting the filter
}filter_t;

__attribute__((aligned(4))) typedef struct{
  uint16_t      calADC_At_250;
  uint16_t      calADC_At_400;
  char          name[TIP_LEN+1];
  pid_values_t  PID;
}tipData_t;

__attribute__((aligned(2048))) typedef struct{          // 2KB aligned, as flash sector erase size
  uint8_t       version;
  uint8_t       currentNumberOfTips;
  tipData_t     tip[NUM_TIPS];
}flashTipData_t;

typedef struct{
  flashTipData_t data;
  uint32_t       crc;
}flashTipSlot_t;

__attribute__((aligned(4))) typedef struct{
  uint8_t       enabled;
  uint8_t       detection;
  uint8_t       pullup;
  uint16_t      pull_res;
  uint16_t      NTC_res;
  uint16_t      NTC_beta;
  uint16_t      high_NTC_res;
  uint16_t      low_NTC_res;
  uint16_t      high_NTC_beta;
  uint16_t      low_NTC_beta;
}ntc_data_t;

__attribute__((aligned(4))) typedef struct{
  uint16_t      version;                // Always 0xFFFF if flash is erased, used to detect blank flash, or settings version mismatch
  uint8_t       ID;
  uint8_t       impedance;
  uint8_t       tempUnit;
  uint8_t       pwmMul;
  uint8_t       errorResumeMode;
  uint8_t       shakeFiltering;
  uint8_t       WakeInputMode;
  uint8_t       StandMode;
  uint8_t       smartActiveEnabled;
  uint8_t       coldBoostEnabled;
  uint8_t       smartActiveLoad;
  uint8_t       standDelay;
  filter_t      tipFilter;
  ntc_data_t    ntc;
  uint16_t      standbyTemperature;
  uint16_t      MaxSetTemperature;
  uint16_t      MinSetTemperature;
  uint16_t      boostTemperature;
  uint16_t      coldBoostTemperature;
  uint16_t      readPeriod;
  uint16_t      readDelay;
  uint16_t      noIronValue;
  uint16_t      power;
  uint16_t      calADC_At_0;
  uint16_t      Cal250_default;
  uint16_t      Cal400_default;
  uint32_t      errorTimeout;
  uint32_t      boostTimeout;
  uint32_t      coldBoostTimeout;
  uint32_t      sleepTimeout;
  uint32_t      standbyTimeout;
}profile_settings_t;

__attribute__((aligned(4))) typedef struct{
  uint16_t      version;                // Always 0xFF if flash is erased, used to detect blank flash, or settings version mismatch
  uint8_t       language;
  uint8_t       contrastOrBrightness;
  uint8_t       displayStartColumn;
  uint8_t       displayStartLine;
  uint8_t       displayXflip;
  uint8_t       displayYflip;
  uint8_t       displayResRatio;
  uint8_t       hasBattery;
  uint8_t       dim_inSleep;
  uint8_t       EncoderMode;
  uint8_t       debugEnabled;
  uint8_t       activeDetection;
  uint8_t       clone_fix;
  uint8_t       dim_mode;
  uint8_t       initMode;
  uint8_t       tempUnit;
  uint8_t       tempStep;
  uint8_t       tempBigStep;
  uint8_t       guiTempDenoise;
  uint8_t       buzzerMode;
  uint8_t       buttonWakeMode;
  uint8_t       shakeWakeMode;
  uint8_t       lvp;
  uint8_t       displayVcom;
  uint8_t       displayClk;
  uint8_t       displayPrecharge;
  uint16_t      guiUpdateDelay;
  uint32_t      dim_Timeout;
}systemSettings_t;

#ifdef ENABLE_ADDONS
__attribute__((aligned(4))) typedef struct {
  // bitmask for enabled addons, used to check if switching on and off multiple addons causes the struct
  // to be the same size, thus matching CRC, but in reality its incompatible due to layout change
  uint8_t version;
  uint64_t enabledAddons;
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  uint8_t fumeExtractorMode;
  uint8_t fumeExtractorAfterrun; // amount of delay in 5 second increments
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
  uint8_t swOffReminderEnabled;         // enabled, disabled
  uint8_t swOffReminderInactivityDelay; // amount of minutes in sleep mode before start beeping,
  uint8_t swOffReminderBeepType;        // beep type: short, medium, long
  uint8_t swOffReminderPeriod;          // amount of minutes between reminders
#endif
}addonSettings_t;
#endif

__attribute__((aligned(4))) typedef struct{
  systemSettings_t   system;
#ifdef ENABLE_ADDONS
  addonSettings_t   addons;
#endif
  profile_settings_t profile;
  tipData_t       currentTipData;
  uint8_t         setupMode;
  uint8_t         isSaving;
  uint8_t         currentProfile;
  uint8_t         currentTip;
}settings_t;


__attribute__((aligned(4))) typedef struct{
  systemSettings_t  system;
#ifdef ENABLE_ADDONS
  addonSettings_t   addons;
#endif
  profile_settings_t profile[NUM_PROFILES];
  uint32_t          systemChecksum;
  uint32_t          addonsChecksum;
  uint32_t          profileChecksum[NUM_PROFILES];
} flashSettings_t;

typedef struct{
  uint16_t profile[128];
  uint16_t tip[NUM_PROFILES][128];
  uint16_t temperature[512];
}temp_settings_t;

typedef struct{
  uint8_t  tipIndex[NUM_PROFILES];
  uint8_t  tip[NUM_PROFILES];
  uint8_t  profileIndex;
  uint8_t  profile;
  uint8_t  prevTip[NUM_PROFILES];
  uint8_t  prevProfile;
  uint16_t prevTemp;
  uint16_t tempIndex;
  uint16_t temp;
  uint32_t tempCheckTime;
  uint32_t tipCheckTime;
  uint32_t profileCheckTime;
}flashTemp_t;


#define BACKUP_RAM_SIZE_IN_BYTES 20u
#define NUM_BACKUP_RAM_REGISTERS (BACKUP_RAM_SIZE_IN_BYTES / 2u) // each register holds 2 byte of data in the lower nibble

typedef union {
  struct {
    struct {
      // max (BACKUP_RAM_SIZE_IN_BYTES - 4) byte of data in total
      uint16_t lastTipTemp[NUM_PROFILES];
      uint8_t  lastSelTip[NUM_PROFILES];
      uint8_t  lastProfile;
    } values;
    uint32_t crc;
  };
  uint8_t bytes[BACKUP_RAM_SIZE_IN_BYTES];
} backupRamData_t;

extern settings_t settings;
extern const systemSettings_t defaultSystemSettings;
extern const profile_settings_t defaultProfileSettings;
extern const tipData_t defaultTipData[NUM_PROFILES];
#ifdef ENABLE_DBG_SAVE
extern uint8_t save_prevstate_debug, save_laststate_debug;
#endif
/** Cyclic task to save the current temperature/tip/profile if needed. */
void updateTempData(bool force);
/** Save the settings to flash. */
void saveSettings(uint8_t save_mode, uint8_t reboot_mode);
void saveTip(uint8_t save_mode, uint8_t tip_index);

/** Reads the save flag. */
uint8_t getSaveFlag(void);

/** Loads the settings from flash on boot. */
void restoreSettings(void);
flashSettings_t * getFlashSettings(void);
systemSettings_t* getSystemSettings(void);
settings_t * getSettings(void);
systemSettings_t * getDefaultSystemSettings(void);
systemSettings_t * getFlashSystemSettings(void);
flashSettings_t * getFlashSettings(void);

/** Load/change to the profile with the given index */
void loadProfile(uint8_t profile);
void setCurrentProfile(uint8_t profile);
uint8_t getCurrentProfile(void);
profile_settings_t * getProfileSettings(void);
profile_settings_t * getDefaultProfileSettings(void);
profile_settings_t * getFlashProfileSettings(void);

#ifdef ENABLE_ADDONS
addonSettings_t * getAddons(void);
addonSettings_t * getDefaultAddons(void);
addonSettings_t * getFlashAddons(void);
bool isAddonSettingsChanged(void);
#endif

void loadTipDataFromFlash(uint8_t tip);
tipData_t * getFlashTipData(uint8_t tip);
tipData_t * getCurrentTipData(void);
tipData_t * getDefaultTipData(void);
void setCurrentTipData(tipData_t * tip);
uint8_t getCurrentTip(void);
uint8_t getCurrentNumberOfTips(void);
void setCurrentTip(uint8_t tip);


/** Checks if the current system settings in RAM were changed */
bool isSystemSettingsChanged(void);
/** Checks if the current profile in RAM is changed */
bool isCurrentProfileChanged(void);
/** Copies last Profile / Temperature / Tip data between flash and ram when battery option is changed*/
void copy_bkp_data(uint8_t mode);
/** Set initial values after setup screen */
void flashTempSettingsInitialSetup(void);
#endif /* SETTINGS_H_ */
