/*
 * screen_common.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */
#include "screen_common.h"

uint32_t settingsTimer;
int32_t temp;
uint8_t profile, Selected_Tip;
char *tipName;
bool disableTipCopy;
bool newTip;
bool troll_enabled;

int longClickReturn(widget_t *w){
  selectable_widget_t *sel=NULL;
  if(w->type!=widget_combo){
    extractSelectablePartFromWidget(w);
  }
  else{
    comboBox_item_t *combo =  ((comboBox_widget_t*)w->content)->currentItem;
    if((combo->type == combo_Editable) || (combo->type == combo_MultiOption)){
      sel = &combo->widget->selectable;
    }
  }
  if(!sel || (sel && (sel->state!=widget_edit))){
      return screen_main;
  }
  return -1;
}

int autoReturn_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){
  if(input!=Rotate_Nothing){
    settingsTimer=HAL_GetTick();
  }
  if(input==LongClick){
    int x = longClickReturn(scr->current_widget);
    if (x!=-1){
      return x;
    }
  }

  if((HAL_GetTick()-settingsTimer)>15000){
    return screen_main;
  }
  return default_screenProcessInput(scr, input, state);
}
