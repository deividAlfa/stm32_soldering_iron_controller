/*
 * gui_strings.h
 *
 *  Created on: 27 ene. 2021
 *      Author: David
 */

#ifndef GRAPHICS_GUI_STRINGS_H_
#define GRAPHICS_GUI_STRINGS_H_


#include "settings.h"

typedef struct{
  char * boot_firstBoot;
  char * boot_Profile;

  char * main_error_noIron;
  char * main_error_noIron_Detected;
  char * main_error_failsafe;
  char * main_error_NTC_high;
  char * main_error_NTC_low;
  char * main_error_VoltageLow;
  char * main_mode_Sleep;
  char * main_mode_Standby;
  char * main_mode_Boost;

  char * settings_IRON;
  char * settings_SYSTEM;
  char * settings_DEBUG;
  char * settings_EDIT_TIPS;
  char * settings_CALIBRATION;
  char * settings_EXIT;

  char * IRON_Max_Temp;
  char * IRON_Min_Temp;
  char * IRON_User_Temp;
  char * IRON_Standby;
  char * IRON_Sleep;
  char * IRON_Boost;
  char * IRON_Boost_Add;
  char * IRON_Power;
  char * IRON_Heater;
  char * IRON_ADC_Time;
  char * IRON_PWM_mul;
  char * IRON_No_Iron;
  char * IRON_Filter_Settings;

  char * FILTER_Filter;
  char * FILTER__Threshold;
  char * FILTER__Count_limit;
  char * FILTER__Step_down;
  char * FILTER__Min;
  char * FILTER_Reset_limit;

  char * SYSTEM_Profile;
  char * SYSTEM_Contrast;
  char * SYSTEM_Auto_Dim;
  char * SYSTEM_Offset;
  char * SYSTEM_Wake_Mode;
  char * SYSTEM_Stand_Mode;
  char * SYSTEM_Boot;
  char * SYSTEM_Button_Wake;
  char * SYSTEM_Shake_Wake;
  char * SYSTEM_Encoder;
  char * SYSTEM_Buzzer;
  char * SYSTEM_Temperature;
  char * SYSTEM__Step;
  char * SYSTEM__Big_Step;
  char * SYSTEM_Active_Detection;
  char * SYSTEM_LVP;
  char * SYSTEM_Gui_Time;
  char * SYSTEM_DEBUG;
  char * SYSTEM_NTC_MENU;
  char * SYSTEM_RESET_MENU;

  char * NTC_Enable_NTC;
  char * NTC_Pull;
  char * NTC__Res;
  char * NTC__Beta;
  char * NTC_NTC_Detect;
  char * NTC__High;
  char * NTC__Low;

  char * RESET_Reset_Settings;
  char * RESET_Reset_Profile;
  char * RESET_Reset_Profiles;
  char * RESET_Reset_All;

  char * RESET_Reset_msg_settings_1;
  char * RESET_Reset_msg_settings_2;
  char * RESET_Reset_msg_profile_1;
  char * RESET_Reset_msg_profile_2;
  char * RESET_Reset_msg_profiles_1;
  char * RESET_Reset_msg_profiles_2;
  char * RESET_Reset_msg_all_1;
  char * RESET_Reset_msg_all_2;

  char * TIP_SETTINGS_Name;
  char * TIP_SETTINGS_PID_kp;
  char * TIP_SETTINGS_PID_ki;
  char * TIP_SETTINGS_PID_kd;
  char * TIP_SETTINGS_PID_Imax;
  char * TIP_SETTINGS_PID_Imin;
  char * TIP_SETTINGS_DELETE;
  char * TIP_SETTINGS_COPY;

  char * CAL_Step;
  char * CAL_Wait;
  char * CAL_Measured;
  char * CAL_Succeed;
  char * CAL_Failed;
  char * CAL_DELTA_HIGH_1;
  char * CAL_DELTA_HIGH_2;
  char * CAL_DELTA_HIGH_3;
  char * CAL_Error;
  char * CAL_Aborting;

  char * _Language;
  char * __Temp;
  char * __Delay;
  char * _Cal_250;
  char * _Cal_350;
  char * _Cal_450;
  char * _BACK;
  char * _SAVE;
  char * _CANCEL;
  char * _STOP;
  char * _RESET;
  char * _START;
  char * _SETTINGS;
  char * _ADD_NEW;

  char * ERROR_RUNAWAY;
  char * ERROR_EXCEEDED;
  char * ERROR_UNKNOWN;
  char * ERROR_SYSTEM_HALTED;
  char * ERROR_BTN_RESET;

  char *OffOn[2];
  char *DownUp[2];
  char *wakeMode[2];
  char *encMode[2];
  char *InitMode[3];
  char *WakeModes[4];

}strings_t;

extern const strings_t strings[LANGUAGE_COUNT];
extern char * tempUnit[2];
extern char *profileStr[ProfileSize];
extern char *Langs[LANGUAGE_COUNT];

#endif /* GRAPHICS_GUI_STRINGS_H_ */
