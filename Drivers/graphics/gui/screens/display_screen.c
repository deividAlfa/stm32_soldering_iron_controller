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
screen_t Screen_display_adv;
static comboBox_item_t *comboitem_display_Dim_Timeout;
static comboBox_item_t *comboitem_display_Dim_PowerOff;
static char bf[3];
static uint8_t clk, precharge, vcom;
#endif


void update_display_menu(void){
#ifndef ST7565
  bool mode = (getSystemSettings()->dim_mode>dim_off);
  comboitem_display_Dim_PowerOff->enabled = mode;
  comboitem_display_Dim_Timeout->enabled = mode;
#endif
}

//=========================================================
static void * getDisplayContrastOrBrightness_() {
#ifdef ST7565
  temp = getSystemSettings()->contrastOrBrightness;
#else
  temp = getSystemSettings()->contrastOrBrightness/25;
#endif
  return &temp;
}
static void setDisplayContrast_(uint32_t *val) {
#ifdef ST7565
  getSystemSettings()->contrastOrBrightness=*val;
#else
  if(*val==0){
    getSystemSettings()->contrastOrBrightness=5;
  }
  else if(*val==10){
    getSystemSettings()->contrastOrBrightness=255;
  }
  else{
    getSystemSettings()->contrastOrBrightness=*val*25;
  }
#endif
  setDisplayContrastOrBrightness(getSystemSettings()->contrastOrBrightness);
}
//=========================================================
static void * getdisplayStartColumn() {
  temp = getSystemSettings()->displayStartColumn;
  return &temp;
}
static void setdisplayStartColumn(uint32_t *val) {
  getSystemSettings()->displayStartColumn= *val;
}
//=========================================================
static void * getdisplayStartLine() {
  temp = getSystemSettings()->displayStartLine;
  return &temp;
}
static void setdisplayStartLine(uint32_t *val) {
  getSystemSettings()->displayStartLine= *val;
  setDisplayStartLine(getSystemSettings()->displayStartLine);
}
//=========================================================
static void * getdisplayXflip() {
  temp = getSystemSettings()->displayXflip;
  return &temp;
}
static void setdisplayXflip(uint32_t *val) {
  getSystemSettings()->displayXflip= *val;
  setDisplayXflip(getSystemSettings()->displayXflip);
}
//=========================================================
static void * getdisplayYflip() {
  temp = getSystemSettings()->displayYflip;
  return &temp;
}
static void setdisplayYflip(uint32_t *val) {
  getSystemSettings()->displayYflip= *val;
  setDisplayYflip(getSystemSettings()->displayYflip);
}

#ifdef SSD1306
static void * getdimMode() {
  temp = getSystemSettings()->dim_mode;
  return &temp;
}
static void setdimMode(uint32_t *val) {
  getSystemSettings()->dim_mode = * val;
  update_display_menu();
}
//=========================================================
static void * getDimTimeout() {
  temp = getSystemSettings()->dim_Timeout/1000;
  return &temp;
}
static void setDimTimeout(uint32_t *val) {
  getSystemSettings()->dim_Timeout = *val*1000;
}
//=========================================================
static void * getDimTurnOff() {
  temp = getSystemSettings()->dim_inSleep;
  return &temp;
}
static void setDimTurnOff(uint32_t *val) {
  getSystemSettings()->dim_inSleep = *val;
}

#elif defined ST7565
//=========================================================
static void * getdisplayResRatio() {
  temp = getSystemSettings()->displayResRatio;
  return &temp;
}
static void setdisplayResRatio(uint32_t *val) {
  getSystemSettings()->displayResRatio= *val;
  setDisplayResRatio(getSystemSettings()->displayResRatio);
}

#endif

static void display_onEnter(screen_t *scr){
#ifndef ST7565
  if(scr != &Screen_display_adv){
    comboResetIndex(Screen_display.current_widget);
  }
#else
  comboResetIndex(Screen_display.current_widget);
#endif
}

static void display_onExit(screen_t *scr){
#if defined ST7565
    if((scr != &Screen_display) && (scr != &Screen_system) && isSystemSettingsChanged())
#else
    if((scr != &Screen_display) && (scr != &Screen_display_adv) && (scr != &Screen_system) && isSystemSettingsChanged())    // Going to main screen?
#endif
    saveSettings(save_settings, no_reboot);                                                                               // Save
}

static void display_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ DISPLAY COMBO ]
  //
  newWidget(&w,widget_combo,scr,NULL);

  //  [ Contrast Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_ContrastOrBrightness, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=3;
  dis->getData = &getDisplayContrastOrBrightness_;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setDisplayContrast_;
#ifdef ST7565
  edit->max_value = 0x3f;
#else
  edit->max_value = 10;
#endif
  edit->min_value = 0;

  //  [ Display Column Offset Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_StartColumn, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getdisplayStartColumn;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setdisplayStartColumn;
  edit->max_value = 15;
  edit->min_value = 0;

  //  [ Display Line Offset Widget ]
  //
  newComboEditable(w, strings[lang].DISPLAY_StartLine, &edit, NULL);
  dis=&edit->inputData;
  dis->reservedChars=2;
  dis->getData = &getdisplayStartLine;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setdisplayStartLine;
  edit->max_value = 63;
  edit->min_value = 0;

  //  [ Display X flip Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Xflip, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdisplayXflip;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setdisplayXflip;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Display Y flip Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Yflip, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getdisplayYflip;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setdisplayYflip;
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
  edit->setData = (setterFn)&setdisplayResRatio;
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
  edit->setData = (setterFn)&setdimMode;
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
  edit->setData = (setterFn)&setDimTimeout;
  edit->max_value = 600;
  edit->min_value = 5;

  //  [ Display dim turn off Widget ]
  //
  newComboMultiOption(w, strings[lang].DISPLAY_Dim_inSleep, &edit, &comboitem_display_Dim_PowerOff);
  dis=&edit->inputData;
  dis->getData = &getDimTurnOff;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setDimTurnOff;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif
#ifdef SSD1306
  newComboScreen(w, strings[lang].DISPLAY_Advanced, screen_display_adv, NULL);
#endif
  newComboScreen(w, strings[lang]._BACK, screen_system, NULL);
  update_display_menu();
}

void display_screen_setup(screen_t *scr){
  scr->onEnter = &display_onEnter;
  scr->onExit= &display_onExit;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &display_create;
}

#ifdef SSD1306
const char *hex = "0123456789ABCDEF";
static char * int2hex(uint8_t h){
  bf[0] = hex[h>>4];
  bf[1] = hex[h&0xF];
  bf[2] = '\0';
  return bf;
}
static uint8_t hex2int(char *s){
  uint8_t i;
  i  = (s[0]-'0'-(s[0]>'9' ? 7 : 0))<<4;
  i |= s[1]-'0'-(s[1]>'9' ? 7 : 0);
  return i;
}
//=========================================================
static void * getdisplayClk() {
  return int2hex(clk);
}
static void setdisplayClk(char *s) {
  clk = hex2int(s);
  setDisplayClk(clk);
}
//=========================================================
static void * getdisplayVcom() {
  return int2hex(vcom);
}
static void setdisplayVcom(char *s) {
  vcom = hex2int(s);
  setDisplayVcom(vcom);
}
//=========================================================
static void * getdisplayPrecharge() {
  return int2hex(precharge);
}
static void setdisplayPrecharge(char *s) {
  precharge = hex2int(s);
  setDisplayPrecharge(precharge);
}
//=========================================================
static int display_adv_reset(widget_t *w, RE_Rotation_t input) {
  clk = getDefaultSystemSettings()->displayClk;
  precharge = getDefaultSystemSettings()->displayPrecharge;
  vcom = getDefaultSystemSettings()->displayVcom;
  setDisplayClk(clk);
  setDisplayPrecharge(precharge);
  setDisplayVcom(vcom);
  w->refresh = refresh_triggered;
  return -1;
}
//=========================================================
static int display_adv_save(widget_t *w, RE_Rotation_t input) {
  getSystemSettings()->displayClk = clk;
  getSystemSettings()->displayPrecharge = precharge;
  getSystemSettings()->displayVcom = vcom;
  return last_scr;
}
//=========================================================
static int display_adv_cancel(widget_t *w, RE_Rotation_t input) {
  setDisplayClk(getSystemSettings()->displayClk);
  setDisplayPrecharge(getSystemSettings()->displayPrecharge);
  setDisplayVcom(getSystemSettings()->displayVcom);
  return last_scr;
}
//=========================================================
static void display_adv_init(screen_t *scr){
  comboResetIndex(scr->current_widget);
  clk = getSystemSettings()->displayClk;
  precharge = getSystemSettings()->displayPrecharge;
  vcom = getSystemSettings()->displayVcom;
}
//=========================================================
static void display_adv_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ DISPLAY COMBO ]
  //
  newWidget(&w,widget_combo,scr,NULL);
  //  [ Display Clk  Widget ]
  //
  newComboEditableString(w, "CLK", &edit, NULL, bf);
  dis=&edit->inputData;
  dis->type = field_hex;
  dis->reservedChars=2;
  dis->displayString = bf;
  dis->getData = &getdisplayClk;
  edit->setData = (setterFn)&setdisplayClk;

  //  [ Display Precharge Widget ]
  //
  newComboEditableString(w, "PRE", &edit, NULL, bf);
  dis=&edit->inputData;;
  dis->type = field_hex;
  dis->reservedChars=2;
  dis->displayString = bf;
  dis->getData = &getdisplayPrecharge;
  edit->setData = (setterFn)&setdisplayPrecharge;

  //  [ Display Vcom Widget ]
  //
  newComboEditableString(w, "VCOM", &edit, NULL, bf);
  dis=&edit->inputData;;
  dis->type = field_hex;
  dis->reservedChars=2;
  dis->displayString = bf;
  dis->getData = &getdisplayVcom;
  edit->setData = (setterFn)&setdisplayVcom;

  newComboAction(w, strings[lang]._RESET, &display_adv_reset, NULL);
  newComboAction(w, strings[lang]._SAVE, &display_adv_save, NULL);
  newComboAction(w, strings[lang]._CANCEL, &display_adv_cancel, NULL);
}

void display_screen_adv_setup(screen_t *scr){
  scr->init = &display_adv_init;
  scr->onExit= &display_onExit;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &display_adv_create;
}
#endif
