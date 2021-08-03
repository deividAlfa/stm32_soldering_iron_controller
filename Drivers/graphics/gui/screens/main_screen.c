                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "main_screen.h"
#include "screen_common.h"

#define SCREENSAVER

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen variables
//-------------------------------------------------------------------------------------------------------------------------------

static uint32_t barTime;
slide_t screenSaver = {
    .x = 34,
    .y = 0,
    .xAdd = 1,
    .yAdd = 1,
};

static char *tipNames[TipSize];
enum mode{  main_none=0, main_irontemp, main_error, main_ironstatus, main_setpoint, main_tipselect };
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

#ifdef SCREENSAVER
const uint8_t ScrSaverXBM[] = {
 60, 49,
 0x00, 0x00, 0x00, 0xC0, 0xE1, 0x3F, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF,
 0x1F, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x7E, 0x00, 0x60, 0xFC, 0x01, 0x00,
 0x80, 0xFF, 0xFF, 0xFF, 0x1F, 0xFB, 0x03, 0x00, 0xE0, 0x1F, 0xFF, 0x1F,
 0xC0, 0xE4, 0x03, 0x00, 0xF0, 0xF3, 0xF1, 0xEF, 0x3F, 0xD9, 0x07, 0x00,
 0xF8, 0xF9, 0xFF, 0xEF, 0xFF, 0xF4, 0x0F, 0x00, 0xF8, 0xFD, 0xFB, 0xFF,
 0xFF, 0xEB, 0x0F, 0x00, 0xF8, 0xFF, 0xF7, 0xFF, 0x80, 0xFF, 0x1F, 0x00,
 0xF8, 0xFF, 0xFF, 0x3F, 0x00, 0xFE, 0x1F, 0x00, 0xFC, 0x03, 0xFC, 0x0F,
 0x03, 0xF8, 0x3F, 0x00, 0x7E, 0x01, 0xF8, 0xC7, 0x01, 0x98, 0x0F, 0x00,
 0x03, 0x01, 0xC0, 0x27, 0x00, 0x98, 0x0F, 0x00, 0xFC, 0x7F, 0xC0, 0x07,
 0xEE, 0x6F, 0xE0, 0x00, 0xF8, 0xFF, 0xE3, 0x8F, 0x8F, 0x1F, 0xC0, 0x01,
 0x05, 0xFE, 0xF3, 0xFF, 0x1F, 0x00, 0x8F, 0x05, 0x01, 0xE0, 0xF3, 0xFF,
 0x3F, 0xE0, 0x1C, 0x05, 0xFF, 0xE0, 0xF1, 0xFF, 0xFF, 0x7F, 0x3C, 0x0D,
 0xBF, 0xF9, 0xF8, 0x8F, 0xFE, 0x1F, 0x38, 0x0D, 0x9D, 0x7F, 0xFC, 0x0F,
 0xE0, 0x07, 0x21, 0x0D, 0x1F, 0x3F, 0xFC, 0x33, 0xFF, 0xC1, 0x21, 0x05,
 0x0C, 0x07, 0xFC, 0x20, 0x3F, 0xF0, 0x38, 0x05, 0x0B, 0xF6, 0xF8, 0x07,
 0x07, 0x78, 0xBC, 0x00, 0x0E, 0xFC, 0xC3, 0x7F, 0xC0, 0x19, 0x7C, 0x02,
 0x0E, 0xE0, 0xE7, 0x07, 0xF8, 0x00, 0xBE, 0x01, 0x0E, 0x00, 0x00, 0x00,
 0x7C, 0x00, 0x7F, 0x00, 0x0E, 0x06, 0x00, 0xF8, 0x1C, 0x10, 0x7F, 0x00,
 0x0E, 0xE4, 0x71, 0xFC, 0x00, 0x98, 0x3F, 0x00, 0x0E, 0xE4, 0x71, 0x3E,
 0x80, 0xC9, 0x3F, 0x00, 0x0E, 0x00, 0x00, 0x00, 0xE0, 0xE1, 0x1F, 0x00,
 0x0E, 0x00, 0x00, 0x00, 0xF8, 0xE3, 0x1F, 0x00, 0x0E, 0x00, 0x00, 0x80,
 0xF9, 0xF0, 0x0F, 0x00, 0x0E, 0x00, 0x00, 0xF0, 0x79, 0xFC, 0x07, 0x00,
 0x1E, 0x00, 0x80, 0xF9, 0x33, 0xFE, 0x03, 0x00, 0x1E, 0x92, 0xF3, 0xF9,
 0x03, 0xFF, 0x01, 0x00, 0x3E, 0x32, 0xF3, 0xF9, 0xC1, 0xBB, 0x00, 0x00,
 0x7E, 0x20, 0xE7, 0x19, 0xF0, 0x4C, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00,
 0x3E, 0x13, 0x00, 0x00, 0xFE, 0x0F, 0x00, 0xE0, 0xC7, 0x0C, 0x00, 0x00,
 0x7E, 0xFF, 0xFF, 0xFF, 0x39, 0x03, 0x00, 0x00, 0xFE, 0xFE, 0xFF, 0x3F,
 0xE6, 0x00, 0x00, 0x00, 0xEE, 0xF1, 0x7F, 0x40, 0x3C, 0x00, 0x00, 0x00,
 0xDE, 0x1F, 0xC0, 0x83, 0x07, 0x00, 0x00, 0x00, 0x3E, 0xFF, 0x0F, 0xFC,
 0x01, 0x00, 0x00, 0x00, 0xFC, 0x08, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00,
 0xFC, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0x01,
 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
#endif
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
  uint8_t updateReadings;
  uint8_t lastPwr;
  uint8_t idle;
  uint8_t shakeActive;
  int8_t  dimStep;
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
  uint32_t idleTimer;
  uint32_t inputBlockTime;
  uint32_t updateTimer;
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
  if(mainScr.updateReadings){
    mainScr.lastTip=readTipTemperatureCompensated(stored_reading,read_Avg);
  }
  temp=mainScr.lastTip;
  return &temp;
}

#ifdef USE_VIN
static void * main_screen_getVin() {
  if(mainScr.updateReadings){
    mainScr.lastVin = getSupplyVoltage_v_x10();
  }
  temp=mainScr.lastVin;
  return &temp;
}
#endif

#ifdef USE_NTC
static void * main_screen_getAmbTemp() {
  if(mainScr.updateReadings){
    mainScr.lastAmb = readColdJunctionSensorTemp_x10(stored_reading, systemSettings.settings.tempUnit);
  }
  temp=mainScr.lastAmb;
  return &temp;
}
#endif

static void updateIronPower() {
  static uint32_t stored=0;
  static uint32_t updateTim;
  if((current_time-updateTim)>19){
    updateTim = current_time;
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
  mainScr.inputBlockTime = current_time+time;
}

void restore_contrast(void){
  if(getContrast() != systemSettings.settings.contrast){
    setContrast(systemSettings.settings.contrast);
  }
}

void updateScreenSaver(void){
#ifdef SCREENSAVER
  if(!screenSaver.enabled || Iron.CurrentMode!=mode_sleep || (Iron.Error.Flags & _ACTIVE)){
    return;
  }
  if(current_time-screenSaver.timer>50){
    screenSaver.timer=current_time;
    screenSaver.x+=screenSaver.xAdd;
    screenSaver.y+=screenSaver.yAdd;
    //uint16_t _x = (screenSaver.x*5)/2;
    if(screenSaver.x<1 || screenSaver.x>(OledWidth-2)){
      screenSaver.xAdd = -screenSaver.xAdd;
    }
    if(screenSaver.y<1 || screenSaver.y>(OledHeight-2)){
      screenSaver.yAdd = -screenSaver.yAdd;
    }
    screenSaver.update=1;
  }
#endif
}

// Switch main screen modes
int8_t switchScreenMode(void){
  if(mainScr.setMode!=main_none){
    plot.enabled=0;
    mainScr.updateReadings=1;
    mainScr.idleTimer=current_time;
    Screen_main.refresh=screen_Erase;
    switch(mainScr.setMode){
      case main_irontemp:
        setMainWidget(Widget_IronTemp);
        if(mainScr.ironStatus!=status_error){
          if(mainScr.displayMode==temp_graph){
            widgetDisable(Widget_IronTemp);
            plot.enabled=1;
            plot.update=1;
          }
          break;
        }
        mainScr.setMode=main_error;
        // No break intentionally
      case main_error:
        widgetDisable(Widget_IronTemp);
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
    mainScr.currentMode=mainScr.setMode;
    mainScr.setMode=main_none;
    return 1;                                 // Changed mode
  }
  return 0;                                   // No changes
}

int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  int16_t contrast = getContrast();
  uint8_t current_mode = getCurrentMode();
  int16_t current_temp = readTipTemperatureCompensated(stored_reading,read_Avg);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    current_temp = TempConversion(current_temp, mode_Celsius, 0);
  }
  updateIronPower();

  mainScr.updateReadings = 0;

  // Display values update timer
  if((current_time-mainScr.updateTimer)>(systemSettings.settings.guiUpdateDelay*100)){
    mainScr.updateReadings=1;
    mainScr.updateTimer=current_time;
  }

  if(Iron.Error.Flags & _ACTIVE){
    mainScr.ironStatus = status_error;
    current_temp=0;
    mainScr.shakeActive=0;
  }
  else{
    mainScr.ironStatus = status_ok;
  }

  updatePlot();
  updateScreenSaver();



  // Timer for ignoring user input
  if(current_time < mainScr.inputBlockTime){
    input=Rotate_Nothing;
  }

  if(input!=Rotate_Nothing){
    mainScr.idleTimer = current_time;
  }

  if(systemSettings.settings.screenDimming){
    if(mainScr.dimStep==0){
      // Wake up screen. If source was the encoder, block it the first time
      if(contrast<systemSettings.settings.contrast){
        if(Iron.shakeActive || current_mode!=mode_sleep || input!=Rotate_Nothing){
          mainScr.dimStep=5;
          input=Rotate_Nothing;
          mainScr.dimTimer = current_time;
        }
      }
      // Screen dimming timer
      if(contrast>5 && current_mode==mode_sleep && current_temp<100 && ((current_time-mainScr.idleTimer)>10000)){
        mainScr.dimStep=-5;
      }
    }
    // Smooth screen brightness dimming
    else if(current_time-mainScr.dimTimer>9){
      mainScr.dimTimer = current_time;
      contrast+=mainScr.dimStep;
      if(contrast>4 && (contrast<systemSettings.settings.contrast)){
        mainScr.dimTimer=current_time;
        setContrast(contrast);
      }
      else{
        if(mainScr.dimStep>0){
          restore_contrast();
        }
        mainScr.dimStep=0;
        mainScr.idleTimer = current_time;
      }
    }
  }

  // Handle shake wake icon drawing and timeout
  if( !mainScr.shakeActive && Iron.shakeActive){
    Iron.shakeActive=0;
    if(current_mode == mode_run){
      mainScr.shakeActive=1;
    }
  }
  else if(mainScr.shakeActive==2 && (current_time-Iron.lastShakeTime)>50){
    mainScr.shakeActive=3; // Clear
  }

  // Handle main screen
  switch(mainScr.currentMode){


    case main_irontemp:

      if(mainScr.ironStatus!=status_ok){                // When the screen goes to error state
        memset(&plot,0,sizeof(plotData_t));            // Clear plotdata
        mainScr.setMode=main_error;
        break;
      }
      switch((uint8_t)input){

        case LongClick:
          return screen_settings;

        case Rotate_Increment_while_click:
          blockInput(100);
          mainScr.setMode=main_tipselect;
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
          if(Iron.CurrentMode!=mode_run){
            IronWake(wakeButton);
            if(getCurrentMode()==mode_run){   // If mode changed, don't process the click
              break;
            }
          }
          mainScr.updateReadings=1;
          scr->refresh=screen_Erase;
          if(mainScr.displayMode==temp_numeric){
            mainScr.displayMode=temp_graph;
            widgetDisable(Widget_IronTemp);
            plot.enabled=1;
            plot.update=1;
          }
          else if(mainScr.displayMode==temp_graph){
            mainScr.displayMode=temp_numeric;
            widgetEnable(Widget_IronTemp);
            plot.enabled=0;
          }

        default:
          break;
      }
      break;


    case main_error:

      switch((uint8_t)input){
        case LongClick:
          return screen_settings;

        case Rotate_Increment_while_click:
          mainScr.setMode=main_tipselect;
          break;

        case Rotate_Increment:
        case Rotate_Decrement:
            mainScr.setMode=main_setpoint;

        default:
          break;
      }
      if(mainScr.ironStatus==status_ok){
        mainScr.setMode=main_irontemp;
      }
      break;


    case main_tipselect:

      switch((uint8_t)input){
        case LongClick:
          return screen_tip_settings;

        case Click:
          blockInput(100);
          mainScr.setMode=main_irontemp;
          break;

        case Rotate_Nothing:
          if(current_time-mainScr.idleTimer > 5000){
            mainScr.setMode=main_irontemp;
          }
          break;

        case Rotate_Increment_while_click:
          input=Rotate_Increment;
          break;

        case Rotate_Decrement_while_click:
          input=Rotate_Decrement;

        default:
          break;
      }
      break;

    case main_setpoint:

      switch((uint8_t)input){
        case LongClick:
          break;;

        case Click:
          blockInput(100);
          if(mainScr.ironStatus != status_error){
            setCurrentMode(mode_boost);
          }
          mainScr.setMode=main_irontemp;
          break;

        case Rotate_Nothing:
          if(current_time-mainScr.idleTimer > 1000){
            mainScr.setMode=main_irontemp;
          }
          break;

        case Rotate_Increment_while_click:
          input=Rotate_Increment;
          break;

        case Rotate_Decrement_while_click:
          input=Rotate_Decrement;

        default:
          break;
      }
    default:
      break;
  }

  if(switchScreenMode()){
    return -1;
  }

  return default_screenProcessInput(scr, input, state);
}

static void drawIcons(uint8_t refresh){
  if(refresh){
    #ifdef USE_NTC
    u8g2_DrawXBMP(&u8g2, Widget_AmbTemp->posX-tempXBM[0]-2, 0, tempXBM[0], tempXBM[1], &tempXBM[2]);
    #endif

    #ifdef USE_VIN
    u8g2_DrawXBMP(&u8g2, 0, 0, voltXBM[0], voltXBM[1], &voltXBM[2]);
    #endif
  }

  if(mainScr.shakeActive==1 || (mainScr.shakeActive==2 && refresh) ){ //1 = new draw, 2 = already drawn
    mainScr.shakeActive=2;
    u8g2_DrawXBMP(&u8g2, (OledWidth-shakeXBM[1])/2, 0, shakeXBM[0], shakeXBM[1], &shakeXBM[2]);
  }
  else if(mainScr.shakeActive==3){  // 3 = clear
    mainScr.shakeActive=0;
    u8g2_SetDrawColor(&u8g2,BLACK);
    u8g2_DrawBox(&u8g2, (OledWidth-shakeXBM[1])/2, 0, shakeXBM[0], shakeXBM[1]);
    u8g2_SetDrawColor(&u8g2,WHITE);
  }
}


static void drawError(void){
  if(Iron.Error.Flags==(_ACTIVE | _NO_IRON)){                               // Only "No iron detected". Don't show error screen just for it
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


static void drawScreenSaver(void){
#ifdef SCREENSAVER
  if(!screenSaver.enabled || getCurrentMode()!=mode_sleep || mainScr.currentMode!=main_irontemp){
    return;
  }
  screenSaver.update=0;
  //uint16_t _x=(screenSaver.x*5)/2;
  if(screenSaver.x<OledWidth || screenSaver.y<OledHeight){
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_SetBitmapMode(&u8g2, 1);
    u8g2_DrawXBMP(&u8g2, screenSaver.x, screenSaver.y, ScrSaverXBM[0], ScrSaverXBM[1], &ScrSaverXBM[2]);
    u8g2_SetBitmapMode(&u8g2, 0);
  }
#endif
}

static void drawMode(void){
  u8g2_SetFont(&u8g2, u8g2_font_labels);

  switch(getCurrentMode()){

    case mode_sleep:
      u8g2_DrawStr(&u8g2, 42, 0, "SLEEP");
    break;

    case mode_standby:
      u8g2_DrawStr(&u8g2, 48, 0, "STBY");
      break;

    case mode_boost:
      u8g2_DrawStr(&u8g2, 42, 0, "BOOST");

    default:
      break;
  }
}

static void drawPowerBar(uint8_t refresh){
  static uint8_t previousPower=0;
  if((current_time-barTime)>9){
    barTime = current_time;
    if(previousPower!=mainScr.lastPwr){
      previousPower = mainScr.lastPwr;
      refresh=1;
    }
  }
  if(refresh){                          // Update every 10mS or if screen was erased
    if(Screen_main.refresh<screen_Erase){                           // If screen not erased
      u8g2_SetDrawColor(&u8g2,BLACK);                               // Draw a black square to wipe old widget data
      u8g2_DrawBox(&u8g2, 47 , OledHeight-8, 80, 8);
      u8g2_SetDrawColor(&u8g2,WHITE);
    }
    u8g2_DrawBox(&u8g2, 47, OledHeight-7, mainScr.lastPwr, 6);
    u8g2_DrawRFrame(&u8g2, 47, OledHeight-8, 80, 8, 2);
  }
}

static void drawPlot(uint8_t refresh){
  if(!plot.enabled){ return; }
  if(refresh || plot.update){
    int16_t ref;
    if(Iron.CurrentMode!=mode_sleep){
      ref=Iron.CurrentSetTemperature;
      if(systemSettings.settings.tempUnit==mode_Farenheit){
        ref = TempConversion(ref, mode_Celsius, 0);
      }
    }
    else{
      ref = 100;
    }

    plot.update=0;
    // plot is 16-56 V, 14-113 H ?
    u8g2_DrawVLine(&u8g2, 11, 13, 41);                              // left scale

    for(uint8_t y=13; y<54; y+=10){
      u8g2_DrawHLine(&u8g2, 7, y, 4);                               // left ticks
    }

    for(uint8_t x=0; x<100; x++){
      uint8_t pos=plot.index+x;
      if(pos>99){ pos-=100; }                                       // Reset index if > 99

      uint16_t plotV = plot.d[pos];

      if (plotV < (ref-20)) plotV = 0;
      else if (plotV > (ref+20)) plotV = 40;
      else plotV = (plotV-ref+20) ;                    // relative to t, +-20C
      u8g2_DrawVLine(&u8g2, x+13, 53-plotV, plotV);                 // data points
    }
    #define set 33
    u8g2_DrawTriangle(&u8g2, 122, set-4, 122, set+4, 115, set);     // Setpoint marker
  }
}

void main_screen_draw(screen_t *scr){
  uint8_t scr_refresh;
  static uint32_t lastState = 0;
  uint32_t currentState = (uint32_t)Iron.Error.Flags<<24 | (uint32_t)Iron.CurrentMode<<16 | mainScr.currentMode;    // Simple method to detect changes

  if((lastState!=currentState) || Widget_SetPoint->refresh || Widget_IronTemp->refresh || plot.update || screenSaver.update){
    lastState=currentState;
    scr->refresh=screen_Erase;
  }
  scr_refresh=scr->refresh;
  default_screenDraw(scr);
  u8g2_SetDrawColor(&u8g2, WHITE);

  if(!scr_refresh){
    if(mainScr.ironStatus != status_error){
      drawPowerBar(0);
      drawPlot(0);
    }
    drawIcons(0);
    return;
  }

  drawMode();
  drawIcons(1);

  switch(mainScr.currentMode){
    case main_error:
      drawError();
      break;

    case main_setpoint:
      if(mainScr.ironStatus == status_error){ break; }
    case main_irontemp:
      // Tip name label
      u8g2_SetFont(&u8g2, u8g2_font_labels);
      u8g2_DrawStr(&u8g2, 0, 55, systemSettings.Profile.tip[systemSettings.Profile.currentTip].name);
      break;

    case main_tipselect:
        u8g2_SetFont(&u8g2, default_font);
        putStrAligned("TIP SELECTION", 16, align_center);
  }

  if(mainScr.ironStatus != status_error){
    drawPowerBar(1);
    drawPlot(1);
    drawScreenSaver();
  }
}

static void main_screen_init(screen_t *scr) {
  editable_widget_t *edit;
  default_init(scr);
  mainScr.dimStep=0;
  plot.timeStep = (systemSettings.Profile.readPeriod+1)/200;                                                         // Update at the same rate as the system pwm

  mainScr.setMode = main_irontemp;
  switchScreenMode();

  edit = extractEditablePartFromWidget(Widget_TipSelect);
  edit->numberOfOptions = systemSettings.Profile.currentNumberOfTips;

  edit = extractEditablePartFromWidget(Widget_SetPoint);
  edit->step = systemSettings.settings.tempStep;
  edit->big_step = systemSettings.settings.tempStep;
  edit->max_value = systemSettings.Profile.MaxSetTemperature;
  edit->min_value = systemSettings.Profile.MinSetTemperature;
  setMainScrTempUnit();
  mainScr.idleTimer=current_time;
}

static void main_screen_onExit(screen_t *scr) {
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
  edit->options = tipNames;
  w->posY = 32;
  w->enabled=0;
  w->frameType=frame_disabled;
}


void main_screen_setup(screen_t *scr) {
  scr->draw = &main_screen_draw;
  scr->init = &main_screen_init;
  scr->processInput = &main_screenProcessInput;
  scr->create = &main_screen_create;
  scr->onExit = &main_screen_onExit;

  for(int x = 0; x < TipSize; x++) {
    tipNames[x] = systemSettings.Profile.tip[x].name;
  }
}

