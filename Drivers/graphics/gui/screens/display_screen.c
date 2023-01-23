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

#ifdef SSD1306
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

#elif defined ST7565
//=========================================================
static void * getdisplayResRatio() {
  temp = systemSettings.settings.displayResRatio;
  return &temp;
}
static void setdisplayResRatio(uint32_t *val) {
  systemSettings.settings.displayResRatio= *val;
  setDisplayResRatio(systemSettings.settings.displayResRatio);
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
#ifdef SSD1306
  newComboScreen(w, strings[lang].DISPLAY_Advanced, screen_display_adv, NULL);
#endif
  newComboScreen(w, strings[lang]._BACK, screen_system, NULL);
  update_display_menu();
}

void display_screen_setup(screen_t *scr){
  scr->onEnter = &display_onEnter;
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
  clk = defaultSettings.displayClk;
  precharge = defaultSettings.displayPrecharge;
  vcom = defaultSettings.displayVcom;
  setDisplayClk(clk);
  setDisplayPrecharge(precharge);
  setDisplayVcom(vcom);
  w->refresh = refresh_triggered;
  return -1;
}
//=========================================================
static int display_adv_save(widget_t *w, RE_Rotation_t input) {
  systemSettings.settings.displayClk = clk;
  systemSettings.settings.displayPrecharge = precharge;
  systemSettings.settings.displayVcom = vcom;
  return last_scr;
}
//=========================================================
static int display_adv_cancel(widget_t *w, RE_Rotation_t input) {
  setDisplayClk(systemSettings.settings.displayClk);
  setDisplayPrecharge(systemSettings.settings.displayPrecharge);
  setDisplayVcom(systemSettings.settings.displayVcom);
  return last_scr;
}
//=========================================================
static void display_adv_init(screen_t *scr){
  comboResetIndex(scr->current_widget);
  clk = systemSettings.settings.displayClk;
  precharge = systemSettings.settings.displayPrecharge;
  vcom = systemSettings.settings.displayVcom;
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
  edit->setData = (void (*)(void *))&setdisplayClk;

  //  [ Display Precharge Widget ]
  //
  newComboEditableString(w, "PRE", &edit, NULL, bf);
  dis=&edit->inputData;;
  dis->type = field_hex;
  dis->reservedChars=2;
  dis->displayString = bf;
  dis->getData = &getdisplayPrecharge;
  edit->setData = (void (*)(void *))&setdisplayPrecharge;

  //  [ Display Vcom Widget ]
  //
  newComboEditableString(w, "VCOM", &edit, NULL, bf);
  dis=&edit->inputData;;
  dis->type = field_hex;
  dis->reservedChars=2;
  dis->displayString = bf;
  dis->getData = &getdisplayVcom;
  edit->setData = (void (*)(void *))&setdisplayVcom;

  newComboAction(w, strings[lang]._RESET, &display_adv_reset, NULL);
  newComboAction(w, strings[lang]._SAVE, &display_adv_save, NULL);
  newComboAction(w, strings[lang]._CANCEL, &display_adv_cancel, NULL);
}

void display_screen_adv_setup(screen_t *scr){
  scr->init = &display_adv_init;
  scr->processInput=&autoReturn_ProcessInput;
  scr->create = &display_adv_create;
}
#endif
