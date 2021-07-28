/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "settings_screen.h"
#include "oled.h"
#include "gui.h"

typedef enum { cal_250=0, cal_350=1, cal_450=2, cal_input_250=10, cal_input_350=11, cal_input_450=12, cal_suceed=20, cal_failed=21, cal_needsAdjust=22 }state_t;
static uint32_t temp;
static bool error;
static uint32_t errorTimer;
static uint32_t screenTimer;
static uint16_t adjustSetPoint;
static uint32_t lastUpdateTick;
static int16_t lastTipTemp;
static uint16_t backupTemp;
static bool backupTempUnit;
static uint16_t backupCal250;
static uint16_t backupCal350;
static uint16_t backupCal450;
const static uint16_t state_temps[3] = {250, 350, 450};
static char* state_tempstr[3] = {"250\260C", "350\260C", "450\260C"};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static bool cal_drawText;
static state_t current_state = cal_250;
static uint8_t tempReady;
static int32_t measuredTemp;
static uint16_t adcCal[3];
static uint8_t processCalibration();
static tipData * Currtip;

screen_t Screen_edit_calibration;
screen_t Screen_edit_calibration_start;
screen_t Screen_edit_calibration_adjust;

static widget_t comboWidget_Cal;
static comboBox_widget_t comboBox_Cal;
static comboBox_item_t Cal_Combo_Start;
static comboBox_item_t Cal_Combo_Adjust;
static comboBox_item_t Cal_Combo_Exit;

static widget_t Widget_Cal_Back;
static button_widget_t button_Cal_Back;
static widget_t Widget_Cal_Measured;
static editable_widget_t editable_Cal_Measured;

static widget_t comboWidget_Cal_Adjust;
static comboBox_widget_t comboBox_Cal_Adjust;
static comboBox_item_t Cal_Combo_Adjust_Target;
static comboBox_item_t Cal_Combo_Adjust_Setpoint;
static comboBox_item_t Cal_Combo_Adjust_Save;
static comboBox_item_t Cal_Combo_Adjust_Cancel;

static editable_widget_t editable_Cal_Adjust_Target;
static editable_widget_t editable_Cal_Adjust_Setpoint;



static void tempReached(uint16_t temp) {
  if(temp == state_temps[(int)current_state])
    tempReady = 1;
}
static setTemperatureReachedCallback tempReachedCallback = &tempReached;

static void *getMeasuredTemp() {
  return &measuredTemp;
}
static void setMeasuredTemp(int32_t *val) {
  measuredTemp = *val;
}
//******************************************************
static void *getCalStep() {
  temp = current_state;
  return &temp;
}
static void setCalStep(int32_t *val) {
  current_state = *val;
  adjustSetPoint = adcAtTemp[current_state];
  setDebugTemp(adjustSetPoint);
}
static void *getAdjustSetpoint() {
  temp = adjustSetPoint;
  return &temp;
}
static void setAdjustSetpoint(int32_t *val) {
  adjustSetPoint = *val;
  adcAtTemp[current_state] = *val;
  setDebugTemp(*val);
}
static int cal_adjust_SaveAction(widget_t* w) {
  systemSettings.Profile.Cal250_default = adcAtTemp[cal_250];
  systemSettings.Profile.Cal350_default = adcAtTemp[cal_350];
  systemSettings.Profile.Cal450_default = adcAtTemp[cal_450];
  systemSettings.Profile.CalNTC = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
  saveSettingsFromMenu(save_Settings);
  return screen_edit_calibration;
}
static int cal_adjust_CancelAction(widget_t* w) {
  return screen_edit_calibration;
}
//***************************************************
static void setCalState(state_t s) {
  current_state = s;
  cal_drawText = 1;
  if(current_state <= cal_450) {
    setCurrentMode(mode_run);
    setUserTemperature(state_temps[(int)s]);
    measuredTemp = state_temps[(int)s];
  }
  else if(current_state == cal_suceed) {
    if(processCalibration()){
      backupCal250 = adcCal[cal_250];
      backupCal350 = adcCal[cal_350];
      backupCal450 = adcCal[cal_450];
    }
    else{
      current_state = cal_failed;
    }
  }
}

static int cancelAction(widget_t* w) {
  return screen_edit_calibration;
}
//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void Cal_onEnter(screen_t *scr) {
  screenTimer = HAL_GetTick();
  errorTimer=0;
  error=0;
  if(scr == &Screen_settingsmenu) {
    comboResetIndex(&comboWidget_Cal);
  }
}
static void Cal_draw(screen_t *scr){
  if(error){
    if(errorTimer==0){
      errorTimer=HAL_GetTick();
      FillBuffer(BLACK,fill_dma);
      scr->refresh=screen_Erased;
      putStrAligned("ERROR DETECTED!", 10, align_center);
      putStrAligned("Aborting...", 25, align_center);
    }
  }
  else{
    default_screenDraw(scr);
  }
}

static int Cal_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    error=1;
  }
  if(error){
    if((errorTimer==0) || ((HAL_GetTick()-errorTimer)<2000) ){
      return -1;
    }
    else{
      return screen_settingsmenu;
    }
  }
  else{
    if(input!=Rotate_Nothing){
      screenTimer=HAL_GetTick();
    }
    if(input==LongClick || (HAL_GetTick()-screenTimer)>15000){
      return screen_main;
    }
  }
  return default_screenProcessInput(scr, input, s);
}
//-------------------------------------------------------------------------------------------------------------------------------
// Calibration start screen functions
//-------------------------------------------------------------------------------------------------------------------------------


static void Cal_Start_onEnter(screen_t *scr) {
  Currtip = getCurrentTip();
  Iron.calibrating=1;
  tempReady = 0;
  setCurrentMode(mode_run);
  backupTemp = getUserTemperature();
  backupTempUnit=systemSettings.settings.tempUnit;
  setSystemTempUnit(mode_Celsius);

  backupCal250 = Currtip->calADC_At_250;
  backupCal350 = Currtip->calADC_At_350;
  backupCal450 = Currtip->calADC_At_450;

  Currtip->calADC_At_250 = systemSettings.Profile.Cal250_default;
  Currtip->calADC_At_350 =  systemSettings.Profile.Cal350_default;
  Currtip->calADC_At_450 =  systemSettings.Profile.Cal450_default;

  setCalState(cal_250);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    return screen_edit_calibration;
  }

  if(tempReady){
    if(current_state<=cal_450){
      setCalState(current_state+10);
      widgetDisable(&Widget_Cal_Back);
      button_Cal_Back.selectable.previous_state=widget_idle;
      button_Cal_Back.selectable.state=widget_selected;

      widgetEnable(&Widget_Cal_Measured);
      editable_Cal_Measured.selectable.previous_state=widget_selected;
      editable_Cal_Measured.selectable.state=widget_edit;
      Screen_edit_calibration_start.current_widget=&Widget_Cal_Measured;
    }
    else if(current_state<=cal_input_450){
      if(editable_Cal_Measured.selectable.state!=widget_edit){
        tempReady = 0;
        if( measuredTemp > (state_temps[current_state-10]+50)){      // Abort if the measured temp is >50ºC than requested
          setCalState(cal_needsAdjust);
        }
        else{
          measured_temps[current_state-10] = measuredTemp - (readColdJunctionSensorTemp_x10(mode_Celsius) / 10);
          adcAtTemp[(int)current_state-10] = TIP.last_avg;
          if(current_state<cal_input_450){
            setCalState(current_state-9);
          }
          else{
            setCalState(cal_suceed);
          }
          widgetEnable(&Widget_Cal_Back);
          button_Cal_Back.selectable.previous_state=widget_selected;
          button_Cal_Back.selectable.state=widget_selected;

          widgetDisable(&Widget_Cal_Measured);
          editable_Cal_Measured.selectable.previous_state=widget_idle;
          editable_Cal_Measured.selectable.state=widget_idle;

          Screen_edit_calibration_start.current_widget=&Widget_Cal_Back;
        }
      }
    }
  }
  return default_screenProcessInput(scr, input, s);
}

static void Cal_Start_OnExit(screen_t *scr) {
  setSystemTempUnit(backupTempUnit);
  tempReady = 0;
  current_state = cal_250;
  setUserTemperature(backupTemp);
  setCurrentMode(mode_run);
  Iron.calibrating=0;
  Currtip->calADC_At_250 = backupCal250;
  Currtip->calADC_At_350 = backupCal350;
  Currtip->calADC_At_450 = backupCal450;
}

static void Cal_Start_draw(screen_t *scr){
  char currTemp[6];

  if(cal_drawText || (HAL_GetTick()-lastUpdateTick)>199){
    cal_drawText=0;
    lastUpdateTick=HAL_GetTick();

    FillBuffer(BLACK, fill_dma);
    scr->refresh=screen_Erased;
    u8g2_SetDrawColor(&u8g2, WHITE);
    lastTipTemp = readTipTemperatureCompensated(stored_reading,read_Avg);
    switch((int)current_state){
      case cal_250:
      case cal_350:
      case cal_450:
        u8g2_DrawStr(&u8g2, 6, 12, "CAL STEP:");            // Draw current cal state
        u8g2_DrawStr(&u8g2, 83, 12, state_tempstr[(int)current_state]);
        u8g2_DrawStr(&u8g2, 6, 30, "WAIT...");               // Draw current temp
        sprintf(currTemp, "%3u\260C",lastTipTemp);
        u8g2_DrawStr(&u8g2, 82, 30, currTemp);
        break;
      case cal_suceed:
      {
        char str[20];
        for(uint8_t x=0;x<3;x++){
          sprintf(str, "Cal %s: %u", state_tempstr[x], adcCal[x]);
          u8g2_DrawStr(&u8g2, 6, (x*14), str);
        }
        u8g2_DrawStr(&u8g2, 0, 50, "SUCCEED!");
        break;
      }
      case cal_failed:
          putStrAligned("FAILED!", 15, align_center);
        break;
      case cal_needsAdjust:
          putStrAligned("DELTA TOO HIGH!", 0, align_center);
          putStrAligned("Adjust manually", 15, align_center);
          putStrAligned("and try again", 30, align_center);
        break;

      case cal_input_250:
      case cal_input_350:
      case cal_input_450:
          u8g2_DrawStr(&u8g2, 6, 12, "CAL STEP:");                                 // Draw current cal state
          u8g2_DrawStr(&u8g2, 83, 12, state_tempstr[(int)current_state-10]);
          u8g2_DrawStr(&u8g2, 6, 30, "MEASURED:");//12
        break;
    }
  }
  default_screenDraw(scr);
}


//-------------------------------------------------------------------------------------------------------------------------------
// Calibration adjust screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void Cal_Adjust_init(screen_t *scr) {
  comboResetIndex(&comboWidget_Cal_Adjust);
  adcAtTemp[cal_250] = systemSettings.Profile.Cal250_default;
  adcAtTemp[cal_350] = systemSettings.Profile.Cal350_default;
  adcAtTemp[cal_450] = systemSettings.Profile.Cal450_default;
  Cal_Combo_Adjust_Save.enabled=0;

  current_state = cal_250;
  adjustSetPoint = adcAtTemp[cal_250];
  setDebugTemp(adjustSetPoint);

  setDebugMode(debug_On);
  setCurrentMode(mode_run);
}

static void Cal_Adjust_OnExit(screen_t *scr) {
  setDebugMode(debug_Off);
  setCurrentMode(mode_run);
}

static int Cal_Adjust_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    return screen_edit_calibration;
  }
  // Only enable save option is valid and different data is entered
  if( (adcAtTemp[cal_250] && adcAtTemp[cal_350] && adcAtTemp[cal_450]) &&

    ((adcAtTemp[cal_250] != systemSettings.Profile.Cal250_default)||
     (adcAtTemp[cal_350] != systemSettings.Profile.Cal350_default)||
     (adcAtTemp[cal_450] != systemSettings.Profile.Cal450_default)) &&

    ((adcAtTemp[cal_250] < adcAtTemp[cal_350]) && (adcAtTemp[cal_350] < adcAtTemp[cal_450]))){
    Cal_Combo_Adjust_Save.enabled=1;
  }
  else{
    Cal_Combo_Adjust_Save.enabled=0;
  }
  return default_screenProcessInput(scr, input, s);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screens setup
//-------------------------------------------------------------------------------------------------------------------------------
void calibration_screen_setup(screen_t *scr) {

  widget_t* w;
  displayOnly_widget_t *dis;
  editable_widget_t* edit;
  screen_t* sc;

  //########################################## CALIBRATION SCREEN ##########################################
  //
  screen_setDefaults(scr);
  scr->onEnter = &Cal_onEnter;
  scr->processInput = &Cal_ProcessInput;
  scr->draw = &Cal_draw;
  addSetTemperatureReachedCallback(tempReachedCallback);

  w = &comboWidget_Cal;
  w->content = &comboBox_Cal;
  screen_addWidget(w, scr);
  widgetDefaultsInit(w, widget_combo, &comboBox_Cal);
  comboAddScreen(&Cal_Combo_Start, w,   "START",    screen_edit_calibration_start);
  comboAddScreen(&Cal_Combo_Adjust, w,   "ADJUST",     screen_edit_calibration_adjust);
  comboAddScreen(&Cal_Combo_Exit, w,     "BACK",     screen_settingsmenu);
  w->posY=10;

  //########################################## CALIBRATION START SCREEN ##########################################
  //
  sc = &Screen_edit_calibration_start;
  oled_addScreen(sc, screen_edit_calibration_start);
  screen_setDefaults(sc);
  sc->processInput = &Cal_Start_ProcessInput;
  sc->draw = &Cal_Start_draw;
  sc->onExit = &Cal_Start_OnExit;
  sc->onEnter = &Cal_Start_onEnter;

  w = &Widget_Cal_Back;
  widgetDefaultsInit(w, widget_button, &button_Cal_Back);
  screen_addWidget(w,sc);
  w->posX = 86;
  w->posY = 48;
  w->width = 42;
  button_Cal_Back.displayString="BACK";
  button_Cal_Back.selectable.tab=0;
  button_Cal_Back.action = &cancelAction;

  w=&Widget_Cal_Measured;
  widgetDefaultsInit(w, widget_editable, &editable_Cal_Measured);
  screen_addWidget(w,sc);
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->reservedChars = 5;
  dis->endString = "\260C";
  dis->getData = &getMeasuredTemp;
  edit->setData =  (void (*)(void *)) &setMeasuredTemp;
  edit->selectable.tab = 1;
  w->posX = 80;
  w->posY = 28;
  w->width = 42;
  w->enabled=0;

  //########################################## CALIBRATION ADJUST SCREEN ##########################################
  //
  sc = &Screen_edit_calibration_adjust;
  oled_addScreen(sc, screen_edit_calibration_adjust);
  screen_setDefaults(sc);
  sc->init = &Cal_Adjust_init;
  sc->processInput = &Cal_Adjust_ProcessInput;
  sc->onExit = &Cal_Adjust_OnExit;

  dis=&editable_Cal_Adjust_Target.inputData;
  edit=&editable_Cal_Adjust_Target;
  editableDefaultsInit(edit,widget_editable);
  dis->getData = &getCalStep;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCalStep;
  edit->max_value = cal_450;
  edit->min_value = cal_250 ;
  edit->options = state_tempstr;
  edit->numberOfOptions = 3;

  dis=&editable_Cal_Adjust_Setpoint.inputData;
  edit=&editable_Cal_Adjust_Setpoint;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=4;
  dis->getData = &getAdjustSetpoint;
  edit->big_step = 100;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setAdjustSetpoint;
  edit->max_value = 4000;
  edit->min_value = 0;

  w = &comboWidget_Cal_Adjust;
  screen_addWidget(w, sc);
  widgetDefaultsInit(w, widget_combo, &comboBox_Cal_Adjust);
  comboAddMultiOption(&Cal_Combo_Adjust_Target, w,   "Cal. Step",   &editable_Cal_Adjust_Target);
  comboAddEditable(&Cal_Combo_Adjust_Setpoint,  w,   "ADC Value",   &editable_Cal_Adjust_Setpoint);
  comboAddAction(&Cal_Combo_Adjust_Save,        w,   "SAVE",        &cal_adjust_SaveAction);
  comboAddAction(&Cal_Combo_Adjust_Cancel,      w,   "CANCEL",      &cal_adjust_CancelAction);

}

static uint8_t processCalibration() {
    uint16_t delta = (state_temps[cal_350] - state_temps[cal_250])/2;
    uint16_t ambient = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;

    //  Ensure measured temps are valid (250<350<450)
    if (  (measured_temps[cal_250] >= measured_temps[cal_350]) ||  (measured_temps[cal_350] >= measured_temps[cal_450]) ){
      return 0;
    }

    //  Ensure adc values are valid (250<350<450)
    if (  (adcAtTemp[cal_250] >=adcAtTemp[cal_350]) || (adcAtTemp[cal_350] >= adcAtTemp[cal_450]) ){
      return 0;
    }

    if ((measured_temps[cal_250] + delta) < measured_temps[cal_350]){
      adcCal[cal_350] = map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_350], adcAtTemp[cal_250], adcAtTemp[cal_350]);
    }
    else{
      adcCal[cal_350] = map(state_temps[cal_350], ambient, measured_temps[cal_350], 0, adcAtTemp[cal_350]);
    }
    adcCal[cal_350] += map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]) + 1;
    adcCal[cal_350] >>= 1;

    if((measured_temps[cal_250] + delta) < measured_temps[cal_350]){
      adcCal[cal_250] = map(state_temps[cal_250], measured_temps[cal_250], state_temps[cal_350], adcAtTemp[cal_250], adcCal[cal_350]);
    }
    else{
      adcCal[cal_250] = map(state_temps[cal_250], ambient, state_temps[cal_350], 0, adcCal[cal_350]);
    }

    if ((measured_temps[cal_350] + delta) < measured_temps[cal_450]){
      adcCal[cal_450] = map(state_temps[cal_450], state_temps[cal_350], measured_temps[cal_450], adcCal[cal_350], adcAtTemp[cal_450]);
    }
    else{
      adcCal[cal_450] = map(state_temps[cal_450], state_temps[cal_250], measured_temps[cal_450], adcCal[cal_350], adcAtTemp[cal_450]);
    }

    if (((adcCal[cal_250] + delta) > adcCal[cal_350]) || ((adcCal[cal_350] + delta) > adcCal[cal_450])) {
      adcCal[cal_250] = map(state_temps[cal_250], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
      adcCal[cal_350] = map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
      adcCal[cal_450] = map(state_temps[cal_450], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
    }
  if(adcCal[cal_250]>4090 || adcCal[cal_350]>4090 || adcCal[cal_450]>4090){    // Check that values don't exceed ADC range
    return 0;
  }
  return 1;

}

