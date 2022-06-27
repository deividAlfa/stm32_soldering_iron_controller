/*
 * settings_system_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#include "system_screen.h"
#include "screen_common.h"

screen_t Screen_system;
screen_t Screen_system_ntc;

static comboBox_item_t *comboitem_system_ButtonWakeMode;
static comboBox_item_t *comboitem_system_ShakeWakeMode;
static comboBox_item_t *comboitem_system_BootMode;


static editable_widget_t *editable_system_TempStep;
static editable_widget_t *editable_system_bigTempStep;
static editable_widget_t *editable_system_GuiTempDenoise;

void update_System_menu(void){
  bool mode = (systemSettings.Profile.WakeInputMode==mode_shake);
  comboitem_system_BootMode->enabled        = mode;
  comboitem_system_ShakeWakeMode->enabled   = mode;
  comboitem_system_ButtonWakeMode->enabled  = mode;
}

void updateTemperatureUnit(void){
  if(getSystemTempUnit()==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
    editable_system_bigTempStep->inputData.endString="\260F";
    editable_system_GuiTempDenoise->inputData.endString="\260F";
    editable_system_GuiTempDenoise->max_value=70;
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
    editable_system_bigTempStep->inputData.endString="\260C";
    editable_system_GuiTempDenoise->inputData.endString="\260C";
    editable_system_GuiTempDenoise->max_value=20;
  }
}

//=========================================================
#ifdef ENABLE_DEBUG_SCREEN
static void * getDbgScr() {
  temp = systemSettings.settings.debugEnabled;
  return &temp;
}
static void setDbgScr(uint32_t *val) {
  systemSettings.settings.debugEnabled = *val;
}
#endif

int hwAction(widget_t *w, RE_Rotation_t input){
  static uint32_t last=0;
  if(input==Click){
    if((current_time-last)<500){
      screenSaver.enabled=1;
    }
    last=current_time;
  }
  return -1;
}

static void * getTmpUnit() {
  temp = getSystemTempUnit();
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  updateTemperatureUnit();
}
//=========================================================
static void * getTmpStep() {
  temp = systemSettings.settings.tempStep;
  return &temp;
}
static void setTmpStep(uint32_t *val) {
  systemSettings.settings.tempStep = *val;
}
//=========================================================

static void * getBigTmpStep() {
  temp = systemSettings.settings.tempBigStep;
  return &temp;
}

static void setBigTmpStep(uint32_t *val) {
  systemSettings.settings.tempBigStep = *val;
}
//=========================================================

static void * getGuiTempDenoise() {
  temp = systemSettings.settings.guiTempDenoise;
  return &temp;
}
static void setGuiTempDenoise(uint32_t *val) {
  systemSettings.settings.guiTempDenoise = *val;
}
//=========================================================
static void * getActiveDetection() {
  temp = systemSettings.settings.activeDetection;
  return &temp;
}
static void setActiveDetection(uint32_t *val) {
  systemSettings.settings.activeDetection = * val;
}
//=========================================================
static void * getEncoderMode() {
  temp = systemSettings.settings.EncoderMode;
  return &temp;
}
static void setEncoderMode(uint32_t *val) {
  systemSettings.settings.EncoderMode = * val;
  RE_SetMode(&RE1_Data, systemSettings.settings.EncoderMode);
}
//=========================================================
static void * getGuiUpd_ms() {
  temp = systemSettings.settings.guiUpdateDelay;
  return &temp;
}
static void setGuiUpd_ms(uint32_t *val) {
  systemSettings.settings.guiUpdateDelay = *val;
}
//=========================================================
static void * getLVP() {
  temp = systemSettings.settings.lvp;
  return &temp;
}
static void setLVP(uint32_t *val) {
  systemSettings.settings.lvp = *val;
}
//=========================================================
static void * getbuzzerMode() {
  temp = systemSettings.settings.buzzerMode;
  return &temp;
}
static void setbuzzerMode(uint32_t *val) {
  systemSettings.settings.buzzerMode = *val;
}
//=========================================================
static void * getInitMode() {
  temp = systemSettings.settings.initMode;
  return &temp;
}
static void setInitMode(uint32_t *val) {
  systemSettings.settings.initMode = *val;
}
//=========================================================
static void * getProfile() {
  temp = profile;
  return &temp;
}

static void setProfile(uint32_t *val) {
  profile=*val;
}
//=========================================================
static void * getLanguage() {
  temp = systemSettings.settings.language;
  return &temp;
}
static void setLanguage(uint32_t *val) {
  lang = *val;
  systemSettings.settings.language=*val;
}
//=========================================================
static void * getButtonWakeMode() {
  temp = systemSettings.settings.buttonWakeMode;
  return &temp;
}
static void setButtonWakeMode(uint32_t *val) {
  systemSettings.settings.buttonWakeMode = *val;
}
//=========================================================
static void * getShakeWakeMode() {
  temp = systemSettings.settings.shakeWakeMode;
  return &temp;
}
static void setShakeWakeMode(uint32_t *val) {
  systemSettings.settings.shakeWakeMode = *val;
}
//=========================================================
static void * getRememberLastProfile() {
  temp = systemSettings.settings.rememberLastProfile;
  return &temp;
}
static void setRememberLastProfile(uint32_t *val) {
  systemSettings.settings.rememberLastProfile = *val;
}
//=========================================================
static void * getRememberLastTip() {
  temp = systemSettings.settings.rememberLastTip;
  return &temp;
}
static void setRememberLastTip(uint32_t *val) {
  systemSettings.settings.rememberLastTip = *val;
}
//=========================================================
#ifdef HAS_BATTERY
static void * getRememberLastTemp() {
  temp = systemSettings.settings.rememberLastTemp;
  return &temp;
}
static void setRememberLastTemp(uint32_t *val) {
  systemSettings.settings.rememberLastTemp = *val;
}
#endif
//=========================================================
static void system_onEnter(screen_t *scr){
  if(scr==&Screen_settings){
    comboResetIndex(Screen_system.current_widget);
    profile=systemSettings.currentProfile;
  }
}

static void system_onExit(screen_t *scr){
  if(profile!=systemSettings.currentProfile){
    loadProfile(profile);
  }
}

static void system_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;
  comboBox_item_t* combo;

  current_lang = lang;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);

  //  [ Language Widget ]
  //
  newComboMultiOption(w, strings[lang]._Language, &edit, NULL);
  edit->inputData.getData = &getLanguage;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setLanguage;
  edit->options = Langs;
  edit->numberOfOptions = LANGUAGE_COUNT;

  //  [ Profile Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Profile, &edit, NULL);
  edit->inputData.getData = &getProfile;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setProfile;
  edit->options = profileStr;
  edit->numberOfOptions = NUM_PROFILES;

  //  [ Boot mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Boot, &edit, &comboitem_system_BootMode);
  dis=&edit->inputData;
  dis->getData = &getInitMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setInitMode;
  edit->options = strings[lang].InitMode;
  edit->numberOfOptions = 3;

  //  [ Encoder wake mode  Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Button_Wake, &edit,&comboitem_system_ButtonWakeMode);
  dis=&edit->inputData;
  dis->getData = &getButtonWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonWakeMode;
  edit->options = strings[lang].WakeModes;
  edit->numberOfOptions = 4;

  //  [ Shake wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Shake_Wake, &edit,&comboitem_system_ShakeWakeMode);
  dis=&edit->inputData;
  dis->getData = &getShakeWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeWakeMode;
  edit->options = strings[lang].WakeModes;
  edit->numberOfOptions = 4;

  //  [ Encoder inversion Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Encoder, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getEncoderMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setEncoderMode;
  edit->options = strings[lang].encMode;
  edit->numberOfOptions = 2;

  //  [ Buzzer Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Buzzer, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getbuzzerMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setbuzzerMode;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Temp display unit Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Temperature, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getTmpUnit;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTmpUnit;
  edit->options = tempUnit;
  edit->numberOfOptions = 2;

  //  [ Temp step Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM__Step, &edit, NULL);
  editable_system_TempStep=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getTmpStep;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTmpStep;
  edit->max_value = 50;
  edit->min_value = 1;

  // [ Temp big step Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM__Big_Step, &edit, NULL);
  editable_system_bigTempStep=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getBigTmpStep;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setBigTmpStep;
  edit->max_value = 50;
  edit->min_value = 1;

  // [ De-noise threshold Widget ]
  //
  newComboEditable(w, strings[lang].FILTER__Threshold, &edit, NULL);
  editable_system_GuiTempDenoise=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getGuiTempDenoise;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setGuiTempDenoise;
  edit->max_value = 50;
  edit->min_value = 0;

  //  [ Active detection Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Active_Detection,&edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getActiveDetection;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setActiveDetection;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Low voltage protection Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_LVP, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="V";
  dis->reservedChars=5;
  dis->getData = &getLVP;
  dis->number_of_dec = 1;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setLVP;
  edit->max_value = 250;
  edit->min_value = 90;

  //  [ Gui refresh rate Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Gui_Time, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="ms";
  dis->reservedChars=5;
  dis->getData = &getGuiUpd_ms;
  edit->big_step = 20;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setGuiUpd_ms;
  edit->max_value = 250;
  edit->min_value = 20;

  //  [ Remember text Widget ]
  //
  newComboScreen(w, strings[lang].SYSTEM_Remember, -1, &combo);
  combo->dispAlign = align_left;

  //  [ Remember last used profile Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_RememberLastProfile, &edit, NULL);
  edit->inputData.getData = &getRememberLastProfile;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setRememberLastProfile;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Remember last used tip Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_RememberLastTip, &edit, NULL);
  edit->inputData.getData = &getRememberLastTip;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setRememberLastTip;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

#ifdef HAS_BATTERY
  //  [ Remember last temp tip Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_RememberLastTemp, &edit, NULL);
  edit->inputData.getData = &getRememberLastTemp;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setRememberLastTemp;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif

#ifdef ENABLE_DEBUG_SCREEN
  //  [ Debug enable Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_DEBUG, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getDbgScr;
  dis->reservedChars=3;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setDbgScr;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif
  newComboScreen(w, strings[lang].SYSTEM_RESET_MENU, screen_reset, NULL);
  newComboScreen(w, strings[lang].SYSTEM_DISPLAY_MENU, screen_display, NULL);
  newComboScreen(w, SWSTRING, -1, NULL);
  newComboAction(w, HWSTRING, &hwAction, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);

  updateTemperatureUnit();
  update_System_menu();
}

int system_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){

  if(current_lang!=lang){                                                       // If language changed
    selectable_widget_t *sel = &((comboBox_widget_t*)scr->current_widget->content)->currentItem->widget->selectable;
    current_lang=lang;
    oled_backup_comboStatus(scr);
    oled_destroy_screen(scr);                                                   // Destroy and create the screen
    system_create(scr);
    oled_restore_comboStatus(scr);
    sel->state=widget_edit;
    sel->previous_state=widget_edit;
    scr->refresh = refresh_triggered;
  }
  return autoReturn_ProcessInput(scr, input, state);
}



void system_screen_setup(screen_t *scr){

  scr->onEnter = &system_onEnter;
  scr->onExit = &system_onExit;
  scr->processInput=&system_ProcessInput;
  scr->create = &system_create;
}
