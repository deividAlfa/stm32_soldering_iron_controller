                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "main_screen.h"
#include "oled.h"
#include "gui.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen variables
//-------------------------------------------------------------------------------------------------------------------------------
typedef struct{
  uint16_t p[100];
}plotData_t;

plotData_t *plotData;
static uint8_t plot_Index;
static uint32_t plotTimer;
static bool plotUpdate;

static uint32_t barTime;

static int32_t temp;
static char *tipName[TipSize];
enum mode{  main_irontemp=0, main_disabled, main_ironstatus, main_setpoint, main_tipselect, main_setMode};
enum{ status_ok=0x20, status_error };
enum { temp_numeric, temp_graph };
const uint8_t shakeXBM[] ={
  9, 9,
  0x70, 0x00, 0x80, 0x00, 0x30, 0x01, 0x40, 0x01, 0x45, 0x01, 0x05, 0x00,
  0x19, 0x00, 0x02, 0x00, 0x1C, 0x00, };


const uint8_t tempXBM[] ={
  5, 9,
  0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x0E, 0x1F, 0x1F, 0x0E, };

#ifdef USE_VIN
const uint8_t voltXBM[] ={
  6, 9,
  0x20, 0x18, 0x0C, 0x06, 0x3F, 0x18, 0x0C, 0x06, 0x01, };
#endif

const uint8_t warningXBM[] ={
  9, 8,
  0x10, 0x00, 0x28, 0x00, 0x54, 0x00, 0x54, 0x00, 0x82, 0x00, 0x92, 0x00,
  0x01, 0x01, 0xFF, 0x01, };
/*
const uint8_t savingXBM[] ={
    9, 9,
    0x00, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0x82, 0x00,
    0xB2, 0x00, 0xB2, 0x00, 0x01, 0x00, };
*/
//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
screen_t Screen_main;

#ifdef USE_NTC
static widget_t *Widget_AmbTemp;
#endif
static widget_t *Widget_IronTemp;

static widget_t *Widget_TipSelect;

static widget_t *Widget_SetPoint;

static struct{
  uint8_t update;
  uint8_t lastPwr;
  uint8_t idle;
  uint8_t ActivityOn;
  int8_t dimDisplay;
  uint8_t ironStatus;
  uint8_t prevIronStatus;
  uint8_t setMode;
  uint8_t currentMode;
  uint8_t displayMode;
  uint8_t menuPos;
  int16_t lastTip;
  #ifdef USE_NTC
  int16_t lastAmb;
  #endif
  #ifdef USE_VIN
  uint16_t lastVin;
  #endif
  widget_t* Selected;
  uint32_t dimTimer;
  uint32_t drawTick;
  uint32_t idleTick;
  uint32_t inputBlockTime;
  uint32_t updateTick;
}mainScr;

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------


static void setTemp(uint16_t *val) {
  setUserTemperature(*val);
}

static void * getTemp() {
  temp = getUserTemperature();
  return &temp;
}


static void setTip(uint8_t *val) {
  if(systemSettings.Profile.currentTip != *val){        // Tip temp uses huge font that partially overlaps other widgets
    systemSettings.Profile.currentTip = *val;
    setCurrentTip(*val);
    Screen_main.refresh=screen_Erase;         // So, we must redraw the screen. Tip temp is drawed first, then the rest go on top.
  }
}

static void * getTip() {
  temp = systemSettings.Profile.currentTip;
  return &temp;
}

static void * main_screen_getIronTemp() {
  if(mainScr.update){
    mainScr.lastTip=readTipTemperatureCompensated(stored_reading,read_Avg);
  }
  temp=mainScr.lastTip;
  return &temp;
}

#ifdef USE_VIN
static void * main_screen_getVin() {
  if(mainScr.update){
    mainScr.lastVin = getSupplyVoltage_v_x10();
  }
  temp=mainScr.lastVin;
  return &temp;
}
#endif

#ifdef USE_NTC
static void * main_screen_getAmbTemp() {
  if(mainScr.update){
    mainScr.lastAmb = readColdJunctionSensorTemp_x10(stored_reading, systemSettings.settings.tempUnit);
  }
  temp=mainScr.lastAmb;
  return &temp;
}
#endif

static void updateIronPower() {
  static uint32_t stored=0;
  static uint32_t updateTim;
  if((HAL_GetTick()-updateTim)>19){
    updateTim = HAL_GetTick();
    int32_t tmpPwr = getCurrentPower();
    if(tmpPwr < 0){
      tmpPwr = 0 ;
    }
    tmpPwr = tmpPwr<<12;
    stored = ( ((stored<<3)-stored)+tmpPwr+(1<<11))>>3 ;
    tmpPwr = stored>>12;
    tmpPwr = (tmpPwr*205)>>8;
    mainScr.lastPwr=tmpPwr;
  }
}

static void setMainWidget(widget_t* w){
  selectable_widget_t* sel =extractSelectablePartFromWidget(w);
  mainScr.drawTick=HAL_GetTick();
  Screen_main.refresh=screen_Erase;
  widgetDisable(mainScr.Selected);
  mainScr.Selected=w;
  widgetEnable(w);
  Screen_main.current_widget=w;
  if(sel){
    sel->state=widget_edit;
    sel->previous_state=widget_selected;
  }
}

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void setMainScrTempUnit(void) {
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    ((displayOnly_widget_t*)Widget_IronTemp->content)->endString="\260F";      // \260 = ASCII dec. 176(°) in octal representation
    #ifdef USE_NTC
    ((displayOnly_widget_t*)Widget_AmbTemp->content)->endString="\260F";
    #endif
    ((editable_widget_t*)Widget_SetPoint->content)->inputData.endString="\260F";
  }
  else{
    ((displayOnly_widget_t*)Widget_IronTemp->content)->endString="\260C";

    #ifdef USE_NTC
    ((displayOnly_widget_t*)Widget_AmbTemp->content)->endString="\260C";
    #endif
    ((editable_widget_t*)Widget_SetPoint->content)->inputData.endString="\260C";
  }
}

// Ignore future input for specified amount of time
void blockInput(uint32_t time){
  mainScr.inputBlockTime = HAL_GetTick()+time;
}

void restore_contrast(void){
  if(getContrast() != systemSettings.settings.contrast){
    setContrast(systemSettings.settings.contrast);
  }
}

int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  int16_t contrast = getContrast();
  uint32_t currentTime = HAL_GetTick();
  uint8_t error = Iron.Error.Flags;
  uint8_t current_mode = getCurrentMode();
  uint16_t plot_time = (systemSettings.Profile.readPeriod+1)/200;                                                         // Update at the same rate as the system pwm

  updateIronPower();

  if(plot_time<20){ plot_time = 20; }
  if(mainScr.currentMode!=main_disabled && (currentTime-plotTimer)>plot_time){                                          // Only store values if running
    plotUpdate=1;
    plotTimer=currentTime;
    int16_t t = readTipTemperatureCompensated(stored_reading,read_Avg);
    if(systemSettings.settings.tempUnit==mode_Farenheit){
      t = TempConversion(t, mode_Celsius, 0);
    }

    if (t<20) t = 20;
    if (t>500) t = 500;

    plotData->p[plot_Index] = t;
    if(++plot_Index>99){
      plot_Index=0;
    }
  }

  mainScr.update = 0;

  if((currentTime-mainScr.updateTick)>systemSettings.settings.guiUpdateDelay){
    mainScr.update=1;                          // Update realtime readings slower than the rest of the GUI
    mainScr.updateTick=currentTime;
  }

  if(error & _ACTIVE){
    mainScr.ironStatus = status_error;
  }
  else{
    mainScr.ironStatus = status_ok;
  }
  if(!mainScr.ActivityOn && mainScr.ironStatus == status_ok && Iron.newActivity && mainScr.currentMode!=main_disabled){
    mainScr.ActivityOn=1;
  }
  else if(mainScr.ActivityOn && (mainScr.ironStatus == status_error || ((currentTime-Iron.lastActivityTime)>50))){
    Iron.newActivity=0;
    mainScr.ActivityOn=0;
  }

  if(input!=Rotate_Nothing){
    mainScr.idleTick=currentTime;
    if(currentTime < mainScr.inputBlockTime){
      input=Rotate_Nothing;
    }
    if(input==Click && current_mode<mode_run && mainScr.currentMode==main_irontemp){
      IronWake(wakeButton);
      current_mode = getCurrentMode();
      if(current_mode==mode_run){
        input=Rotate_Nothing;
      }
    }
  }

  if(current_mode==mode_sleep){
    if(input==Rotate_Nothing){
      if(contrast>5 && (currentTime-mainScr.idleTick)>10000){
        mainScr.dimDisplay=-5;
      }
    }
  }
  else if(contrast<systemSettings.settings.contrast){
    mainScr.dimDisplay=5;
  }

  if(mainScr.dimDisplay!=0){
    if(systemSettings.settings.screenDimming && currentTime-mainScr.dimTimer>9){
      mainScr.dimTimer = currentTime;
      mainScr.idleTick = currentTime;
      contrast+=mainScr.dimDisplay;
      if(contrast>4 && (contrast<systemSettings.settings.contrast)){
        mainScr.dimTimer=currentTime;
        setContrast(contrast);
      }
      else{
        if(mainScr.dimDisplay>0){
          restore_contrast();
        }
        mainScr.dimDisplay=0;
      }
    }
  }

  switch(mainScr.currentMode){
    case main_irontemp:
      if(mainScr.ironStatus!=status_ok){               // When the screen goes to disabled state
        memset(plotData->p,0,sizeof(plotData_t));           // Clear plotdata
        plot_Index=0;                                       // Reset X
        mainScr.setMode=main_disabled;
        mainScr.currentMode=main_setMode;
        break;
      }
      switch((uint8_t)input){
        case LongClick:
          return screen_settings;
        case Rotate_Increment_while_click:
          blockInput(200);
          mainScr.setMode=main_tipselect;
          mainScr.currentMode=main_setMode;
          break;
        case Rotate_Decrement_while_click:
          blockInput(100);
          if(Iron.CurrentMode>mode_standby){
            setCurrentMode(mode_standby);
          }
          else{
            setCurrentMode(mode_sleep);
          }
          break;
        case Rotate_Increment:
        case Rotate_Decrement:
          mainScr.setMode=main_setpoint;
          mainScr.currentMode=main_setMode;
          if(Iron.CurrentMode==mode_boost){
            setCurrentMode(mode_run);
          }
          else if(current_mode!=mode_run){
            IronWake(wakeButton);
          }
          break;
        case Click:
          blockInput(100);
          if(Iron.CurrentMode==mode_boost){
            setCurrentMode(mode_run);
            break;
          }
          else{
            IronWake(wakeButton);
            break;
          }
          mainScr.update=1;
          scr->refresh=screen_Erase;
          if(mainScr.displayMode==temp_numeric){
            mainScr.displayMode=temp_graph;
            widgetDisable(Widget_IronTemp);
          }
          else if(mainScr.displayMode==temp_graph){
            mainScr.displayMode=temp_numeric;
            widgetEnable(Widget_IronTemp);
          }
          break;
        default:
          break;
      }
      break;

    case main_disabled:
      switch((uint8_t)input){
        case LongClick:
        return screen_settings;
        case Rotate_Increment_while_click:
          mainScr.setMode=main_tipselect;
          mainScr.currentMode=main_setMode;
        default:
          break;
      }
      if(mainScr.ironStatus==status_ok){
        mainScr.setMode=main_irontemp;
        mainScr.currentMode=main_setMode;
      }
      break;
    case main_tipselect:
      if(input==LongClick){
        return screen_tip_settings;
      }
    case main_setpoint:
      switch((uint8_t)input){
        case LongClick:
          return -1;
        case Click:
          blockInput(200);
          if(mainScr.currentMode==main_setpoint){
            setCurrentMode(mode_boost);
          }
          mainScr.currentMode=main_setMode;
          mainScr.setMode=main_irontemp;
          break;
        case Rotate_Nothing:
          if( (mainScr.currentMode==main_setpoint && currentTime-mainScr.idleTick > 1000) || (mainScr.currentMode!=main_setpoint && currentTime-mainScr.idleTick > 5000)){
            mainScr.currentMode=main_setMode;
            if(mainScr.ironStatus!=status_error){
              mainScr.setMode=main_irontemp;
            }
            else{
              mainScr.setMode=main_disabled;
            }
            return -1;
          }
          break;
        case Rotate_Increment_while_click:
          input=Rotate_Increment;
          break;
        case Rotate_Decrement_while_click:
          input=Rotate_Decrement;
          break;
        default:
          break;
      }

    default:
      break;
  }

  if(mainScr.currentMode==main_setMode){
    mainScr.update=1;
    mainScr.idleTick=currentTime;
    scr->refresh=screen_Erase;
    mainScr.currentMode=mainScr.setMode;
    switch(mainScr.currentMode){
    case main_disabled:
      widgetDisable(Widget_IronTemp);
      break;
    case main_irontemp:
      setMainWidget(Widget_IronTemp);
      if(mainScr.displayMode==temp_graph){
        widgetDisable(Widget_IronTemp);
      }
      break;
    case main_setpoint:
      setMainWidget(Widget_SetPoint);
      break;
    case main_tipselect:
      setMainWidget(Widget_TipSelect);
      break;
    default:
      break;
    }
    return -1;
  }
  return default_screenProcessInput(scr, input, state);
}


void main_screen_draw(screen_t *scr){
  uint8_t scr_refresh;
  static uint32_t lastState = 0;
  uint32_t currentTime = HAL_GetTick();
  uint32_t currentState = (uint32_t)Iron.Error.Flags<<24 | (uint32_t)mainScr.ironStatus<<16 | mainScr.currentMode;    // Simple method to detect changes

  if((lastState!=currentState) || Widget_SetPoint->refresh || Widget_IronTemp->refresh || plotUpdate){
    lastState=currentState;
    scr->refresh=screen_Erase;
  }
  scr_refresh=scr->refresh;
  default_screenDraw(scr);

  if(scr_refresh){
    u8g2_SetDrawColor(&u8g2, WHITE);

    #ifdef USE_NTC
    u8g2_DrawXBMP(&u8g2, Widget_AmbTemp->posX-tempXBM[0]-2, 0, tempXBM[0], tempXBM[1], &tempXBM[2]);
    #endif

    #ifdef USE_VIN
    u8g2_DrawXBMP(&u8g2, 0, 0, voltXBM[0], voltXBM[1], &voltXBM[2]);
    #endif

    if(mainScr.currentMode==main_disabled){
      if(mainScr.ironStatus==status_error){
        if(Iron.Error.Flags==(_ACTIVE | _NO_IRON)){                               // Only "No iron detected". Don't show error just for it
          u8g2_SetFont(&u8g2, u8g2_font_noIron_Sleep);
          putStrAligned("NO IRON", 26, align_center);
        }
        else{
          uint8_t Err_ypos;

          uint8_t err = (uint8_t)Iron.Error.V_low+Iron.Error.safeMode+(Iron.Error.NTC_low|Iron.Error.NTC_high)+Iron.Error.noIron;
          if(err<4){
            Err_ypos= 14+ ((50-(err*13))/2);
          }
          else{
            Err_ypos=14;
          }
          u8g2_SetFont(&u8g2, default_font);
          if(Iron.Error.V_low){
            putStrAligned("Voltage low!", Err_ypos, align_center);
            Err_ypos+=13;
          }
          if(Iron.Error.safeMode){
            putStrAligned("Failsafe mode", Err_ypos, align_center);
            Err_ypos+=13;
          }
          if(Iron.Error.NTC_high){
            putStrAligned("NTC read high!", Err_ypos, align_center);
            Err_ypos+=13;
          }
          else if(Iron.Error.NTC_low){
            putStrAligned("NTC read low!", Err_ypos, align_center);
            Err_ypos+=13;
          }
          if(Iron.Error.noIron){
            putStrAligned("No iron detected", Err_ypos, align_center);
            Err_ypos+=13;
          }
        }
      }
    }
    else{
      if(mainScr.currentMode==main_tipselect){
        u8g2_SetFont(&u8g2, default_font);
        putStrAligned("TIP SELECTION", 16, align_center);
      }
      if(mainScr.ActivityOn){
        u8g2_DrawXBMP(&u8g2, (OledWidth-shakeXBM[1])/2, 0, shakeXBM[0], shakeXBM[1], &shakeXBM[2]);
      }
    }
  }

  if(mainScr.ironStatus==status_ok){
    if( scr_refresh || (currentTime-barTime)>9){                    // Update every 10mS or if screen was erased
      barTime = currentTime;
      if(scr_refresh<screen_Erase){                                   // If screen not erased
         u8g2_SetDrawColor(&u8g2,BLACK);                              // Draw a black square to wipe old widget data
        u8g2_DrawBox(&u8g2, 47 , OledHeight-8, 80, 8);
      }
      u8g2_SetDrawColor(&u8g2,WHITE);
      u8g2_DrawBox(&u8g2, 47, OledHeight-7, mainScr.lastPwr, 6);
      u8g2_DrawRFrame(&u8g2, 47, OledHeight-8, 80, 8, 2);
    }

    if((scr_refresh || plotUpdate) && mainScr.currentMode==main_irontemp && mainScr.displayMode==temp_graph){
      plotUpdate=0;
      int16_t t = Iron.CurrentSetTemperature;

      if(systemSettings.settings.tempUnit==mode_Farenheit){
        t = TempConversion(t, mode_Celsius, 0);
      }
      // plot is 16-56 V, 14-113 H ?
      u8g2_DrawVLine(&u8g2, 11, 13, 41);                              // left scale

      for(uint8_t y=13; y<54; y+=10){
        u8g2_DrawHLine(&u8g2, 7, y, 4);                             // left ticks
      }

      for(uint8_t x=0; x<100; x++){
        uint8_t pos=plot_Index+x;
        if(pos>99){ pos-=100; }                                     // Reset index if > 99

        uint16_t plotV = plotData->p[pos];

        if (plotV < t-20) plotV = 0;
        else if (plotV >= t+20) plotV = 40;
        else plotV = (plotV-t+20) ;                                 // relative to t, +-20C
        u8g2_DrawVLine(&u8g2, x+13, 53-plotV, plotV);               // data points
      }
      #define set 33
      u8g2_DrawTriangle(&u8g2, 122, set-4, 122, set+4, 115, set);     // set temp marker
    }
    if(scr_refresh){
      u8g2_SetFont(&u8g2, u8g2_font_labels);
      if(getCurrentMode()==mode_sleep){
        u8g2_DrawStr(&u8g2, 42, 0, "SLEEP");
      }
      if(getCurrentMode()==mode_standby){
        u8g2_DrawStr(&u8g2, 48, 0, "STBY");
      }
      else if(getCurrentMode()==mode_boost){
        u8g2_DrawStr(&u8g2, 42, 0, "BOOST");
      }
      u8g2_DrawStr(&u8g2, 0, 55, systemSettings.Profile.tip[systemSettings.Profile.currentTip].name);
    }
  }
}

static void main_screen_init(screen_t *scr) {
  editable_widget_t *edit;
  default_init(scr);
  mainScr.dimDisplay=0;
  if(mainScr.currentMode != main_disabled){
    mainScr.currentMode = main_irontemp;
    setMainWidget(Widget_IronTemp);
    if(mainScr.displayMode==temp_graph){
      widgetDisable(Widget_IronTemp);
    }
  }
  edit = extractEditablePartFromWidget(Widget_TipSelect);
  edit->numberOfOptions = systemSettings.Profile.currentNumberOfTips;

  edit = extractEditablePartFromWidget(Widget_SetPoint);
  edit->step = systemSettings.settings.tempStep;
  edit->big_step = systemSettings.settings.tempStep;
  edit->max_value = systemSettings.Profile.MaxSetTemperature;
  edit->min_value = systemSettings.Profile.MinSetTemperature;
  setMainScrTempUnit();
  mainScr.idleTick=HAL_GetTick();
}

static void main_screen_onExit(screen_t *scr) {
  free(plotData);
  restore_contrast();
}

static void main_screen_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ Iron Temp Widget ]
  //
  newWidget(&w,widget_display,scr);
  Widget_IronTemp = w;
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=5;
  dis->dispAlign=align_center;
  dis->textAlign=align_center;
  dis->font=u8g2_font_ironTemp;
  w->posY = 15;
  dis->getData = &main_screen_getIronTemp;
  w->enabled=0;

  //  [ Iron Setpoint Widget ]
  //
  newWidget(&w,widget_editable,scr);
  Widget_SetPoint=w;
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=5;
  w->posY = Widget_IronTemp->posY-2;
  dis->getData = &getTemp;
  dis->dispAlign=align_center;
  dis->textAlign=align_center;
  dis->font=((displayOnly_widget_t*)Widget_IronTemp->content)->font;
  edit->selectable.tab = 1;
  edit->setData = (void (*)(void *))&setTemp;
  w->frameType=frame_solid;
  w->radius=8;
  w->enabled=0;
  w->width=128;

  #ifdef USE_VIN
  //  [ V. Supply Widget ]
  //
  newWidget(&w,widget_display,scr);
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &main_screen_getVin;
  dis->endString="V";
  dis->reservedChars=5;
  dis->textAlign=align_center;
  dis->number_of_dec=1;
  dis->font=u8g2_font_labels;
  w->posY= 0;
  w->posX = voltXBM[0]+2;
  edit=extractEditablePartFromWidget(w);
  //w->width = 40;
  #endif

  #ifdef USE_NTC
  //  [ Ambient Temp Widget ]
  //
  newWidget(&w,widget_display,scr);
  Widget_AmbTemp=w;
  dis=extractDisplayPartFromWidget(w);
  dis->reservedChars=7;
  dis->dispAlign=align_right;
  dis->textAlign=align_center;
  dis->number_of_dec=1;
  dis->font=u8g2_font_labels;
  dis->getData = &main_screen_getAmbTemp;
  w->posY = 0;
  //w->posX = 90;
  #endif

  //  [ Tip Selection Widget ]
  //
  newWidget(&w,widget_multi_option,scr);
  Widget_TipSelect=w;
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=TipCharSize-1;
  dis->dispAlign=align_center;
  dis->textAlign=align_center;
  edit->inputData.getData = &getTip;
  edit->inputData.number_of_dec = 0;
  edit->big_step = 0;
  edit->step = 0;
  edit->selectable.tab = 2;
  edit->setData = (void (*)(void *))&setTip;
  edit->options = tipName;
  w->posY = 32;
  w->enabled=0;
  w->frameType=frame_disabled;

  plotData = calloc(1,sizeof(plotData_t));
}


void main_screen_setup(screen_t *scr) {
  scr->draw = &main_screen_draw;
  scr->init = &main_screen_init;
  scr->processInput = &main_screenProcessInput;
  scr->create = &main_screen_create;
  scr->onExit = &main_screen_onExit;

  for(int x = 0; x < TipSize; x++) {
    tipName[x] = systemSettings.Profile.tip[x].name;
  }
}

