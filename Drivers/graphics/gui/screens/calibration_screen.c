/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "screen_common.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#endif
#define __BASE_FILE__ "calibration_screen.c"

#define CAL_TIMEOUT 300000                                          // Inactivity timeout in milliseconds

const char* state_tempstr[2] = { "250\260C", "400\260C" };
const  int16_t state_temps[2] = { 2500, 4000 };                     // Temp *10 for better accuracy

typedef enum { zero_disabled, zero_sampling, zero_capture }zero_state_t;
typedef struct {
  char zeroStr[32];
  uint8_t error, tempReady, backupMode, backupTempUnit, update, update_draw;
  zero_state_t zero_state;
  uint16_t measured_temps[2];
  uint16_t adcAtTemp[2];
  int16_t adcCal[2];
  uint16_t calAdjust[3];
  uint16_t backup_calADC_At_0;
  state_t current_state;
  int32_t measuredTemp;
}cal_t;

static cal_t * cal;

screen_t Screen_calibration;
screen_t Screen_calibration_start;
screen_t Screen_calibration_settings;
static widget_t *Widget_Cal_Button;
static widget_t *Widget_Cal_Measured;

static comboBox_item_t *Cal_Combo_Adjust_zero;
static comboBox_item_t *Cal_Combo_Adjust_C250;
static comboBox_item_t *Cal_Combo_Adjust_C400;


static uint8_t processCalibration(void);
static void setCalState(state_t s);

static void restore_tip(void){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
   setCurrentTipData(&backupTip);
   __set_PRIMASK(_irq);
}
//=========================================================
static uint8_t processCalibration(void) {

  //  Ensure measured temps and adc measures are valid (cold<250<350<450)
  if (  (cal->measured_temps[cal_400]<cal->measured_temps[cal_250]) || (cal->adcAtTemp[cal_400]<cal->adcAtTemp[cal_250]) ){
    return 1;
  }
  cal->adcCal[cal_250] = map(state_temps[cal_250], 0, cal->measured_temps[cal_250], getProfileSettings()->calADC_At_0, cal->adcAtTemp[cal_250]);
  cal->adcCal[cal_400] = map(state_temps[cal_400], cal->measured_temps[cal_250], cal->measured_temps[cal_400], cal->adcAtTemp[cal_250], cal->adcAtTemp[cal_400]);
  if(cal->adcCal[cal_250]>4090 || cal->adcCal[cal_250]<0 || cal->adcCal[cal_400]>4090 || cal->adcCal[cal_400]<0 || cal->adcCal[cal_400]<cal->adcCal[cal_250]){    // Check that values are valid and don't exceed ADC range
    return 1;
  }
  return 0;
}
//=========================================================
static void tempReached(uint16_t temp) {
  if(temp*10 == state_temps[(int)cal->current_state])
    cal->tempReady = 1;
}
static setTemperatureReachedCallback tempReachedCallback = &tempReached;
//=========================================================
static void *getMeasuredTemp() {
  return &cal->measuredTemp;
}
static void setMeasuredTemp(int32_t *val) {
  cal->measuredTemp = *val;
}
//=========================================================
static void *getCal250() {
  temp = cal->calAdjust[cal_250];
  return &temp;
}
static void setCal250(int32_t *val) {
  int16_t temp=*val;
  if(temp>=cal->calAdjust[cal_400]){
    temp=cal->calAdjust[cal_400]-10;
  }
  cal->calAdjust[cal_250] = temp;
  getCurrentTipData()->calADC_At_250 = cal->calAdjust[cal_250];
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
  temp = cal->calAdjust[cal_400];
  return &temp;
}
static void setCal400(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=cal->calAdjust[cal_250]){
    temp=cal->calAdjust[cal_250]+10;
  }
  cal->calAdjust[cal_400] = temp;
  getCurrentTipData()->calADC_At_400 = cal->calAdjust[cal_400];
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
  if( getProfileSettings()->Cal250_default != cal->calAdjust[cal_250] ||
      getProfileSettings()->Cal400_default != cal->calAdjust[cal_400] ||
      cal->backup_calADC_At_0 != cal->calAdjust[cal_0] ){

    getProfileSettings()->Cal250_default = cal->calAdjust[cal_250];
    getProfileSettings()->Cal400_default = cal->calAdjust[cal_400];
    cal->backup_calADC_At_0 = cal->calAdjust[cal_0];                               // cal->backup_calADC_At_0 is transferred to profile on screen exiting
    cal->current_state = cal_save;
  }
  return last_scr;
}

static int cancelAction(widget_t* w) {
  return last_scr;
}

static int zero_setAction(widget_t* w, RE_Rotation_t input) {
  if(input==Click){
    if(++cal->zero_state>zero_capture){
      cal->zero_state=zero_disabled;
      getProfileSettings()->calADC_At_0 = cal->calAdjust[cal_0] = cal->backup_calADC_At_0;   // Apply zero value in real time
    }
    else if(cal->zero_state==zero_capture){
      getProfileSettings()->calADC_At_0 = cal->calAdjust[cal_0] = TIP.last_avg;
    }
    cal->update=1;
  }
  return -1;
}
//=========================================================
static void setCalState(state_t s) {
  cal->current_state = s;

  if(cal->current_state <= cal_400) {
    setCurrentMode(mode_run);
    setUserTemperature(state_temps[s]/10);
    widgetDisable(Widget_Cal_Measured);
    widgetEnable(Widget_Cal_Button);
    Screen_calibration_start.current_widget=Widget_Cal_Button;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.previous_state=widget_selected;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.state=widget_selected;
    cal->measuredTemp = state_temps[(int)s]/10;
  }
  else if(cal->current_state <= cal_input_400) {
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
    if(cal->current_state == cal_finished){
      if(processCalibration()){
        cal->current_state = cal_failed;
      }
      else{
        backupTip.calADC_At_250 = cal->adcCal[cal_250];        // If calibration correct, save values to backup tip
        backupTip.calADC_At_400 = cal->adcCal[cal_400];        // Which will be transferred to the current tip on exiting the screen
        cal->current_state = cal_finished;
      }
    }
  }
  cal->update_draw = 1;
}
//=========================================================
static void Cal_onEnter(screen_t *scr) {
  if(scr == &Screen_settings) {
    cal = _calloc(1,sizeof(cal_t));
    if(!cal)
      Error_Handler();
    backupMode=getCurrentMode();
    backupTempUnit=getUserTemperature();
    backupTip = *getCurrentTipData();
    comboResetIndex(Screen_calibration.current_widget);
    cal->error=0;
    setIronCalibrationMode(enable);
  }

  setUserTemperature(0);
}
static void Cal_onExit(screen_t *scr) {
  if(scr!=&Screen_calibration_start && scr!=&Screen_calibration_settings ){
    _free(cal);
    setIronCalibrationMode(disable);
    setCurrentMode(backupMode);
    setUserTemperature(backupTemp);
  }
}

static uint8_t Cal_draw(screen_t *scr){
  if(cal->error==1){
    cal->error=2;
    Screen_calibration.current_widget->enabled=0;
    fillBuffer(BLACK,fill_dma);
    scr->state=screen_Erased;
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

  if(cal->error){
    if(checkScreenTimer(2000)){
      resetScreenTimer();                       // Reset screen idle timer
      cal->error=0;
      widgetEnable(Screen_calibration.current_widget);
      scr->state=screen_Erase;
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

  newWidget(&w,widget_combo,scr, NULL);
  w->posY=10;

  newComboScreen(w, strings[lang]._START, screen_calibration_start, NULL);
  newComboScreen(w, strings[lang]._SETTINGS, screen_calibration_settings, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
}


static void Cal_Start_init(screen_t *scr) {
  default_init(scr);
  backupTempUnit=getSystemTempUnit();
  setSystemTempUnit(mode_Celsius);
  getCurrentTipData()->calADC_At_250 = getProfileSettings()->Cal250_default;
  getCurrentTipData()->calADC_At_400 = getProfileSettings()->Cal400_default;
  setCalState(cal_250);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  cal->update = update_GUI_Timer();
  cal->update_draw |= cal->update;
  wakeOledDim();
  handleOledDim();
  updatePlot();
  updateScreenTimer(input);

  if(cal->current_state>=cal_finished){
    if(checkScreenTimer(15000) || input==Click){
      if(cal->current_state==cal_finished)
        cal->current_state=cal_save;
      return last_scr;
    }
  }
  else{
    if(checkScreenTimer(CAL_TIMEOUT)){
      setUserTemperature(backupTemp);
      setIronCalibrationMode(disable);
      setCurrentMode(mode_sleep);
      return screen_main;
    }
  }

  if(getIronError()){
    cal->error=1;
    return last_scr;
  }

  if(cal->tempReady){
    if(cal->current_state<cal_input_250){
      setCalState(cal->current_state+10);
    }
    else if(cal->current_state<cal_finished){
      if(((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state!=widget_edit){
        cal->measuredTemp*=10;
        if( abs(cal->measuredTemp - (state_temps[cal->current_state-10])) > 500 ){      // Abort if the measured temp is >50ºC away from target
          setCalState(cal_needsAdjust);
        }
        else{
          cal->measured_temps[cal->current_state-10] = cal->measuredTemp - last_NTC_C;
          cal->adcAtTemp[cal->current_state-10] = TIP.last_avg;
          if(cal->current_state<cal_input_400){
            cal->tempReady = 0;
            setCalState(cal->current_state-9);
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
  cal->tempReady = 0;
  setSystemTempUnit(backupTempUnit);
  restore_tip();                              // Restore default calibration temps if calibration was cancelled, or new calibration values if ok.
  if(cal->current_state==cal_save)
    saveSettings(save_Tip, mode_SaveTip, getCurrentTip(), no_reboot);              // Save now we have all heap free
}

static uint8_t Cal_Start_draw(screen_t *scr){
  char str[20];

  if(cal->update_draw){
    cal->update_draw=0;

    fillBuffer(BLACK, fill_dma);
    scr->state=screen_Erased;
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_SetFont(&u8g2, u8g2_font_menu);

    if(cal->current_state<cal_finished){
      uint8_t s = cal->current_state;
      u8g2_DrawUTF8(&u8g2, 0, 50, backupTip.name);  // Draw current tip name
      u8g2_DrawUTF8(&u8g2, 8, 6, strings[lang].CAL_Step);            // Draw current cal state

      if(cal->current_state<cal_input_250){
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
    else if(cal->current_state==cal_finished){
      for(uint8_t x=cal_250;x<(cal_400+1);x++){
        sprintf(str, "%s: %u", state_tempstr[x], cal->adcCal[x]);
        u8g2_DrawUTF8(&u8g2, 20, (x*14), str);
      }
      putStrAligned(strings[lang].CAL_Success, 30, align_center);
    }
    else if(cal->current_state==cal_failed){
      putStrAligned(strings[lang].CAL_Failed, 24, align_center);
    }
    else if(cal->current_state==cal_needsAdjust){
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
  button_widget_t* button;

  newWidget(&w,widget_button,scr,(void*)&button);
  Widget_Cal_Button=w;
  button->font=u8g2_font_menu;
  button->displayString=strings[lang]._CANCEL;
  button->selectable.tab=0;
  button->action = &cancelAction;
  button->font=u8g2_font_menu;
  button->dispAlign=align_right;
  w->posY = 48;
  w->enabled=0;

  newWidget(&w,widget_editable,scr, NULL);
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
  cal->zero_state = zero_disabled;
  cal->calAdjust[cal_250] = getProfileSettings()->Cal250_default;
  cal->calAdjust[cal_400] = getProfileSettings()->Cal400_default;
  cal->calAdjust[cal_0] = getProfileSettings()->calADC_At_0;
  cal->backup_calADC_At_0 = getProfileSettings()->calADC_At_0;
  backupTip = *getCurrentTipData();
  getCurrentTipData()->calADC_At_250 = cal->calAdjust[cal_250];
  getCurrentTipData()->calADC_At_400 = cal->calAdjust[cal_400];
}

static void Cal_Settings_OnExit(screen_t *scr) {
  getProfileSettings()->calADC_At_0 = cal->backup_calADC_At_0;
  if(cal->current_state==cal_save)
    saveSettings(save_Profile, no_mode, no_mode, no_reboot);              // Save now we have all heap free
  else
    restore_tip();
}

static int Cal_Settings_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(getIronError()){
    cal->error=1;
    return last_scr;
  }
  wakeOledDim();
  handleOledDim();
  updatePlot();
  updateScreenTimer(input);

  if(cal->update || update_GUI_Timer()){
    scr->current_widget->refresh=refresh_triggered;
    switch(cal->zero_state){
      case zero_disabled:
        sprintf(cal->zeroStr, "%s%4u", strings[lang].CAL_ZeroSet, cal->backup_calADC_At_0 );
        break;
      case zero_sampling:
        sprintf(cal->zeroStr, "%s%4u", strings[lang].CAL_Sampling, TIP.last_avg );
        break;
      case zero_capture:
        sprintf(cal->zeroStr, "%s%4u", strings[lang].CAL_Captured, cal->calAdjust[cal_0]);
        break;
    }
  }

  if(checkScreenTimer(CAL_TIMEOUT)){
    setUserTemperature(backupTemp);
    setIronCalibrationMode(disable);
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
  newWidget(&w,widget_combo,scr,NULL);

  newComboAction(w, cal->zeroStr, &zero_setAction, &Cal_Combo_Adjust_zero);
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
  scr->processInput = &Cal_ProcessInput;
  scr->draw = &Cal_draw;
  scr->onEnter = &Cal_onEnter;
  scr->onExit = &Cal_onExit;
  scr->create = &Cal_create;

  scr = &Screen_calibration_start;
  oled_addScreen(scr, screen_calibration_start);
  scr->processInput = &Cal_Start_ProcessInput;
  scr->draw = &Cal_Start_draw;
  scr->onExit = &Cal_Start_OnExit;
  scr->init = &Cal_Start_init;
  scr->create = &Cal_Start_create;

  scr = &Screen_calibration_settings;
  oled_addScreen(scr, screen_calibration_settings);
  scr->init = &Cal_Settings_init;
  scr->processInput = &Cal_Settings_ProcessInput;
  scr->onExit = &Cal_Settings_OnExit;
  scr->create = &Cal_Settings_create;

  addSetTemperatureReachedCallback(tempReachedCallback);
}
