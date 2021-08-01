/*
 * settings_system_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#include "system_screen.h"
#include "screen_common.h"

screen_t Screen_system;
static widget_t *comboWidget_SYSTEM;
static comboBox_item_t *comboitem_SYSTEM_ButtonWake;
static comboBox_item_t *comboitem_SYSTEM_ShakeWake;
static comboBox_item_t *comboitem_SYSTEM_InitMode;
static comboBox_item_t *comboitem_SYSTEM_StandMode;
static comboBox_item_t *comboitem_SYSTEM_BootMode;
static editable_widget_t *editable_SYSTEM_TempStep;



//=========================================================
static void * getTmpUnit() {
  temp = systemSettings.settings.tempUnit;
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_SYSTEM_TempStep->inputData.endString="\260F";
  }
  else{
    editable_SYSTEM_TempStep->inputData.endString="\260C";
  }
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
static void * getContrast_() {
  temp = systemSettings.settings.contrast;
  return &temp;
}
static void setContrast_(uint32_t *val) {
  systemSettings.settings.contrast=*val;
  setContrast(*val);
}
//=========================================================
static void * getOledOffset() {
  temp = systemSettings.settings.OledOffset;
  return &temp;
}
static void setOledOffset(uint32_t *val) {
  systemSettings.settings.OledOffset= * val;
}
//=========================================================
static void * getOledDimming() {
  temp = systemSettings.settings.screenDimming;
  return &temp;
}
static void setOledDimming(uint32_t *val) {
  systemSettings.settings.screenDimming = * val;
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
  comboitem_SYSTEM_ButtonWake->enabled = !systemSettings.settings.WakeInputMode;   // 0=shake, 1=stand
  comboitem_SYSTEM_ShakeWake->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_InitMode->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_StandMode->enabled = systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_BootMode->enabled  = !systemSettings.settings.WakeInputMode;
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
  RE_SetMode(&RE1_Data, systemSettings.settings.EncoderMode);
  return &temp;
}
static void setEncoderMode(uint32_t *val) {
  systemSettings.settings.EncoderMode = * val;
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

static void setButtonWake(uint32_t *val) {
  systemSettings.settings.wakeOnButton = *val;
}
static void * getButtonWake() {
  temp = systemSettings.settings.wakeOnButton;
  return &temp;
}

static void setShakeWake(uint32_t *val) {
  systemSettings.settings.wakeOnShake = *val;
}
static void * getShakeWake() {
  temp = systemSettings.settings.wakeOnShake;
  return &temp;
}


static void SYSTEM_init(screen_t *scr){
  default_init(scr);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_SYSTEM_TempStep->inputData.endString="\260F";
  }
  else{
    editable_SYSTEM_TempStep->inputData.endString="\260C";
  }
  profile=systemSettings.settings.currentProfile;
}

static void SYSTEM_onEnter(screen_t *scr){
  if(scr!=&Screen_reset){
    comboResetIndex(comboWidget_SYSTEM);
  }
}

static void SYSTEM_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);
  comboWidget_SYSTEM = w;

  //  [ Profile Widget ]
  //
  newComboMultiOption(w, "Profile", &edit, NULL);
  edit->inputData.getData = &getProfile;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setProfile;
  edit->max_value = ProfileSize-1;
  edit->min_value = 0;
  edit->options = profileStr;
  edit->numberOfOptions = ProfileSize;

  //  [ Contrast Widget ]
  //
  newComboEditable(w, "Contrast", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &getContrast_;
  edit->big_step = 25;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setContrast_;
  edit->max_value = 255;
  edit->min_value = 5;

  //  [ Oled dimming Widget ]
  //
  newComboMultiOption(w, "Auto dim", &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getOledDimming;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setOledDimming;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ Oled Offset Widget ]
  //

  newComboEditable(w, "Offset", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getOledOffset;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setOledOffset;
  edit->max_value = 15;
  edit->min_value = 0;

  //  [ Wake mode Widget ]
  //
  newComboMultiOption(w, "Wake Mode", &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setWakeMode;
  edit->max_value = wakeInputmode_stand;
  edit->min_value = wakeInputmode_shake;
  edit->options = wakeMode;
  edit->numberOfOptions = 2;


  //  [ Stand mode Widget ]
  //
  newComboMultiOption(w, "Stand mode", &edit, &comboitem_SYSTEM_StandMode);
  dis=&edit->inputData;
  dis->getData = &getStandMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandMode;
  edit->max_value = mode_standby;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
  edit->numberOfOptions = 2;

  //  [ Encoder wake Widget ]
  //
  newComboMultiOption(w, "Btn wake", &edit,&comboitem_SYSTEM_ButtonWake);
  dis=&edit->inputData;
  dis->getData = &getButtonWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
  edit->numberOfOptions = 2;

  //  [ Boot mode Widget ]
  //
  newComboMultiOption(w, "Boot", &edit, &comboitem_SYSTEM_BootMode);
  dis=&edit->inputData;
  dis->getData = &getInitMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setInitMode;
  edit->max_value = mode_run;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
  edit->numberOfOptions = 3;

  //  [ Shake wake Widget ]
  //
  newComboMultiOption(w, "Shake wake", &edit,&comboitem_SYSTEM_ShakeWake);
  dis=&edit->inputData;
  dis->getData = &getShakeWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ Encoder inversion Widget ]
  //
  newComboMultiOption(w, "Encoder", &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getEncoderMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setEncoderMode;
  edit->max_value = encoder_reverse;
  edit->min_value = encoder_normal;
  edit->options = encMode;
  edit->numberOfOptions = 2;

  //  [ Buzzer Widget ]
  //
  newComboMultiOption(w, "Buzzer", &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getbuzzerMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setbuzzerMode;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ Temp display unit Widget ]
  //
  newComboMultiOption(w, "Unit", &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getTmpUnit;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTmpUnit;
  edit->max_value = mode_Farenheit;
  edit->min_value = mode_Celsius;
  edit->options = tempUnit;
  edit->numberOfOptions = 2;


  //  [ Temp step Widget ]
  //
  newComboEditable(w, "Step", &edit, NULL);
  editable_SYSTEM_TempStep=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getTmpStep;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTmpStep;
  edit->max_value = 50;
  edit->min_value = 1;

  //  [ Active detection Widget ]
  //
  newComboMultiOption(w, "Active det.",&edit,&comboitem_SYSTEM_InitMode);
  dis=&edit->inputData;
  dis->getData = &getActiveDetection;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setActiveDetection;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ Low voltage protection Widget ]
  //
  newComboEditable(w, "LVP", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="V";
  dis->reservedChars=5;
  dis->getData = &getLVP;
  dis->number_of_dec = 1;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setLVP;
  edit->max_value = 240;
  edit->min_value = 90;

  //  [ Gui refresh rate Widget ]
  //
  newComboEditable(w, "Gui time", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="mS";
  dis->reservedChars=5;
  dis->getData = &getGuiUpd_ms;
  edit->big_step = 50;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setGuiUpd_ms;
  edit->max_value = 500;
  edit->min_value = 20;

  newComboScreen(w, "RESET MENU", screen_reset, NULL);
  newComboScreen(w, SWSTRING, -1, NULL);
  newComboScreen(w, HWSTRING, -1, NULL);
  newComboScreen(w, "BACK", screen_settings, NULL);
}

void system_screen_setup(screen_t *scr){
  scr->init = &SYSTEM_init;
  scr->onEnter = &SYSTEM_onEnter;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &SYSTEM_create;
}
