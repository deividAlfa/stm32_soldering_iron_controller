/*
 * reset_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#include "reset_screen.h"
#include "screen_common.h"

screen_t Screen_reset;
screen_t Screen_reset_confirmation;

typedef enum resStatus_t{ reset_settings, reset_profile, reset_profiles, reset_all }resStatus_t;
resStatus_t resStatus;


static int cancelReset(widget_t *w) {
  return screen_reset;
}
static int doReset(widget_t *w) {
  switch(resStatus){
    case reset_settings:
      saveSettingsFromMenu(reset_Settings);
      break;
    case reset_profile:
      saveSettingsFromMenu(reset_Profile);
      break;
    case reset_profiles:
      saveSettingsFromMenu(reset_Profiles);
      break;
    case reset_all:
      saveSettingsFromMenu(reset_All);
      break;
    default:
      return screen_main;
    }
    return -1;
}
static int doSettingsReset(void) {
  resStatus=reset_settings;
  return screen_reset_confirmation;
}
static int doProfileReset(void) {
  resStatus=reset_profile;
  return screen_reset_confirmation;
}
static int doProfilesReset(void) {
  resStatus=reset_profiles;
  return screen_reset_confirmation;
}
static int doFactoryReset(void) {
  resStatus=reset_all;
  return screen_reset_confirmation;
}


static void reset_onEnter(screen_t *scr){
  if(scr==&Screen_system){
    comboResetIndex(Screen_reset.widgets);
  }
}


static void reset_create(screen_t *scr){
  widget_t* w;
  comboBox_item_t *item;

  //  [ RESET OPTIONS COMBO ]
  //
  newWidget(&w,widget_combo, scr);
  newComboAction(w, "Reset Settings", &doSettingsReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, "Reset Profile", &doProfileReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, "Reset Profiles", &doProfilesReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, "Reset All", &doFactoryReset, &item);
  item->dispAlign=align_left;
  newComboScreen(w, "BACK", screen_system, NULL);
}


static void reset_confirmation_init(screen_t *scr){
  default_init(scr);
  //FillBuffer(BLACK, fill_dma);                              // Manually clear the screen
  //Screen_reset_confirmation.refresh=screen_Erased;          // Set to already cleared so it doesn't get erased automatically

  u8g2_SetFont(&u8g2,default_font);
  u8g2_SetDrawColor(&u8g2, WHITE);

  switch(resStatus){
  case 0:
    putStrAligned("RESET SYSTEM", 0, align_center);
    putStrAligned("SETTINGS?", 16, align_center);
    break;
  case 1:
    putStrAligned("RESET CURRENT",0 , align_center);
    putStrAligned("PROFILE?", 16, align_center);
    break;
  case 2:
    putStrAligned("RESET ALL",0 , align_center);
    putStrAligned("PROFILES?", 16, align_center);
    break;
  case 3:
    putStrAligned("PERFORM FULL", 0, align_center);
    putStrAligned("SYSTEM RESET?", 16, align_center);
    break;
  }
}


static void reset_confirmation_create(screen_t *scr){
  widget_t* w;

  //  [ Name Save Button Widget ]
  //
  newWidget(&w,widget_button,scr);
  ((button_widget_t*)w->content)->displayString="RESET";
  ((button_widget_t*)w->content)->selectable.tab = 1;
  ((button_widget_t*)w->content)->action = &doReset;
  w->posX = 0;
  w->posY = 48;
  w->width = 50;

  //  [ Name Back Button Widget ]
  //
  newWidget(&w,widget_button,&Screen_reset_confirmation);
  ((button_widget_t*)w->content)->displayString="CANCEL";
  ((button_widget_t*)w->content)->selectable.tab = 0;
  ((button_widget_t*)w->content)->action = &cancelReset;
  w->posX = 72;
  w->posY = 48;
  w->width = 56;
}


void reset_screen_setup(screen_t *scr){
  screen_t *sc;

  scr->onEnter = &reset_onEnter;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &reset_create;

  sc=&Screen_reset_confirmation;
  oled_addScreen(sc, screen_reset_confirmation);
  sc->init = &reset_confirmation_init;
  sc->processInput=&autoReturn_ProcessInput;
  sc->create = &reset_confirmation_create;
}
