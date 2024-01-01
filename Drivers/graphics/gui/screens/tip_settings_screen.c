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
static widget_t        *widget_tip_settings;
static comboBox_item_t *comboitem_tip_settings_save;
static comboBox_item_t *comboitem_tip_settings_copy;
static comboBox_item_t *comboitem_tip_settings_new;
static comboBox_item_t *comboitem_tip_settings_delete;
static editable_widget_t *editable_tip_settings_cal250;
static editable_widget_t *editable_tip_settings_cal400;
static uint8_t tipUpdateMode;

//=========================================================
static void Flash_to_Edit(char* s){                             // [TIPNAME\0                ]
  uint8_t i = 0;
  while(i<TIP_LEN && s[i]){ i++; }                              // Find end of string
  while(i<TIP_LEN){ s[i++]=' '; }                               // Fill with spaces
  s[TIP_LEN] = 0;                                               // Terminate string
}                                                               // [TIPNAME                \0]
//=========================================================
static void Edit_to_Flash(char* s){                             // [TIPNAME                \0]
  uint8_t end=0;
  for(uint8_t i=0; i<TIP_LEN; i++){                             // Clear trailing spaces
    if(s[i]==' '){                                              // Find space, store index
      if(!end){end=i;}
    }
    else{end=0;}                                                // Clear index if other char found
  }
  if(end){s[end]=0;}                                            // Terminate string
}                                                               // [TIPNAME\0                ]
//=========================================================
static void *getTipName() {
  return backupTip.name;
}
static void setTipName(char *s) {
  strcpy(backupTip.name, s);
}
//=========================================================
static void * getKp() {
  temp = backupTip.PID.Kp;
  return &temp;
}
static void setKp(int32_t *val) {
  backupTip.PID.Kp = *val;
}
//=========================================================
static void * getKi() {
  temp = backupTip.PID.Ki;
  return &temp;
}
static void setKi(int32_t *val) {
  backupTip.PID.Ki = *val;
}
//=========================================================
static void * getKd() {
  temp = backupTip.PID.Kd;
  return &temp;
}
static void setKd(int32_t *val) {
  backupTip.PID.Kd = *val;
}
//=========================================================
static void * getImax() {
  temp = backupTip.PID.maxI;
  return &temp;
}
static void setImax(int32_t *val) {
  backupTip.PID.maxI = *val;
}
//=========================================================
static void * getImin() {
  temp = backupTip.PID.minI;
  return &temp;
}
static void setImin(int32_t *val) {
  backupTip.PID.minI= *val;
}
//=========================================================
static void * getCal250() {
  temp = backupTip.calADC_At_250;
  return &temp;
}
static void setCal250(int32_t *val) {
  if(*val>=backupTip.calADC_At_400){
    *val = backupTip.calADC_At_400-1;
  }
  backupTip.calADC_At_250 = *val;
}
//=========================================================
static void * getCal400() {
  temp = backupTip.calADC_At_400;
  return &temp;
}
static void setCal400(int32_t *val) {
  if(*val<=backupTip.calADC_At_250){
    *val = backupTip.calADC_At_250+1;
  }
  backupTip.calADC_At_400 = *val;
}
//=========================================================
static int tip_save(widget_t *w, RE_Rotation_t input) {
  Edit_to_Flash(backupTip.name);

// TODO: Last time we got Hard Fault here when TIP_LEN >9! 

  if(Selected_Tip==systemSettings.Profile.currentNumberOfTips){                                                 // If new tip
    tipUpdateMode=mode_AddTip;                                                                                  // Set flag for saveSettings
  }
  else{
    if(memcmp(&backupTip, getCurrentTipData(), sizeof(tipData_t)))
      tipUpdateMode=mode_SaveTip;                                                                             // Set save flag if different
  }
  return last_scr;
}
//=========================================================
static int tip_delete(widget_t *w, RE_Rotation_t input) {
  tipUpdateMode=mode_DeleteTip;                                                                                 // Set flag for saveSettings
  return last_scr;                                                                                              // Return to main screen or system menu screen
}
//=========================================================
static int tip_copy(widget_t *w, RE_Rotation_t input) {
  Selected_Tip = systemSettings.Profile.currentNumberOfTips;                                                    // Select next available slot
  if(newTip){                                                                                                   // If new, load default name, otherwise keep existigg
    newTip=0;                                                                                                   // This is sort a hack, can only happen from tip_new function, otherwise copyign is always disabled for new tips
    backupTip = *getFlashTipData(0);
    Flash_to_Edit(backupTip.name);
  }

  comboitem_tip_settings_save->enabled=0;                                                                       // Disable save, will be enabled when the name is modified (If not matching any other tip)
  comboitem_tip_settings_new->enabled=0;                                                                        // Cannot copy a new tip
  comboitem_tip_settings_copy->enabled=0;                                                                       // Cannot copy a new tip
  comboitem_tip_settings_delete->enabled=0;                                                                     // Cannot delete a new tip
  comboResetIndex(Screen_tip_settings.current_widget);                                                          // Reset menu to 1st option
  return -1;                                                                                                    //
}
//=========================================================
static int tip_new(widget_t *w, RE_Rotation_t input) {                                                          // Create new
  newTip=1;
  tip_copy(w, input);
  return -1;
}
//=========================================================
static int tip_reset(widget_t *w, RE_Rotation_t input) {                                                        // Reset to default, but keep name
  char tipName[TIP_LEN+1];
  strcpy(tipName, backupTip.name);
  backupTip = defaultTipData[getCurrentProfile()];
  strcpy(backupTip.name,tipName);
  return -1;                                                                                              // Return to main screen or system menu screen
}
//=========================================================

static int tip_settings_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  int ret;

  wakeOledDim();
  handleOledDim();
  updatePlot();
  updateScreenTimer(input);

  if(input==LongClick){
    int x = longClickReturn(scr->current_widget);
    if (x!=-1){
      return x;
    }
  }
  else if(checkScreenTimer(30000)){                                                                   // 30s timeout if no activity is detected
    return screen_main;
  }

  if(input==Rotate_Decrement_while_click){
   comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
    if( item->type!=combo_Editable || (item->type==combo_Editable && item->widget->selectable.state!=widget_edit)){
      return last_scr;
    }
  }

  ret  = default_screenProcessInput(scr, input, state);                                                           // Process screen

  if(widget_tip_settings->refresh > refresh_idle){                                                                // If something changed, check conditions
    bool enable=1;
    if(strcmp(backupTip.name, _BLANK_TIP) == 0){                                                                  // Check that the name is not empty
      enable=0;                                                                                                   // If empty, disable save button
    }
    else{
      for(uint8_t i = 0; i < systemSettings.Profile.currentNumberOfTips; i++) {                                   // Seek through all tips
        char flashTip[TIP_LEN];                                                                                   // Flash tip is null-terminated, but editor field is tabbed with spaces
        strcpy(flashTip, flashProfilesSettings.Profile[getCurrentProfile()].tip[i].name);               // Copy tip name from flash
        Flash_to_Edit(flashTip);                                                                                  // Convert to editor format (Fill with spaces)
        if((i!=Selected_Tip) && strcmp(backupTip.name, flashTip) == 0){  // Compare names with other tips                              // Compare
          enable=0;                                                                                             // If match is found with a different tip, disable save button
          break;
        }
      }
    }
    comboitem_tip_settings_save->enabled=enable;
  }

  return ret;
}

static void tip_settings_onExit(screen_t *scr){
  if(tipUpdateMode){                                                                                          // Pending tip change
    setCurrentTipData(&backupTip);                                                                                   // Store tip
    saveSettings(save_Tip, tipUpdateMode, Selected_Tip, no_reboot);                                           // Save now we have all heap free
  }
}
static void tip_settings_onEnter(screen_t *scr){
  tipUpdateMode = no_mode;
  comboResetIndex(Screen_tip_settings.current_widget);
  if(newTip){                                                                                                   // If new tip selected
    newTip=0;
    backupTip = *getFlashTipData(0);                            // Copy data from first tip in the system
    strcpy(backupTip.name, defaultTipData[getCurrentProfile()].name);                                                                         // Set an empty name
    Flash_to_Edit(backupTip.name);
    Selected_Tip=systemSettings.Profile.currentNumberOfTips;                                                    // Selected tip is next position
    comboitem_tip_settings_copy->enabled=0;                                                                     // Cannot copy a new tip
    comboitem_tip_settings_new->enabled=0;                                                                       // Cannot copy a new tip
    comboitem_tip_settings_delete->enabled=0;                                                                   // Cannot delete a new tip
  }
  else{
    if(scr != &Screen_tip_list)                                                                                 // If coming from tip list screen, Selected_Tip will be set already
      Selected_Tip = getCurrentTip();                                                                 // Otherwise use current tip

    backupTip = *getFlashTipData(Selected_Tip);

    comboitem_tip_settings_delete->enabled = (systemSettings.Profile.currentNumberOfTips>1);                    // If more than 1 tip in the system, enable delete
    comboitem_tip_settings_copy->enabled = (systemSettings.Profile.currentNumberOfTips<NUM_TIPS);               // If tip slots available, enable copy button
    comboitem_tip_settings_new->enabled = comboitem_tip_settings_copy->enabled;
    Flash_to_Edit(backupTip.name);
  }
}

static void tip_settings_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ TIP settings combo ]
  //
  newWidget(&w,widget_combo,scr,NULL);
  widget_tip_settings = w;

  //[ TIP label]
  //
  newComboEditableString(w, strings[lang].TIP_SETTINGS_Name, &edit, NULL, backupTip.name);
  dis=&edit->inputData;
  dis->reservedChars=TIP_LEN;
  dis->getData = &getTipName;
  dis->type = field_string;
  edit->big_step = 1;
  edit->step = 1;
  edit->selectable.tab = 0;
  edit->setData = (void (*)(void *))&setTipName;


  //[ KP Widget]
  //
  newComboEditable(w, strings[lang].TIP_SETTINGS_PID_kp, &edit, NULL);
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
  newComboEditable(w, strings[lang].TIP_SETTINGS_PID_ki, &edit, NULL);
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
  newComboEditable(w, strings[lang].TIP_SETTINGS_PID_kd, &edit, NULL);
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
  newComboEditable(w, strings[lang].TIP_SETTINGS_PID_Imax, &edit, NULL);
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
  newComboEditable(w, strings[lang].TIP_SETTINGS_PID_Imin, &edit, NULL);
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
  newComboEditable(w, strings[lang]._Cal_250, &edit, NULL);
  editable_tip_settings_cal250=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getCal250;
  edit->max_value = 4090;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal250;

  //[ Cal400 Widget ]
  //
  newComboEditable(w, strings[lang]._Cal_400, &edit, NULL);
  editable_tip_settings_cal400=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getCal400;
  edit->max_value = 4090;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal400;

  newComboAction(w, strings[lang]._RESET, &tip_reset, NULL);
  newComboAction(w, strings[lang]._ADD_NEW, &tip_new, &comboitem_tip_settings_new);
  newComboAction(w, strings[lang].TIP_SETTINGS_COPY, &tip_copy, &comboitem_tip_settings_copy);
  newComboAction(w, strings[lang].TIP_SETTINGS_DELETE, &tip_delete, &comboitem_tip_settings_delete);
  newComboAction(w, strings[lang]._SAVE, &tip_save, &comboitem_tip_settings_save);
  newComboScreen(w, strings[lang]._CANCEL, last_scr, NULL);
}


void tip_settings_screen_setup(screen_t *scr){
  scr->onEnter = &tip_settings_onEnter;
  scr->onExit = &tip_settings_onExit;
  scr->create = &tip_settings_create;
  scr->processInput=&tip_settings_processInput;
}
