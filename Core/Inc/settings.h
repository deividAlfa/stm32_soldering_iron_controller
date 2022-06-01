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

#define SWSTRING          "SW: "__DATE__                            // Software version reported in settings screen
#define SETTINGS_VERSION  17                                        // Change this if you change the settings/profile struct to prevent getting out of sync
#define LANGUAGE_COUNT    5                                         // Number of languages
#define ProfileSize       3                                         // Number of profiles
#define TipSize           40                                        // Number of tips for each profile
#define TipCharSize       5                                         // String size for each tip name (Including null termination)
#define _BLANK_TIP        "    "                                    // Empty tip name, 4 spaces. Defined here for quick updating if TipCharSize is modified.

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

  lang_english             = 0,
  lang_russian             = 1,
  lang_swedish             = 2,
  lang_german              = 3,
  lang_turkish             = 4,


  dim_off                  = 0,
  dim_sleep                = 1,
  dim_always               = 2,

  error_sleep              = 0,
  error_run                = 1,
  error_resume             = 2,

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
  char          name[TipCharSize];
  pid_values_t  PID;
}tipData_t;

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
  uint8_t       state;                // Always 0xFF if flash is erased
  uint8_t       ID;
  uint8_t       impedance;
  uint8_t       tempUnit;
  uint8_t       currentNumberOfTips;
  uint8_t       currentTip;
  uint8_t       pwmMul;
  uint8_t       errorResumeMode;
  uint8_t       shakeFiltering;
  uint8_t       WakeInputMode;
  uint8_t       StandMode;
  uint8_t       reserved_u8_001;
  uint8_t       reserved_u8_002;
  uint8_t       reserved_u8_003;
  uint8_t       reserved_u8_004;
  uint8_t       reserved_u8_005;
  uint8_t       reserved_u8_006;
  uint8_t       reserved_u8_007;
  uint8_t       reserved_u8_008;
  uint8_t       reserved_u8_009;
  uint8_t       reserved_u8_010;
  filter_t      tipFilter;
  ntc_data_t    ntc;
  uint16_t      standbyTemperature;
  uint16_t      UserSetTemperature;
  uint16_t      MaxSetTemperature;
  uint16_t      MinSetTemperature;
  uint16_t      boostTemperature;
  uint16_t      readPeriod;
  uint16_t      readDelay;
  uint16_t      noIronValue;
  uint16_t      power;
  uint16_t      calADC_At_0;
  uint16_t      Cal250_default;
  uint16_t      Cal400_default;
  uint16_t      reserved_u16_001;
  uint16_t      reserved_u16_002;
  uint16_t      reserved_u16_003;
  uint16_t      reserved_u16_004;
  uint16_t      reserved_u16_005;
  tipData_t     tip[TipSize];
  uint32_t      errorTimeout;
  uint32_t      boostTimeout;
  uint32_t      sleepTimeout;
  uint32_t      standbyTimeout;
  uint32_t      reserved_u32_001;
  uint32_t      reserved_u32_002;
  uint32_t      reserved_u32_003;
  uint32_t      reserved_u32_004;
  uint32_t      reserved_u32_005;
}profile_t;

__attribute__((aligned(4))) typedef struct{
  uint8_t       state;              // Always 0xFF if flash is erased
  uint8_t       language;
  uint8_t       contrast;
  uint8_t       displayOffset;
  uint8_t       displayXflip;
  uint8_t       displayYflip;
  uint8_t       displayResRatio;
  uint8_t       dim_mode;
  uint8_t       dim_inSleep;
  uint8_t       currentProfile;
  uint8_t       saveSettingsDelay;
  uint8_t       initMode;
  uint8_t       tempUnit;
  uint8_t       tempStep;
  uint8_t       tempBigStep;
  uint8_t       guiTempDenoise;
  uint8_t       activeDetection;
  uint8_t       buzzerMode;
  uint8_t       buttonWakeMode;
  uint8_t       shakeWakeMode;
  uint8_t       EncoderMode;
  uint8_t       lvp;
  uint8_t       debugEnabled;
  uint8_t       reserved_u8_001;
  uint8_t       reserved_u8_002;
  uint8_t       reserved_u8_003;
  uint8_t       reserved_u8_004;
  uint8_t       reserved_u8_005;
  uint8_t       reserved_u8_006;
  uint8_t       reserved_u8_007;
  uint8_t       reserved_u8_008;
  uint8_t       reserved_u8_009;
  uint8_t       reserved_u8_010;
  uint16_t      guiUpdateDelay;
  uint16_t      reserved_u16_001;
  uint16_t      reserved_u16_002;
  uint16_t      reserved_u16_003;
  uint16_t      reserved_u16_004;
  uint16_t      reserved_u16_005;
  uint32_t      dim_Timeout;
  uint32_t      reserved_u32_001;
  uint32_t      reserved_u32_002;
  uint32_t      reserved_u32_003;
  uint32_t      reserved_u32_004;
  uint32_t      reserved_u32_005;
  uint32_t      version;            // Used to track if a reset is needed on firmware upgrade
}settings_t;

__attribute__((aligned(4))) typedef struct{
  settings_t    settings;
  uint32_t      settingsChecksum;
  profile_t     Profile;
  uint32_t      ProfileChecksum;
  uint8_t       save_Flag;
  uint8_t       setupMode;
  uint8_t       isSaving;
}systemSettings_t;

__attribute__((aligned(4))) typedef struct{
  profile_t     Profile[ProfileSize];
  uint32_t      ProfileChecksum[ProfileSize];
  settings_t    settings;
  uint32_t      settingsChecksum;
}flashSettings_t;

extern systemSettings_t systemSettings;

void Oled_error_init(void);
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
