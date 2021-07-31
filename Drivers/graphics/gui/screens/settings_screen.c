/*
 * settings_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "screen_common.h"

screen_t Screen_settings;


static void SETTINGS_init(screen_t *scr) {
  default_init(scr);
  settingsTimer=HAL_GetTick();
}


static void SETTINGS_create(screen_t *scr) {
  widget_t* w;
  //  [ SETTINGS MAIN SCREEN ]
  //
  newWidget(&w,widget_combo,scr);
  newComboScreen(w, "IRON", screen_iron, NULL);
  newComboScreen(w, "SYSTEM", screen_system, NULL);
  newComboScreen(w, "EDIT TIPS", screen_tip_list, NULL);
  newComboScreen(w, "CALIBRATION", screen_calibration, NULL);
  #ifdef ENABLE_PID_DEBUG_SCREEN
  newComboScreen(w, "PID DEBUG", screen_pid_debug, NULL);
  #endif
  #ifdef ENABLE_DEBUG_SCREEN
  newComboScreen(w, "DEBUG", screen_debug, NULL);
  #endif
  newComboScreen(w, "EXIT", screen_main, NULL);
}


static void SETTINGS_OnEnter(screen_t *scr) {
  if(scr==&Screen_main){
    comboResetIndex(Screen_settings.widgets);
  }
  if(ChecksumProfile(&systemSettings.Profile)!=systemSettings.ProfileChecksum){         // If there's unsaved profile data
    saveSettingsFromMenu(save_Settings);                                                // Save settings
  }
}


void settings_screen_setup(screen_t *scr) {
  scr->init = &SETTINGS_init;
  scr->create = &SETTINGS_create;
  scr->onEnter = &SETTINGS_OnEnter;
  scr->processInput=&autoReturn_ProcessInput;
}
