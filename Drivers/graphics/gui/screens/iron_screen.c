/*
 * settings_iron_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */


#include "iron_screen.h"
#include "screen_common.h"

screen_t Screen_iron;

static editable_widget_t *editable_IRON_StandbyTemp;
static editable_widget_t *editable_IRON_BoostTemp;
static editable_widget_t *editable_IRON_MaxTemp;
static editable_widget_t *editable_IRON_MinTemp;

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
  systemSettings.Profile.sleepTimeout= *val;
}
static void * getSleepTime() {
  temp = systemSettings.Profile.sleepTimeout;
  return &temp;
}
//=========================================================
static void setStandbyTime(uint32_t *val) {
  systemSettings.Profile.standbyTimeout= *val;
}
static void * getStandbyTime() {
  temp = systemSettings.Profile.standbyTimeout;
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
  /*
  if(systemSettings.Profile.standbyTemperature<temp){
    systemSettings.Profile.standbyTemperature = temp;
  }
  */
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
static void * getfilterFactor() {
  temp = systemSettings.Profile.filterFactor;
  return &temp;
}
static void setfilterFactor(uint32_t *val) {
  systemSettings.Profile.filterFactor = *val;
  TIP.filter_normal=*val;
}
//=========================================================
static void * geterrorDelay() {
  temp = systemSettings.settings.errorDelay*100;
  return &temp;
}
static void seterrorDelay(uint32_t *val) {
  systemSettings.settings.errorDelay = *val/100;
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
  systemSettings.Profile.boostTimeout= *val;
}
static void * getBoostTime() {
  temp = systemSettings.Profile.boostTimeout;
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

static void iron_init(screen_t *scr){
  default_init(scr);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_IRON_MaxTemp->inputData.endString="\260F";
    editable_IRON_MinTemp->inputData.endString="\260F";
    editable_IRON_StandbyTemp->inputData.endString="\260F";
    editable_IRON_BoostTemp->inputData.endString="\260F";
  }
  else{
    editable_IRON_MaxTemp->inputData.endString="\260C";
    editable_IRON_MinTemp->inputData.endString="\260C";
    editable_IRON_StandbyTemp->inputData.endString="\260C";
    editable_IRON_BoostTemp->inputData.endString="\260C";
  }
  comboResetIndex(Screen_iron.widgets);
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
  newComboEditable(w, "Max temp", &edit, NULL);
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
  newComboEditable(w, "Min temp", &edit, NULL);
  editable_IRON_MinTemp=edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getMinTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 480;
  edit->min_value = 50;
  edit->setData = (void (*)(void *))&setMinTemp;

  //  [ Stby Time Widget ]
  //
  newComboEditable(w, "Stby time", &edit, NULL);
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
  newComboEditable(w, "Stby temp", &edit, NULL);
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
  newComboEditable(w, "Slp time", &edit, NULL);
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
  newComboEditable(w, "Boost time", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="s";
  dis->reservedChars=5;
  dis->getData = &getBoostTime;
  edit->big_step = 20;
  edit->step = 5;
  edit->max_value = 120;
  edit->min_value = 10;
  edit->setData = (void (*)(void *))&setBoostTime;

  //  [ Boost Temp Widget ]
  //
  newComboEditable(w, "Boost add", &edit, NULL);
  editable_IRON_BoostTemp = edit;
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->getData = &getBoostTemp;
  edit->big_step = 50;
  edit->step = 10;
  edit->max_value = 200;
  edit->min_value = 10;
  edit->setData = (void (*)(void *))&setBoostTemp;

  #ifdef USE_VIN
  //  [ Power Widget ]
  //
  newComboEditable(w, "Power", &edit, NULL);
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
  newComboEditable(w, "Heater", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=5;
  dis->number_of_dec=1;
  dis->endString="\261";
  dis->getData = &getTipImpedance;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTipImpedance;
  edit->max_value = 160;
  edit->min_value = 10;
  #endif

  //  [ Read Period Widget ]
  //
  newComboEditable(w, "ADC time", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="mS";
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
  newComboEditable(w, "Delay", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="mS";
  dis->reservedChars=7;
  dis->number_of_dec = 1;
  dis->getData = &_getReadDelay;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&_setReadDelay;
  edit->max_value = 1000;
  edit->min_value = 1;

  //  [ PWM Mult Widget ]
  //
  newComboEditable(w, "PWM mult.", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="x";
  dis->reservedChars=7;
  dis->number_of_dec = 0;
  dis->getData = &_getPwmMul;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&_setPwmMul;
  edit->max_value = 20;
  edit->min_value = 1;

  //  [ Filter Coefficient Widget ]
  //
  newComboEditable(w, "Filter", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=1;
  dis->getData = &getfilterFactor;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setfilterFactor;
  edit->max_value = 8;
  edit->min_value = 2;

  //  [ ADC Limit Widget ]
  //
  newComboEditable(w, "No iron", &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getNoIronADC;
  edit->big_step = 200;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setNoIronADC;
  edit->max_value = 4100;
  edit->min_value = 200;

  //  [ No Iron Delay Widget ]
  //
  newComboEditable(w, "Delay", &edit, NULL);
  dis=&edit->inputData;
  dis->endString="mS";
  dis->reservedChars=5;
  dis->getData = &geterrorDelay;
  edit->big_step = 100;
  edit->step = 50;
  edit->setData = (void (*)(void *))&seterrorDelay;
  edit->max_value = 950;
  edit->min_value = 100;

  //  [ BACK button ]
  //
  newComboScreen(w, "BACK", screen_settings, NULL);
}



void iron_screen_setup(screen_t *scr){
  scr->init = &iron_init;
  scr->processInput = &autoReturn_ProcessInput;
  scr->create = &iron_create;
}

