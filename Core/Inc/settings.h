/*
 * settings.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "main.h"
#include "pid.h"
#include "board.h"

#define ProfileSize       3                                         // Number of profiles
#define TipSize           10                                        // Number of tips for each profile
#define TipCharSize       5                                         // String size for each tip name (Including null terminator)
#define _BLANK_TIP        "    "

#ifndef T12_Cal250
#define T12_Cal250        1100                                      // Default values to be used in the calibration if not adjusted
#define T12_Cal350        1200                                      // TODO: Move these values to the board profile, so each board can have a closer default calibration
#define T12_Cal450        1300                                      // But we don't have calibration data from users!
#endif

#define C210_Cal250       300
#define C210_Cal350       400
#define C210_Cal450       500

#define C245_Cal250       900
#define C245_Cal350       1000
#define C245_Cal450       1100

//#define SWSTRING        "SW: v1.10"                               // For releases
#define SWSTRING          "SW: 2021-07-04"                          // For git
#define SETTINGS_VERSION  4                                         // Change this if you change the struct below to prevent people getting out of sync
#define StoreSize         2                                         // In KB
#define FLASH_ADDR     (0x8000000 + ((FLASH_SZ-StoreSize)*1024))    // Last 2KB flash (Minimum erase size, page size=2KB)

enum{
  wakeInputmode_shake     = 0,
  wakeInputmode_stand     = 1,

  wakeButton_Off          = 0,
  wakeButton_On           = 1,

  wakeShake_Off           = 0,
  wakeShake_On            = 1,

  source_wakeInput        = 0,
  source_wakeButton       = 1,

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

  debug_Off               = 0,
  debug_On                = 1,

  calibration_Off         = 0,
  calibration_On          = 1,

  setup_Off               = 0,
  setup_On                = 1,

  encoder_normal          = 0,
  encoder_reverse         = 1,

  mode_Celsius            = 0,
  mode_Farenheit          = 1,

  mode_sleep              = 0,
  mode_standby            = 1,
  mode_run                = 2,

  initialized             = 0,

  buzzer_Off              = 0,
  buzzer_On               = 1,

  profile_T12             = 0,
  profile_C245            = 1,
  profile_C210            = 2,
  profile_None            = 0xff,

  save_Settings           = 1,
  reset_Profile           = 0x80,
  reset_Profiles          = 0x81,
  reset_Settings          = 0x82,
  reset_All               = 0x83,
  keepProfiles            = 1,
  wipeProfiles            = 0x80
};


typedef struct tipData {
  uint16_t      calADC_At_250;
  uint16_t      calADC_At_350;
  uint16_t      calADC_At_450;
  char          name[TipCharSize];
  pid_values_t  PID;
}tipData;

typedef struct{
  uint8_t       NotInitialized;
  uint8_t       ID;
  uint8_t       impedance;
  uint8_t       tempUnit;
  uint8_t       currentNumberOfTips;
  uint8_t       currentTip;
  uint8_t       filterFactor;
  int8_t        CalNTC;
  uint8_t       sleepTimeout;
  uint8_t       standbyTimeout;
  uint8_t       standbyTemperature;
  uint16_t      UserSetTemperature;
  uint16_t      MaxSetTemperature;
  uint16_t      MinSetTemperature;
  uint16_t      pwmPeriod;
  uint16_t      pwmDelay;
  uint16_t      noIronValue;
  uint16_t      power;
  uint16_t      Cal250_default;
  uint16_t      Cal350_default;
  uint16_t      Cal450_default;
  tipData       tip[TipSize];
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
  uint8_t       wakeOnButton;
  uint8_t       wakeOnShake;
  uint8_t       WakeInputMode;
  uint8_t       StandMode;
  uint8_t       EncoderMode;
  uint16_t      errorDelay;
  uint16_t      guiUpdateDelay;
  uint16_t      lvp;
  uint32_t      version;                                            // Used to track if a reset is needed on firmware upgrade
}settings_t;

typedef __attribute__((aligned(4)))  struct{
  settings_t    settings;
  uint32_t      settingsChecksum;
  profile_t     Profile;
  uint32_t      ProfileChecksum;
  bool          setupMode;
  uint8_t       save_Flag;

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
