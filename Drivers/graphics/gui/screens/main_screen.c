                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "main_screen.h"
#include "screen_common.h"

#define SCREENSAVER
#define PWR_BAR_WIDTH   60
#define SCALE_FACTOR    (int)((65536*PWR_BAR_WIDTH*1.005)/100)
#define TIME_DIFF 		80
#define NUM_ROT			1

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

static uint16_t incrCount,decrCount;

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

#ifdef USE_VIN
static widget_t *Widget_Voltage;
#endif

static widget_t *Widget_IronTemp;

//static widget_t *Widget_TipSelect;

static widget_t *Widget_SetPoint;

static struct{
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
  uint8_t updateReadings;
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
  uint32_t inputBlockTimer;
  uint32_t modeTimer;
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

static void * main_screen_getIronTemp() {
  if(mainScr.updateReadings){
    mainScr.lastTip=readTipTemperatureCompensated(old_reading,read_average);
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
    mainScr.lastAmb = readColdJunctionSensorTemp_x10(old_reading, systemSettings.settings.tempUnit);
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
    tmpPwr = (tmpPwr*SCALE_FACTOR)>>16;
    mainScr.lastPwr=tmpPwr;
  }
}

static void setMainWidget(widget_t* w){
  Screen_main.refresh=screen_Erase;
  mainScr.Selected=w;
  Screen_main.current_widget=w;
  widgetEnable(w);
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
  mainScr.inputBlockTimer = current_time+time;
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
    mainScr.updateReadings=1;
    mainScr.idleTimer=current_time;
    mainScr.modeTimer=current_time;

    plot.enabled = (mainScr.displayMode==temp_graph);
    Screen_main.refresh=screen_Erase;

    switch(mainScr.setMode){

      case main_irontemp:
        widgetDisable(Widget_SetPoint);
        if(mainScr.ironStatus!=status_error){
          if(!plot.enabled){
            setMainWidget(Widget_IronTemp);
          }
          break;
        }
        mainScr.setMode=main_error;
        // No break intentionally
      case main_error:
        plot.enabled = 0;
        widgetDisable(Widget_IronTemp);
        break;

      case main_setpoint:
        plot.enabled = 0;
        setMainWidget(Widget_SetPoint);
        break;

      case main_tipselect:
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

  uint8_t current_mode = getCurrentMode();
  int16_t current_temp = readTipTemperatureCompensated(old_reading,read_average);
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    current_temp = TempConversion(current_temp, mode_Celsius, 0);
  }
  mainScr.updateReadings=update_GUI_Timer();
  updateIronPower();
  updatePlot();
  updateScreenSaver();


  if(Iron.Error.Flags & _ACTIVE){
    if(mainScr.ironStatus != status_error){
      mainScr.ironStatus = status_error;
      mainScr.idleTimer = current_time;
    }
    current_temp=0;
    if(mainScr.shakeActive){
      mainScr.shakeActive=3;
    }
  }
  else{
    mainScr.ironStatus = status_ok;
  }




  // Timer for ignoring user input
  if(current_time < mainScr.inputBlockTimer){
    input=Rotate_Nothing;
  }

  if(input!=Rotate_Nothing){
    mainScr.idleTimer = current_time;
    refreshOledDim();
  }
  if(current_mode!=mode_sleep || current_temp>99 || Iron.shakeActive){
    refreshOledDim();
  }

  handleOledDim();

  // Handle shake wake icon drawing and timeout
  if( !mainScr.shakeActive && Iron.shakeActive){
    Iron.shakeActive=0;
    mainScr.shakeActive=1;
  }
  else if(mainScr.shakeActive==2 && (current_time-Iron.lastShakeTime)>50){
    mainScr.shakeActive=3; // Clear
  }

  // Handle main screen
  switch(mainScr.currentMode){


    case main_irontemp:

      if(mainScr.ironStatus!=status_ok){                // When the screen goes to error state
        memset(&plot,0,sizeof(plotData_t));             // Clear plotdata
        plot.timeStep = (systemSettings.Profile.readPeriod+1)/200;
        mainScr.setMode=main_error;
        break;
      }
      switch((uint8_t)input){

        case LongClick:
          return screen_settings;

        case Rotate_Increment_while_click:
          //blockInput(100);
          mainScr.setMode=main_tipselect;
          break;

        case Rotate_Decrement_while_click:
          //blockInput(100);
          if(Iron.CurrentMode>mode_standby){
            setCurrentMode(mode_standby);
          }
          else{
            setCurrentMode(mode_sleep);
          }
          break;

        case Rotate_Increment:
        case Rotate_Decrement:
          if(Iron.CurrentMode==mode_boost){
            setCurrentMode(mode_run);
          }
          else if(current_mode!=mode_run){
            IronWake(wakeButton);
            if(getCurrentMode()==mode_run){   // If mode changed, don't process the click
              break;
            }
          }
          if(mainScr.displayMode==temp_graph){
            Widget_SetPoint->enabled=1;
            default_widgetProcessInput(Widget_SetPoint, input, state);
          }
          else{
            mainScr.setMode=main_setpoint;
          }
          return -1;
          break;

        case Click:
          //blockInput(100);
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
      if(mainScr.ironStatus==status_error){  // If error appears while adjusting tip select, it needs to update now to avoid overlapping problems
        plot.enabled = 0;
        widgetDisable(Widget_IronTemp);
      }
      else{
        if(mainScr.displayMode==temp_numeric){
          widgetEnable(Widget_IronTemp);
        }
        else{
          plot.enabled=1;
        }
      }
      switch((uint8_t)input){
        case LongClick:
          return screen_tip_settings;

        case Click:
          //blockInput(100);
          mainScr.setMode=main_irontemp;
          break;

        case Rotate_Nothing:
          if(current_time-mainScr.idleTimer > 2000){
            mainScr.setMode=main_irontemp;
          }
          break;

        default:
        {
          uint8_t tip = systemSettings.Profile.currentTip;
          if(input==Rotate_Increment_while_click || input==Rotate_Increment){
            if(++tip >= systemSettings.Profile.currentNumberOfTips){
              tip=0;
            }
          }
          else if(input==Rotate_Decrement_while_click || input==Rotate_Decrement){
            if(--tip>=systemSettings.Profile.currentNumberOfTips){    // If underflowed
              tip = systemSettings.Profile.currentNumberOfTips-1;
            }
          }
          if(tip!=systemSettings.Profile.currentTip){
            systemSettings.Profile.currentTip = tip;
            setCurrentTip(tip);
            Screen_main.refresh=screen_Erase;
          }
          break;
        }
      }
      if(input!=Rotate_Nothing){
        IronWake(wakeButton);
      }
      break;

    case main_setpoint:



      switch((uint8_t)input){
        case LongClick:
        case Click:
          //blockInput(100);
          if(mainScr.ironStatus != status_error && current_mode==mode_run && (current_time-mainScr.modeTimer < 1000)){
            setCurrentMode(mode_boost);
          }
          mainScr.setMode=main_irontemp;
          break;

        case Rotate_Nothing:
          if(current_time-mainScr.idleTimer > 1000){
            mainScr.setMode=main_irontemp;
          }
          break;
        case Rotate_Increment:
        	incrCount++;
        	if(incrCount>NUM_ROT && current_time-mainScr.modeTimer < TIME_DIFF){
                incrCount=0;
        		input=Rotate_Increment_while_click;

        	}
    		if(current_time-mainScr.modeTimer >= TIME_DIFF){
    			mainScr.modeTimer=current_time;
                incrCount=0;
    		}

        	break;

        case Rotate_Decrement:
        	decrCount++;
			if(decrCount>NUM_ROT && current_time-mainScr.modeTimer < TIME_DIFF){
				decrCount=0;
				input=Rotate_Decrement_while_click;

			}
			if(current_time-mainScr.modeTimer >= TIME_DIFF){
				mainScr.modeTimer=current_time;
				decrCount=0;
			}
        	break;

        case Rotate_Increment_while_click:
           //Left blank intentionally
        	break;

        case Rotate_Decrement_while_click:
          //Left blank intentionally
        	break;

        default:
          break;
      }
      if(input!=Rotate_Nothing){
        IronWake(wakeButton);
      }
    default:
      break;
  }

  if(switchScreenMode()){
    return -1;
  }

  return default_screenProcessInput(scr, input, state);
}

static void drawIcons(uint8_t *refresh){
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
    u8g2_DrawXBMP(&u8g2, 49, OledHeight-shakeXBM[1], shakeXBM[0], shakeXBM[1], &shakeXBM[2]);
  }
  else if(mainScr.shakeActive==3){  // 3 = clear
    mainScr.shakeActive=0;
    u8g2_SetDrawColor(&u8g2,BLACK);
    u8g2_DrawBox(&u8g2, 49, OledHeight-shakeXBM[1], shakeXBM[0], shakeXBM[1]);
    u8g2_SetDrawColor(&u8g2,WHITE);
  }
}


static void drawError(void){
  if(Iron.Error.Flags==(_ACTIVE | _NO_IRON)){                               // Only "No iron detected". Don't show error screen just for it
    u8g2_SetFont(&u8g2, u8g2_font_noIron_Sleep);
    putStrAligned("NO IRON", 20, align_center);
  }
  else{
    uint8_t Err_ypos;

    uint8_t err = (uint8_t)Iron.Error.V_low+Iron.Error.safeMode+(Iron.Error.NTC_low|Iron.Error.NTC_high)+Iron.Error.noIron;
    if(err<4){
      Err_ypos= 12+ ((40-(err*12))/2);
    }
    else{
      Err_ypos=12;
    }
    u8g2_SetFont(&u8g2, u8g2_font_small);
    if(Iron.Error.V_low){
      putStrAligned("VOLTAGE LOW", Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(Iron.Error.safeMode){
      putStrAligned("FAILSAFE MODE", Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(Iron.Error.NTC_high){
      putStrAligned("NTC READ HIGH", Err_ypos, align_center);
      Err_ypos+=12;
    }
    else if(Iron.Error.NTC_low){
      putStrAligned("NTC READ LOW", Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(Iron.Error.noIron){
      putStrAligned("NO IRON DETECTED", Err_ypos, align_center);
      Err_ypos+=12;
    }
  }
}


static void drawScreenSaver(uint8_t *refresh){
#ifdef SCREENSAVER
  if(!*refresh || !screenSaver.enabled || getCurrentMode()!=mode_sleep || mainScr.currentMode!=main_irontemp){
    return;
  }
  screenSaver.update=0;
  //uint16_t _x=(screenSaver.x*5)/2;
  if(screenSaver.x<OledWidth || screenSaver.y<OledHeight){
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_DrawXBMP(&u8g2, screenSaver.x, screenSaver.y, ScrSaverXBM[0], ScrSaverXBM[1], &ScrSaverXBM[2]);
  }
#endif
}

static void drawMode(uint8_t *refresh){
  if(!*refresh) return;

  u8g2_SetFont(&u8g2, u8g2_font_small);

  switch(getCurrentMode()){

    case mode_run:
    {
      char SetTemp[6];
      sprintf(SetTemp,"%u\260C", Iron.CurrentSetTemperature);
      u8g2_DrawStr(&u8g2, 44, 0, SetTemp);
      break;
    }

    case mode_sleep:
      u8g2_DrawStr(&u8g2, 41, 0, "SLEEP");
      break;

    case mode_standby:
      u8g2_DrawStr(&u8g2, 46, 0, "STBY");
      break;

    case mode_boost:
      u8g2_DrawStr(&u8g2, 42, 0, "BOOST");

    default:
      break;
  }
}

static void drawPowerBar(uint8_t *refresh){
  static uint8_t previousPower=0;
  uint8_t update=*refresh;
  if((current_time-barTime)>9){
    barTime = current_time;
    if(previousPower!=mainScr.lastPwr){
      previousPower = mainScr.lastPwr;
      update=1;
    }
  }
  if(update){                          // Update every 10mS or if screen was erased
    if(!*refresh){                           // If screen not erased
      u8g2_SetDrawColor(&u8g2,BLACK);                               // Draw a black square to wipe old widget data
      u8g2_DrawBox(&u8g2, OledWidth-PWR_BAR_WIDTH-2 , OledHeight-7, PWR_BAR_WIDTH, 5);
      u8g2_SetDrawColor(&u8g2,WHITE);
    }
    else{
      u8g2_DrawRFrame(&u8g2, OledWidth-PWR_BAR_WIDTH-4, OledHeight-9, PWR_BAR_WIDTH+4, 9, 2);
    }
    u8g2_DrawBox(&u8g2, OledWidth-PWR_BAR_WIDTH-2, OledHeight-7, mainScr.lastPwr, 5);
  }
}

static void drawPlot(uint8_t *refresh){
#define PLOT_X  7
#define PLOT_Y  12
  if(!plot.enabled){ return; }
  if(*refresh || plot.update){
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
    u8g2_DrawVLine(&u8g2, PLOT_X+3, PLOT_Y, 41);                                // left scale
    for(uint8_t t=0;t<5;t++){
      u8g2_DrawHLine(&u8g2, PLOT_X, PLOT_Y+(10*t), 3);                                     // left ticks
    }
    /*
    12-13-14-15-16-17-18-19-20-21
    22-
    32------
    42-
    52-
    */
    for(uint8_t x=0; x<100; x++){
      uint8_t pos=plot.index+x;
      if(pos>99){ pos-=100; }                                             // Reset index if > 99

      int16_t plotV = (plot.d[pos]-ref)+20;                               // relative to t, +-20C

      if (plotV < 1) plotV = 0;
      else if (plotV > 40) plotV = 40;

      u8g2_DrawVLine(&u8g2, x+PLOT_X+7, (PLOT_Y+40)-plotV, plotV+1);              // data points
    }
    #define set (PLOT_Y+20)
    u8g2_DrawTriangle(&u8g2, PLOT_X+116, set-3, PLOT_X+116, set+3, PLOT_X+110, set);           // Setpoint marker
  }
}

void drawAux(uint8_t *refresh){
  if(!*refresh) return;
  uint8_t frame=0, error=0;
  switch(mainScr.currentMode){
    case main_error:
      error=1;
      break;

    case main_tipselect:
      error=(mainScr.ironStatus==status_error);
      plot.enabled &= !error;
      frame=1;            // In "edit" mode
    case main_irontemp:
      Widget_SetPoint->enabled=0;
    case main_setpoint:
    default:
      break;
  }
  if(error) drawError();
  u8g2_SetFont(&u8g2, u8g2_font_small);
  if(frame){
    uint8_t len = u8g2_GetStrWidth(&u8g2, tipNames[systemSettings.Profile.currentTip])+4;   // Draw edit frame
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_DrawRBox(&u8g2, 0, 54, len, 10, 2);
  }
  u8g2_SetDrawColor(&u8g2, XOR);
  u8g2_DrawStr(&u8g2, 2, 54, tipNames[systemSettings.Profile.currentTip]);                  // Draw tip name
  u8g2_SetDrawColor(&u8g2, WHITE);

}

void main_screen_draw(screen_t *scr){
  uint8_t refresh=0;
  static uint32_t lastState = 0;
  uint32_t currentState = (uint32_t)Iron.Error.Flags<<24 | (uint32_t)Iron.CurrentMode<<16 | mainScr.currentMode;    // Simple method to detect changes

  if( lastState!=currentState || Widget_SetPoint->refresh || Widget_IronTemp->refresh || plot.update || screenSaver.update || scr->refresh==screen_Erase
      #ifdef USE_NTC
      || Widget_AmbTemp->refresh
      #endif
      #ifdef USE_VIN
      || Widget_Voltage->refresh
      #endif
                              ){

    lastState=currentState;
    refresh=1;
  }
  if(refresh){
    scr->refresh=screen_Erased;
    FillBuffer(BLACK, fill_dma);
  }

  u8g2_SetDrawColor(&u8g2, WHITE);

  if(mainScr.ironStatus != status_error){
    drawScreenSaver(&refresh);
  }
  drawPowerBar(&refresh);
  drawIcons(&refresh);
  drawMode(&refresh);
  drawAux(&refresh);
  drawPlot(&refresh);

  default_screenDraw(scr);
}

static void main_screen_init(screen_t *scr) {
  editable_widget_t *edit;
  default_init(scr);
  mainScr.dimStep=0;
  plot.timeStep = (systemSettings.Profile.readPeriod+1)/200;                                                         // Update at the same rate as the system pwm

  mainScr.setMode = main_irontemp;
  switchScreenMode();

  edit = extractEditablePartFromWidget(Widget_SetPoint);
  edit->step = systemSettings.settings.tempStep;
  edit->big_step = systemSettings.settings.tempBigStep;
  edit->max_value = systemSettings.Profile.MaxSetTemperature;
  edit->min_value = systemSettings.Profile.MinSetTemperature;
  setMainScrTempUnit();
  mainScr.idleTimer=current_time;
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
  edit->selectable.state=widget_edit;
  w->radius=8;
  w->enabled=0;
  w->width=128;

  #ifdef USE_VIN
  //  [ V. Supply Widget ]
  //
  newWidget(&w,widget_display,scr);
  Widget_Voltage=w;
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &main_screen_getVin;
  dis->endString="V";
  dis->reservedChars=5;
  dis->textAlign=align_center;
  dis->number_of_dec=1;
  dis->font=u8g2_font_small;
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
  dis->textAlign=align_right;
  dis->number_of_dec=1;
  dis->font=u8g2_font_small;
  dis->getData = &main_screen_getAmbTemp;
  w->posY = 0;
  //w->posX = 90;
  #endif
}


void main_screen_setup(screen_t *scr) {
  scr->draw = &main_screen_draw;
  scr->init = &main_screen_init;
  scr->processInput = &main_screenProcessInput;
  scr->create = &main_screen_create;

  for(int x = 0; x < TipSize; x++) {
    tipNames[x] = systemSettings.Profile.tip[x].name;
  }
}

