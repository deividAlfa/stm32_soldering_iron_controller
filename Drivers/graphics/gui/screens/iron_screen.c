/*
 * settings_iron_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */


#include "iron_screen.h"
#include "screen_common.h"

screen_t Screen_iron;
screen_t Screen_advFilter;
static comboBox_item_t *comboItem_advFilter;
static comboBox_item_t *comboitem_ShakeFiltering;
static comboBox_item_t *comboitem_StandMode;
static editable_widget_t *editable_IRON_StandbyTemp;
static editable_widget_t *editable_IRON_BoostTemp;
static editable_widget_t *editable_IRON_MaxTemp;
static editable_widget_t *editable_IRON_MinTemp;
static editable_widget_t *editable_IRON_UserTemp;
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

filter_t bak_f;

void updateTempValues();

#ifdef USE_NTC
ntc_data_t backup_ntc;

void update_NTC_menu(void){
  uint8_t NTC_auto = (backup_ntc.detection && backup_ntc.enabled);
  uint8_t NTC_fixed = (!backup_ntc.detection && backup_ntc.enabled);
  comboitem_PullMode->enabled = backup_ntc.enabled;
  comboitem_PullRes->enabled =  backup_ntc.enabled;
  comboitem_AutoDetect->enabled =  backup_ntc.enabled;
  comboitem_NTC_res->enabled = NTC_fixed;
  comboitem_NTC_res_beta->enabled = NTC_fixed;
  comboitem_Detect_high_res->enabled = NTC_auto;
  comboitem_Detect_low_res->enabled = NTC_auto;
  comboitem_Detect_high_res_beta->enabled = NTC_auto;
  comboitem_Detect_low_res_beta->enabled = NTC_auto;
}
#endif



//=========================================================
static void set_filter(uint32_t *val) {
  bak_f.coefficient = *val;
  if(bak_f.min > bak_f.coefficient){
    bak_f.min = bak_f.coefficient;
  }
}
static void * get_filter() {
  temp = bak_f.coefficient;
  return &temp;
}
//=========================================================
static void set_threshold(uint32_t *val) {
  if(*val>=(bak_f.reset_threshold-100)){
    *val=bak_f.reset_threshold-100;
  }
  bak_f.threshold= *val;
}
static void * get_threshold() {
  temp = bak_f.threshold;
  return &temp;
}
//=========================================================
static void set_count_limit(uint32_t *val) {
  bak_f.count_limit= *val;
}
static void * get_count_limit() {
  temp = bak_f.count_limit;
  return &temp;
}
//=========================================================
static void set_limit_step(uint32_t *val) {
  bak_f.step= *val;
}
static void * get_limit_step() {
  temp = bak_f.step;
  return &temp;
}
//=========================================================
static void set_min_filter(uint32_t *val) {
  bak_f.min = *val;
  if(bak_f.min > bak_f.coefficient){
    bak_f.min = bak_f.coefficient;
  }
}
static void * get_min_filter() {
  temp = bak_f.min;
  return &temp;
}
//=========================================================
static void set_reset_threshold(uint32_t *val) {
  if(*val<=(bak_f.threshold+100)){
    *val=bak_f.threshold+100;
  }
  bak_f.reset_threshold= *val;
}
static void * get_reset_threshold() {
  temp = bak_f.reset_threshold;
  return &temp;
}
//=========================================================
void update_Iron_menu(void){
  bool mode = (systemSettings.Profile.WakeInputMode==mode_shake);
  comboitem_StandMode->enabled       = !mode;
  comboitem_ShakeFiltering->enabled  = mode;
}
//=========================================================
#ifdef USE_VIN
static void * getMaxPower() {
  temp = systemSettings.Profile.power;
  return &temp;
}
static void setMaxPower(uint32_t *val) {
  systemSettings.Profile.power = *val;
}
//=========================================================
static void * getTipImpedance() {
  temp = systemSettings.Profile.impedance;
  return &temp;
}
static void setTipImpedance(uint32_t *val) {
  systemSettings.Profile.impedance = *val;
}
#endif
//=========================================================
static void setSleepTime(uint32_t *val) {
  systemSettings.Profile.sleepTimeout= *val*60000;
}
static void * getSleepTime() {
  temp = systemSettings.Profile.sleepTimeout/60000;
  return &temp;
}
//=========================================================
static void setStandbyTime(uint32_t *val) {
  systemSettings.Profile.standbyTimeout= *val*60000;
}
static void * getStandbyTime() {
  temp = systemSettings.Profile.standbyTimeout/60000;
  return &temp;
}
//=========================================================
static void setStandbyTemp(uint32_t *val) {
  systemSettings.Profile.standbyTemperature= *val;
}
static void * getStandbyTemp() {
  temp = systemSettings.Profile.standbyTemperature;
  return &temp;
}
//=========================================================
static void setDefaultTemp(uint32_t *val) {
  if(*val > systemSettings.Profile.MaxSetTemperature){
    *val = systemSettings.Profile.MaxSetTemperature;
  }
  else if(*val < systemSettings.Profile.MinSetTemperature){
    *val = systemSettings.Profile.MinSetTemperature;
  }
  systemSettings.Profile.defaultTemperature = *val;
}
static void * getDefaultTemp() {
  temp = systemSettings.Profile.defaultTemperature;
  return &temp;
}
//=========================================================
static void * _getPwmMul() {
  temp=systemSettings.Profile.pwmMul;
  return &temp;
}
static void _setPwmMul(uint32_t *val) {
  setPwmMul(*val);
}
//=========================================================
static void * _getReadDelay() {
  temp=(systemSettings.Profile.readDelay+1)/20;
  return &temp;
}
static void _setReadDelay(uint32_t *val) {
  uint16_t delay=(*val*20)-1;
  if(delay>(systemSettings.Profile.readPeriod-200)){
    delay = systemSettings.Profile.readPeriod-200;
  }
  setReadDelay(delay);
}
//=========================================================
static void * _getReadPeriod() {
  temp=(systemSettings.Profile.readPeriod+1)/200;
  return &temp;
}
static void _setReadPeriod(uint32_t *val) {
  uint16_t period=(*val*200)-1;

  if(period<(systemSettings.Profile.readDelay+200)){
      period=systemSettings.Profile.readDelay+200;
  }
  setReadPeriod(period);
}
//=========================================================
static void * getMaxTemp() {
  temp=systemSettings.Profile.MaxSetTemperature;
  return &temp;
}
static void setMaxTemp(uint32_t *val) {
  systemSettings.Profile.MaxSetTemperature=*val;
  updateTempValues();
}
//=========================================================
static void * getMinTemp() {
  temp=systemSettings.Profile.MinSetTemperature;
  return &temp;
}
static void setMinTemp(uint32_t *val) {
  systemSettings.Profile.MinSetTemperature=*val;
  updateTempValues();
}
//=========================================================
static void * geterrorDelay() {
  temp = systemSettings.Profile.errorTimeout/100;
  return &temp;
}
static void seterrorDelay(uint32_t *val) {
  systemSettings.Profile.errorTimeout = *val*100;
}
//=========================================================
static void * geterrorResume() {
  temp = systemSettings.Profile.errorResumeMode;
  return &temp;
}
static void seterrorResume(uint32_t *val) {
  systemSettings.Profile.errorResumeMode = *val;
}
//=========================================================
static void * getNoIronADC() {
  temp = systemSettings.Profile.noIronValue;
  return &temp;
}
static void setNoIronADC(uint32_t *val) {
  systemSettings.Profile.noIronValue = *val;
}
//=========================================================
static void setBoostTime(uint32_t *val) {
  systemSettings.Profile.boostTimeout= *val*60000;
}
static void * getBoostTime() {
  temp = systemSettings.Profile.boostTimeout/60000;
  return &temp;
}
//=========================================================
static void setBoostTemp(uint32_t *val) {
  systemSettings.Profile.boostTemperature= *val;
}
static void * getBoostTemp() {
  temp = systemSettings.Profile.boostTemperature;
  return &temp;
}
//=========================================================
static void * getWakeMode() {
  temp = systemSettings.Profile.WakeInputMode;
  update_Iron_menu();
  return &temp;
}
static void setWakeMode(uint32_t *val) {
  systemSettings.Profile.WakeInputMode = *val;
}
//=========================================================
static void * getStandMode() {
  temp = systemSettings.Profile.StandMode;
  return &temp;
}
static void setStandMode(uint32_t *val) {
  systemSettings.Profile.StandMode = *val;
}
//=========================================================
static void * getShakeFiltering() {
  temp = systemSettings.Profile.shakeFiltering;
  return &temp;
}
static void setShakeFiltering(uint32_t *val) {
  systemSettings.Profile.shakeFiltering = *val;
}
//=========================================================

static void iron_onEnter(screen_t *scr){
  update_Iron_menu();
  if(getSystemTempUnit()==mode_Farenheit){
    editable_IRON_MaxTemp->inputData.endString="\260F";
    editable_IRON_MinTemp->inputData.endString="\260F";
    editable_IRON_StandbyTemp->inputData.endString="\260F";
    editable_IRON_BoostTemp->inputData.endString="\260F";
    editable_IRON_UserTemp->inputData.endString="\260F";
  }
  else{
    editable_IRON_MaxTemp->inputData.endString="\260C";
    editable_IRON_MinTemp->inputData.endString="\260C";
    editable_IRON_StandbyTemp->inputData.endString="\260C";
    editable_IRON_BoostTemp->inputData.endString="\260C";
    editable_IRON_UserTemp->inputData.endString="\260C";
  }
  if(scr==&Screen_settings){
    comboResetIndex(Screen_iron.current_widget);
  }
}

static void iron_onExit(screen_t *scr){
  uint16_t const userTemp = getUserTemperature();
  if(userTemp > systemSettings.Profile.MaxSetTemperature)
  {
    setUserTemperature(systemSettings.Profile.MaxSetTemperature);
  }
  else if (userTemp < systemSettings.Profile.MinSetTemperature)
  {
    setUserTemperature(systemSettings.Profile.MinSetTemperature);
  }
}

int filter_Save(widget_t *w, RE_Rotation_t input){
  __disable_irq();
  systemSettings.Profile.tipFilter = bak_f;
  TIP.filter=bak_f;
  __enable_irq();
  return last_scr;
}


#ifdef USE_NTC

static void set_enable_NTC(uint32_t *val) {
  backup_ntc.enabled = *val;
  update_NTC_menu();
}
static void * get_enable_NTC() {
  temp = backup_ntc.enabled;
  return &temp;
}
//=========================================================
static void set_NTC_beta(uint32_t *val) {
  backup_ntc.NTC_beta = *val;
}
static void * get_NTC_beta() {
  temp = backup_ntc.NTC_beta;
  return &temp;
}
//=========================================================
static void set_NTC_res(uint32_t *val) {
  backup_ntc.NTC_res = *val;
}
static void * get_NTC_res() {
  temp = backup_ntc.NTC_res;
  return &temp;
}
//=========================================================
static void set_Pull_res(uint32_t *val) {
  backup_ntc.pull_res = *val;
}
static void * get_Pull_res() {
  temp = backup_ntc.pull_res;
  return &temp;
}
//=========================================================
static void set_Pull_mode(uint32_t *val) {
  backup_ntc.pullup = *val;
}
static void * get_Pull_mode() {
  temp = backup_ntc.pullup;
  return &temp;
}
//=========================================================
static void set_NTC_detect(uint32_t *val) {
  backup_ntc.detection = *val;
  update_NTC_menu();
}
static void * get_NTC_detect() {
  temp = backup_ntc.detection;
  return &temp;
}
//=========================================================
static void set_NTC_detect_high_res(uint32_t *val) {
  backup_ntc.high_NTC_res = *val;
}
static void * get_NTC_detect_high_res() {
  temp = backup_ntc.high_NTC_res;
  return &temp;
}
//=========================================================
static void set_NTC_detect_low_res(uint32_t *val) {
  backup_ntc.low_NTC_res = *val;
}
static void * get_NTC_detect_low_res() {
  temp = backup_ntc.low_NTC_res;
  return &temp;
}
//=========================================================
static void set_NTC_detect_high_res_beta(uint32_t *val) {
  backup_ntc.high_NTC_beta = *val;
}
static void * get_NTC_detect_high_res_beta() {
  temp = backup_ntc.high_NTC_beta;
  return &temp;
}
//=========================================================
static void set_NTC_detect_low_res_beta(uint32_t *val) {
  backup_ntc.low_NTC_beta = *val;
}
static void * get_NTC_detect_low_res_beta() {
  temp = backup_ntc.low_NTC_beta;
  return &temp;
}
//=========================================================
static int saveNTC(widget_t *w, RE_Rotation_t input) {
  __disable_irq();
  systemSettings.Profile.ntc = backup_ntc;
  detectNTC();

  __enable_irq();
  return last_scr;
}
//=========================================================

static void system_ntc_onEnter(screen_t *scr){
  comboResetIndex(Screen_system_ntc.current_widget);
  backup_ntc = systemSettings.Profile.ntc;
  update_NTC_menu();
}

static void system_ntc_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr);

  //  [ NTC enabled Widget ]
  //
  newComboMultiOption(w, strings[lang].NTC_Enable_NTC,&edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &get_enable_NTC;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_enable_NTC;
  edit->options = strings[lang].OffOn;
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
  edit->options = strings[lang].DownUp;
  edit->numberOfOptions = 2;

  //  [ Pull res Widget ]
  //
  newComboEditable(w, strings[lang].NTC__Res, &edit, &comboitem_PullRes);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="KΩ";
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
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ NTC auto higher Widget ]
  //
  newComboEditable(w, strings[lang].NTC__High, &edit, &comboitem_Detect_high_res);
  dis=&edit->inputData;
  dis->number_of_dec=1;
  dis->reservedChars=7;
  dis->endString="KΩ";
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
  dis->endString="KΩ";
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
  dis->endString="KΩ";
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
  newComboScreen(w, strings[lang]._BACK, last_scr , NULL);
}

#endif

static void iron_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;
  uint16_t maxTemp = (getSystemTempUnit()==mode_Celsius ? 480 : 900);

  //  [ IRON COMBO ]
  //
  newWidget(&w,widget_combo, scr);

  //  [ Max Temp Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Max_Temp, &edit, NULL);
  editable_IRON_MaxTemp=edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getMaxTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = maxTemp;
  edit->setData = (void (*)(void *))&setMaxTemp;

  //  [ Min Temp Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Min_Temp, &edit, NULL);
  editable_IRON_MinTemp=edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getMinTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = maxTemp;
  edit->min_value = 50;
  edit->setData = (void (*)(void *))&setMinTemp;

  //  [ user Temp Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Default_Temp, &edit, NULL);
  editable_IRON_UserTemp=edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getDefaultTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setDefaultTemp;

  //  [ Stby Time Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Standby, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="min";
  dis->reservedChars=5;
  dis->getData = &getStandbyTime;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandbyTime;
  edit->max_value = 60;

  //  [ Stby Temp Widget ]
  //
  newComboEditable(w, strings[lang].__Temp, &edit, NULL);
  editable_IRON_StandbyTemp = edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getStandbyTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 350;
  edit->min_value = 50;
  edit->setData = (void (*)(void *))&setStandbyTemp;

  //  [ Sleep Time Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Sleep, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="min";
  dis->reservedChars=5;
  dis->getData = &getSleepTime;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setSleepTime;
  edit->max_value = 60;
  edit->min_value = 1;

  //  [ Boost Time Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Boost, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="min";
  dis->reservedChars=5;
  dis->getData = &getBoostTime;
  edit->big_step = 10;
  edit->step = 1;
  edit->max_value = 60;
  edit->min_value = 1;
  edit->setData = (void (*)(void *))&setBoostTime;

  //  [ Boost Temp Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Boost_Add, &edit, NULL);
  editable_IRON_BoostTemp = edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getBoostTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 200;
  edit->min_value = 10;
  edit->setData = (void (*)(void *))&setBoostTemp;

  //  [ Wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].IRON_Wake_Mode, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setWakeMode;
  edit->options = strings[lang].wakeMode;
  edit->numberOfOptions = 2;

  //  [ Shake filtering Widget ]
  //
  newComboMultiOption(w, strings[lang].IRON_Shake_Filtering, &edit, &comboitem_ShakeFiltering);
  dis=&edit->inputData;
  dis->getData = &getShakeFiltering;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeFiltering;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Stand mode Widget ]
  //
  newComboMultiOption(w, strings[lang].IRON_Stand_Mode, &edit, &comboitem_StandMode);
  dis=&edit->inputData;
  dis->getData = &getStandMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandMode;
  edit->options = strings[lang].InitMode;
  edit->numberOfOptions = 2;
  #ifdef USE_VIN
  //  [ Power Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Power, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->endString="W";
  dis->getData = &getMaxPower;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setMaxPower;
  edit->max_value = 500;
  edit->min_value = 5;

  //  [ Impedance Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Heater, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=6;
  dis->number_of_dec=1;
  dis->endString="Ω";									// Unicode, needs 2 bytes. So "10.0Ω" string uses 6 bytes
  dis->getData = &getTipImpedance;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTipImpedance;
  edit->max_value = 160;
  edit->min_value = 10;
  #endif

  //  [ Read Period Widget ]
  //
  newComboEditable(w, strings[lang].IRON_ADC_Time, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="ms";
  dis->reservedChars=7;
  dis->number_of_dec = 0;
  dis->getData = &_getReadPeriod;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&_setReadPeriod;
  edit->max_value = 200;
  edit->min_value = 10;

  //  [ Read Delay Widget ]
  //
  newComboEditable(w, strings[lang].__Delay, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="ms";
  dis->reservedChars=7;
  dis->number_of_dec = 1;
  dis->getData = &_getReadDelay;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&_setReadDelay;
  edit->max_value = 900;
  edit->min_value = 1;

  //  [ PWM Mult Widget ]
  //
  newComboEditable(w, strings[lang].IRON_PWM_mul, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="x";
  dis->reservedChars=7;
  dis->number_of_dec = 0;
  dis->getData = &_getPwmMul;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&_setPwmMul;
  edit->max_value = 20;
  edit->min_value = 1;

  //  [ ADC Limit Widget ]
  //
  newComboEditable(w, strings[lang].IRON_No_Iron, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getNoIronADC;
  edit->big_step = 50;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setNoIronADC;
  edit->max_value = 4100;
  edit->min_value = 200;

  //  [ Error Delay Widget ]
  //
  newComboEditable(w, strings[lang].IRON_Error_Timeout, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="s";
  dis->reservedChars=5;
  dis->number_of_dec = 1;
  dis->getData = &geterrorDelay;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&seterrorDelay;
  edit->max_value = 250;
  edit->min_value = 1;

  //  [ Error resume mode Widget ]
  //
  newComboMultiOption(w, strings[lang].IRON_Error_Resume_Mode, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &geterrorResume;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&seterrorResume;
  edit->options = strings[lang].errMode;
  edit->numberOfOptions = 3;

  //  [ Filter screen ]
  //
  newComboScreen(w, strings[lang].IRON_FILTER_MENU, screen_advFilter, &comboItem_advFilter);

  #ifdef USE_NTC
  //  [ NTC screen ]
  //
  newComboScreen(w, strings[lang].IRON_NTC_MENU, screen_ntc, NULL);
  #endif

  //  [ BACK button ]
  //
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);

  updateTempValues();
  update_Iron_menu();
}

static void iron_advFilter_onEnter(screen_t *scr){
  comboResetIndex(Screen_advFilter.current_widget);
  bak_f = systemSettings.Profile.tipFilter;
}
static void iron_advFilter_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t *dis;
  editable_widget_t *edit;

  //  [ IRON COMBO ]
  //
  newWidget(&w,widget_combo, scr);

  //  [ Low noise filter Widget ]
  //
  newComboEditable(w, strings[lang].FILTER_Filter, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &get_filter;
  dis->endString = "%";
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_filter;
  edit->max_value = 99;
  edit->min_value = 0;

  //  [ Threshold Widget ]
  //
  newComboEditable(w, strings[lang].FILTER__Threshold, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &get_threshold;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_threshold;
  edit->max_value = 500;
  edit->min_value = 10;

  //  [ Count limit widget]
  //
  newComboEditable(w, strings[lang].FILTER__Count_limit, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &get_count_limit;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_count_limit;
  edit->max_value = 100;
  edit->min_value = 0;

  //  [ Filter down steppping Widget ]
  //
  newComboEditable(w, strings[lang].FILTER__Step_down, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &get_limit_step;
  dis->endString="%";
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_limit_step;
  edit->max_value = -1;
  edit->min_value = -20;

  //  [ Min Filtering Widget ]
  //
  newComboEditable(w, strings[lang].FILTER__Min, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &get_min_filter;
  dis->endString="%";
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&set_min_filter;
  edit->max_value = 99;
  edit->min_value = 0;

  //  [ Reset threshold Widget ]
  //
  newComboEditable(w, strings[lang].FILTER_Reset_limit, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &get_reset_threshold;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (void (*)(void *))&set_reset_threshold;
  edit->max_value = 1000;
  edit->min_value = 10;

  newComboAction(w, strings[lang]._SAVE, &filter_Save , NULL);
  newComboScreen(w, strings[lang]._CANCEL, last_scr, NULL);
}

void iron_screen_setup(screen_t *scr){
  screen_t *sc;
  scr->onEnter = &iron_onEnter;
  scr->processInput = &autoReturn_ProcessInput;
  scr->create = &iron_create;
  scr->onExit = &iron_onExit;

  sc = &Screen_advFilter;
  oled_addScreen(&Screen_advFilter, screen_advFilter);
  sc->onEnter = &iron_advFilter_onEnter;
  sc->processInput = &autoReturn_ProcessInput;
  sc->create = &iron_advFilter_create;

  #ifdef USE_NTC
  sc=&Screen_system_ntc;
  oled_addScreen(sc, screen_ntc);
  sc->onEnter = &system_ntc_onEnter;
  sc->processInput=&autoReturn_ProcessInput;
  sc->create = &system_ntc_create;
  #endif

}

void updateTempValues()
{
  editable_IRON_MinTemp->max_value = systemSettings.Profile.MaxSetTemperature - 1;
  editable_IRON_MaxTemp->min_value = systemSettings.Profile.MinSetTemperature + 1;

  if(systemSettings.Profile.defaultTemperature > systemSettings.Profile.MaxSetTemperature)
  {
    systemSettings.Profile.defaultTemperature = systemSettings.Profile.MaxSetTemperature;
  }
  else if(systemSettings.Profile.defaultTemperature < systemSettings.Profile.MinSetTemperature)
  {
    systemSettings.Profile.defaultTemperature = systemSettings.Profile.MinSetTemperature;
  }
}

