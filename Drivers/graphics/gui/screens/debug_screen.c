/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */


#include "debug_screen.h"
#include "screen_common.h"
#include "widgets.h"

#ifdef ENABLE_DEBUG_SCREEN
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen variables
//-------------------------------------------------------------------------------------------------------------------------------
int32_t debug_temp = 0;
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets
//-------------------------------------------------------------------------------------------------------------------------------

screen_t Screen_debug;



static void setTemp(uint16_t *val) {
  debug_temp=*val;
  setDebugTemp(human2adc(temp));
}

static void * getTemp() {
  return &debug_temp;
}
static int32_t clampValues(int32_t val){
  if(val>=10000){
    val=9999;
  }
  else if(val<=-10000){
    val=-9999;
  }
  return val;
}
static void * get_PID_P() {
  temp=clampValues(getPID_P()*1000);
  return &temp;
}

static void * get_PID_I() {
  temp=clampValues(getPID_I()*1000);
  return &temp;
}

static void * get_PID_D() {
  temp=clampValues(getPID_D()*1000);
  return &temp;
}


int debug_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  if(input==LongClick){
    return screen_settings;
  }
  return (default_screenProcessInput(scr, input, state));
}

void debug_screenDraw(screen_t *scr){
  static uint32_t time=0;
  if(current_time-time > systemSettings.settings.guiUpdateDelay){
    char str[16];
    time = current_time;
    u8g2_SetFont(&u8g2, u8g2_font_small);
    u8g2_SetDrawColor(&u8g2, WHITE);
    FillBuffer(BLACK,fill_dma);
    scr->refresh=screen_Erased;

    #define y_step 11
    uint8_t yp=0;

    u8g2_DrawStr(&u8g2, 60, yp, "ADC");
    sprintf(str, "%u", TIP.last_avg);
    u8g2_DrawStr(&u8g2,90,yp,str);
    yp+=y_step;

    u8g2_DrawStr(&u8g2, 60, yp, "RAW");
    sprintf(str, "%u", TIP.last_raw);
    u8g2_DrawStr(&u8g2,90,yp,str);
    yp+=y_step;

    u8g2_DrawStr(&u8g2, 60, yp, "PWM");
    sprintf(str, "%lu", Iron.Pwm_Out);
    u8g2_DrawStr(&u8g2,90,yp,str);
    yp+=y_step;

    u8g2_DrawStr(&u8g2, 60, yp, "PWR");
    sprintf(str, "%u%%", getCurrentPower());
    u8g2_DrawStr(&u8g2,90,yp,str);
    yp+=y_step;

    u8g2_DrawStr(&u8g2, 60, yp, "ERR");
    sprintf(str, "%ld", (int32_t)getPID_Error());
    u8g2_DrawStr(&u8g2,90,yp,str);

    u8g2_DrawStr(&u8g2, 0, 0, "P");
    u8g2_DrawStr(&u8g2, 0, 11, "I");
    u8g2_DrawStr(&u8g2, 0, 22, "D");

    default_screenDraw(scr);
  }
}

static void enabledEnter(screen_t *scr){
  debug_temp=100;
  setDebugMode(enable);
  setDebugTemp(human2adc(debug_temp));
}

static void enabledExit(screen_t *scr){
  setDebugMode(disable);
}

static void debug_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t* dis;
  editable_widget_t *edit;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 0;
  w->posX= 12;
  w->width=30;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_I;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 11;
  w->posX= 12;
  w->width=30;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_D;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 22;
  w->posX= 12;
  w->width=30;

  newWidget(&w, widget_editable,scr);
  edit = w->content;
  dis= &edit->inputData;
  dis->getData = &getTemp;
  dis->reservedChars=3;
  dis->font=u8g2_font_small;
  w->posY = 48;
  w->posX = 0;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setTemp;
  edit->max_value = 450;
  edit->min_value = 0;
  edit->selectable.tab=0;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
  screen_setDefaults(scr);
  scr->create = &debug_create;
  scr->onEnter = &enabledEnter;
  scr->onExit = &enabledExit;
  scr->processInput = &debug_screenProcessInput;
  scr->draw = &debug_screenDraw;
}

#endif
