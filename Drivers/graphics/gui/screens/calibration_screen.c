/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "screen_common.h"

#define CAL_TIMEOUT 300000                                          // Inactivity timeout in milliseconds

typedef enum { zero_disabled, zero_sampling, zero_capture }zero_state_t;
const  int16_t state_temps[2] = { 2500, 4000 };                     // Temp *10 for better accuracy
static uint8_t error;
static bool backupTempUnit;
static char* state_tempstr[2] = { "250\260C", "400\260C" };
static uint16_t measured_temps[2];
static uint16_t adcAtTemp[2];
static int16_t adcCal[2];
static uint16_t calAdjust[3];
static uint16_t backup_calADC_At_0;
static state_t current_state;
static uint8_t tempReady;
static int32_t measuredTemp;
static uint8_t processCalibration(void);
static void setCalState(state_t s);
static tipData_t *Currtip;
static uint8_t update, update_draw;
static zero_state_t zero_state;
screen_t Screen_calibration;
screen_t Screen_calibration_start;
screen_t Screen_calibration_settings;
static char zeroStr[32];
static widget_t *Widget_Cal_Button;
static widget_t *Widget_Cal_Measured;

static comboBox_item_t *Cal_Combo_Adjust_zero;
static comboBox_item_t *Cal_Combo_Adjust_C250;
static comboBox_item_t *Cal_Combo_Adjust_C400;

static void restore_tip(void){
  __disable_irq();
   *Currtip = backupTip;
   __enable_irq();
}
static void backup_tip(void){
  backupTip = *Currtip ;
}
//=========================================================
static uint8_t processCalibration(void) {

  //  Ensure measured temps and adc measures are valid (cold<250<350<450)
  if (  (measured_temps[cal_400]<measured_temps[cal_250]) || (adcAtTemp[cal_400]<adcAtTemp[cal_250]) ){
    return 1;
  }
  adcCal[cal_250] = map(state_temps[cal_250], 0, measured_temps[cal_250], systemSettings.Profile.calADC_At_0, adcAtTemp[cal_250]);
  adcCal[cal_400] = map(state_temps[cal_400], measured_temps[cal_250], measured_temps[cal_400], adcAtTemp[cal_250], adcAtTemp[cal_400]);
  if(adcCal[cal_250]>4090 || adcCal[cal_250]<0 || adcCal[cal_400]>4090 || adcCal[cal_400]<0 || adcCal[cal_400]<adcCal[cal_250]){    // Check that values are valid and don't exceed ADC range
    return 1;
  }
  return 0;
}
//=========================================================
static void tempReached(uint16_t temp) {
  if(temp*10 == state_temps[(int)current_state])
    tempReady = 1;
}
static setTemperatureReachedCallback tempReachedCallback = &tempReached;
//=========================================================
static void *getMeasuredTemp() {
  return &measuredTemp;
}
static void setMeasuredTemp(int32_t *val) {
  measuredTemp = *val;
}
//=========================================================
static void *getCal250() {
  temp = calAdjust[cal_250];
  return &temp;
}
static void setCal250(int32_t *val) {
  int16_t temp=*val;
  if(temp>=calAdjust[cal_400]){
    temp=calAdjust[cal_400]-10;
  }
  calAdjust[cal_250] = temp;

  __disable_irq();
  Currtip->calADC_At_250 = calAdjust[cal_250];
  __enable_irq();
}
static int Cal250_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setUserTemperature(state_temps[cal_250]/10);
  }
  else{
    setUserTemperature(0);
  }
  return ret;
}
//=========================================================
static void *getCal400() {
  temp = calAdjust[cal_400];
  return &temp;
}
static void setCal400(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=calAdjust[cal_250]){
    temp=calAdjust[cal_250]+10;
  }
  calAdjust[cal_400] = temp;
  __disable_irq();
  Currtip->calADC_At_400 = calAdjust[cal_400];
  __enable_irq();
}
static int Cal400_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setUserTemperature(state_temps[cal_400]/10);
  }
  else{
    setUserTemperature(0);
  }
  return ret;
}
//=========================================================
static int Cal_Settings_SaveAction(widget_t *w, RE_Rotation_t input) {
  if( systemSettings.Profile.Cal250_default != calAdjust[cal_250] ||
      systemSettings.Profile.Cal400_default != calAdjust[cal_400] ||
      backup_calADC_At_0 != calAdjust[cal_0] ){

    __disable_irq();
    systemSettings.Profile.Cal250_default = calAdjust[cal_250];
    systemSettings.Profile.Cal400_default = calAdjust[cal_400];
    backup_calADC_At_0 = calAdjust[cal_0];                               // backup_calADC_At_0 is transferred to profile on screen exiting
    __enable_irq();

    saveSettingsFromMenu(save_Settings);
  }
  return last_scr;
}

static int cancelAction(widget_t* w) {
  return last_scr;
}

static int zero_setAction(widget_t* w, RE_Rotation_t input) {
  if(input==Click){
    if(++zero_state>zero_capture){
      zero_state=zero_disabled;
      __disable_irq();
      systemSettings.Profile.calADC_At_0 = calAdjust[cal_0] = backup_calADC_At_0;   // Apply zero value in real time
      __enable_irq();
    }
    else if(zero_state==zero_capture){
      __disable_irq();
      systemSettings.Profile.calADC_At_0 = calAdjust[cal_0] = TIP.last_avg;
      __enable_irq();
    }
    update=1;
  }
  return -1;
}
//=========================================================
static void setCalState(state_t s) {
  current_state = s;

  if(current_state <= cal_400) {
    setUserTemperature(state_temps[s]/10);
    widgetDisable(Widget_Cal_Measured);
    widgetEnable(Widget_Cal_Button);
    Screen_calibration_start.current_widget=Widget_Cal_Button;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.previous_state=widget_selected;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.state=widget_selected;
    measuredTemp = state_temps[(int)s]/10;
  }
  else if(current_state <= cal_input_400) {
    widgetDisable(Widget_Cal_Button);
    widgetEnable(Widget_Cal_Measured);
    Screen_calibration_start.current_widget=Widget_Cal_Measured;
    ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.previous_state=widget_selected;
    ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state=widget_edit;
  }
  else{
    setUserTemperature(0);
    widgetDisable(Widget_Cal_Measured);
    widgetEnable(Widget_Cal_Button);
    Screen_calibration_start.current_widget=Widget_Cal_Button;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.previous_state=widget_selected;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.state=widget_selected;
    ((button_widget_t*)Widget_Cal_Button->content)->displayString = strings[lang]._BACK;
    if(current_state == cal_finished){
      if(processCalibration()){
        current_state = cal_failed;
      }
      else{
        backupTip.calADC_At_250 = adcCal[cal_250];        // If calibration correct, save values to backup tip
        backupTip.calADC_At_400 = adcCal[cal_400];        // Which will be transferred to the current tip on exiting the screen
      }
    }
  }

  update_draw = 1;
}
//=========================================================
static void Cal_onEnter(screen_t *scr) {
  if(scr == &Screen_settings) {
    backupMode=getCurrentMode();
    backupTemp=getUserTemperature();
    Currtip = getCurrentTip();
    comboResetIndex(Screen_calibration.current_widget);
    error=0;
    setCalibrationMode(enable);
  }

  setUserTemperature(0);
}
static void Cal_onExit(screen_t *scr) {
  if(scr!=&Screen_calibration_start && scr!=&Screen_calibration_settings ){
    setCalibrationMode(disable);
    setCurrentMode(backupMode);
    setUserTemperature(backupTemp);
  }
}

static uint8_t Cal_draw(screen_t *scr){
  if(error==1){
    error=2;
    Screen_calibration.current_widget->enabled=0;
    fillBuffer(BLACK,fill_dma);
    scr->refresh=screen_Erased;
    putStrAligned(strings[lang].CAL_Error, 10, align_center);
    putStrAligned(strings[lang].CAL_Aborting, 25, align_center);
  }
  return (default_screenDraw(scr));
}

static int Cal_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  updatePlot();
  wakeOledDim();
  handleOledDim();
  updateScreenTimer(input);

  if(error){
    if(checkScreenTimer(2000)){
      resetScreenTimer();                       // Reset screen idle timer
      error=0;
      widgetEnable(Screen_calibration.current_widget);
      scr->refresh=screen_Erase;
    }
  }
  else{
    if(input==LongClick || checkScreenTimer(15000)){
      return screen_main;
    }
    else if(input==Rotate_Decrement_while_click){
      return screen_settings;
    }
    return default_screenProcessInput(scr, input, s);
  }
  return -1;
}
static void Cal_create(screen_t *scr) {
  widget_t* w;

  newWidget(&w,widget_combo,scr);
  w->posY=10;

  newComboScreen(w, strings[lang]._START, screen_calibration_start, NULL);
  newComboScreen(w, strings[lang]._SETTINGS, screen_calibration_settings, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
}


static void Cal_Start_init(screen_t *scr) {
  default_init(scr);
  backupTempUnit=getSystemTempUnit();
  setSystemTempUnit(mode_Celsius);
  backup_tip();

  __disable_irq();
  Currtip->calADC_At_250 = systemSettings.Profile.Cal250_default;
  Currtip->calADC_At_400 = systemSettings.Profile.Cal400_default;
  __enable_irq();

  setCalState(cal_250);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  update = update_GUI_Timer();
  update_draw |= update;
  wakeOledDim();
  handleOledDim();
  updatePlot();
  updateScreenTimer(input);

  if(current_state>=cal_finished){
    if(checkScreenTimer(15000) || input==Click){
      return last_scr;
    }
  }
  else{
    if(checkScreenTimer(CAL_TIMEOUT)){
      setUserTemperature(backupTemp);
      setCalibrationMode(disable);
      setCurrentMode(mode_sleep);
      return screen_main;
    }
  }

  if(isIronInError()){
    error=1;
    return last_scr;
  }

  if(tempReady){
    if(current_state<cal_input_250){
      setCalState(current_state+10);
    }
    else if(current_state<cal_finished){
      if(((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state!=widget_edit){
        measuredTemp*=10;
        if( abs(measuredTemp - (state_temps[current_state-10])) > 500 ){      // Abort if the measured temp is >50ºC away from target
          setCalState(cal_needsAdjust);
        }
        else{
          measured_temps[current_state-10] = measuredTemp - last_NTC_C;
          adcAtTemp[current_state-10] = TIP.last_avg;
          if(current_state<cal_input_400){
            tempReady = 0;
            setCalState(current_state-9);
          }
          else{
            setCalState(cal_finished);
          }
        }
      }
    }
  }
  return default_screenProcessInput(scr, input, s);
}

static void Cal_Start_OnExit(screen_t *scr) {
  tempReady = 0;
  setSystemTempUnit(backupTempUnit);
  restore_tip();
}

static uint8_t Cal_Start_draw(screen_t *scr){
  char str[20];

  if(update_draw){
    update_draw=0;

    fillBuffer(BLACK, fill_dma);
    scr->refresh=screen_Erased;
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_SetFont(&u8g2, u8g2_font_menu);

    if(current_state<cal_finished){
      uint8_t s = current_state;
      u8g2_DrawUTF8(&u8g2, 0, 50, systemSettings.Profile.tip[systemSettings.currentTip].name);  // Draw current tip name
      u8g2_DrawUTF8(&u8g2, 8, 6, strings[lang].CAL_Step);            // Draw current cal state

      if(current_state<cal_input_250){
        u8g2_DrawUTF8(&u8g2, 8, 24, strings[lang].CAL_Wait);               // Draw current temp
        sprintf(str, "%3u\260C", last_TIP_C);
        u8g2_DrawUTF8(&u8g2, 85, 24, str);
      }
      else{
        u8g2_DrawUTF8(&u8g2, 8, 24, strings[lang].CAL_Measured);
        s-=10;
      }
      u8g2_DrawUTF8(&u8g2, 85, 6, state_tempstr[s]);
    }
    else if(current_state==cal_finished){
      for(uint8_t x=cal_250;x<(cal_400+1);x++){
        sprintf(str, "%s: %u", state_tempstr[x], adcCal[x]);
        u8g2_DrawUTF8(&u8g2, 20, (x*14), str);
      }
      putStrAligned(strings[lang].CAL_Success, 30, align_center);
    }
    else if(current_state==cal_failed){
      putStrAligned(strings[lang].CAL_Failed, 24, align_center);
    }
    else if(current_state==cal_needsAdjust){
      putStrAligned(strings[lang].CAL_DELTA_HIGH_1, 0, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_2, 15, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_3, 30, align_center);
    }
  }
  return (default_screenDraw(scr));
}

static void Cal_Start_create(screen_t *scr) {
  widget_t* w;
  displayOnly_widget_t *dis;
  editable_widget_t* edit;

  newWidget(&w,widget_button,scr);
  Widget_Cal_Button=w;
  w->width = 65;
  w->posX = displayWidth - w->width - 1;
  w->posY = 48;
  ((button_widget_t*)w->content)->displayString=strings[lang]._CANCEL;
  ((button_widget_t*)w->content)->selectable.tab=0;
  ((button_widget_t*)w->content)->action = &cancelAction;
  ((button_widget_t*)w->content)->font=u8g2_font_menu;
  w->enabled=0;

  newWidget(&w,widget_editable,scr);
  Widget_Cal_Measured=w;
  dis=extractDisplayPartFromWidget(w);
  edit=extractEditablePartFromWidget(w);
  dis->font=u8g2_font_menu;
  dis->reservedChars = 5;
  dis->endString = "\260C";
  dis->getData = &getMeasuredTemp;
  edit->setData =  (void (*)(void *)) &setMeasuredTemp;
  edit->selectable.tab = 1;
  edit->big_step=5;
  edit->step=1;
  w->posX = 82;
  w->posY = 22;
  w->width = 42;
  w->enabled=0;
}


static void Cal_Settings_init(screen_t *scr) {
  default_init(scr);
  comboResetIndex(Screen_calibration_settings.current_widget);
  zero_state = zero_disabled;
  calAdjust[cal_250] = systemSettings.Profile.Cal250_default;
  calAdjust[cal_400] = systemSettings.Profile.Cal400_default;
  calAdjust[cal_0] = systemSettings.Profile.calADC_At_0;
  backup_calADC_At_0 = systemSettings.Profile.calADC_At_0;
  backup_tip();

  __disable_irq();
  Currtip->calADC_At_250 = calAdjust[cal_250];
  Currtip->calADC_At_400 = calAdjust[cal_400];
  __enable_irq();
}

static void Cal_Settings_OnExit(screen_t *scr) {
  restore_tip();
  systemSettings.Profile.calADC_At_0 = backup_calADC_At_0;
}

static int Cal_Settings_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(isIronInError()){
    error=1;
    return last_scr;
  }
  wakeOledDim();
  handleOledDim();
  updatePlot();
  updateScreenTimer(input);

  if(update || update_GUI_Timer()){
    scr->current_widget->refresh=refresh_triggered;
    switch(zero_state){
      case zero_disabled:
        sprintf(zeroStr, "%s%4u", strings[lang].CAL_ZeroSet, backup_calADC_At_0 );
        break;
      case zero_sampling:
        sprintf(zeroStr, "%s%4u", strings[lang].CAL_Sampling, TIP.last_avg );
        break;
      case zero_capture:
        sprintf(zeroStr, "%s%4u", strings[lang].CAL_Captured, calAdjust[cal_0]);
        break;
    }
  }

  if(checkScreenTimer(CAL_TIMEOUT)){
    setUserTemperature(backupTemp);
    setCalibrationMode(disable);
    setCurrentMode(mode_sleep);
    return screen_main;
  }

  if(input==Rotate_Decrement_while_click){
   comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
    if(item->type!=combo_Editable || (item->type==combo_Editable && item->widget->selectable.state!=widget_edit)){
        return last_scr;
    }
  }
  return default_screenProcessInput(scr, input, s);
}

static void Cal_Settings_create(screen_t *scr){
  widget_t* w;
  editable_widget_t* edit;


  // Combo Start
  newWidget(&w,widget_combo,scr);

  newComboAction(w, zeroStr, &zero_setAction, &Cal_Combo_Adjust_zero);
  Cal_Combo_Adjust_zero->dispAlign=align_left;

  newComboEditable(w, strings[lang]._Cal_250, &edit, &Cal_Combo_Adjust_C250);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal250;
  edit->big_step = 20;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setCal250;
  edit->max_value = 4000;
  edit->min_value = 0;
  edit->selectable.processInput=&Cal250_processInput;

  newComboEditable(w, strings[lang]._Cal_400, &edit, &Cal_Combo_Adjust_C400);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal400;
  edit->big_step = 20;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setCal400;
  edit->max_value = 4000;
  edit->min_value = 0;
  edit->selectable.processInput=&Cal400_processInput;

  newComboAction(w, strings[lang]._SAVE, &Cal_Settings_SaveAction, NULL);
  newComboScreen(w, strings[lang]._CANCEL, last_scr , NULL);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screens setup
//-------------------------------------------------------------------------------------------------------------------------------
void calibration_screen_setup(screen_t *scr) {
  screen_t *sc;
  screen_setDefaults(scr);
  scr->processInput = &Cal_ProcessInput;
  scr->draw = &Cal_draw;
  scr->onEnter = &Cal_onEnter;
  scr->onExit = &Cal_onExit;
  scr->create = &Cal_create;

  sc = &Screen_calibration_start;
  oled_addScreen(sc, screen_calibration_start);
  sc->processInput = &Cal_Start_ProcessInput;
  sc->draw = &Cal_Start_draw;
  sc->onExit = &Cal_Start_OnExit;
  sc->init = &Cal_Start_init;
  sc->create = &Cal_Start_create;

  sc = &Screen_calibration_settings;
  oled_addScreen(sc, screen_calibration_settings);
  sc->init = &Cal_Settings_init;
  sc->processInput = &Cal_Settings_ProcessInput;
  sc->onExit = &Cal_Settings_OnExit;
  sc->create = &Cal_Settings_create;

  addSetTemperatureReachedCallback(tempReachedCallback);
}
