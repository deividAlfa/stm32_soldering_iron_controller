                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "main_screen.h"
#include "screen_common.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#endif
#define __BASE_FILE__ "main_screen.c"

#define SCREENSAVER
#define PWR_BAR_WIDTH   52
#define SCALE_FACTOR    (int)((65536*PWR_BAR_WIDTH*1.005)/100)

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static void main_screen_create(screen_t *scr);
static void main_screen_init(screen_t *scr);

static uint32_t barTime;
slide_t screenSaver = {
    .x = 34,
    .y = 0,
    .xAdd = 1,
    .yAdd = 1,
};

enum mode{  main_none=0, main_irontemp, main_error, main_setpoint, main_tipselect,  main_tipselect_auto, main_profileselect, main_tipchange };
enum{ status_ok=0x20, status_error };
enum { temp_numeric, temp_graph };
xbm_t boostAllowXBM = {
  .width=9,
  .height=9,
  .xbm=(const uint8_t[]){
      0x10, 0x00, 0x28, 0x00, 0x44, 0x00, 0x82, 0x00, 0x11, 0x01, 0x28, 0x00,
        0x44, 0x00, 0x82, 0x00, 0x01, 0x01,
  },
};
xbm_t shakeXBM = {
  .width=9,
  .height=9,
  .xbm=(const uint8_t[]){
    0x70, 0x00, 0x80, 0x00, 0x30, 0x01, 0x40, 0x01, 0x45, 0x01, 0x05, 0x00,
    0x19, 0x00, 0x02, 0x00, 0x1C, 0x00,
  },
};
xbm_t tempXBM = {
  .width=5,
  .height=9,
  .xbm=(const uint8_t[]){
    0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x0E, 0x1F, 0x1F, 0x0E,
  },
};
#ifdef USE_VIN
xbm_t voltXBM = {
  .width=6,
  .height=9,
  .xbm=(const uint8_t[]){
    0x20, 0x18, 0x0C, 0x06, 0x3F, 0x18, 0x0C, 0x06, 0x01,
  },
};
#endif
xbm_t warningXBM = {
  .width=6,
  .height=9,
  .xbm=(uint8_t*)(const uint8_t[]){
    0x10, 0x00, 0x28, 0x00, 0x54, 0x00, 0x54, 0x00, 0x82, 0x00, 0x92, 0x00,
    0x01, 0x01, 0xFF, 0x01,
  },
};
#ifdef SCREENSAVER
xbm_t ScrSaverXBM = {
  .width=60,
  .height=49,
  .xbm=(const uint8_t[]){
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
    0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 },
};
#endif
 xbm_t ironXBM_T12 = {
  .width=105,
  .height=7,
  .xbm=(const uint8_t[]){
    0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x07, 0x00, 0x01, 0x3E, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1C, 0x00, 0x01, 0x3E, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x01, 0x3E, 0xF8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x01, 0x01, 0x3E, 0xF8, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x01, 0x3E,
    0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00,
    0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x07, 0x00,
  },
};
 xbm_t ironXBM_JBC = {
  .width=105,
  .height=7,
  .xbm=(const uint8_t[]){
    0x00, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0xF8, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF,
    0x0F, 0x80, 0x07, 0x00, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x80, 0x01,
    0x00, 0x00, 0x08, 0x80, 0x3F, 0x00, 0xFF, 0x03, 0x07, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x08, 0x80, 0xFF, 0x01, 0x80, 0xFF, 0x0F, 0x00,
    0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x08, 0x80, 0x3F, 0x00, 0x00, 0x00,
    0x1C, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x07, 0x00,
    0x00, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0xF8, 0xFF,
    0x00, 0x00,
  },
};
xbm_t x_markXBM = {
  .width=12,
  .height=15,
  .xbm=(const uint8_t[]){
    0x03, 0x0C, 0x07, 0x0E, 0x06, 0x06, 0x0C, 0x03, 0x9C, 0x03, 0xF8, 0x01,
    0xF0, 0x00, 0x60, 0x00, 0xF0, 0x00, 0xF8, 0x01, 0x9C, 0x03, 0x0C, 0x03,
    0x06, 0x06, 0x07, 0x0E, 0x03, 0x0C,
  },
};
xbm_t arrowXBM = {
  .width=12,
  .height=15,
  .xbm=(const uint8_t[]){
    0x00, 0x00, 0x60, 0x00, 0xE0, 0x00, 0xE0, 0x01, 0xC0, 0x03, 0x80, 0x07,
    0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x0F, 0x80, 0x07, 0xC0, 0x03, 0xE0, 0x01,
    0xE0, 0x00, 0x60, 0x00, 0x00, 0x00,
  },
};
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
static widget_t *Widget_SetPoint;

static struct{
  uint8_t lastIronMode;                   // Last stored iron mode
  uint8_t lastIronMode_onClick;           // Last stored iron mode before changing by click
  uint8_t lastIronMode_EncoderFix;        // Last stored iron mode for encoder workaround
  uint8_t lastPwr;                        // Last stored power for widget
  uint8_t shakeActive;                    // Shake icon status: 0=disabled, 1=needs drawing, 2=drawign done, 3=needs clearing
  uint8_t ironStatus;                     // iron status: status_ok, status_error
  uint8_t lastError;                      // Last error, stored to detect error changes
  uint8_t setMode;                        // Main screen variable, set to switch between modes (iron_temp, setpoint, tip_select)
  uint8_t currentMode;                    // Current screen mode (iron_temp, setpoint, tip_select)
  uint8_t displayMode;                    // Iron temp display mode (temp_numeric, temp_graph)
  uint8_t updateReadings;                 // Flag to update power, tip, ambient, voltage widgets
  uint8_t boost_allow;                    // Flag used only for boost mode while in plot graph display
  int16_t lastTip;                        // Last stored tip temperature for widget
  #ifdef USE_NTC
  int16_t lastAmb;                        // Last stored ambient temperature for widget
  #endif
  #ifdef USE_VIN
  uint16_t lastVin;                       // Last stored voltage for widget
  #endif
  uint32_t modeTimer;                     // Timer to track current screen mode time
  uint32_t lastClickTimer;                // Timer to track last user click
  uint32_t inputBlockTimer;               // Timer to block user input Load current time+blocking time in ms
  uint32_t lastEncoderWakeTimer;          // Timer to track last wake send by encoder
  uint32_t lastErrorTimer;                // Timer to track last error time
}mainScr;

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------

void resetModeTimer(void){
  mainScr.modeTimer=current_time;
}

// Checks elapsed time since the current main screen mode was set
uint8_t checkMainScreenModeTimer(uint32_t time){
  if((current_time-mainScr.modeTimer)>time){
    return 1;
  }
  return 0;
}

// Checks elapsed time since the current iron mode was set
uint8_t checkIronModeTimer(uint32_t time){
  if((current_time-getIronCurrentModeTimer())>time){
    return 1;
  }
  return 0;
}

uint8_t checkEncoderWakeTimer(uint32_t time){
  if((current_time-mainScr.lastEncoderWakeTimer)>time){
    return 1;
  }
  return 0;
}
static void setTemp(uint16_t *val) {
  setUserTemperature(*val);
}

static void * getTemp() {
  temp = getUserTemperature();
  return &temp;
}

static void * main_screen_getIronTemp() {
  if(mainScr.updateReadings){
    mainScr.lastTip = readLastTipTemperatureCompensated(read_average, getSystemTempUnit());
    if(getCurrentMode()>mode_sleep){
      uint16_t const targetTemp = getIronTargetTemperature();
      // Lock numeric display if within limits
      if(isIronTargetTempReached() && abs(mainScr.lastTip-targetTemp) < getSystemSettings()->guiTempDenoise){
        mainScr.lastTip = targetTemp;
      }
    }
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
    if(getSystemTempUnit()==mode_Celsius){
      mainScr.lastAmb = last_NTC_C;
    }
    else{
      mainScr.lastAmb = last_NTC_F;
    }
  }
  temp=mainScr.lastAmb;
  return &temp;
}
#endif

void updateIronPower(void) {

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
  Screen_main.state=screen_Erase;
  Screen_main.current_widget=w;
  widgetEnable(w);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void setMainScrTempUnit(void) {
  if(getSystemTempUnit()==mode_Farenheit){
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
void blockInput(uint16_t time){
  mainScr.inputBlockTimer = current_time+time;
}

void updateScreenSaver(void){
#ifdef SCREENSAVER
  if(!screenSaver.enabled || getCurrentMode() !=mode_sleep || (getIronErrorFlags().active)){
    return;
  }
  if(current_time-screenSaver.timer>50){
    screenSaver.timer=current_time;
    screenSaver.x+=screenSaver.xAdd;
    screenSaver.y+=screenSaver.yAdd;
    if(screenSaver.x<-(ScrSaverXBM.width+10) || screenSaver.x>(displayWidth+10)){
      screenSaver.xAdd = -screenSaver.xAdd;
    }
    if(screenSaver.y<-(ScrSaverXBM.height+10) || screenSaver.y>(displayHeight+10)){
      screenSaver.yAdd = -screenSaver.yAdd;
    }
    screenSaver.update=1;
  }
#endif
}

// Switch main screen modes
int8_t switchScreenMode(void){
  if(mainScr.setMode!=main_none){
    resetScreenTimer();
    resetModeTimer();
    plot.enabled = (mainScr.displayMode==temp_graph);
    Screen_main.state=screen_Erase;
    mainScr.updateReadings=1;
    switch(mainScr.setMode){

      case main_irontemp:
        widgetDisable(Widget_SetPoint);
        mainScr.boost_allow=3;
        if(mainScr.ironStatus!=status_error){
          if(!plot.enabled){
            setMainWidget(Widget_IronTemp);
          }
          break;
        }
        mainScr.setMode=main_error;
        // No break intentionally
      case main_tipchange:
      case main_error:
        plot.enabled = 0;
        widgetDisable(Widget_IronTemp);
        break;

      case main_setpoint:
        plot.enabled = 0;
        if(getCurrentMode()==mode_run)
          mainScr.boost_allow=1;
        setMainWidget(Widget_SetPoint);
        break;

      case main_tipselect:
      case main_tipselect_auto:
        break;

      case main_profileselect:
        break;

      default:
        break;
    }
    mainScr.currentMode=mainScr.setMode;
    mainScr.setMode=main_none;
    return 1;                                                             // Changed mode
  }
  return 0;                                                               // No changes
}

int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  uint8_t currentIronMode = getCurrentMode();
  mainScr.updateReadings=update_GUI_Timer();
  updateIronPower();
  updatePlot();
  updateScreenSaver();

  IronError_t ironErrorFlags = getIronErrorFlags();

  if(ironErrorFlags.active){
    if(mainScr.shakeActive){
      mainScr.shakeActive=3;
    }
    if(mainScr.ironStatus!=status_error || mainScr.lastError != ironErrorFlags.Flags){  // If error appeared or changed
      wakeOledDim();                                                      // Wake up screen
      mainScr.ironStatus = status_error;
      mainScr.lastError=ironErrorFlags.Flags;
      mainScr.lastErrorTimer = getIronLastErrorTime();
    }
  }
  else if(mainScr.ironStatus != status_ok){                               // If error is gone
    mainScr.ironStatus = status_ok;
    wakeOledDim();                                                        // Wake up screen
    if(mainScr.currentMode != main_tipchange){
      if( (mainScr.lastError == (FLAG_ACTIVE | FLAG_NO_IRON)) && (current_time - mainScr.lastErrorTimer > 2000) ){    // If last error was no tip and >2 second passed, enable automatic tip selection
        mainScr.setMode = main_tipselect_auto;
        switchScreenMode();
        input=Rotate_Nothing;
      }
    }
  }
  
  // If more than 1 second since last rotation, disable boost allow flag
  if(mainScr.boost_allow && checkMainScreenModeTimer(1000)){
    mainScr.boost_allow=3;
  }

  // Timer for ignoring user input
  if(current_time < mainScr.inputBlockTimer){
    input=Rotate_Nothing;
  }

  if(input!=Rotate_Nothing){
    resetScreenTimer();                                                   // Reset screen idle timer
    if(getDisplayPower()==disable){                                       // If oled off, block user action
      input=Rotate_Nothing;
    }
    wakeOledDim();                                                        // But  wake up screen
  }

  if( ( getSystemSettings()->dim_mode != dim_always && currentIronMode>mode_standby ) ||  // If dim not enabled in all modes
      ( mainScr.lastIronMode != currentIronMode )){                                       // Or working mode changed
    mainScr.lastIronMode = currentIronMode;
    wakeOledDim();                                                        // Refresh timeout / wake up screen
  }

  handleOledDim();

  // Handle shake wake icon drawing and timeout
  if( !mainScr.shakeActive && getIronShakeFlag()){
    clearIronShakeFlag();
    mainScr.shakeActive=1;
  }
  else if((mainScr.shakeActive == 2) && ((current_time- getIronLastShakeTime()) > 150)){
    mainScr.shakeActive=3; // Clear
  }

  // If at main temperature screen
  if(mainScr.currentMode == main_irontemp && (input == Rotate_Increment || input == Rotate_Decrement || input == Click)){  // Capture rotation and click events
    if(currentIronMode!=mode_run && getIronWakeSource()==wakeSrc_Button && !checkIronModeTimer(200)){  // To avoid unwanted mode change, ignore action if low power/boost mode was set <200ms ago
      input = Rotate_Nothing;
    }
    else if(currentIronMode==mode_boost){                               // Click / rotation events will exit boost mode, don't process the input afterwards
      setCurrentMode(mode_run, MEDIUM_BEEP);
      mainScr.lastClickTimer = current_time;
      input = Rotate_Nothing;
    }
    else if(currentIronMode<mode_run && input != Click){               // Ignore click in low power mode. Only rotation will resume run mode.
      mainScr.lastIronMode_EncoderFix = getCurrentMode();
      mainScr.lastEncoderWakeTimer = current_time;
      IronWake(wakeSrc_Button);                                         // Send wake signal
      if(getCurrentMode()==mode_run){                                   // If mode actually changed
        input = Rotate_Nothing;                                         // Ignore rotation to prevent setpoint adjustment
        //resetModeTimer();                                               // Reset mode timer
      }
    }
  }
  // Handle main screen
  switch(mainScr.currentMode){
    case main_irontemp:

      if(mainScr.currentMode!=main_tipchange && mainScr.ironStatus!=status_ok){                                  // When the screen goes to error state (Ignore in tip change)
        mainScr.setMode=main_error;                                       // Set error screen
        break;
      }

      switch((uint8_t)input){

        case LongClick:

          if(current_time -  mainScr.lastClickTimer < 1000){          // Click + long click within 1 second, enter tip change mode.
            setCurrentMode(mainScr.lastIronMode_onClick, 0);          // Restore mode changed by previous click, so "Error resume mode" works as intended
            setIronTipChange(enable);
            mainScr.setMode=main_tipchange;
          }
          else
            return screen_settings;
          break;

        case Rotate_Increment_while_click:
          mainScr.setMode=main_tipselect;
          break;

        case Click:                                                         // Received a Click, enter low power mode.
        {
          uint8_t bad_enc_detect = !checkEncoderWakeTimer(250) && (mainScr.lastIronMode_EncoderFix<mode_run); // Iron was sleeping but mode was changed less than 250ms ago (Rotation event) and we got a click, assume this is encoder malfunction
          mainScr.lastClickTimer = current_time;

          if(!checkEncoderWakeTimer(250) && mainScr.lastIronMode_EncoderFix < mode_boost){// If click issued too soon after iron working mode was changed to non-boost mode
            currentIronMode = mainScr.lastIronMode_EncoderFix;                         // Assume it was a encoder fault rotating while trying to click, restore previous mode, then proceed with low power mode
          }
          if(mainScr.displayMode==temp_graph){                              // If in graph display
            if(mainScr.boost_allow && currentIronMode==mode_run){           // If boost flag enabled and iron running
              mainScr.boost_allow=3;                                        // Clear flag
              setCurrentMode(mode_boost, MEDIUM_BEEP);                      // Set boost mode
              return -1;                                                    // Exit now to prevent processing of the click event, triggering sleep mode
            }
          }
          if( checkMainScreenModeTimer(300) || bad_enc_detect ){            // Wait at least 300ms after entering the screen before allowing click events from entering low power modes, otherwise the user might accidentally enter them.
            uint16_t beep_time =  bad_enc_detect ? 0 : MEDIUM_BEEP;         // Restore mode if bad encoder detected, we shouldn't have waken up, don't beep in this case.
            mainScr.lastIronMode_onClick = currentIronMode;
            if(currentIronMode > mode_standby){
              setCurrentMode(mode_standby, beep_time);
            }
            else{
              setCurrentMode(mode_sleep, beep_time);
            }
          }
          break;
        }
        case Rotate_Increment:
        case Rotate_Decrement:
          if(mainScr.displayMode==temp_graph){
            if(!checkMainScreenModeTimer(1000)){                            // If last rotation step happened <1000ms ago, disable boost allow flag and modify the setpoint.
              mainScr.boost_allow=3;                                        // Disable boost allow flag
              widgetEnable(Widget_SetPoint);                                // Enable the setpoint widget, but don't set it as current widget (Dirty hack), just to be able to process the input
              default_widgetProcessInput(Widget_SetPoint, input, state);    // If the widget is disabled, the widget process will skip it. It will be disabled before drawing in drawMisc function
            }
            else{                                                           // If last step was more than 500ms ago, enable boost flag
              mainScr.boost_allow=1;                                        // Set boost flag. Click within 1 second to enable boost mode
            }
            resetModeTimer();                                               // Reset mode timer
          }
          else{
            mainScr.setMode=main_setpoint;
          }
          return -1;
          break;

        case Rotate_Decrement_while_click:                                  // Switch between numeric and graph displays
          scr->state=screen_Erase;
          if(mainScr.displayMode==temp_numeric){
            mainScr.updateReadings=1;
            mainScr.displayMode=temp_graph;
            widgetDisable(Widget_IronTemp);
            plot.enabled=1;
            plot.update=1;
          }
          else{
            mainScr.displayMode=temp_numeric;
            mainScr.updateReadings=1;
            mainScr.boost_allow=3;                                        // Clear flag
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


    case main_tipselect_auto:
      if(input!=Rotate_Nothing){
        mainScr.currentMode = main_tipselect;                               // Set normal tipselect on user input to use small timeout.
      }
    case main_tipselect:
      {
        uint8_t tip = getCurrentTip();

        if(mainScr.ironStatus==status_error){                                 // If error appears while in tip selection, it needs to update now to avoid overlapping problems
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
            Selected_Tip = getCurrentTip();
            return screen_tip_settings;

          case Click:
            mainScr.setMode=main_irontemp;
            break;

          case Rotate_Nothing:        // Return after 2 seconds of inactivity, or after 5 seconds or error appears when a new tip was placed
            if( (checkScreenTimer(2000)) ||
                (mainScr.currentMode == main_tipselect_auto && (mainScr.ironStatus==status_error || checkScreenTimer(3000)) )){
              mainScr.setMode=main_irontemp;
            }
            break;

          case Rotate_Increment:
            if(++tip >= getCurrentNumberOfTips()){
              tip=0;
            }
            break;

          case Rotate_Decrement:
            if(--tip >= getCurrentNumberOfTips()){          // If underflowed
              tip = getCurrentNumberOfTips()-1;
            }
            break;

          case Rotate_Increment_while_click:
          case Rotate_Decrement_while_click:
            mainScr.setMode=main_profileselect;
            break;

          default:
            break;
        }
        if(tip!=getCurrentTip()){
          loadTipDataFromFlash(tip);
          Screen_main.state=screen_Erase;
        }
        break;
      }

    case  main_profileselect:
      {
        profile = getCurrentProfile();

        if(mainScr.ironStatus==status_error){                                 // If error appears while in tip selection, it needs to update now to avoid overlapping problems
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
          case Rotate_Nothing:                                                // Return after 2 seconds of inactivity
            if(checkScreenTimer(2000)){
              mainScr.setMode=main_irontemp;
            }
          break;
          case Click:
            mainScr.setMode=main_irontemp;
            break;

          case Rotate_Increment:
          case Rotate_Increment_while_click:
            if(++profile >= NUM_PROFILES){
              profile = 0;
            }
            break;

          case Rotate_Decrement:
          case Rotate_Decrement_while_click:
            if(--profile >= NUM_PROFILES){          // If underflowed
              profile = NUM_PROFILES-1;
            }
            break;

          default:
            break;
        }

        if(profile!=getCurrentProfile()){
          updateTempData(force_update);                         // Update current data
          if(isCurrentProfileChanged())                         // If there's unsaved profile data
            Error_Handler();                                    // We shouldn't have anything unsaved here

          loadProfile(profile);
          updateTempData(force_update);                         // Update current data
          Screen_main.state=screen_Erase;
        }
      }
      break;

    case main_setpoint:

      switch((uint8_t)input){
        case LongClick:
        case Click:
          if(!checkMainScreenModeTimer(250)){                                   // Click issued very soon after entering the screen
            if(!checkIronModeTimer(250)){                                       // Check if also iron working mode was recently changed
              currentIronMode = mainScr.lastIronMode_EncoderFix;                // Assume it was a encoder fault rotating while trying to click, restore previous mode, then proceed with low power mode
            }
            if(currentIronMode > mode_standby){                                 // Enter low power mode
              setCurrentMode(mode_standby, MEDIUM_BEEP);
            }
            else{
              setCurrentMode(mode_sleep, MEDIUM_BEEP);
            }
          }
          else if(mainScr.boost_allow && mainScr.ironStatus != status_error && currentIronMode==mode_run){
            setCurrentMode(mode_boost, MEDIUM_BEEP);
          }

          mainScr.setMode=main_irontemp;
          break;

        case Rotate_Nothing:
          if(checkScreenTimer(1000)){
            mainScr.setMode=main_irontemp;
          }
          break;


        case Rotate_Decrement:          // Rotated while in screen, abort boost, change setpoint
        case Rotate_Increment:
          mainScr.boost_allow=3;
          break;
            
        case Rotate_Increment_while_click:
        case Rotate_Decrement_while_click:
          mainScr.setMode=main_irontemp;
          break;

        default:
          break;
      }
      break;

    case main_tipchange:
    {
      static uint8_t trig;

      wakeOledDim();
      if(mainScr.ironStatus==status_error)
        trig=1;

      if((trig && !getIronErrorFlags().noIron) || (current_time - mainScr.modeTimer > 20000) || input==Click){ // Tip is back, >20s in this screen, or click, return to main screen
        setIronTipChange(disable);
        mainScr.setMode = mainScr.ironStatus==status_ok ? (trig ? main_tipselect_auto : main_irontemp) : main_irontemp; // Only go to tip select if the tip was actually changed. On error or unchanged, go to main screen.
        trig=0;
      }
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

static uint8_t  drawIcons(uint8_t refresh){
  if(refresh){
    #ifdef USE_NTC
    u8g2_DrawXBMP(&u8g2, Widget_AmbTemp->posX-tempXBM.width-2, 0, tempXBM.width, tempXBM.height, tempXBM.xbm);
    #endif

    #ifdef USE_VIN
    u8g2_DrawXBMP(&u8g2, 0, 0, voltXBM.width, voltXBM.height, voltXBM.xbm);
    #endif
  }
  if(mainScr.boost_allow==1){    
    u8g2_DrawXBMP(&u8g2, displayWidth-boostAllowXBM.width-PWR_BAR_WIDTH-8, displayHeight-boostAllowXBM.height, boostAllowXBM.width, boostAllowXBM.height, boostAllowXBM.xbm);
  }
  else if(mainScr.boost_allow==3){
      mainScr.boost_allow=0;
      u8g2_SetDrawColor(&u8g2,BLACK);
      u8g2_DrawBox(&u8g2, displayWidth-boostAllowXBM.width-PWR_BAR_WIDTH-8, displayHeight-boostAllowXBM.height, boostAllowXBM.width, boostAllowXBM.height);
      u8g2_SetDrawColor(&u8g2,WHITE);
  }
  else if(mainScr.shakeActive==1 || (mainScr.shakeActive==2 && refresh) ){ //1 = needs drawing, 2 = already drawn
    mainScr.shakeActive=2;
    u8g2_DrawXBMP(&u8g2, displayWidth-shakeXBM.width-PWR_BAR_WIDTH-8, displayHeight-shakeXBM.height, shakeXBM.width, shakeXBM.height, shakeXBM.xbm);
    return 1;
  }
  else if(mainScr.shakeActive==3){                                    // 3 = needs clearing
    mainScr.shakeActive=0;
    u8g2_SetDrawColor(&u8g2,BLACK);
    u8g2_DrawBox(&u8g2, displayWidth-shakeXBM.width-PWR_BAR_WIDTH-8, displayHeight-shakeXBM.height, shakeXBM.width, shakeXBM.height);
    u8g2_SetDrawColor(&u8g2,WHITE);
    return 1;
  }
  return 0;
}


static uint8_t  drawScreenSaver(uint8_t refresh){
#ifdef SCREENSAVER
  if(!refresh || !screenSaver.enabled || getCurrentMode()!=mode_sleep || mainScr.currentMode!=main_irontemp){
    return 0;
  }
  screenSaver.update=0;
  if(screenSaver.x>(-ScrSaverXBM.width) ||screenSaver.x<displayWidth || screenSaver.y>(-ScrSaverXBM.height) || screenSaver.y<displayHeight ){
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_DrawXBMP(&u8g2, screenSaver.x, screenSaver.y, ScrSaverXBM.width, ScrSaverXBM.height, ScrSaverXBM.xbm);
    return 1;
  }
#endif
  return 0;
}

static void  drawMode(uint8_t refresh){
  if(!refresh) return;

  u8g2_SetFont(&u8g2, u8g2_font_small);

  switch(getCurrentMode()){

    case mode_run:
    {
      char SetTemp[8];
      char c;
      if(getSystemTempUnit()==mode_Celsius){
        c='C';
      }
      else{
        c='F';
      }
      sprintf(SetTemp,"%u\260%c", getIronTargetTemperature(),c);
      u8g2_DrawUTF8(&u8g2, 43, 0, SetTemp);
      break;
    }

    case mode_sleep:
      u8g2_DrawUTF8(&u8g2,  strings[lang].main_mode_Sleep_xpos, 0, strings[lang].main_mode_Sleep);
      break;

    case mode_standby:
      u8g2_DrawUTF8(&u8g2, strings[lang].main_mode_Standby_xpos, 0, strings[lang].main_mode_Standby);
      break;

    case mode_boost:
    case mode_coldboost:
      u8g2_DrawUTF8(&u8g2, strings[lang].main_mode_Boost_xpos, 0, strings[lang].main_mode_Boost);

    default:
      break;
  }
}

static uint8_t  drawPowerBar(uint8_t refresh){
  static uint8_t previousPower=0;
  uint8_t update=refresh;
  if((current_time-barTime)>9){
    barTime = current_time;
    if(previousPower!=mainScr.lastPwr){
      previousPower = mainScr.lastPwr;
      update=1;
    }
  }
  if(update){                          // Update every 10mS or if screen was erased
    if(!refresh){                           // If screen not erased
      u8g2_SetDrawColor(&u8g2,BLACK);                               // Draw a black square to wipe old widget data
      u8g2_DrawBox(&u8g2, displayWidth-PWR_BAR_WIDTH-2 , displayHeight-7, PWR_BAR_WIDTH, 5);
      u8g2_SetDrawColor(&u8g2,WHITE);
    }
    else{
      u8g2_DrawRFrame(&u8g2, displayWidth-PWR_BAR_WIDTH-4, displayHeight-9, PWR_BAR_WIDTH+4, 9, 2);
    }
    u8g2_DrawBox(&u8g2, displayWidth-PWR_BAR_WIDTH-2, displayHeight-7, mainScr.lastPwr, 5);
    return 1;
  }
  return 0;
}

static uint8_t  drawPlot(uint8_t refresh){
#define PLOT_X  7
#define PLOT_Y  12

  plot.enabled &= !(getIronErrorFlags().active);

  if(!plot.enabled){ return 0; }
  if(refresh || plot.update){
    int16_t ref;
    if(getCurrentMode() != mode_sleep){
      ref=getIronTargetTemperature();
      if(getSystemTempUnit()==mode_Farenheit){
        ref = TempConversion(ref, mode_Celsius, 0);
      }
    }
    else{
      ref = (last_NTC_C+5)/10;                                          // Use ambient temperature as reference when sleeping
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
    return 1;
  }
  return 0;
}

static void drawTip(void){
  if(getCurrentProfile() == profile_T12)
    u8g2_DrawXBM(&u8g2, (displayWidth-ironXBM_T12.width-x_markXBM.width-5)/2, (displayHeight-ironXBM_T12.height)/2, ironXBM_T12.width, ironXBM_T12.height, ironXBM_T12.xbm);
  else
    u8g2_DrawXBM(&u8g2, (displayWidth-ironXBM_JBC.width-x_markXBM.width-5)/2, (displayHeight-ironXBM_JBC.height)/2, ironXBM_JBC.width, ironXBM_JBC.height, ironXBM_JBC.xbm);
}

static uint8_t  drawError(uint8_t refresh){
  static uint32_t last_time;
  static uint8_t x_mark_state;
  if(mainScr.ironStatus!=status_error || mainScr.currentMode==main_setpoint ){
    x_mark_state=0;
    last_time = current_time;
    return 0;
  }

  IronError_t const ironErrorFlags = getIronErrorFlags();

  if(ironErrorFlags.Flags == (FLAG_ACTIVE | FLAG_NO_IRON)){                               // Only "No iron detected". Don't show error screen just for it
    if(mainScr.currentMode==main_tipchange)                                               // Ignore no tip error when changing the tip
      return 0;

    uint8_t xp, update = 0;

    if(getCurrentProfile() == profile_T12)
      xp = (displayWidth-ironXBM_T12.width-x_markXBM.width-5)/2;
    else
      xp = (displayWidth-ironXBM_JBC.width-x_markXBM.width-5)/2;

    if(refresh){
      drawTip();
      update = 1;
    }

    if(current_time-last_time>500){
      last_time=current_time;
      x_mark_state ^=1;
      update=1;
    }

    if(update){
      if(x_mark_state){
        u8g2_SetDrawColor(&u8g2, BLACK);
        if(getCurrentProfile() == profile_T12)
          u8g2_DrawBox(&u8g2, xp+ironXBM_T12.width+5, (displayHeight-x_markXBM.height)/2, x_markXBM.width, x_markXBM.height);
        else
          u8g2_DrawBox(&u8g2, xp+ironXBM_JBC.width+5, (displayHeight-x_markXBM.height)/2, x_markXBM.width, x_markXBM.height);
        u8g2_SetDrawColor(&u8g2, WHITE);
      }
      else{
        if(getCurrentProfile() == profile_T12)
          u8g2_DrawXBM(&u8g2, xp+ironXBM_T12.width+5, (displayHeight-x_markXBM.height)/2, x_markXBM.width, x_markXBM.height, x_markXBM.xbm);
        else
          u8g2_DrawXBM(&u8g2, xp+ironXBM_JBC.width+5, (displayHeight-x_markXBM.height)/2, x_markXBM.width, x_markXBM.height, x_markXBM.xbm);
      }
    }
    return update;
  }
  else if(refresh){
    uint8_t Err_ypos;

    uint8_t err = (uint8_t)ironErrorFlags.V_low +
                  (uint8_t)ironErrorFlags.safeMode +
                  (uint8_t)(ironErrorFlags.NTC_low | ironErrorFlags.NTC_high) +
                  ironErrorFlags.noIron;
    if(err==0)
      return 0;
    else if(err<4){
      Err_ypos= 12+ ((40-(err*12))/2);
    }
    else{
      Err_ypos=12;
    }
    u8g2_SetFont(&u8g2, u8g2_font_small);
    if(ironErrorFlags.V_low){
      putStrAligned(strings[lang].main_error_VoltageLow, Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(ironErrorFlags.safeMode){
      putStrAligned(strings[lang].main_error_failsafe, Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(ironErrorFlags.NTC_high){
      putStrAligned(strings[lang].main_error_NTC_high, Err_ypos, align_center);
      Err_ypos+=12;
    }
    else if(ironErrorFlags.NTC_low){
      putStrAligned(strings[lang].main_error_NTC_low, Err_ypos, align_center);
      Err_ypos+=12;
    }
    if(ironErrorFlags.noIron){
      putStrAligned(strings[lang].main_error_noIron_Detected, Err_ypos, align_center);
      Err_ypos+=12;
    }
    return 1;
  }
  return 0;
}

static void  drawMisc(uint8_t refresh){
  static uint8_t arrow_state;

  if(!refresh) return;
  uint8_t len = 0;
  char *s = NULL;

  Widget_SetPoint->enabled &= (mainScr.currentMode==main_setpoint);                            // Disable setpoint widget if not in setpoint screen

  u8g2_SetFont(&u8g2, u8g2_font_small);
  if(mainScr.currentMode==main_tipselect || mainScr.currentMode==main_tipselect_auto){

  }
  if(mainScr.currentMode==main_profileselect){
    len = u8g2_GetUTF8Width(&u8g2, profileStr[getCurrentProfile()])+4;              // Profile string len
    s = profileStr[getCurrentProfile()];                                            // Profile name
  }
  else{
    len = u8g2_GetUTF8Width(&u8g2, getCurrentTipData()->name)+4;                    // Tip string len
    s = getCurrentTipData()->name;                                                  // Tip name
  }
  if(mainScr.currentMode==main_tipselect || mainScr.currentMode==main_tipselect_auto || mainScr.currentMode==main_profileselect){     // Tip / profile selection active
    u8g2_DrawRBox(&u8g2, 0, 54, len, 10, 2);                                                  // Draw edit frame
    u8g2_SetDrawColor(&u8g2, BLACK);
  }
  else if(mainScr.currentMode==main_tipchange){
    if( refresh || arrow_state==0){
      uint8_t xp;

      if(getCurrentProfile() == profile_T12)
        xp = (displayWidth-ironXBM_T12.width-x_markXBM.width-5)/2;
      else
        xp = (displayWidth-ironXBM_JBC.width-x_markXBM.width-5)/2;

      arrow_state=1;
      drawTip();
      if(getCurrentProfile() == profile_T12)
        u8g2_DrawXBM(&u8g2, xp+ironXBM_T12.width+5, (displayHeight-arrowXBM.height)/2, arrowXBM.width, arrowXBM.height, arrowXBM.xbm);
      else
        u8g2_DrawXBM(&u8g2, xp+ironXBM_JBC.width+5, (displayHeight-arrowXBM.height)/2, arrowXBM.width, arrowXBM.height, arrowXBM.xbm);
    }
  }
  else
    arrow_state=0;

  u8g2_DrawUTF8(&u8g2, 2, 54, s);                                                             // Draw tip/profile name
  u8g2_SetDrawColor(&u8g2, WHITE);
  return;
}

static uint8_t main_screen_draw(screen_t *scr){
  static uint32_t lastState = 0;

  uint8_t refresh= 0u;
  uint8_t ret    = 0u;

  IronError_t const ironErrorFlags = getIronErrorFlags();

  uint32_t currentState = (uint32_t)ironErrorFlags.Flags<<24 | (uint32_t)getCurrentMode()<<16 | mainScr.currentMode;    // Simple method to detect changes

  if( lastState!=currentState || Widget_SetPoint->refresh || Widget_IronTemp->refresh || plot.update || screenSaver.update || scr->state==screen_Erase
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
    scr->state=screen_Erased;
    fillBuffer(BLACK, fill_dma);
  }

  mainScr.updateReadings = 0;                   // If update flag was set in main_screenProcessInput, widgets were already updated. Clear flag now to prevent readings from updating again.

  u8g2_SetDrawColor(&u8g2, WHITE);

  if(mainScr.ironStatus != status_error){
    ret |= drawScreenSaver(refresh);
  }
  ret |= drawPowerBar(refresh);
  ret |= drawIcons(refresh);
  drawMode(refresh);
  drawMisc(refresh);
  ret |= drawPlot(refresh);
  ret |= drawError(refresh);
  ret |= default_screenDraw(scr);
  if(ironErrorFlags.active && mainScr.ironStatus!=status_error){
    return 0u;                                                                                // If a new error appeared during the screen draw, skip oled update to avoid random artifacts
  }
  return (ret);
}

static void main_screen_init(screen_t *scr) {
  editable_widget_t *edit;
  default_init(scr);
  clearIronShakeFlag();
  mainScr.shakeActive = 0;
  plot.timeStep = (getProfileSettings()->readPeriod+1)/200;                                                         // Update at the same rate as the system pwm

  mainScr.setMode = main_irontemp;
  switchScreenMode();

  edit = extractEditablePartFromWidget(Widget_SetPoint);
  edit->step = getSystemSettings()->tempStep;
  edit->big_step = getSystemSettings()->tempBigStep;
  edit->max_value = getProfileSettings()->MaxSetTemperature;
  edit->min_value = getProfileSettings()->MinSetTemperature;
  setMainScrTempUnit();
}

static void main_screen_create(screen_t *scr){
  widget_t *w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  //  [ Iron Temp Widget ]
  //
  newWidget(&w,widget_display,scr,NULL);
  Widget_IronTemp = w;
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars=5;
  dis->dispAlign=align_center;
  dis->textAlign=align_center;
  dis->font=u8g2_font_iron_temp;
  w->posY = 16;
  dis->getData = &main_screen_getIronTemp;
  w->enabled=0;

  //  [ Iron Setpoint Widget ]
  //
  newWidget(&w,widget_editable,scr,NULL);
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
  edit->setData = (setterFn)&setTemp;
  w->frameType=frame_solid;
  edit->selectable.state=widget_edit;
  w->radius=4;
  w->enabled=0;
  w->width=128;

  #ifdef USE_VIN
  //  [ V. Supply Widget ]
  //
  newWidget(&w,widget_display,scr,NULL);
  Widget_Voltage=w;
  dis=extractDisplayPartFromWidget(w);
  dis->getData = &main_screen_getVin;
  dis->endString="V";
  dis->reservedChars=5;
  dis->textAlign=align_center;
  dis->number_of_dec=1;
  dis->font=u8g2_font_small;
  w->posY= 0;
  w->posX = voltXBM.width+2;
  edit=extractEditablePartFromWidget(w);
  //w->width = 40;
  #endif

  #ifdef USE_NTC
  //  [ Ambient Temp Widget ]
  //
  newWidget(&w,widget_display,scr,NULL);
  Widget_AmbTemp=w;
  dis=extractDisplayPartFromWidget(w);
  dis->reservedChars=7;
  dis->dispAlign=align_right;
  dis->textAlign=align_right;
  dis->number_of_dec=1;
  dis->font=u8g2_font_small;
  dis->getData = &main_screen_getAmbTemp;
  w->posY = 0;
  w->posX = 90;
  w->width = 38;
  #endif
}


void main_screen_setup(screen_t *scr) {
  scr->draw = &main_screen_draw;
  scr->init = &main_screen_init;
  scr->processInput = &main_screenProcessInput;
  scr->create = &main_screen_create;
}
