/*
 * pid_debug_screen.c
 *
 *  Created on: 30 jul. 2021
 *      Author: David
 */


#include "pid_debug_screen.h"
#include "screen_common.h"
#include "pid.h"

#ifdef ENABLE_PID_DEBUG_SCREEN

screen_t Screen_pid_debug;
#define PID_SZ  95
typedef struct {
  uint8_t p[PID_SZ];
  uint8_t i[PID_SZ];
  uint8_t d[PID_SZ];
  //uint8_t o[100];
}pid_plot_t;

static uint8_t plot_Index;
static uint8_t plotUpdate;
static pid_plot_t *PIDplotData;
uint8_t _i;


static void * get_PID_P() {
  temp=getPID_P()*1000;
  return &temp;
}

static void * get_PID_I() {
  temp=getPID_I()*1000;
  return &temp;
}

static void * get_PID_D() {
  temp=getPID_D()*1000;
  return &temp;
}

static uint8_t scalePlot(int32_t val){
  val+=10;
  if(val>21){
    val=21;
  }
  else if(val<0){
    val=0;
  }
  return (uint8_t)val;
}

static int pid_debug_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){
  static uint32_t plotTime=0;
  uint32_t currentTime=HAL_GetTick();

  if((currentTime-plotTime)>=((systemSettings.Profile.readPeriod+1)/200)){
    plotTime=currentTime;
    PIDplotData->p[plot_Index] = scalePlot(getPID_P()* 11);
    PIDplotData->i[plot_Index] = getPID_I()* _i;
    PIDplotData->d[plot_Index] = scalePlot(getPID_D()* 11);
    //PIDplotData->o[plot_Index] = getPID_Output()* 10;// 0...1
    if(++plot_Index>(PID_SZ-1)){
      plot_Index=0;
    }
    plotUpdate=1;
  }
  if(input==LongClick){
    return screen_settings;
  }
  return -1;
}



static void pid_debug_draw(screen_t * scr){
  if(!plotUpdate){
    return;
  }
  plotUpdate=0;
  FillBuffer(BLACK, fill_dma);
  scr->refresh=screen_Erased;
  default_screenDraw(scr);
  for(uint8_t x=1; x<(PID_SZ-1); x++){
    uint8_t pos=plot_Index+x;
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
    u8g2_DrawLine(&u8g2, x+(OledWidth-PID_SZ), 21-PIDplotData->p[prev], x+(OledWidth-PID_SZ+1), 21-PIDplotData->p[pos]);
    u8g2_DrawLine(&u8g2, x+(OledWidth-PID_SZ), 42-PIDplotData->i[prev], x+(OledWidth-PID_SZ+1), 42-PIDplotData->i[pos]);
    u8g2_DrawLine(&u8g2, x+(OledWidth-PID_SZ), 63-PIDplotData->d[prev], x+(OledWidth-PID_SZ+1), 63-PIDplotData->d[pos]);

    //u8g2_DrawLine(&u8g2, x-1, 63-plotData->o[prev], x, 63-plotData->o[pos]);
    /*
    u8g2_DrawPixel(&u8g2, 13+x, 56-plotData->p[pos]);
    u8g2_DrawPixel(&u8g2, 13+x, 56-plotData->i[pos]);
    u8g2_DrawPixel(&u8g2, 13+x, 56-plotData->d[pos]);
    u8g2_DrawPixel(&u8g2, 13+x, 56-plotData->o[pos]);
    */
  }
  /*
  u8g2_SetFont(&u8g2, default_font);
  uint8_t h=u8g2_GetMaxCharHeight(&u8g2);
  u8g2_DrawStr(&u8g2, 0, 21-h, "P");
  u8g2_DrawStr(&u8g2, 0, 42-h, "I");
  u8g2_DrawStr(&u8g2, 0, 63-h, "D");
  u8g2_SetFont(&u8g2, u8g2_font_labels);
  h=u8g2_GetMaxCharHeight(&u8g2);
  char str[16];
  sprintf(str,"%-6.3f",getPID_P());
  u8g2_DrawStr(&u8g2, 9, 21-h, str);

  sprintf(str,"%-6.3f",getPID_I());
  u8g2_DrawStr(&u8g2, 9, 42-h, str);

  sprintf(str,"-%-6.3f",getPID_D());
  u8g2_DrawStr(&u8g2, 9, 63-h, str);
  */
}


static void pid_debug_init(screen_t *scr) {
  default_init(scr);
  if(pid.limMaxInt>=1.0f){
    _i = 21;
  }
  else{
    _i = (float)21/pid.limMaxInt;
  }
}


static void pid_debug_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t* dis;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &get_PID_P;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_labels;
  w->posY= 9;
  w->width=30;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &get_PID_I;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_labels;
  w->posY= 30;
  w->width=30;

  newWidget(&w, widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &get_PID_D;
  dis->reservedChars=6;
  dis->number_of_dec=3;
  dis->font=u8g2_font_labels;
  w->posY= 51;
  w->width=30;

  PIDplotData=calloc(1,sizeof(pid_plot_t));
}

static void pid_debug_onExit(screen_t *scr){
  free(PIDplotData);
}

void pid_debug_screen_setup(screen_t *scr){
  scr->init = &pid_debug_init;
  scr->processInput=&pid_debug_ProcessInput;
  scr->create=&pid_debug_create;
  scr->draw=&pid_debug_draw;
  scr->onExit=&pid_debug_onExit;
}

#endif
