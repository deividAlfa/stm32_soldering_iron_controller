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

  newComboScreen(w, strings[lang].settings_IRON, screen_iron, NULL);
  newComboScreen(w, strings[lang].settings_SYSTEM, screen_system, NULL);
  #ifdef ENABLE_DEBUG_SCREEN
  newComboScreen(w, strings[lang].settings_DEBUG, screen_debug, &comboitem_system_debug);
  comboitem_system_debug->enabled = (systemSettings.settings.debugEnabled);
  #endif
  newComboScreen(w, strings[lang].settings_EDIT_TIPS, screen_tip_list, NULL);
  newComboScreen(w, strings[lang].settings_CALIBRATION, screen_calibration, NULL);
#ifdef ENABLE_ADDONS
  newComboScreen(w, strings[lang].settings_ADDONS, screen_addons, NULL);
#endif
  newComboScreen(w, strings[lang].settings_EXIT, screen_main, NULL);
}


static void SETTINGS_OnEnter(screen_t *scr) {
  if(scr==&Screen_main){
    comboResetIndex(Screen_settings.current_widget);
  }
  if(isCurrentProfileChanged()){         // If there's unsaved profile data
    saveSettingsFromMenu(save_Settings); // Save settings
  }
}


void settings_screen_setup(screen_t *scr) {
  scr->create = &SETTINGS_create;
  scr->onEnter = &SETTINGS_OnEnter;
  scr->processInput=&autoReturn_ProcessInput;
}
