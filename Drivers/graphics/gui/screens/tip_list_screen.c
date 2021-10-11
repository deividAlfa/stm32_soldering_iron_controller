/*
 * settings_tips_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */
#include "tip_list_screen.h"
#include "screen_common.h"

screen_t Screen_tip_list;

static comboBox_item_t *comboitem_tip_list_addNewTip;
static comboBox_item_t *comboitem_tip_list_back;

static int addNewTip(widget_t *w, RE_Rotation_t input){
  newTip=1;
  return screen_tip_settings;
}

static int editTip(widget_t *w, RE_Rotation_t input) {
  tipName=((comboBox_widget_t*)w->content)->currentItem->text;
  return screen_tip_settings;
}

static void tip_list_init(screen_t *scr) {
  default_init(scr);
  newTip=0;
  comboResetIndex(Screen_tip_list.widgets);
  comboBox_item_t *i = ((comboBox_widget_t*)Screen_tip_list.widgets->content)->first;
  for(int x = 0; x < TipSize; x++) {
    if(x < systemSettings.Profile.currentNumberOfTips) {
      i->text = systemSettings.Profile.tip[x].name;
      i->enabled = 1;
    }
    else
      i->enabled = 0;
    i = i->next_item;
  }
  comboitem_tip_list_addNewTip->enabled = (systemSettings.Profile.currentNumberOfTips < TipSize);
}

int tip_list_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){
  comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
  if(input==LongClick){
    if( item!=comboitem_tip_list_addNewTip && item!=comboitem_tip_list_back){                         // If long click over a tip
      for(uint8_t i=0; i<systemSettings.Profile.currentNumberOfTips; i++){                            // Find selected tip
        if(strcmp(item->text, systemSettings.Profile.tip[i].name) == 0){                              // If matching name
          setCurrentTip(i);                                                                           // Set tip
          systemSettings.Profile.currentTip = i;
          break;
        }
      }
      return screen_main;                                                                             // Exit to main screen
    }
  }
  return (autoReturn_ProcessInput(scr,input,state));
}
static void tip_list_create(screen_t *scr){
  widget_t* w;
  //  [ IRON TIPS COMBO ]
  //
  newWidget(&w,widget_combo,scr);

  for(int x = 0; x < TipSize; x++) {
    newComboAction(w, " ", &editTip, NULL);              // Names filled when entering the menu
  }
  newComboAction(w, strings[lang]._ADD_NEW, &addNewTip, &comboitem_tip_list_addNewTip);
  newComboScreen(w, strings[lang]._BACK, screen_settings, &comboitem_tip_list_back);
}


void tip_list_screen_setup(screen_t *scr){
  scr->init = &tip_list_init;
  scr->processInput=&tip_list_ProcessInput;
  scr->create=&tip_list_create;
}
