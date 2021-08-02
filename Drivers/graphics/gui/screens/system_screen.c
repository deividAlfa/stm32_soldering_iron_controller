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

static comboBox_item_t *comboitem_system_ButtonSleepWake;
static comboBox_item_t *comboitem_system_ButtonStandbyWake;
static comboBox_item_t *comboitem_system_ShakeSleepWake;
static comboBox_item_t *comboitem_system_ShakeStandbyWake;
static comboBox_item_t *comboitem_system_InitMode;
static comboBox_item_t *comboitem_system_StandMode;
static comboBox_item_t *comboitem_system_BootMode;
static editable_widget_t *editable_system_TempStep;

uint8_t backup_Pullup;
uint16_t backup_NTC_Beta;
uint32_t backup_Pull_res, backup_NTC_res;


//=========================================================
static void * getTmpUnit() {
  temp = systemSettings.settings.tempUnit;
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
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
int OledDimming_ProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  if(input==LongClick){
    screenSaver.enabled=1;
  }
  return default_widgetProcessInput(w, input, state);
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
  comboitem_system_ButtonSleepWake->enabled = !systemSettings.settings.WakeInputMode;   // 0=shake, 1=stand
  comboitem_system_ButtonStandbyWake->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_system_ShakeSleepWake->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_system_ShakeStandbyWake->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_system_InitMode->enabled = !systemSettings.settings.WakeInputMode;
  comboitem_system_StandMode->enabled = systemSettings.settings.WakeInputMode;
  comboitem_system_BootMode->enabled  = !systemSettings.settings.WakeInputMode;
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
static void setButtonSleepWake(uint32_t *val) {
  systemSettings.settings.wakeSlpButton = *val;
}
static void * getButtonSleepWake() {
  temp = systemSettings.settings.wakeSlpButton;
  return &temp;
}
//=========================================================
static void setButtonStandbyWake(uint32_t *val) {
  systemSettings.settings.wakeStbyButton = *val;
}
static void * getButtonStandbyWake() {
  temp = systemSettings.settings.wakeStbyButton;
  return &temp;
}
//=========================================================
static void setShakeSleepWake(uint32_t *val) {
  systemSettings.settings.wakeSlpShake = *val;
}
static void * getShakeSleepWake() {
  temp = systemSettings.settings.wakeSlpShake;
  return &temp;
}
//=========================================================
static void setShakeStandbyWake(uint32_t *val) {
  systemSettings.settings.wakeStbyShake = *val;
}
static void * getShakeStandbyWake() {
  temp = systemSettings.settings.wakeStbyShake;
  return &temp;
}
//=========================================================
static void set_NTC_beta(uint32_t *val) {
  backup_NTC_Beta = *val;
}
static void * get_NTC_beta() {
  temp = backup_NTC_Beta;
  return &temp;
}
//=========================================================
static void set_NTC_res(uint32_t *val) {
  backup_NTC_res = *val*100;
}
static void * get_NTC_res() {
  temp = backup_NTC_res/100;
  return &temp;
}
//=========================================================
static void set_Pull_res(uint32_t *val) {
  backup_Pull_res = *val*100;
}
static void * get_Pull_res() {
  temp = backup_Pull_res/100;
  return &temp;
}
//=========================================================
static void set_Pull_mode(uint32_t *val) {
  backup_Pullup = *val;
}
static void * get_Pull_mode() {
  temp = backup_Pullup;
  return &temp;
}
//=========================================================
static int saveNTC() {
  __disable_irq();
  systemSettings.settings.Pullup=backup_Pullup;
  systemSettings.settings.Pull_res=backup_Pull_res;
  systemSettings.settings.NTC_res=backup_NTC_res;
  systemSettings.settings.NTC_Beta=backup_NTC_Beta;
  __enable_irq();
  return screen_system;
}
//=========================================================


static void system_init(screen_t *scr){
  default_init(scr);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
  }
  profile=systemSettings.settings.currentProfile;
}

static void system_onEnter(screen_t *scr){
  if(scr==&Screen_settings){
    comboResetIndex(Screen_system.widgets);
  }
}

static void system_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);

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
  edit->selectable.processInput=&OledDimming_ProcessInput;

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
  newComboMultiOption(w, "Stand mode", &edit, &comboitem_system_StandMode);
  dis=&edit->inputData;
  dis->getData = &getStandMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandMode;
  edit->max_value = mode_standby;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
  edit->numberOfOptions = 2;

  //  [ Boot mode Widget ]
  //
  newComboMultiOption(w, "Boot", &edit, &comboitem_system_BootMode);
  dis=&edit->inputData;
  dis->getData = &getInitMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setInitMode;
  edit->max_value = mode_run;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
  edit->numberOfOptions = 3;

  //  [ Encoder wake from sleep  Widget ]
  //
  newComboMultiOption(w, "Btn Slp", &edit,&comboitem_system_ButtonSleepWake);
  dis=&edit->inputData;
  dis->getData = &getButtonSleepWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonSleepWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
  edit->numberOfOptions = 2;

  //  [ Encoder wake from standby Widget ]
  //
  newComboMultiOption(w, "Btn Stby", &edit,&comboitem_system_ButtonStandbyWake);
  dis=&edit->inputData;
  dis->getData = &getButtonStandbyWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonStandbyWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
  edit->numberOfOptions = 2;

  //  [ Shake wake from sleep  Widget ]
  //
  newComboMultiOption(w, "Shake Slp", &edit,&comboitem_system_ShakeSleepWake);
  dis=&edit->inputData;
  dis->getData = &getShakeSleepWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeSleepWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
  edit->numberOfOptions = 2;

  //  [ Shake wake from standby Widget ]
  //
  newComboMultiOption(w, "Shake Stby", &edit,&comboitem_system_ShakeStandbyWake);
  dis=&edit->inputData;
  dis->getData = &getShakeStandbyWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeStandbyWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
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
  editable_system_TempStep=edit;
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
  newComboMultiOption(w, "Active det.",&edit,&comboitem_system_InitMode);
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

  newComboScreen(w, "NTC MENU", screen_ntc, NULL);
  newComboScreen(w, "RESET MENU", screen_reset, NULL);
  newComboScreen(w, SWSTRING, -1, NULL);
  newComboScreen(w, HWSTRING, -1, NULL);
  newComboScreen(w, "BACK", screen_settings, NULL);
}


static void system_ntc_init(screen_t *scr){
  default_init(scr);
  comboResetIndex(Screen_system_ntc.widgets);
  backup_Pullup=systemSettings.settings.Pullup;
  backup_Pull_res=systemSettings.settings.Pull_res;
  backup_NTC_res=systemSettings.settings.NTC_res;
  backup_NTC_Beta=systemSettings.settings.NTC_Beta;
}


static void system_ntc_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);

  //  [ Pullup mode Widget ]
  //
  newComboMultiOption(w, "Pull mode",&edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &get_Pull_mode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_Pull_mode;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = DownUp;
  edit->numberOfOptions = 2;

  //  [ Pull res Widget ]
  //
  newComboEditable(w, "Pull", &edit, NULL);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_Pull_res;
  edit->big_step = 50;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_Pull_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ NTC res Widget ]
  //
  newComboEditable(w, "NTC", &edit, NULL);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_NTC_res;
  edit->big_step = 50;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_NTC_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ NTC Beta Widget ]
  //
  newComboEditable(w, "NTC beta", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->getData = &get_NTC_beta;
  edit->big_step = 500;
  edit->step = 50;
  edit->setData = (void (*)(void *))&set_NTC_beta;
  edit->max_value = 50000;
  edit->min_value = 500;

  newComboAction(w, "SAVE", &saveNTC, NULL);
  newComboScreen(w, "BACK", screen_system , NULL);
}


void system_screen_setup(screen_t *scr){
  screen_t *sc;

  scr->init = &system_init;
  scr->onEnter = &system_onEnter;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &system_create;

  sc=&Screen_system_ntc;
  oled_addScreen(sc, screen_ntc);
  sc->init = &system_ntc_init;
  sc->processInput=&autoReturn_ProcessInput;
  sc->create = &system_ntc_create;
}
