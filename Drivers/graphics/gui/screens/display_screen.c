/*
 * settings_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "display_screen.h"
#include "screen_common.h"

screen_t Screen_display;

#ifndef ST7565
static comboBox_item_t *comboitem_display_Dim_Timeout;
static comboBox_item_t *comboitem_display_Dim_PowerOff;
#endif


void update_display_menu(void){
#ifndef ST7565
  bool mode = (systemSettings.settings.dim_mode>dim_off);
  comboitem_display_Dim_PowerOff->enabled = mode;
  comboitem_display_Dim_Timeout->enabled = mode;
#endif
}

//=========================================================
static void * getDisplayContrastOrBrightness_() {
#ifdef ST7565
  temp = systemSettings.settings.contrastOrBrightness;
#else
  temp = systemSettings.settings.contrastOrBrightness/25;
#endif
  return &temp;
}
static void setDisplayContrast_(uint32_t *val) {
#ifdef ST7565
  systemSettings.settings.contrastOrBrightness=*val;
#else
  if(*val==0){
    systemSettings.settings.contrastOrBrightness=5;
  }
  else if(*val==10){
    systemSettings.settings.contrastOrBrightness=255;
  }
  else{
    systemSettings.settings.contrastOrBrightness=*val*25;
  }
#endif
  setDisplayContrastOrBrightness(systemSettings.settings.contrastOrBrightness);
}
//=========================================================
static void * getdisplayOffset() {
  temp = systemSettings.settings.displayOffset;
  return &temp;
}
static void setdisplayOffset(uint32_t *val) {
  systemSettings.settings.displayOffset= *val;
}
//=========================================================
static void * getdisplayXflip() {
  temp = systemSettings.settings.displayXflip;
  return &temp;
}
static void setdisplayXflip(uint32_t *val) {
  systemSettings.settings.displayXflip= *val;
  setDisplayXflip(systemSettings.settings.displayXflip);
}
//=========================================================
static void * getdisplayYflip() {
  temp = systemSettings.settings.displayYflip;
  return &temp;
}
static void setdisplayYflip(uint32_t *val) {
  systemSettings.settings.displayYflip= *val;
  setDisplayYflip(systemSettings.settings.displayYflip);
}
//=========================================================
#ifdef ST7565
static void * getdisplayResRatio() {
  temp = systemSettings.settings.displayResRatio;
  return &temp;
}
static void setdisplayResRatio(uint32_t *val) {
  systemSettings.settings.displayResRatio= *val;
  setDisplayResRatio(systemSettings.settings.displayResRatio);
}
//=========================================================
#else
static void * getdimMode() {
  temp = systemSettings.settings.dim_mode;
  return &temp;
}
static void setdimMode(uint32_t *val) {
  systemSettings.settings.dim_mode = * val;
  update_display_menu();
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
#endif

//=========================================================
static void display_onEnter(screen_t *scr){
  comboResetIndex(Screen_display.current_widget);
}

static void display_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;


  //  [ DISPLAY COMBO ]
  //
  newWidget(&w,widget_combo,scr);

  //  [ Contrast Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_ContrastOrBrightness, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &getDisplayContrastOrBrightness_;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setDisplayContrast_;
#ifdef ST7565
  edit->max_value = 0x3f;
#else
  edit->max_value = 10;
#endif
  edit->min_value = 0;

  //  [ Display Offset Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_Offset, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getdisplayOffset;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdisplayOffset;
  edit->max_value = 15;
  edit->min_value = 0;

  //  [ Display X flip Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Xflip, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdisplayXflip;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdisplayXflip;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Display Y flip Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Yflip, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdisplayYflip;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdisplayYflip;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

#ifdef ST7565
  //  [ Display Resistor ratio Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_Ratio, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getdisplayResRatio;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdisplayResRatio;
  edit->max_value = 7;
  edit->min_value = 1;
#else
  //  [ Display dimming Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Dim, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdimMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setdimMode;
  edit->options = strings[lang].dimMode;
  edit->numberOfOptions = 3;

  //  [ Display dim delay Widget ]
  //
  newComboEditable(w, strings[lang].__Delay, &edit, &comboitem_display_Dim_Timeout);
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->endString="s";
  dis->getData = &getDimTimeout;
  edit->big_step = 10;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setDimTimeout;
  edit->max_value = 600;
  edit->min_value = 5;

  //  [ Display dim turn off Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Dim_inSleep, &edit, &comboitem_display_Dim_PowerOff);
  dis=&edit->inputData;
  dis->getData = &getDimTurnOff;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setDimTurnOff;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif
  newComboScreen(w, strings[lang]._BACK, screen_system, NULL);
  update_display_menu();
}



void display_screen_setup(screen_t *scr){

  scr->onEnter = &display_onEnter;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &display_create;
}
