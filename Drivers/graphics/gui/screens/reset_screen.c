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
  return last_scr;
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
static int doSettingsReset(widget_t *w, RE_Rotation_t input) {
  resStatus=reset_settings;
  return screen_reset_confirmation;
}
static int doProfileReset(widget_t *w, RE_Rotation_t input) {
  resStatus=reset_profile;
  return screen_reset_confirmation;
}
static int doProfilesReset(widget_t *w, RE_Rotation_t input) {
  resStatus=reset_profiles;
  return screen_reset_confirmation;
}
static int doFactoryReset(widget_t *w, RE_Rotation_t input) {
  resStatus=reset_all;
  return screen_reset_confirmation;
}


static void reset_onEnter(screen_t *scr){
  if(scr==&Screen_system){
    comboResetIndex(Screen_reset.current_widget);
  }
}


static void reset_create(screen_t *scr){
  widget_t* w;
  comboBox_item_t *item;

  //  [ RESET OPTIONS COMBO ]
  //
  newWidget(&w,widget_combo, scr);

  newComboAction(w, strings[lang].RESET_Reset_Settings, &doSettingsReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, strings[lang].RESET_Reset_Profile, &doProfileReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, strings[lang].RESET_Reset_Profiles, &doProfilesReset, &item);
  item->dispAlign=align_left;
  newComboAction(w, strings[lang].RESET_Reset_All, &doFactoryReset, &item);
  item->dispAlign=align_left;
  newComboScreen(w, strings[lang]._BACK, screen_system, NULL);
}


static void reset_confirmation_init(screen_t *scr){
  default_init(scr);
  //fillBuffer(BLACK, fill_dma);                              // Manually clear the screen
  //Screen_reset_confirmation.refresh=screen_Erased;          // Set to already cleared so it doesn't get erased automatically

  u8g2_SetFont(&u8g2,u8g2_font_menu);
  u8g2_SetDrawColor(&u8g2, WHITE);

  switch(resStatus){
  case 0:
    putStrAligned(strings[lang].RESET_Reset_msg_settings_1, 0, align_center);
    putStrAligned(strings[lang].RESET_Reset_msg_settings_2, 16, align_center);
    break;
  case 1:
    putStrAligned(strings[lang].RESET_Reset_msg_profile_1, 0 , align_center);
    putStrAligned(strings[lang].RESET_Reset_msg_profile_2, 16, align_center);
    break;
  case 2:
    putStrAligned(strings[lang].RESET_Reset_msg_profiles_1,0 , align_center);
    putStrAligned(strings[lang].RESET_Reset_msg_profiles_2, 16, align_center);
    break;
  case 3:
    putStrAligned(strings[lang].RESET_Reset_msg_all_1, 0, align_center);
    putStrAligned(strings[lang].RESET_Reset_msg_all_2, 16, align_center);
    break;
  }
}


static void reset_confirmation_create(screen_t *scr){
  widget_t* w;

  //  [ RESET Button Widget ]
  //
  newWidget(&w,widget_button,scr);
  ((button_widget_t*)w->content)->displayString=strings[lang]._RESET;
  ((button_widget_t*)w->content)->selectable.tab = 1;
  ((button_widget_t*)w->content)->action = &doReset;
  ((button_widget_t*)w->content)->font= u8g2_font_menu;
  w->posX = 0;
  w->posY = 48;
  w->width = 60;

  //  [ CANCEL Button Widget ]
  //
  newWidget(&w,widget_button,&Screen_reset_confirmation);
  ((button_widget_t*)w->content)->displayString=strings[lang]._CANCEL;
  ((button_widget_t*)w->content)->selectable.tab = 0;
  ((button_widget_t*)w->content)->action = &cancelReset;
  ((button_widget_t*)w->content)->font= u8g2_font_menu;
  w->posY = 48;
  w->width = 65;
  w->posX = displayWidth - w->width -1;
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
