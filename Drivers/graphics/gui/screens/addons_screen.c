/*
 * addons_screen.c
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#include "addons_screen.h"
#include "gui_strings.h"
#include "screen_common.h"

#ifdef ENABLE_ADDONS

screen_t Screen_addons;
static comboBox_item_t *comboitem_addons_back;

static void addon_screen_create(screen_t *scr){
  widget_t* w;

  newWidget(&w,widget_combo,scr);

  //if(systemSettings.Profile.currentNumberOfTips < TipSize){
  //  newComboAction(w, strings[lang]._ADD_NEW, &addNewTip, &comboitem_tip_list_addNewTip);
  //}
  newComboScreen(w, strings[lang]._BACK, screen_settings, &comboitem_addons_back);
}

static void addon_screen_init(screen_t *scr) {
  default_init(scr);
  comboResetIndex(Screen_addons.current_widget);
}

void addons_screen_setup(screen_t *scr)
{
  scr->create       = &addon_screen_create;
  scr->init         = &addon_screen_init;
  scr->processInput = &autoReturn_ProcessInput;
}

#endif
