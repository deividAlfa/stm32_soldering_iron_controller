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

static comboBox_item_t *comboitem_system_ButtonWakeMode;
static comboBox_item_t *comboitem_system_ShakeWakeMode;
static comboBox_item_t *comboitem_system_StandMode;
static comboBox_item_t *comboitem_system_BootMode;

#ifdef USE_NTC
static comboBox_item_t *comboitem_PullRes;
static comboBox_item_t *comboitem_PullMode;
static comboBox_item_t *comboitem_AutoDetect;
static comboBox_item_t *comboitem_NTC_res;
static comboBox_item_t *comboitem_NTC_res_beta;
static comboBox_item_t *comboitem_Detect_high_res;
static comboBox_item_t *comboitem_Detect_low_res;
static comboBox_item_t *comboitem_Detect_high_res_beta;
static comboBox_item_t *comboitem_Detect_low_res_beta;
#endif

static editable_widget_t *editable_system_TempStep;
static editable_widget_t *editable_system_bigTempStep;


#ifdef USE_NTC
uint8_t backup_Pullup, backup_NTC_detect, backup_enableNTC;
uint16_t backup_Pull_res, backup_NTC_res, backup_NTC_Beta, backup_NTC_detect_high_res, backup_NTC_detect_low_res, backup_NTC_detect_high_res_beta, backup_NTC_detect_low_res_beta;
static uint8_t current_lang = lang_english;

void update_NTC_menu(void){
  uint8_t NTC_auto = (backup_NTC_detect && backup_enableNTC);
  uint8_t NTC_fixed = (!backup_NTC_detect && backup_enableNTC);
  comboitem_PullMode->enabled = backup_enableNTC;
  comboitem_PullRes->enabled =  backup_enableNTC;
  comboitem_AutoDetect->enabled =  backup_enableNTC;
  comboitem_NTC_res->enabled = NTC_fixed;
  comboitem_NTC_res_beta->enabled = NTC_fixed;
  comboitem_Detect_high_res->enabled = NTC_auto;
  comboitem_Detect_low_res->enabled = NTC_auto;
  comboitem_Detect_high_res_beta->enabled = NTC_auto;
  comboitem_Detect_low_res_beta->enabled = NTC_auto;
}
#endif
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

static void * getTmpUnit() {
  temp = systemSettings.settings.tempUnit;
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
    editable_system_bigTempStep->inputData.endString="\260F";
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
    editable_system_bigTempStep->inputData.endString="\260C";
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

static void * getBigTmpStep() {
  temp = systemSettings.settings.tempBigStep;
  return &temp;
}

static void setBigTmpStep(uint32_t *val) {
  systemSettings.settings.tempBigStep = *val;
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

  bool mode = (systemSettings.settings.WakeInputMode==mode_shake);   // 0=shake, 1=stand

  comboitem_system_StandMode->enabled       = !mode;
  comboitem_system_BootMode->enabled        = mode;
  comboitem_system_ShakeWakeMode->enabled   = mode;
  comboitem_system_ButtonWakeMode->enabled  = mode;

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
static void * getLanguage() {
  temp = systemSettings.settings.language;
  return &temp;
}

static void setLanguage(uint32_t *val) {
  lang = *val;
  systemSettings.settings.language=*val;
}
//=========================================================
static void setButtonWakeMode(uint32_t *val) {
  systemSettings.settings.buttonWakeMode = *val;
}
static void * getButtonWakeMode() {
  temp = systemSettings.settings.buttonWakeMode;
  return &temp;
}
//=========================================================
static void setShakeWakeMode(uint32_t *val) {
  systemSettings.settings.shakeWakeMode = *val;
}
static void * getShakeWakeMode() {
  temp = systemSettings.settings.shakeWakeMode;
  return &temp;
}
//=========================================================
static void system_onEnter(screen_t *scr){
  if(scr==&Screen_settings){
    comboResetIndex(Screen_system.widgets);
  }
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
    editable_system_bigTempStep->inputData.endString="\260F";
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
    editable_system_bigTempStep->inputData.endString="\260C";

  }
  profile=systemSettings.settings.currentProfile;
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
  update_language();
  current_lang = lang;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);
  ((comboBox_widget_t*)w->content)->font = font_menu;

  //  [ Language Widget ]
  //
  newComboMultiOption(w, strings[lang]._Language, &edit, NULL);
  edit->inputData.getData = &getLanguage;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setLanguage;
  edit->max_value = LANGUAGE_COUNT-1;
  edit->min_value = 0;
  edit->options = Langs;
  edit->numberOfOptions = LANGUAGE_COUNT;

  //  [ Profile Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Profile, &edit, NULL);
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
  newComboEditable(w, strings[lang].SYSTEM_Contrast, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &getContrast_;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setContrast_;
  edit->max_value = 10;
  edit->min_value = 0;

  //  [ Oled dimming Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Auto_Dim, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->endString="s";
  dis->getData = &getOledDimming;
  edit->big_step = 10;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setOledDimming;
  edit->max_value = 240;
  edit->min_value = 0;
  edit->selectable.processInput=&OledDimming_ProcessInput;

  //  [ Oled Offset Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Offset, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getOledOffset;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setOledOffset;
  edit->max_value = 15;
  edit->min_value = 0;

  //  [ Wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Wake_Mode, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setWakeMode;
  edit->max_value = mode_stand;
  edit->min_value = mode_shake;
  edit->options = wakeMode;
  edit->numberOfOptions = 2;


  //  [ Stand mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Stand_Mode, &edit, &comboitem_system_StandMode);
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
  newComboMultiOption(w, strings[lang].SYSTEM_Boot, &edit, &comboitem_system_BootMode);
  dis=&edit->inputData;
  dis->getData = &getInitMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setInitMode;
  edit->max_value = mode_run;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
  edit->numberOfOptions = 3;

  //  [ Encoder wake mode  Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Button_Wake, &edit,&comboitem_system_ButtonWakeMode);
  dis=&edit->inputData;
  dis->getData = &getButtonWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonWakeMode;
  edit->max_value = 3;
  edit->min_value = 0;
  edit->options =WakeModes;
  edit->numberOfOptions = 4;

  //  [ Shake wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Shake_Wake, &edit,&comboitem_system_ShakeWakeMode);
  dis=&edit->inputData;
  dis->getData = &getShakeWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeWakeMode;
  edit->max_value = 3;
  edit->min_value = 0;
  edit->options =WakeModes;
  edit->numberOfOptions = 4;

  //  [ Encoder inversion Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Encoder, &edit, NULL);
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
  newComboMultiOption(w, strings[lang].SYSTEM_Buzzer, &edit, NULL);
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
  newComboMultiOption(w, strings[lang].SYSTEM_Temperature, &edit, NULL);
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
  
  //  [ Active detection Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Active_Detection,&edit, NULL);
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
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;
#endif
#ifdef USE_NTC
  newComboScreen(w, strings[lang].SYSTEM_NTC_MENU, screen_ntc, NULL);
#endif
  newComboScreen(w, strings[lang].SYSTEM_RESET_MENU, screen_reset, NULL);
  newComboScreen(w, SWSTRING, -1, NULL);
  newComboScreen(w, HWSTRING, -1, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
}

int system_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){

  if(lang!=current_lang){                                                       // If language changed
    oled_backup_comboStatus(scr);
    oled_destroy_screen(scr);                                                   // Destroy and create the screen
    system_create(scr);
    oled_restore_comboStatus(scr);
    ((comboBox_widget_t*)scr->widgets->content)->currentItem->widget->selectable.state=widget_edit;
    ((comboBox_widget_t*)scr->widgets->content)->currentItem->widget->selectable.previous_state=widget_edit;
    scr->refresh = refresh_triggered;
  }
  return autoReturn_ProcessInput(scr, input, state);
}

#ifdef USE_NTC


static void set_enable_NTC(uint32_t *val) {
  backup_enableNTC = *val;
  update_NTC_menu();
}
static void * get_enable_NTC() {
  temp = backup_enableNTC;
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
  backup_NTC_res = *val;
}
static void * get_NTC_res() {
  temp = backup_NTC_res;
  return &temp;
}
//=========================================================
static void set_Pull_res(uint32_t *val) {
  backup_Pull_res = *val;
}
static void * get_Pull_res() {
  temp = backup_Pull_res;
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
static void set_NTC_detect(uint32_t *val) {
  backup_NTC_detect = *val;
  update_NTC_menu();
}
static void * get_NTC_detect() {
  temp = backup_NTC_detect;
  return &temp;
}
//=========================================================
static void set_NTC_detect_high_res(uint32_t *val) {
  backup_NTC_detect_high_res = *val;
}
static void * get_NTC_detect_high_res() {
  temp = backup_NTC_detect_high_res;
  return &temp;
}
//=========================================================
static void set_NTC_detect_low_res(uint32_t *val) {
  backup_NTC_detect_low_res = *val;
}
static void * get_NTC_detect_low_res() {
  temp = backup_NTC_detect_low_res;
  return &temp;
}
//=========================================================
static void set_NTC_detect_high_res_beta(uint32_t *val) {
  backup_NTC_detect_high_res_beta = *val;
}
static void * get_NTC_detect_high_res_beta() {
  temp = backup_NTC_detect_high_res_beta;
  return &temp;
}
//=========================================================
static void set_NTC_detect_low_res_beta(uint32_t *val) {
  backup_NTC_detect_low_res_beta = *val;
}
static void * get_NTC_detect_low_res_beta() {
  temp = backup_NTC_detect_low_res_beta;
  return &temp;
}
//=========================================================
static int saveNTC() {
  __disable_irq();
  systemSettings.settings.enableNTC=backup_enableNTC;
  systemSettings.settings.NTC_detect=backup_NTC_detect;
  systemSettings.settings.NTC_detect_high_res = backup_NTC_detect_high_res;
  systemSettings.settings.NTC_detect_low_res = backup_NTC_detect_low_res;
  systemSettings.settings.NTC_detect_high_res_beta = backup_NTC_detect_high_res_beta;
  systemSettings.settings.NTC_detect_low_res_beta = backup_NTC_detect_low_res_beta;
  systemSettings.settings.Pullup=backup_Pullup;
  systemSettings.settings.Pull_res=backup_Pull_res;
  systemSettings.settings.NTC_res=backup_NTC_res;
  systemSettings.settings.NTC_Beta=backup_NTC_Beta;
  detectNTC();

  __enable_irq();
  return screen_system;
}
//=========================================================

static void system_ntc_onEnter(screen_t *scr){
  comboResetIndex(Screen_system_ntc.widgets);
  backup_enableNTC=systemSettings.settings.enableNTC;
  backup_NTC_detect=systemSettings.settings.NTC_detect;
  backup_NTC_detect_high_res=systemSettings.settings.NTC_detect_high_res;
  backup_NTC_detect_low_res=systemSettings.settings.NTC_detect_low_res;
  backup_NTC_detect_high_res_beta=systemSettings.settings.NTC_detect_high_res_beta;
  backup_NTC_detect_low_res_beta=systemSettings.settings.NTC_detect_low_res_beta;
  backup_Pullup=systemSettings.settings.Pullup;
  backup_Pull_res=systemSettings.settings.Pull_res;
  backup_NTC_res=systemSettings.settings.NTC_res;
  backup_NTC_Beta=systemSettings.settings.NTC_Beta;
  update_NTC_menu();
}

static void system_ntc_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;
  update_language();
  current_lang=lang;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);
  ((comboBox_widget_t*)w->content)->font = font_menu;

  //  [ NTC enabled Widget ]
  //
  newComboMultiOption(w, strings[lang].NTC_Enable_NTC,&edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &get_enable_NTC;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_enable_NTC;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ Pullup mode Widget ]
  //
  newComboMultiOption(w, strings[lang].NTC_Pull,&edit, &comboitem_PullMode);
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
  newComboEditable(w, strings[lang].NTC__Res, &edit, &comboitem_PullRes);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_Pull_res;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_Pull_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ Auto detect Widget ]
  //
  newComboMultiOption(w, strings[lang].NTC_NTC_Detect,&edit, &comboitem_AutoDetect);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &get_NTC_detect;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_NTC_detect;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //  [ NTC auto higher Widget ]
  //
  newComboEditable(w, strings[lang].NTC__High, &edit, &comboitem_Detect_high_res);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_NTC_detect_high_res;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_NTC_detect_high_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ NTC auto higher beta Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Beta, &edit, &comboitem_Detect_high_res_beta);
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &get_NTC_detect_high_res_beta;
  edit->big_step = 100;
  edit->step = 10;
  edit->setData = (void (*)(void *))&set_NTC_detect_high_res_beta;
  edit->max_value = 50000;
  edit->min_value = 500;

  //  [ NTC auto lower Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Low, &edit, &comboitem_Detect_low_res);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_NTC_detect_low_res;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_NTC_detect_low_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ NTC auto lower beta Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Beta, &edit, &comboitem_Detect_low_res_beta);
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &get_NTC_detect_low_res_beta;
  edit->big_step = 100;
  edit->step = 10;
  edit->setData = (void (*)(void *))&set_NTC_detect_low_res_beta;
  edit->max_value = 50000;
  edit->min_value = 500;

  //  [ NTC res Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Res, &edit, &comboitem_NTC_res);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="K\261";
  dis->getData = &get_NTC_res;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_NTC_res;
  edit->max_value = 5000;
  edit->min_value = 1;

  //  [ NTC Beta Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Beta, &edit, &comboitem_NTC_res_beta);
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &get_NTC_beta;
  edit->big_step = 100;
  edit->step = 10;
  edit->setData = (void (*)(void *))&set_NTC_beta;
  edit->max_value = 50000;
  edit->min_value = 500;

  newComboAction(w, strings[lang]._SAVE, &saveNTC, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_system , NULL);
}

#endif





void system_screen_setup(screen_t *scr){

  scr->onEnter = &system_onEnter;
  scr->onExit = &system_onExit;
  scr->processInput=&system_ProcessInput;
  scr->create = &system_create;

  #ifdef USE_NTC
  screen_t *sc;
  sc=&Screen_system_ntc;
  oled_addScreen(sc, screen_ntc);
  sc->onEnter = &system_ntc_onEnter;
  sc->processInput=&autoReturn_ProcessInput;
  sc->create = &system_ntc_create;
  #endif
}
