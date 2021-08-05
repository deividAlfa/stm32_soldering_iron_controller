/*
 * settings_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "screen_common.h"

screen_t Screen_settings;
#ifdef ENABLE_DEBUG_SCREEN
static comboBox_item_t *comboitem_system_debug;
#endif

static void SETTINGS_create(screen_t *scr) {
  widget_t* w;
  //  [ SETTINGS MAIN SCREEN ]
  //
  newWidget(&w,widget_combo,scr);
  newComboScreen(w, "IRON", screen_iron, NULL);
  newComboScreen(w, "SYSTEM", screen_system, NULL);
  #ifdef ENABLE_DEBUG_SCREEN
  newComboScreen(w, "DEBUG", screen_debug, &comboitem_system_debug);
  #endif
  newComboScreen(w, "EDIT TIPS", screen_tip_list, NULL);
  newComboScreen(w, "CALIBRATION", screen_calibration, NULL);
  newComboScreen(w, "EXIT", screen_main, NULL);
}


static void SETTINGS_OnEnter(screen_t *scr) {
  if(scr==&Screen_main){
    comboResetIndex(Screen_settings.widgets);
  }
  if(ChecksumProfile(&systemSettings.Profile)!=systemSettings.ProfileChecksum){         // If there's unsaved profile data
    saveSettingsFromMenu(save_Settings);                                                // Save settings
  }

#ifdef ENABLE_DEBUG_SCREEN
  comboitem_system_debug->enabled = (systemSettings.settings.debugEnabled);
#endif

}


void settings_screen_setup(screen_t *scr) {
  scr->create = &SETTINGS_create;
  scr->onEnter = &SETTINGS_OnEnter;
  scr->processInput=&autoReturn_ProcessInput;
}
