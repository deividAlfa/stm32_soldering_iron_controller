/*
 * settings_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "screen_common.h"

screen_t Screen_settings;
static char * StringProfile;
#ifdef ENABLE_DEBUG_SCREEN
static comboBox_item_t *comboitem_system_debug;
#endif

static void SETTINGS_create(screen_t *scr) {
  widget_t* w;
  StringProfile = malloc(16);
  if(StringProfile == NULL){
    Error_Handler();
  }

  strcpy(StringProfile, strings[lang].settings_IRON);
  strcat(StringProfile, " (");
  strcat(StringProfile, profileStr[getCurrentProfile()]);
  strcat(StringProfile, ")");

  //  [ SETTINGS MAIN SCREEN ]
  //
  newWidget(&w,widget_combo,scr, NULL);

  newComboScreen(w, StringProfile, screen_iron, NULL);
  newComboScreen(w, strings[lang].settings_SYSTEM, screen_system, NULL);
  #ifdef ENABLE_DEBUG_SCREEN
  newComboScreen(w, strings[lang].settings_DEBUG, screen_debug, &comboitem_system_debug);
  comboitem_system_debug->enabled = (getSystemSettings()->debugEnabled);
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
}

static void SETTINGS_OnExit(screen_t *scr) {
  free(StringProfile);
}

void settings_screen_setup(screen_t *scr) {
  scr->create = &SETTINGS_create;
  scr->onEnter = &SETTINGS_OnEnter;
  scr->onExit= &SETTINGS_OnExit;
  scr->processInput=&autoReturn_ProcessInput;
}
