/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "debug_screen.h"
#include "screen_common.h"

#ifdef ENABLE_DEBUG_SCREEN

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#endif
#define __BASE_FILE__ "debug_screen.c"

screen_t Screen_debug;
screen_t Screen_pid_debug;

#define PID_SZ  95
typedef struct{
  struct {
    uint8_t p[PID_SZ];
    uint8_t i[PID_SZ];
    uint8_t d[PID_SZ];
    uint8_t index;
    uint8_t i_scale;
  }pidPlot;
  uint8_t update, update_draw;
  int32_t debug_temp;
}dbgScrData_t;

dbgScrData_t * dbgScrData;
widget_t *widget_setPoint;
widget_t *widget_Temp;




//=========================================================

static int32_t clampValues(int32_t val){
  if(val>=10000){
    val=9999;
  }
  else if(val<=-10000){
    val=-9999;
  }
  return val;
}

//=========================================================
static void * getTemp() {
  static int32_t value;
  if(dbgScrData->update){
    value = readLastTipTemperatureCompensated(read_average, getSystemTempUnit());
  }
  temp=value;
  return &temp;
}
//=========================================================
static void setSetpoint(uint32_t *val) {
  dbgScrData->debug_temp=*val;
  setUserTemperature(dbgScrData->debug_temp);
}
static void * getSetpoint() {
  return &dbgScrData->debug_temp;
}
//=========================================================
static void * get_PID_P() {
  static int32_t value;
  if(dbgScrData->update){
    value=clampValues(getPID_P()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PID_I() {
  static int32_t value;
  if(dbgScrData->update){
    value=clampValues(getPID_I()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PID_D() {
  static int32_t value;
  if(dbgScrData->update){
    value=clampValues(getPID_D()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_AVG() {
  static int32_t value;
  if(dbgScrData->update){
    value=TIP.last_avg;
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_RAW() {
  static int32_t value;
  if(dbgScrData->update){
    value=TIP.last_raw;
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_SET() {
  static int32_t value;
  if(dbgScrData->update){
    value = human2adc(dbgScrData->debug_temp);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_ERR() {
  static int32_t value;
  if(dbgScrData->update){
    value=getPID_Error();
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PWM() {
  static int32_t value;
  if(dbgScrData->update){
    value=getIronPwmOutValue();
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PWR() {
  static int32_t value;
  if(dbgScrData->update){
    value=getCurrentPower();
  }
  temp=value;
  return &temp;
}
//=========================================================
static uint8_t scalePlot(int32_t val){
  if(val>20)
    val=20;
  else if(val<0)
    val=0;
  return (uint8_t)val;
}

void updatePIDplot(void){
  if(dbgScrData->update){
    dbgScrData->pidPlot.p[dbgScrData->pidPlot.index] = scalePlot((getPID_P()*10)+10);
    dbgScrData->pidPlot.i[dbgScrData->pidPlot.index] = scalePlot(getPID_I()*dbgScrData->pidPlot.i_scale);
    dbgScrData->pidPlot.d[dbgScrData->pidPlot.index] = scalePlot((getPID_D()*10)+10);
    if(++dbgScrData->pidPlot.index > (PID_SZ-1))
      dbgScrData->pidPlot.index=0;
  }
}


int debug_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {

  dbgScrData->update=update_GUI_Timer();
  dbgScrData->update_draw |= dbgScrData->update;
  updatePlot();
  updatePIDplot();

  wakeOledDim();                                         // Prevent display dim
  handleOledDim();
  updateScreenTimer(input);
  setCurrentMode(mode_run);                                 // Prevent mode timeout

  if(input==Rotate_Decrement || input==Rotate_Increment ){
    return (default_screenProcessInput(scr, input, state));
  }
  if(input==LongClick){
    return screen_main;
  }
  if(checkScreenTimer(300000)){   // 5 min timeout
    setCurrentMode(mode_sleep);
    return screen_main;
  }
  else if(input==Click){
    if(scr==&Screen_debug){
      return screen_pid_debug;
    }
    else{
      return screen_debug;
    }
  }
  else if(input==Rotate_Decrement_while_click){
    return screen_settings;
  }
  return -1;
}

static uint8_t debug_Draw(screen_t *scr){
  if(scr->state==screen_Erased){
    u8g2_SetFont(&u8g2, u8g2_font_small);
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_DrawUTF8(&u8g2, 0, 0, "P");
    u8g2_DrawUTF8(&u8g2, 0, 11, "I");
    u8g2_DrawUTF8(&u8g2, 0, 22, "D");
    u8g2_DrawUTF8(&u8g2, 64, 0, "AVG");
    u8g2_DrawUTF8(&u8g2, 64, 11, "RAW");
    u8g2_DrawUTF8(&u8g2, 64, 22, "SET");
    u8g2_DrawUTF8(&u8g2, 64, 33, "ERR");
    u8g2_DrawUTF8(&u8g2, 64, 44, "PWM");
    u8g2_DrawUTF8(&u8g2, 64, 55, "PWR");
  }
  return (default_screenDraw(scr));
}

static void debug_onEnter(screen_t *scr){

  editable_widget_t *edit = extractEditablePartFromWidget(widget_setPoint);
  displayOnly_widget_t *dis = extractDisplayPartFromWidget(widget_Temp);

  if(scr!=&Screen_debug && scr!=&Screen_pid_debug ){                             // Coming from system, allocate data
    dbgScrData = _calloc(1, sizeof(dbgScrData_t));
    if(!dbgScrData){
      Error_Handler();
    }
  }
  if(getSystemTempUnit()==mode_Celsius){
    edit->max_value = 490;
    edit->min_value = 0;
    edit->big_step = 20;
    edit->step = 5;
    edit->inputData.endString="\260C";
    dis->endString="\260C";
  }
  else{
    edit->max_value = 910;
    edit->min_value = 0;
    edit->big_step = 50;
    edit->step = 10;
    edit->inputData.endString="\260F";
    dis->endString="\260F";
  }

  if(scr==&Screen_settings){
    backupMode=getCurrentMode();
    backupTemp=getUserTemperature();

    if(getSystemTempUnit()==mode_Celsius){
      dbgScrData->debug_temp=0;
    }
    else{
      dbgScrData->debug_temp=0;
    }
    setUserTemperature(dbgScrData->debug_temp);

    dbgScrData->pidPlot.index=0;

    if(pid.limMaxInt>=1.0f){
      dbgScrData->pidPlot.i_scale = 20;
    }
    else{
      dbgScrData->pidPlot.i_scale = (float)20/pid.limMaxInt;
    }
  }
}

static void debug_onExit(screen_t *scr){
  if(scr!=&Screen_debug && scr!=&Screen_pid_debug){
    _free(dbgScrData);
    setUserTemperature(backupTemp);
    setCurrentMode(backupMode);
  }
}


static void debug_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t* dis;
  editable_widget_t *edit;

  //  [ PID P Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posX= 12;
  w->width=34;

  //  [ PID I Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_I;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posX= 12;
  w->posY= 11;
  w->width=34;

  //  [ PID D Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_D;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posX= 12;
  w->posY= 22;
  w->width=34;

  //  [ Current temp Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  widget_Temp = w;
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_center;
  dis->getData = &getTemp;
  dis->reservedChars=6;
  dis->font=u8g2_font_small;
  w->posX= 10;
  w->posY= 35;
  w->width=32;

  //  [ Setpoint adjust Widget ]
  //
  newWidget(&w, widget_editable,scr,NULL);
  widget_setPoint=w;
  edit=extractEditablePartFromWidget(w);
  dis=extractDisplayPartFromWidget(w);
  dis= &edit->inputData;
  dis->getData = &getSetpoint;
  dis->reservedChars=5;
  w->posY = 48;
  w->width = 46;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (setterFn)&setSetpoint;
  edit->selectable.tab=0;
  edit->selectable.state=widget_edit;

  //  [ Average Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_AVG;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->width=30;

  //  [ Raw Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_RAW;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 11;
  w->width=30;

  //  [ ADC Setpoint Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_SET;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 22;
  w->width=30;

  //  [ PID Error Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_ERR;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 33;
  w->width=30;

  //  [ PWM value Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PWM;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 44;
  w->width=30;

  //  [ Power % Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PWR;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 55;
  w->width=30;

  scr->current_widget = widget_setPoint;

}

static void pid_debug_create(screen_t *scr){
  widget_t *w;
  editable_widget_t *edit;
  displayOnly_widget_t* dis;

  //  [ PID P Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->width=32;

  //  [ PID I Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_I;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 11;
  w->width=32;

  //  [ PID D Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_D;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 22;
  w->width = 32;

  //  [ Current temp Widget ]
  //
  newWidget(&w, widget_display,scr,NULL);
  widget_Temp = w;
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &getTemp;
  dis->reservedChars=6;
  dis->font=u8g2_font_small;
  w->posY= 35;
  w->width=32;

  //  [ Setpoint adjust Widget ]
  //
  newWidget(&w, widget_editable,scr,NULL);
  widget_setPoint=w;
  edit=extractEditablePartFromWidget(w);
  dis=extractDisplayPartFromWidget(w);
  dis= &edit->inputData;
  dis->getData = &getSetpoint;
  dis->reservedChars=5;
  w->posY = 48;
  w->width = 46;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (setterFn)&setSetpoint;
  edit->selectable.tab=0;
  edit->selectable.state=widget_edit;
}

static uint8_t pid_debug_Draw(screen_t * scr){
  if(dbgScrData->update_draw || scr->state==screen_Erased){
    if(dbgScrData->update_draw){
      dbgScrData->update_draw=0;
      fillBuffer(BLACK, fill_dma);
      scr->state=screen_Erased;
    }
    for(uint8_t x=1; x<(PID_SZ-1); x++){
      uint8_t pos=dbgScrData->pidPlot.index+x;
      uint8_t prev;
      if(pos>(PID_SZ-1)){
        pos-=PID_SZ;
      }
      if(!pos){
        prev=PID_SZ-1;
      }
      else{
        prev=pos-1;
      }
      u8g2_SetDrawColor(&u8g2, WHITE);
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 20-dbgScrData->pidPlot.p[prev], x+(displayWidth-PID_SZ+1), 20-dbgScrData->pidPlot.p[pos]);
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 42-dbgScrData->pidPlot.i[prev], x+(displayWidth-PID_SZ+1), 42-dbgScrData->pidPlot.i[pos]);
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 63-dbgScrData->pidPlot.d[prev], x+(displayWidth-PID_SZ+1), 63-dbgScrData->pidPlot.d[pos]);
    }
  }
  return (default_screenDraw(scr));
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
  scr->create = &debug_create;
  scr->onEnter = &debug_onEnter;
  scr->onExit = &debug_onExit;
  scr->processInput = &debug_ProcessInput;
  scr->draw = &debug_Draw;

  scr=&Screen_pid_debug;
  oled_addScreen(scr, screen_pid_debug);
  scr->processInput=&debug_ProcessInput;
  scr->onEnter = &debug_onEnter;
  scr->create=&pid_debug_create;
  scr->draw=&pid_debug_Draw;
  scr->onExit = &debug_onExit;
}

#endif
