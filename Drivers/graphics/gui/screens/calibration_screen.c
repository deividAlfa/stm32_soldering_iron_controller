/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "screen_common.h"

typedef enum { cal_250=0, cal_350=1, cal_450=2, cal_input_250=10, cal_input_350=11, cal_input_450=12, cal_suceed=20, cal_failed=21, cal_needsAdjust=22 }state_t;
static bool error;
static uint32_t errorTimer;
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
static tipData_t *Currtip;
static uint8_t update, update_draw;
screen_t Screen_calibration;
screen_t Screen_calibration_start;
screen_t Screen_calibration_settings;

static widget_t *Widget_Cal_Back;
static widget_t *Widget_Cal_Measured;

static comboBox_item_t *Cal_Combo_Adjust_C250;
static comboBox_item_t *Cal_Combo_Adjust_C350;
static comboBox_item_t *Cal_Combo_Adjust_C450;


//=========================================================
static uint8_t processCalibration() {
  uint16_t delta = (state_temps[cal_350] - state_temps[cal_250])/2;
  uint16_t ambient = readColdJunctionSensorTemp_x10(old_reading, mode_Celsius) / 10;

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
//=========================================================
static void tempReached(uint16_t temp) {
  if(temp == state_temps[(int)current_state])
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
  temp = adcAtTemp[cal_250];
  return &temp;
}
static void setCal250(int32_t *val) {
  uint16_t temp=*val;
  if(temp>=adcAtTemp[cal_350]){
    temp=adcAtTemp[cal_350]-1;
  }
  adcAtTemp[cal_250] = temp;
}
static int Cal250_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setDebugTemp(adcAtTemp[cal_250]);
  }
  else{
    setDebugTemp(0);
  }
  return ret;
}
//=========================================================
static void *getCal350() {
  temp = adcAtTemp[cal_350];
  return &temp;
}
static void setCal350(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=adcAtTemp[cal_250]){
    temp=adcAtTemp[cal_250]+1;
  }
  else if(temp>=adcAtTemp[cal_450]){
    temp=adcAtTemp[cal_450]-1;
  }
  adcAtTemp[cal_350] = temp;
}
static int Cal350_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setDebugTemp(adcAtTemp[cal_350]);
  }
  else{
    setDebugTemp(0);
  }
  return ret;
}
//=========================================================
static void *getCal450() {
  temp = adcAtTemp[cal_450];
  return &temp;
}
static void setCal450(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=adcAtTemp[cal_350]){
    temp=adcAtTemp[cal_350]+1;
  }
  adcAtTemp[cal_450] = temp;
}
static int Cal450_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setDebugTemp(adcAtTemp[cal_450]);
  }
  else{
    setDebugTemp(0);
  }
  return ret;
}
//=========================================================
static int Cal_Settings_SaveAction() {
  if( systemSettings.Profile.Cal250_default != adcAtTemp[cal_250] ||
      systemSettings.Profile.Cal350_default != adcAtTemp[cal_350] ||
      systemSettings.Profile.Cal450_default != adcAtTemp[cal_450] ){

    systemSettings.Profile.Cal250_default = adcAtTemp[cal_250];
    systemSettings.Profile.Cal350_default = adcAtTemp[cal_350];
    systemSettings.Profile.Cal450_default = adcAtTemp[cal_450];
    systemSettings.Profile.CalNTC = readColdJunctionSensorTemp_x10(old_reading, mode_Celsius) / 10;
    saveSettingsFromMenu(save_Settings);
  }
  return screen_calibration;
}
//=========================================================
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
//=========================================================
static int cancelAction(widget_t* w) {
  return screen_calibration;
}
//=========================================================


static void Cal_init(screen_t *scr) {
  default_init(scr);
  errorTimer=0;
  error=0;
}

static void Cal_onEnter(screen_t *scr) {
  if(scr == &Screen_settings) {
    comboResetIndex(Screen_calibration.widgets);
  }
}

static void Cal_draw(screen_t *scr){
  if(error){
    if(errorTimer==0){
      errorTimer=current_time;
      FillBuffer(BLACK,fill_dma);
      scr->refresh=screen_Erased;
      putStrAligned(strings[lang].CAL_Error, 10, align_center);
      putStrAligned(strings[lang].CAL_Aborting, 25, align_center);
    }
  }
  else{
    default_screenDraw(scr);
  }
}

static int Cal_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  updatePlot();
  refreshOledDim();
  handleOledDim();

  if(GetIronError()){
    error=1;
  }
  if(error){
    if((errorTimer==0) || ((current_time-errorTimer)<2000) ){
      return -1;
    }
    else{
      return screen_settings;
    }
  }
  else{
    if(input!=Rotate_Nothing){
      screen_timer=current_time;
    }
    if(input==LongClick || ((current_time-screen_timer)>15000)){
      return screen_main;
    }
    else if(input==Rotate_Decrement_while_click){
      return screen_settings;
    }
  }
  return default_screenProcessInput(scr, input, s);
}
static void Cal_create(screen_t *scr) {
  widget_t* w;

  newWidget(&w,widget_combo,scr);

  newComboScreen(w, strings[lang]._START, screen_calibration_start, NULL);
  newComboScreen(w, strings[lang]._SETTINGS, screen_calibration_settings, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);
  w->posY=10;
}


static void Cal_Start_init(screen_t *scr) {
  default_init(scr);
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

  temp = systemSettings.Profile.Cal250_default;
  temp = (temp*250)/(250-systemSettings.Profile.CalNTC);
  Currtip->calADC_At_250 = temp;

  temp = systemSettings.Profile.Cal350_default;
  temp = (temp*350)/(350-systemSettings.Profile.CalNTC);
  Currtip->calADC_At_350 = temp;

  temp = systemSettings.Profile.Cal450_default;
  temp = (temp*450)/(450-systemSettings.Profile.CalNTC);
  Currtip->calADC_At_450 = temp;

  setCalState(cal_250);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  update = update_GUI_Timer();
  update_draw |= update;

  refreshOledDim();
  handleOledDim();

  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
  if((current_time-screen_timer)>180000){   // 3min inactivity
    setCurrentMode(mode_sleep);
    return screen_main;
  }
  if(current_state<cal_input_250){
    if(input==Rotate_Decrement_while_click){
      return screen_calibration;
    }
  }

  if(tempReady){
    if(current_state<cal_input_250){
      setCalState(current_state+10);
      widgetDisable(Widget_Cal_Back);
      widgetEnable(Widget_Cal_Measured);
      ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.previous_state=widget_selected;
      ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state=widget_edit;
      Screen_calibration_start.current_widget=Widget_Cal_Measured;
    }
    else if(current_state<cal_suceed){
      if(((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state!=widget_edit){
        if( measuredTemp > (state_temps[current_state-10]+50)){      // Abort if the measured temp is >50ºC than requested
          setCalState(cal_needsAdjust);
        }
        else{
          measured_temps[current_state-10] = measuredTemp - (readColdJunctionSensorTemp_x10(old_reading, mode_Celsius) / 10);
          adcAtTemp[(int)current_state-10] = TIP.last_avg;
          if(current_state<cal_input_450){
            tempReady = 0;
            setCalState(current_state-9);
          }
          else{
            setCalState(cal_suceed);
          }
        }
        widgetEnable(Widget_Cal_Back);
        ((button_widget_t*)Widget_Cal_Back->content)->selectable.previous_state=widget_selected;
        ((button_widget_t*)Widget_Cal_Back->content)->selectable.state=widget_selected;
        Screen_calibration_start.current_widget=Widget_Cal_Back;

        widgetDisable(Widget_Cal_Measured);

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
  char str[20];

  if(cal_drawText || update_draw){
    cal_drawText=0;
    update_draw=0;

    FillBuffer(BLACK, fill_dma);
    scr->refresh=screen_Erased;
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_SetFont(&u8g2, u8g2_font_menu);
    lastTipTemp = readTipTemperatureCompensated(old_reading, read_average);

    if(current_state<cal_suceed){
      uint8_t s = current_state;
      u8g2_DrawUTF8(&u8g2, 8, 6, strings[lang].CAL_Step);            // Draw current cal state

      if(current_state<cal_input_250){
        u8g2_DrawUTF8(&u8g2, 8, 24, strings[lang].CAL_Wait);               // Draw current temp
        sprintf(str, "%3u\260C",lastTipTemp);
        u8g2_DrawUTF8(&u8g2, 85, 24, str);
      }
      else{
        u8g2_DrawUTF8(&u8g2, 8, 24, strings[lang].CAL_Measured);
        s-=10;
      }
      u8g2_DrawUTF8(&u8g2, 85, 6, state_tempstr[s]);
      u8g2_DrawUTF8(&u8g2, 8, 49, systemSettings.Profile.tip[systemSettings.Profile.currentTip].name);//12
    }
    else if(current_state==cal_suceed){
      for(uint8_t x=0;x<3;x++){
        sprintf(str, "Cal %s: %u", state_tempstr[x], adcCal[x]);
        u8g2_DrawUTF8(&u8g2, 6, (x*14), str);
      }
      u8g2_DrawUTF8(&u8g2, 0, 49, strings[lang].CAL_Succeed);
      setUserTemperature(0);
    }
    else if(current_state==cal_failed){
      putStrAligned(strings[lang].CAL_Failed, 15, align_center);
      setUserTemperature(0);
    }
    else if(current_state==cal_needsAdjust){
      putStrAligned(strings[lang].CAL_DELTA_HIGH_1, 0, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_2, 15, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_3, 30, align_center);
      setUserTemperature(0);
    }
  }
  default_screenDraw(scr);
}

static void Cal_Start_create(screen_t *scr) {
  widget_t* w;
  displayOnly_widget_t *dis;
  editable_widget_t* edit;

  newWidget(&w,widget_button,scr);
  Widget_Cal_Back=w;
  w->posX = 78;
  w->posY = 48;
  w->width = 42;
  ((button_widget_t*)w->content)->displayString=strings[lang]._STOP;
  ((button_widget_t*)w->content)->selectable.tab=0;
  ((button_widget_t*)w->content)->action = &cancelAction;
  ((button_widget_t*)w->content)->font=u8g2_font_menu;

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
  adcAtTemp[cal_250] = systemSettings.Profile.Cal250_default;
  adcAtTemp[cal_350] = systemSettings.Profile.Cal350_default;
  adcAtTemp[cal_450] = systemSettings.Profile.Cal450_default;
  setDebugTemp(0);
  setDebugMode(enable);
  setCurrentMode(mode_run);
  comboResetIndex(Screen_calibration_settings.widgets);
}

static void Cal_Settings_OnExit(screen_t *scr) {
  if(getCurrentMode()!=mode_sleep){
    setCurrentMode(mode_run);
  }
  setDebugMode(disable);
}

static int Cal_Settings_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    return screen_calibration;
  }
  refreshOledDim();
  handleOledDim();

  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
  if((current_time-screen_timer)>180000){     // 3 min inactivity
    setCurrentMode(mode_sleep);
    return screen_main;
  }

  if(input==Rotate_Decrement_while_click){
   comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
    if(item->type==combo_Editable || item->type==combo_MultiOption){
      if(item->widget->selectable.state!=widget_edit){
        return screen_calibration;
      }
    }
    else{
      return screen_calibration;
    }
  }
  return default_screenProcessInput(scr, input, s);
}

static void Cal_Settings_create(screen_t *scr){
  widget_t* w;
  editable_widget_t* edit;


  // Combo Start
  newWidget(&w,widget_combo,scr);

  newComboEditable(w, strings[lang]._Cal_250, &edit, &Cal_Combo_Adjust_C250);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal250;
  edit->big_step = 100;
  edit->step = 20;
  edit->setData = (void (*)(void *))&setCal250;
  edit->max_value = 4000;
  edit->min_value = 0;
  edit->selectable.processInput=&Cal250_processInput;

  newComboEditable(w, strings[lang]._Cal_350, &edit, &Cal_Combo_Adjust_C350);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal350;
  edit->big_step = 100;
  edit->step = 20;
  edit->setData = (void (*)(void *))&setCal350;
  edit->max_value = 4000;
  edit->min_value = 0;
  edit->selectable.processInput=&Cal350_processInput;

  newComboEditable(w, strings[lang]._Cal_450, &edit, &Cal_Combo_Adjust_C450);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal450;
  edit->big_step = 100;
  edit->step = 20;
  edit->setData = (void (*)(void *))&setCal450;
  edit->max_value = 4000;
  edit->min_value = 0;
  edit->selectable.processInput=&Cal450_processInput;

  newComboAction(w, strings[lang]._SAVE, &Cal_Settings_SaveAction, NULL);
  newComboScreen(w, strings[lang]._CANCEL, screen_calibration, NULL);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screens setup
//-------------------------------------------------------------------------------------------------------------------------------
void calibration_screen_setup(screen_t *scr) {
  screen_t *sc;
  screen_setDefaults(scr);
  scr->init = &Cal_init;
  scr->processInput = &Cal_ProcessInput;
  scr->draw = &Cal_draw;
  scr->onEnter = &Cal_onEnter;
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
