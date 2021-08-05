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

#define ProfileSize       3                                         // Number of profiles
#define TipSize           20                                        // Number of tips for each profile
#define TipCharSize       5                                         // String size for each tip name (Including null terminator)
#define _BLANK_TIP        "    "

#ifndef PROFILE_VALUES

#define T12_Cal250        1100                                      // Default values to be used in the calibration if not adjusted
#define T12_Cal350        1200                                      // TODO: Move these values to the board profile, so each board can have a closer default calibration
#define T12_Cal450        1300                                      // But we don't have calibration data from users!

#define C210_Cal250       300
#define C210_Cal350       400
#define C210_Cal450       500

#define C245_Cal250       900
#define C245_Cal350       1000
#define C245_Cal450       1100

#endif

//#define SWSTRING        "SW: v1.10"                               // For releases
#define SWSTRING          "SW: 2021-08-04b"                          // For git
#define SETTINGS_VERSION  8                                         // Change this if you change the struct below to prevent people getting out of sync
#define StoreSize         2                                         // In KB
#define FLASH_ADDR        (0x8000000 + ((FLASH_SZ-StoreSize)*1024)) // Last 2KB flash (Minimum erase size, page size=2KB)

enum{
  mode_shake              = 0,
  mode_stand              = 1,

  wake_off                = 0,
  wake_standby            = 1,
  wake_sleep              = 2,
  wake_all                = 3,

  wakeInput               = 0,
  wakeButton              = 1,

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

  initialized             = 0,

  profile_T12             = 0,
  profile_C245            = 1,
  profile_C210            = 2,
  profile_None            = 0xff,

  save_Settings           = 1,
  reset_Profiles          = 0x80,
  reset_Profile           = 0x81,
  reset_Settings          = 0x82,
  reset_All               = 0x83,

  keepProfiles            = 1,
  wipeProfiles            = 0x80,

  output_PWM,
  output_Low,
  output_High,
};


typedef struct{
  uint8_t       filter_normal;
  uint8_t       filter_partial;
  uint8_t       filter_spikes;
  uint8_t       filter_reset;
  uint8_t       spike_limit;
  uint16_t      partial_start;
  uint16_t      partial_end;
  uint16_t      reset_limit;
}filter_t;

typedef struct{
  uint16_t      calADC_At_250;
  uint16_t      calADC_At_350;
  uint16_t      calADC_At_450;
  char          name[TipCharSize];
  pid_values_t  PID;
}tipData_t;

typedef struct{
  uint8_t       NotInitialized;
  uint8_t       ID;
  uint8_t       impedance;
  uint8_t       tempUnit;
  uint8_t       currentNumberOfTips;
  uint8_t       currentTip;
  int8_t        CalNTC;
  uint8_t       pwmMul;
  uint8_t       sleepTimeout;
  uint8_t       standbyTimeout;
  filter_t      tipFilter;
  uint16_t      standbyTemperature;
  uint16_t      UserSetTemperature;
  uint16_t      MaxSetTemperature;
  uint16_t      MinSetTemperature;
  uint16_t      boostTimeout;
  uint16_t      boostTemperature;
  uint16_t      readPeriod;
  uint16_t      readDelay;
  uint16_t      noIronValue;
  uint16_t      power;
  uint16_t      Cal250_default;
  uint16_t      Cal350_default;
  uint16_t      Cal450_default;
  tipData_t     tip[TipSize];
}profile_t;

typedef struct{
  uint8_t       NotInitialized;                                     // Always 1 if flash is erased
  uint8_t       contrast;
  uint8_t       OledOffset;
  uint8_t       currentProfile;
  uint8_t       saveSettingsDelay;
  uint8_t       initMode;
  uint8_t       tempStep;
  uint8_t       screenDimming;
  uint8_t       tempUnit;
  uint8_t       activeDetection;
  uint8_t       buzzerMode;
  uint8_t       buttonWakeMode;                                     // 0=Nothing, 1= standby, 2= sleep,  3= both
  uint8_t       shakeWakeMode;
  uint8_t       WakeInputMode;
  uint8_t       StandMode;
  uint8_t       Pullup;
  uint8_t       NTC_detect;
  uint8_t       EncoderMode;
  uint8_t       lvp;
  uint8_t       errorDelay;
  uint8_t       guiUpdateDelay;
  uint16_t      NTC_Beta;
  uint16_t      Pull_res;
  uint16_t      NTC_res;
  uint16_t      NTC_detect_high_res;
  uint16_t      NTC_detect_low_res;
  uint16_t      NTC_detect_high_res_beta;
  uint16_t      NTC_detect_low_res_beta;
  uint16_t      version;                                            // Used to track if a reset is needed on firmware upgrade
}settings_t;

typedef __attribute__((aligned(4)))  struct{
  settings_t    settings;
  uint32_t      settingsChecksum;
  profile_t     Profile;
  uint32_t      ProfileChecksum;
  uint8_t       save_Flag;
  uint8_t       setupMode;
  uint8_t       isSaving;
}systemSettings_t;

typedef __attribute__((aligned(4)))  struct{
  profile_t     Profile[ProfileSize];
  uint32_t      ProfileChecksum[ProfileSize];
  settings_t    settings;
  uint32_t      settingsChecksum;
}flashSettings_t;

extern systemSettings_t systemSettings;
extern flashSettings_t* flashSettings;

void Diag_init(void);
void checkSettings(void);
void saveSettingsFromMenu(uint8_t mode);
void saveSettings(uint8_t mode);
void restoreSettings();
uint32_t ChecksumSettings(settings_t* settings);
uint32_t ChecksumProfile(profile_t* profile);
void resetSystemSettings(void);
void resetCurrentProfile(void);
void storeTipData(uint8_t tip);
void loadProfile(uint8_t tip);

#endif /* SETTINGS_H_ */
