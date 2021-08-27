/*
 * settings_tip_settings_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */
#include "tip_settings_screen.h"
#include "screen_common.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "tip_settings_screen.c"
#endif

screen_t Screen_tip_settings;
static uint8_t return_screen;
static tipData_t tipCfg;

static comboBox_item_t *comboitem_tip_settings_save;
static comboBox_item_t *comboitem_tip_settings_copy;
static comboBox_item_t *comboitem_tip_settings_delete;
static comboBox_item_t *comboitem_tip_settings_cancel;
static editable_widget_t *editable_tip_settings_cal250;
static editable_widget_t *editable_tip_settings_cal350;
static editable_widget_t *editable_tip_settings_cal450;



//=========================================================
static void *getTipName() {
  return tipCfg.name;
}
static void setTipName(char *s) {
  strcpy(tipCfg.name, s);
}
//=========================================================
static void * getKp() {
  temp = tipCfg.PID.Kp;
  return &temp;
}
static void setKp(int32_t *val) {
  tipCfg.PID.Kp = *val;
}
//=========================================================
static void * getKi() {
  temp = tipCfg.PID.Ki;
  return &temp;
}
static void setKi(int32_t *val) {
  tipCfg.PID.Ki = *val;
}
//=========================================================
static void * getKd() {
  temp = tipCfg.PID.Kd;
  return &temp;
}
static void setKd(int32_t *val) {
  tipCfg.PID.Kd = *val;
}
//=========================================================
static void * getImax() {
  temp = tipCfg.PID.maxI;
  return &temp;
}
static void setImax(int32_t *val) {
  tipCfg.PID.maxI = *val;
}
//=========================================================
static void * getImin() {
  temp = tipCfg.PID.minI;
  return &temp;
}
static void setImin(int32_t *val) {
  tipCfg.PID.minI= *val;
}
//=========================================================
static void * getCal250() {
  temp = tipCfg.calADC_At_250;
  return &temp;
}
static void setCal250(int32_t *val) {
  tipCfg.calADC_At_250 = *val;
}
//=========================================================
static void * getCal350() {
  temp = tipCfg.calADC_At_350;
  editable_tip_settings_cal350->min_value = tipCfg.calADC_At_250 + 1;
  editable_tip_settings_cal350->max_value = tipCfg.calADC_At_450 - 1;
  return &temp;
}
static void setCal350(int32_t *val) {
  tipCfg.calADC_At_350 = *val;
}
//=========================================================
static void * getCal450() {
  temp = tipCfg.calADC_At_450;
  editable_tip_settings_cal450->min_value = tipCfg.calADC_At_350 + 1;
  return &temp;
}
static void setCal450(int32_t *val) {
  tipCfg.calADC_At_450 = *val;
}
//=========================================================
static int tip_save() {
  systemSettings.Profile.tip[Selected_Tip] = tipCfg;                                                            // Store tip data
  if(Selected_Tip==systemSettings.Profile.currentTip){                                                          // If current used tip, update PID
    __disable_irq();
    setCurrentTip(Selected_Tip);                                                                                // Reload tip settings
    __enable_irq();
  }
  else if(Selected_Tip==systemSettings.Profile.currentNumberOfTips){                                            // If new tip
    systemSettings.Profile.currentNumberOfTips++;                                                               // Increase number of tips in the system
  }
  return comboitem_tip_settings_cancel->action_screen;
}
//=========================================================
static int tip_delete() {
  char name[TipCharSize]=_BLANK_TIP;
  systemSettings.Profile.currentNumberOfTips--;                                                                 // Decrease the number of tips in the system

  if(Selected_Tip==systemSettings.Profile.currentTip){                                                          // If deleted tip was the current being used
    if(systemSettings.Profile.currentTip){                                                                      // If not zero
      systemSettings.Profile.currentTip--;                                                                      // Select previous tip
    }
    __disable_irq();
    setCurrentTip(systemSettings.Profile.currentTip);                                                           // Reload tip settings
    __enable_irq();
  }

  for(int x = Selected_Tip; x < TipSize-1;x++) {                                                                // Overwrite selected tip and move the rest one position backwards
    systemSettings.Profile.tip[x] = systemSettings.Profile.tip[x+1];
  }

  for(int x = systemSettings.Profile.currentNumberOfTips; x < TipSize;x++) {                                    // Fill the unused tips with blank names
    strcpy(systemSettings.Profile.tip[x].name, name);
  }
                                                                                                                // Skip tip settings (As tip is now deleted)
  return comboitem_tip_settings_cancel->action_screen;                                                          // And return to main screen or system menu screen
}
//=========================================================
static int tip_copy() {
  Selected_Tip = systemSettings.Profile.currentNumberOfTips;                                                    // Select first empty slot
  strcpy(tipCfg.name, _BLANK_TIP);                                                                              // Copy empty name
  comboitem_tip_settings_delete->enabled=0;                                                                     // Disable copying, deleting and saving(Until name is written)
  comboitem_tip_settings_copy->enabled=0;
  comboitem_tip_settings_save->enabled=0;
  comboResetIndex(Screen_tip_settings.widgets);                                                                 // Reset menu to 1st option
  disableTipCopy=1;
  return -1;                                                                                                    // And return to main screen or system menu screen
}
//=========================================================

static int tip_settings_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  static uint32_t last_update=0;
  bool update=0;

  refreshOledDim();
  handleOledDim();
  updatePlot();

  if(input==LongClick){
    int x = longClickReturn(scr->current_widget);
    if (x!=-1){
      return x;
    }
  }
  else if((current_time-screen_timer)>30000){                                                                   // 30s timeout if no activity is detected
    return screen_main;
  }

  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
  else if((current_time-last_update)>99){                                                                       // Update on user activity but also every 100mS
    last_update=current_time;
    update=1;
  }

  if(input==Rotate_Decrement_while_click){
   comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
    if(item->type==combo_Editable || item->type==combo_MultiOption){
      if(item->widget->selectable.state!=widget_edit){
        return return_screen;
      }
    }
    else{
      return return_screen;
    }
  }
  if(update){
    bool enable=1;
    if(strcmp(tipCfg.name, _BLANK_TIP) == 0){                                                                   // Check that the name is not empty
      enable=0;                                                                                                 // If empty, disable save button
    }
    else{
      for(uint8_t x = 0; x < TipSize; x++) {                                                                    // Compare tip names with current edit
        if( (strcmp(tipCfg.name, systemSettings.Profile.tip[x].name) == 0) && x!=Selected_Tip ){                // If match is found, and it's not the tip being edited
          enable=0;                                                                                             // Disable save button
          break;
        }
      }
    }
    comboitem_tip_settings_save->enabled=enable;
    if(disableTipCopy){
      comboitem_tip_settings_delete->enabled=0;
      comboitem_tip_settings_copy->enabled=0;
    }
    else{
      comboitem_tip_settings_copy->enabled=enable;
    }
  }
  return default_screenProcessInput(scr, input, state);
}


static void tip_settings_onEnter(screen_t *scr){
  bool new=0;
  disableTipCopy=0;
  comboResetIndex(Screen_tip_settings.widgets);

  if(scr==&Screen_tip_list){                                                                      // If coming from tips menu
    if(newTip) {                                                                                      // If was Add New tip option
      newTip=0;
      for(uint8_t x = 0; x < TipSize; x++) {                                                            // Find first valid tip and store the position
        if(strcmp(systemSettings.Profile.tip[x].name, _BLANK_TIP)!=0){
          Selected_Tip=x;
          new=1;
          break;
        }
      }
      if(!new){
        Error_Handler();                                                                                // This shouldn't happen
      }
    }
    else{
      for(uint8_t x = 0; x < TipSize; x++) {                                                            // Else, find the selected tip
        if(strcmp(tipName, systemSettings.Profile.tip[x].name)==0){
          Selected_Tip = x;
          break;
        }
      }
    }
    return_screen= screen_tip_list;
  }
  else{
    return_screen = screen_main;
    Selected_Tip = systemSettings.Profile.currentTip;
  }
  comboitem_tip_settings_cancel->action_screen = return_screen;

  tipCfg = systemSettings.Profile.tip[Selected_Tip];                                                      // Copy selected tip

  if(new){                                                                                                // If new tip selected
    strcpy(tipCfg.name, _BLANK_TIP);                                                                      // Set an empty name
    Selected_Tip=systemSettings.Profile.currentNumberOfTips;                                              // Selected tip is next position
    disableTipCopy=1;                                                                                     // Cannot copy a new tip
  }
  else{
    if(systemSettings.Profile.currentNumberOfTips>1){                                                     // If more than 1 tip in the system, enable delete
      comboitem_tip_settings_delete->enabled=1;
    }
    else{
      comboitem_tip_settings_delete->enabled=0;
    }
    if(systemSettings.Profile.currentNumberOfTips<TipSize){                                               // If not full
      comboitem_tip_settings_copy->enabled=1;                                                         // Existing tip, enable copy button;
    }
    else{
      disableTipCopy=1;
      comboitem_tip_settings_copy->enabled=0;                                                         // No space left
    }
  }
}

static void tip_settings_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;
  //########################################## [ TIP SETTINGS SCREEN ] ##########################################
  //

  //========[ PID COMBO ]===========================================================
  //
  newWidget(&w,widget_combo,scr);

  //[ TIP label]
  //
  newComboEditable(w, "Name", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=TipCharSize-1;
  dis->getData = &getTipName;
  dis->type = field_string;
  dis->displayString=tipCfg.name;
  edit->big_step = 1;
  edit->step = 1;
  edit->selectable.tab = 0;
  edit->setData = (void (*)(void *))&setTipName;


  //[ KP Widget]
  //
  newComboEditable(w, "PID Kp", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &getKp;
  dis->number_of_dec = 2;
  edit->max_value=65000;
  edit->big_step = 200;
  edit->step = 50;
  edit->setData =  (void (*)(void *))&setKp;


  //[ KI Widget ]
  //
  newComboEditable(w, "PID Ki", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &getKi;
  dis->number_of_dec = 2;
  edit->max_value=65000;
  edit->big_step = 200;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setKi;


  //[ KD Widget ]
  //
  newComboEditable(w, "PID Kd", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &getKd;
  dis->number_of_dec = 2;
  edit->max_value=65000;
  edit->big_step = 200;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setKd;


  //[ Imax Widget ]
  //
  newComboEditable(w, "PID Imax", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &getImax;
  dis->number_of_dec = 2;
  edit->max_value=1000;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setImax;

  //[ Imin Widget ]
  //
  newComboEditable(w, "PID Imin", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &getImin;
  dis->number_of_dec = 2;
  edit->max_value = 0;
  edit->min_value = -1000;
  edit->big_step = -5;
  edit->step = -1;
  edit->setData = (void (*)(void *))&setImin;

  //[ Cal250 Widget ]
  //
  newComboEditable(w, "Cal250", &edit, NULL);
  editable_tip_settings_cal250=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getCal250;
  edit->max_value = 4090;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal250;


  //[ Cal350 Widget ]
  //
  newComboEditable(w, "Cal350", &edit, NULL);
  editable_tip_settings_cal350=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getCal350;
  edit->max_value = 4090;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal350;


  //[ Cal450 Widget ]
  //
  newComboEditable(w, "Cal450", &edit, NULL);
  editable_tip_settings_cal450=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getCal450;
  edit->max_value = 4090;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal450;

  newComboAction(w, "SAVE", &tip_save, &comboitem_tip_settings_save);
  newComboAction(w, "COPY", &tip_copy, &comboitem_tip_settings_copy);
  newComboAction(w, "DELETE", &tip_delete, &comboitem_tip_settings_delete);
  newComboScreen(w, "CANCEL", -1, &comboitem_tip_settings_cancel);                                         // Return value set automatically on enter
}


void tip_settings_screen_setup(screen_t *scr){
  scr->onEnter = &tip_settings_onEnter;
  scr->create = &tip_settings_create;
  scr->processInput=&tip_settings_processInput;
}
