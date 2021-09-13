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
comboBox_item_t *comboItem_advFilter;
static editable_widget_t *editable_IRON_StandbyTemp;
static editable_widget_t *editable_IRON_BoostTemp;
static editable_widget_t *editable_IRON_MaxTemp;
static editable_widget_t *editable_IRON_MinTemp;
static editable_widget_t *editable_IRON_UserTemp;

filter_t bak_f;


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
static void setUserTemp(uint32_t *val) {
  if(*val > systemSettings.Profile.MaxSetTemperature){
    *val = systemSettings.Profile.MaxSetTemperature;
  }
  else if(*val < systemSettings.Profile.MinSetTemperature){
    *val = systemSettings.Profile.MinSetTemperature;
  }
  systemSettings.Profile.UserSetTemperature = *val;
}
static void * getUserTemp() {
  temp = systemSettings.Profile.UserSetTemperature;
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
  editable_IRON_MinTemp->max_value = temp-1;
  editable_IRON_MaxTemp->min_value = systemSettings.Profile.MinSetTemperature+1;
  if(systemSettings.Profile.standbyTemperature>temp){
    systemSettings.Profile.standbyTemperature = temp;
  }
  if(systemSettings.Profile.UserSetTemperature>temp){
    systemSettings.Profile.UserSetTemperature = temp;
    Iron.CurrentSetTemperature=temp;
  }
  return &temp;
}
static void setMaxTemp(uint32_t *val) {
  systemSettings.Profile.MaxSetTemperature=*val;
}
//=========================================================
static void * getMinTemp() {
  temp=systemSettings.Profile.MinSetTemperature;
  editable_IRON_MaxTemp->min_value = temp+1;
  editable_IRON_MinTemp->max_value = systemSettings.Profile.MaxSetTemperature-1;
  if(systemSettings.Profile.UserSetTemperature<temp){
    systemSettings.Profile.UserSetTemperature = temp;
    Iron.CurrentSetTemperature=temp;
  }
  return &temp;
}
static void setMinTemp(uint32_t *val) {
  systemSettings.Profile.MinSetTemperature=*val;
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

static void iron_onEnter(screen_t *scr){
  if(systemSettings.settings.tempUnit==mode_Farenheit){
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
    comboResetIndex(Screen_iron.widgets);
  }
}

int filter_Save(widget_t *w, RE_Rotation_t input){
  __disable_irq();
  systemSettings.Profile.tipFilter = bak_f;
  TIP.filter=bak_f;
  __enable_irq();
  return screen_iron;
}

static void iron_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

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
  edit->max_value = 480;
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
  edit->max_value = 480;
  edit->min_value = 50;
  edit->setData = (void (*)(void *))&setMinTemp;

  //  [ user Temp Widget ]
  //
  newComboEditable(w, strings[lang].IRON_User_Temp, &edit, NULL);
  editable_IRON_UserTemp=edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getUserTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setUserTemp;

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
  dis->reservedChars=5;
  dis->number_of_dec=1;
  dis->endString="Ω";
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
  edit->max_value = 4096;
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
  edit->max_value = error_resume;
  edit->min_value = error_sleep;
  edit->options = strings[lang].errMode;
  edit->numberOfOptions = 3;

  //  [ BACK button ]
  //
  newComboScreen(w, strings[lang].IRON_Filter_Settings, screen_advFilter, &comboItem_advFilter);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
}

static void iron_advFilter_onEnter(screen_t *scr){
  comboResetIndex(Screen_advFilter.widgets);
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
  newComboScreen(w, strings[lang]._CANCEL, screen_iron, NULL);
}

void iron_screen_setup(screen_t *scr){
  screen_t *sc;
  scr->onEnter = &iron_onEnter;
  scr->processInput = &autoReturn_ProcessInput;
  scr->create = &iron_create;

  sc = &Screen_advFilter;
  oled_addScreen(&Screen_advFilter, screen_advFilter);
  sc->onEnter = &iron_advFilter_onEnter;
  sc->processInput = &autoReturn_ProcessInput;
  sc->create = &iron_advFilter_create;

}

