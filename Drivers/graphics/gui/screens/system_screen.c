/*
 * settings_system_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#include "system_screen.h"
#include "screen_common.h"
#include "tempsensors.h"

screen_t Screen_system;
screen_t Screen_system_ntc;


static comboBox_item_t *comboitem_system_Dim_Timeout;
static comboBox_item_t *comboitem_system_Dim_PowerOff;

static comboBox_item_t *comboitem_system_ButtonWakeMode;
static comboBox_item_t *comboitem_system_ShakeWakeMode;
static comboBox_item_t *comboitem_system_ShakeFiltering;
static comboBox_item_t *comboitem_system_StandMode;
static comboBox_item_t *comboitem_system_BootMode;


static editable_widget_t *editable_system_TempStep;
static editable_widget_t *editable_system_bigTempStep;
static editable_widget_t *editable_system_GuiTempDenoise;

void update_System_menu(void){
  bool mode = (systemSettings.settings.dim_mode>dim_off);
  comboitem_system_Dim_PowerOff->enabled = mode;
  comboitem_system_Dim_Timeout->enabled = mode;

  mode = (systemSettings.settings.WakeInputMode==mode_shake);
  comboitem_system_StandMode->enabled       = !mode;
  comboitem_system_ShakeFiltering->enabled  = mode;
  comboitem_system_BootMode->enabled        = mode;
  comboitem_system_ShakeWakeMode->enabled   = mode;
  comboitem_system_ButtonWakeMode->enabled  = mode;
}

void updateTemperatureUnit(void){
  if(systemSettings.settings.tempUnit==mode_Farenheit){
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
  temp = systemSettings.settings.tempUnit;
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
static void * getContrast_() {
  temp = systemSettings.settings.contrast/25;
  return &temp;
}
static void setContrast_(uint32_t *val) {
  if(*val==0){
    systemSettings.settings.contrast=5;
  }
  else if(*val==10){
    systemSettings.settings.contrast=255;
  }
  else{
    systemSettings.settings.contrast=*val*25;
  }
  setContrast(systemSettings.settings.contrast);
}
//=========================================================
static void * getOledOffset() {
  temp = systemSettings.settings.OledOffset;
  return &temp;
}
static void setOledOffset(uint32_t *val) {
  systemSettings.settings.OledOffset= *val;
}
//=========================================================
static void * getdimMode() {
  temp = systemSettings.settings.dim_mode;
  return &temp;
}
static void setdimMode(uint32_t *val) {
  systemSettings.settings.dim_mode = * val;
  update_System_menu();
}
//=========================================================
static void * getDimTimeout() {
  temp = systemSettings.settings.dim_Timeout/1000;
  return &temp;
}
static void setDimTimeout(uint32_t *val) {
  systemSettings.settings.dim_Timeout = *val*1000;
}
//=========================================================
static void * getDimTurnOff() {
  temp = systemSettings.settings.dim_inSleep;
  return &temp;
}
static void setDimTurnOff(uint32_t *val) {
  systemSettings.settings.dim_inSleep = *val;
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
static void * getWakeMode() {
  temp = systemSettings.settings.WakeInputMode;
  update_System_menu();
  return &temp;
}
static void setWakeMode(uint32_t *val) {
  systemSettings.settings.WakeInputMode = *val;
}
//=========================================================
static void * getStandMode() {
  temp = systemSettings.settings.StandMode;
  return &temp;
}
static void setStandMode(uint32_t *val) {
  systemSettings.settings.StandMode = *val;
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
static void * getShakeFiltering() {
  temp = systemSettings.settings.shakeFiltering;
  return &temp;
}
static void setShakeFiltering(uint32_t *val) {
  systemSettings.settings.shakeFiltering = *val;
}
//=========================================================
static void system_onEnter(screen_t *scr){
  if(scr==&Screen_settings){
    comboResetIndex(Screen_system.current_widget);
    profile=systemSettings.settings.currentProfile;
  }
}

static void system_onExit(screen_t *scr){
  if(profile!=systemSettings.settings.currentProfile){
    loadProfile(profile);
  }
}

static void system_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

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
  edit->numberOfOptions = ProfileSize;

  //  [ Contrast Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Oled_Contrast, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &getContrast_;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setContrast_;
  edit->max_value = 10;
  edit->min_value = 0;

  //  [ Oled Offset Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Oled_Offset, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getOledOffset;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setOledOffset;
  edit->max_value = 15;
  edit->min_value = 0;

  //  [ Oled dimming Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Oled_Dim, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdimMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdimMode;
  edit->options = strings[lang].dimMode;
  edit->numberOfOptions = 3;

  //  [ Oled dim delay Widget ]
  //
  newComboEditable(w, strings[lang].__Delay, &edit, &comboitem_system_Dim_Timeout);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->endString="s";
  dis->getData = &getDimTimeout;
  edit->big_step = 10;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setDimTimeout;
  edit->max_value = 600;
  edit->min_value = 5;

  //  [ Oled dim turn off Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Oled_Dim_inSleep, &edit, &comboitem_system_Dim_PowerOff);
  dis=&edit->inputData;
  dis->getData = &getDimTurnOff;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setDimTurnOff;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Wake_Mode, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setWakeMode;
  edit->options = strings[lang].wakeMode;
  edit->numberOfOptions = 2;

  //  [ Shake filtering Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Shake_Filtering, &edit, &comboitem_system_ShakeFiltering);
  dis=&edit->inputData;
  dis->getData = &getShakeFiltering;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeFiltering;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Stand mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Stand_Mode, &edit, &comboitem_system_StandMode);
  dis=&edit->inputData;
  dis->getData = &getStandMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandMode;
  edit->options = strings[lang].InitMode;
  edit->numberOfOptions = 2;

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
