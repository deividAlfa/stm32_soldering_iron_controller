/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */


#include "debug_screen.h"
#include "screen_common.h"


#ifdef ENABLE_DEBUG_SCREEN
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen variables
//-------------------------------------------------------------------------------------------------------------------------------
int32_t temp;
int32_t debugTemperature = 0;
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets
//-------------------------------------------------------------------------------------------------------------------------------

screen_t Screen_debug;
screen_t Screen_debug2;


//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------
static void * debug_screen_getADC1() {
  temp = TIP.last_avg;
  return &temp;
}
static void * debug_screen_getADC1_raw() {
  temp = TIP.last_raw;
  return &temp;
}

static void * debug_screen_getIronPower() {
  //if(UpdateReadings){
    temp = getCurrentPower();
  //}
  return &temp;
}
static void * getDebugTemperature() {
  return &debugTemperature;
}
static void setDebugTemperature(uint16_t *val) {
  debugTemperature = *val;
  resetPID();
  setDebugTemp(debugTemperature);
}
static void * getCalcAt250() {
  temp = getCurrentTip()->calADC_At_250;
  return &temp;
}
static void setCalcAt250(uint16_t *val) {
  getCurrentTip()->calADC_At_250 = *val;
}
static void * getCalcAt350() {
  temp = getCurrentTip()->calADC_At_350;
  return &temp;
}
static void setCalcAt350(uint16_t *val) {
  getCurrentTip()->calADC_At_350 = *val;
}
static void * getCalcAt450() {
  temp = getCurrentTip()->calADC_At_450;
  return &temp;
}
static void setCalcAt450(uint16_t *val) {
  getCurrentTip()->calADC_At_450 = *val;
}


//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen functions
//-------------------------------------------------------------------------------------------------------------------------------
int debug_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  if(input==LongClick){
                                                  return screen_debug2;
  }
  return (default_screenProcessInput(scr, input, state));
}

int debug2_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  if(input==LongClick){
    return screen_main;
  }
  return (default_screenProcessInput(scr, input, state));
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screenDraw(screen_t *scr){
  static uint32_t time=0;
  if(current_time-time > (systemSettings.settings.guiUpdateDelay*100)){
    char str[16];
    time = current_time;
    u8g2_SetFont(&u8g2,default_font  );
    u8g2_SetDrawColor(&u8g2, WHITE);
    FillBuffer(BLACK,fill_dma);

    sprintf(str, "ADC %u", TIP.last_avg);
    u8g2_DrawStr(&u8g2,65,0,str);

    sprintf(str, "RAW %u", TIP.last_raw);
    u8g2_DrawStr(&u8g2,65,16,str);

    sprintf(str, "PWM %lu", Iron.Pwm_Out);
    u8g2_DrawStr(&u8g2,60,50,str);

    sprintf(str, "ERR %ld", (int32_t)getPID_Error());
    u8g2_DrawStr(&u8g2,65,33,str);

    sprintf(str, "P %ld", (int32_t)(getPID_P()* 1000));
    u8g2_DrawStr(&u8g2,0,0,str);

    sprintf(str, "I %ld", (int32_t)(getPID_I()* 1000));
    u8g2_DrawStr(&u8g2,0,16,str);

    sprintf(str, "D %04ld", (int32_t)(getPID_D()* 1000));
    u8g2_DrawStr(&u8g2,0,33,str);
  }
}
static void debug_init(screen_t *scr){

  default_init(scr);
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void debug2_onExit(screen_t *scr) {
  setDebugMode(RESET);
}
static void debug2_onEnter(screen_t *scr) {
  setDebugMode(SET);
  u8g2_SetFont(&u8g2,default_font );
  u8g2_SetDrawColor(&u8g2, WHITE);
  u8g2_DrawStr(&u8g2,0,2,"SetP");
  u8g2_DrawStr(&u8g2,0,18,"C250");
  u8g2_DrawStr(&u8g2,0,34,"C350");
  u8g2_DrawStr(&u8g2,0,50,"C450");
}
static void debug2_init(screen_t *scr){

  widget_t *w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //power display
  newWidget(&w, widget_display, scr);
  dis=extractDisplayPartFromWidget(w);
  dis->endString="%";
  dis->reservedChars=4;
  w->posX = 92;
  w->posY = 50;
  w->width = 32;
  dis->getData = &debug_screen_getIronPower;
  dis->textAlign = align_right;

  //ADC1 display, filtered
  newWidget(&w, widget_display, scr);
  dis=extractDisplayPartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 92;
  w->posY = 0;
  w->width = 32;
  dis->getData = &debug_screen_getADC1;
  dis->textAlign = align_right;

  //ADC1 display, unfiltered
  newWidget(&w, widget_display, scr);
  dis=extractDisplayPartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 92;
  w->posY = 16;
  w->width = 32;
  dis->getData = &debug_screen_getADC1_raw;
  dis->textAlign = align_right;

  //Debug setpoint
  newWidget(&w, widget_editable, scr);
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 36;
  w->posY = 0;
  w->width = 40;
  dis->getData = &getDebugTemperature;
  edit->big_step = 200;
  edit->step = 20;
  edit->max_value = 4095;
  edit->selectable.tab = 0;
  edit->setData = (void (*)(void *))&setDebugTemperature;


  // Cal at 250
  newWidget(&w, widget_editable, scr);
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 36;
  w->posY = 16;
  w->width = 40;
  dis->getData = &getCalcAt250;
  edit->big_step = 200;
  edit->step = 20;
  edit->max_value = 4095;
  edit->selectable.tab = 1;
  edit->setData = (void (*)(void *))&setCalcAt250;


  // Cal at 350
  newWidget(&w, widget_editable, scr);
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 36;
  w->posY = 32;
  w->width = 40;
  dis->getData = &getCalcAt350;
  edit->big_step = 200;
  edit->step = 20;
  edit->max_value = 4095;
  edit->selectable.tab = 2;
  edit->setData = (void (*)(void *))&setCalcAt350;


  // Cal at 450
  newWidget(&w, widget_editable, scr);
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=4;
  w->posX = 36;
  w->posY = 48;
  w->width=40;
  dis->getData = &getCalcAt450;
  edit->big_step = 200;
  edit->step = 20;
  edit->max_value = 4095;
  edit->selectable.tab = 3;
  edit->setData = (void (*)(void *))&setCalcAt450;
  default_init(scr);
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
  screen_setDefaults(scr);
  scr->processInput = &debug_screenProcessInput;
  scr->draw = &debug_screenDraw;
  scr->init = &debug_init;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug2_screen_setup(screen_t *scr) {
  screen_setDefaults(scr);
  scr->processInput = &debug2_screenProcessInput;
  scr->onEnter = &debug2_onEnter;
  scr->onExit = &debug2_onExit;
  scr->init = &debug2_init;
}

#endif
