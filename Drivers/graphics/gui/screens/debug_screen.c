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
#define __BASE_FILE__ "debug_screen.c"
#endif

screen_t Screen_debug;
screen_t Screen_pid_debug;

widget_t *widget_setPoint;
widget_t *widget_Temp;

static int32_t debug_temp;
static uint8_t update, update_draw;
#define PID_SZ  95
typedef struct {
  uint8_t p[PID_SZ];
  uint8_t i[PID_SZ];
  uint8_t d[PID_SZ];
  uint8_t index;
  uint8_t i_scale;
}pid_plot_t;

static pid_plot_t *pidPlot;


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
  if(update){
    value=readTipTemperatureCompensated(old_reading, read_average, systemSettings.settings.tempUnit);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void setSetpoint(uint32_t *val) {
  debug_temp=*val;
  setUserTemperature(debug_temp);
}
static void * getSetpoint() {
  return &debug_temp;
}
//=========================================================
static void * get_PID_P() {
  static int32_t value;
  if(update){
    value=clampValues(getPID_P()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PID_I() {
  static int32_t value;
  if(update){
    value=clampValues(getPID_I()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PID_D() {
  static int32_t value;
  if(update){
    value=clampValues(getPID_D()*1000);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_AVG() {
  static int32_t value;
  if(update){
    value=TIP.last_avg;
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_RAW() {
  static int32_t value;
  if(update){
    value=TIP.last_raw;
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_SET() {
  static int32_t value;
  if(update){
    value = human2adc(debug_temp);
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_ERR() {
  static int32_t value;
  if(update){
    value=getPID_Error();
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PWM() {
  static int32_t value;
  if(update){
    value=getIronPwmOutValue();
  }
  temp=value;
  return &temp;
}
//=========================================================
static void * get_PWR() {
  static int32_t value;
  if(update){
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
  if(update){
    pidPlot->p[pidPlot->index] = scalePlot((getPID_P()*10)+10);
    pidPlot->i[pidPlot->index] = scalePlot(getPID_I()*pidPlot->i_scale);
    pidPlot->d[pidPlot->index] = scalePlot((getPID_D()*10)+10);
    if(++pidPlot->index>(PID_SZ-1))
      pidPlot->index=0;
  }
}




int debug_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {

  update=update_GUI_Timer();
  update_draw |= update;
  updatePlot();
  updatePIDplot();

  wakeOledDim();                                         // Prevent display dim
  handleOledDim();
  updateScreenTimer(input);
  setCurrentMode(mode_run);                                 // Prevent mode timeout

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
  return (default_screenProcessInput(scr, input, state));
}

static uint8_t debug_Draw(screen_t *scr){
  if(scr->refresh==screen_Erased){
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


  if(systemSettings.settings.tempUnit==mode_Celsius){
    if(scr!=&Screen_debug){
      edit->max_value = 450;
      edit->min_value = 0;
      edit->big_step = 20;
      edit->step = 5;
      edit->inputData.endString="\260C";
    }
    dis->endString="\260C";
  }
  else{
    if(scr!=&Screen_debug){
      edit->max_value = 850;
      edit->min_value = 0;
      edit->big_step = 50;
      edit->step = 10;
      edit->inputData.endString="\260F";
    }
    dis->endString="\260F";
  }

  if(scr==&Screen_settings){
    backupMode=getCurrentMode();
    backupTemp=getUserTemperature();

    if(systemSettings.settings.tempUnit==mode_Celsius){
      debug_temp=0;
    }
    else{
      debug_temp=0;
    }
    setUserTemperature(debug_temp);

    pidPlot=_malloc(sizeof(pid_plot_t));
    pidPlot->index=0;
    if(!pidPlot){
      Error_Handler();
    }

    if(pid.limMaxInt>=1.0f){
      pidPlot->i_scale = 20;
    }
    else{
      pidPlot->i_scale = (float)20/pid.limMaxInt;
    }

    memset(pidPlot->p, scalePlot(getPID_P()* 10), PID_SZ);
    memset(pidPlot->i, scalePlot(getPID_I()* pidPlot->i_scale), PID_SZ);
    memset(pidPlot->d, scalePlot(getPID_D()* 10), PID_SZ);

  }
}

static void debug_onExit(screen_t *scr){
  if(scr!=&Screen_debug && scr!=&Screen_pid_debug){
    _free(pidPlot);
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
  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posX= 12;
  w->posY= 0;
  w->width=34;

  //  [ PID I Widget ]
  //
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
  widget_Temp = w;
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_center;
  dis->getData = &getTemp;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 10;
  w->posY= 35;
  w->width=30;

  //  [ Setpoint adjust Widget ]
  //
  newWidget(&w, widget_editable,scr);
  widget_setPoint=w;
  edit=extractEditablePartFromWidget(w);
  dis=extractDisplayPartFromWidget(w);
  dis= &edit->inputData;
  dis->getData = &getSetpoint;
  dis->reservedChars=5;
  w->posY = 48;
  w->posX = 0;
  w->width = 52;
  edit->big_step = 20;
  edit->step = 5;
  edit->setData = (void (*)(void *))&setSetpoint;
  edit->selectable.tab=0;
  edit->selectable.state=widget_edit;

  //  [ Average Widget ]
  //
  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_AVG;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 95;
  w->posY= 0;
  w->width=30;

  //  [ Raw Widget ]
  //
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
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
  newWidget(&w, widget_display,scr);
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
  displayOnly_widget_t* dis;

  //  [ Current temp Widget ]
  //
  newWidget(&w, widget_display,scr);
  widget_Temp = w;
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &getTemp;
  dis->reservedChars=5;
  dis->font=u8g2_font_small;
  w->posX= 0;
  w->posY= 0;
  w->width=32;

  //  [ PID P Widget ]
  //
  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 13;
  w->width=32;

  //  [ PID I Widget ]
  //
  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_I;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 28;
  w->width=32;

  //  [ PID D Widget ]
  //
  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->textAlign=align_right;
  dis->getData = &get_PID_D;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_small;
  w->posY= 48;
  w->width=32;
}

static uint8_t pid_debug_Draw(screen_t * scr){
  if(update_draw || scr->refresh==screen_Erased){
    if(update_draw){
      update_draw=0;
      fillBuffer(BLACK, fill_dma);
      scr->refresh=screen_Erased;
    }
    for(uint8_t x=1; x<(PID_SZ-1); x++){
      uint8_t pos=pidPlot->index+x;
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
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 20-pidPlot->p[prev], x+(displayWidth-PID_SZ+1), 20-pidPlot->p[pos]);
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 42-pidPlot->i[prev], x+(displayWidth-PID_SZ+1), 42-pidPlot->i[pos]);
      u8g2_DrawLine(&u8g2, x+(displayWidth-PID_SZ), 63-pidPlot->d[prev], x+(displayWidth-PID_SZ+1), 63-pidPlot->d[pos]);
    }
  }
  return (default_screenDraw(scr));
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
  screen_t *sc;
  screen_setDefaults(scr);
  scr->create = &debug_create;
  scr->onEnter = &debug_onEnter;
  scr->onExit = &debug_onExit;
  scr->processInput = &debug_ProcessInput;
  scr->draw = &debug_Draw;

  sc=&Screen_pid_debug;
  oled_addScreen(sc, screen_pid_debug);
  sc->processInput=&debug_ProcessInput;
  sc->onEnter = &debug_onEnter;
  sc->create=&pid_debug_create;
  sc->draw=&pid_debug_Draw;
  sc->onExit = &debug_onExit;
}

#endif
