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


static int addNewTip() {
  newTip=1;
  return screen_tip_settings;
}

static int editTip(widget_t *w) {
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


static void tip_list_create(screen_t *scr){
  widget_t* w;
  //  [ IRON TIPS COMBO ]
  //
  newWidget(&w,widget_combo,scr);
  for(int x = 0; x < TipSize; x++) {
    newComboAction(w, " ", &editTip, NULL);              // Names filled when entering the menu
  }
  newComboAction(w, "ADD NEW", &addNewTip, &comboitem_tip_list_addNewTip);
  newComboScreen(w, "BACK", screen_settings, NULL);
}


void tip_list_screen_setup(screen_t *scr){
  scr->init = &tip_list_init;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create=&tip_list_create;
}
