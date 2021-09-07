/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "screen_common.h"

const  uint16_t state_temps[4] = { 0, 2500, 3500, 4500 };                     // Temp *10 for better accuracy
static uint8_t error;
static uint32_t errorTimer;
static bool backupTempUnit;
static char* state_tempstr[4] = {"Cold ", "250\260C", "350\260C", "450\260C"};
static uint16_t measured_temps[4];
static uint16_t adcAtTemp[4];
static uint16_t adcCal[4];
static uint16_t calAdjust[4];
static bool cal_drawText;
static state_t current_state = cal_cold;
static uint8_t tempReady;
static int32_t measuredTemp;
static uint8_t processCalibration(void);
static void setCalState(state_t s);
static tipData_t *Currtip;
static uint8_t update, update_draw;
screen_t Screen_calibration;
screen_t Screen_calibration_start;
screen_t Screen_calibration_settings;

static widget_t *Widget_Cal_Button;
static widget_t *Widget_Cal_Measured;

static comboBox_item_t *Cal_Combo_Adjust_C250;
static comboBox_item_t *Cal_Combo_Adjust_C350;
static comboBox_item_t *Cal_Combo_Adjust_C450;


//=========================================================
static uint8_t processCalibration(void) {
  uint16_t delta = (state_temps[cal_350] - state_temps[cal_250])/2;

  //  Ensure measured temps and adc measures are valid (cold<250<350<450)
  if (  (measured_temps[cal_350] < measured_temps[cal_250]) ||  (measured_temps[cal_450] < measured_temps[cal_350]) ||
        (adcAtTemp[cal_250]<adcAtTemp[cal_cold]) || (adcAtTemp[cal_350]<adcAtTemp[cal_250]) || (adcAtTemp[cal_450]<adcAtTemp[cal_350]) ){
    return 0;
  }

  if ((measured_temps[cal_250] + delta) < measured_temps[cal_350]){
    adcCal[cal_350] = map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_350], adcAtTemp[cal_250], adcAtTemp[cal_350]);
  }
  else{
    adcCal[cal_350] = map(state_temps[cal_350], last_NTC_C, measured_temps[cal_350], adcAtTemp[cal_cold], adcAtTemp[cal_350]);
  }

  adcCal[cal_350] += map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]) + 1;
  adcCal[cal_350] /= 2;

  if((measured_temps[cal_250] + delta) < measured_temps[cal_350]){
    adcCal[cal_250] = map(state_temps[cal_250], measured_temps[cal_250], state_temps[cal_350], adcAtTemp[cal_250], adcCal[cal_350]);
  }
  else{
    adcCal[cal_250] = map(state_temps[cal_250], last_NTC_C, measured_temps[cal_250], adcAtTemp[cal_cold], adcCal[cal_350]);
  }

  if ((measured_temps[cal_350] + delta) < measured_temps[cal_450]){
    adcCal[cal_450] = map(state_temps[cal_450], measured_temps[cal_350], measured_temps[cal_450], adcCal[cal_350], adcAtTemp[cal_450]);
  }
  else{
    adcCal[cal_450] = map(state_temps[cal_450], measured_temps[cal_250], measured_temps[cal_450], adcCal[cal_350], adcAtTemp[cal_450]);
  }

  if (((adcCal[cal_250] + delta) > adcCal[cal_350]) || ((adcCal[cal_350] + delta) > adcCal[cal_450])) {
    adcCal[cal_250] = map(state_temps[cal_250], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
    adcCal[cal_350] = map(state_temps[cal_350], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
    adcCal[cal_450] = map(state_temps[cal_450], measured_temps[cal_250], measured_temps[cal_450], adcAtTemp[cal_250], adcAtTemp[cal_450]);
  }

  if(adcCal[cal_250]>4090 || adcCal[cal_350]>4090 || adcCal[cal_450]>4090 || adcCal[cal_450]<adcCal[cal_350] || adcCal[cal_350]<adcCal[cal_250]){    // Check that values are valid and don't exceed ADC range
    return 0;
  }
  return 1;


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
  if(temp>=calAdjust[cal_350]){
    temp=calAdjust[cal_350]-1;
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
static void *getCal350() {
  temp = calAdjust[cal_350];
  return &temp;
}
static void setCal350(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=calAdjust[cal_250]){
    temp=calAdjust[cal_250]+1;
  }
  else if(temp>=calAdjust[cal_450]){
    temp=calAdjust[cal_450]-1;
  }
  calAdjust[cal_350] = temp;
  __disable_irq();
  Currtip->calADC_At_350 = calAdjust[cal_350];
  __enable_irq();
}
static int Cal350_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setUserTemperature(state_temps[cal_350]/10);
  }
  else{
    setUserTemperature(0);
  }
  return ret;
}
//=========================================================
static void *getCal450() {
  temp = calAdjust[cal_450];
  return &temp;
}
static void setCal450(int32_t *val) {
  uint16_t temp=*val;
  if(temp<=calAdjust[cal_350]){
    temp=calAdjust[cal_350]+1;
  }
  calAdjust[cal_450] = temp;
  __disable_irq();
  Currtip->calADC_At_450 = calAdjust[cal_450];
  __enable_irq();
}
static int Cal450_processInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  int ret = default_widgetProcessInput(w, input, state);
  selectable_widget_t *sel =extractSelectablePartFromWidget(w);
  if(sel->state==widget_edit){
    setUserTemperature(state_temps[cal_450]/10);
  }
  else{
    setUserTemperature(0);
  }
  return ret;
}
//=========================================================
static int Cal_Settings_SaveAction() {
  if( systemSettings.Profile.Cal250_default != calAdjust[cal_250] ||
      systemSettings.Profile.Cal350_default != calAdjust[cal_350] ||
      systemSettings.Profile.Cal450_default != calAdjust[cal_450] ){

    systemSettings.Profile.Cal250_default = calAdjust[cal_250];
    systemSettings.Profile.Cal350_default = calAdjust[cal_350];
    systemSettings.Profile.Cal450_default = calAdjust[cal_450];

    saveSettingsFromMenu(save_Settings);
  }
  return screen_calibration;
}
//=========================================================
static int startAction(widget_t* w) {
  if(TIP.last_avg<Currtip->calADC_At_250){

    __disable_irq();
    Currtip->calADC_Cold=TIP.last_avg;
    __enable_irq();

    adcCal[cal_cold] = TIP.last_avg;
    setCalState(cal_250);
  }
  return -1;
}
/*
static int okAction(widget_t* w) {
  return screen_main;
}
*/
//=========================================================
static void setCalState(state_t s) {
  current_state = s;
  cal_drawText = 1;
  if(current_state <= cal_450) {
    setUserTemperature(state_temps[s]/10);
  }

  if(current_state == cal_cold) {
    widgetEnable(Widget_Cal_Button);
    widgetDisable(Widget_Cal_Measured);
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.previous_state=widget_selected;
    ((button_widget_t*)Widget_Cal_Button->content)->selectable.state=widget_selected;
    Screen_calibration_start.current_widget=Widget_Cal_Button;
  }
  else if(current_state <= cal_450) {
    widgetDisable(Widget_Cal_Button);
    widgetDisable(Widget_Cal_Measured);
    measuredTemp = state_temps[(int)s]/10;
  }
  else if(current_state <= cal_input_450) {
    widgetDisable(Widget_Cal_Button);
    widgetEnable(Widget_Cal_Measured);
    ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.previous_state=widget_selected;
    ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state=widget_edit;
    Screen_calibration_start.current_widget=Widget_Cal_Measured;
  }
  else if(current_state == cal_finished){
    __disable_irq();
    Currtip->calADC_Cold = backupTip.calADC_Cold;       // Restore backup, we changed this when calibrating the cold tip
    __enable_irq();
    if(processCalibration()){
      backupTip.calADC_Cold   = adcCal[cal_cold];       // If calibration correct, save values to backup tip
      backupTip.calADC_At_250 = adcCal[cal_250];        // Which will be transferred to the curtrent tip on exiting the screen
      backupTip.calADC_At_350 = adcCal[cal_350];
      backupTip.calADC_At_450 = adcCal[cal_450];
    }
    else{
      current_state = cal_failed;
    }
  }

  if(current_state >= cal_finished) {
    setUserTemperature(0);
    widgetDisable(Widget_Cal_Button);
    widgetDisable(Widget_Cal_Measured);
  }

  update_draw = 1;
}
//=========================================================
static void Cal_onEnter(screen_t *scr) {
  if(scr == &Screen_settings) {
    Currtip = getCurrentTip();
    error=0;
    comboResetIndex(Screen_calibration.widgets);
  }
}
static void Cal_onExit(screen_t *scr) {
  if(current_state==cal_finished){
    __disable_irq();
    *Currtip = backupTip;                                 // Apply values to current tip
    __enable_irq();
  }
}

static void Cal_draw(screen_t *scr){
  if(error==1){
    error=2;
    Screen_calibration.widgets->enabled=0;
    errorTimer=current_time;
    FillBuffer(BLACK,fill_dma);
    scr->refresh=screen_Erased;
    putStrAligned(strings[lang].CAL_Error, 10, align_center);
    putStrAligned(strings[lang].CAL_Aborting, 25, align_center);
  }
  default_screenDraw(scr);
}

static int Cal_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  updatePlot();
  refreshOledDim();
  handleOledDim();

  if(error){
    if(error==2 && (current_time-errorTimer)>2000 ){
      error=0;
      widgetEnable(Screen_calibration.widgets);
      scr->refresh=screen_Erase;
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
    return default_screenProcessInput(scr, input, s);
  }
  return -1;
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
  Iron.calibrating=1;
  updateAmbientTemp();
  tempReady = 0;
  backupTemp = getUserTemperature();
  backupMode = getCurrentMode();
  backupTempUnit=systemSettings.settings.tempUnit;
  setSystemTempUnit(mode_Celsius);
  backupTip =  *Currtip;

  __disable_irq();
  Currtip->calADC_At_250 = systemSettings.Profile.Cal250_default;
  Currtip->calADC_At_350 = systemSettings.Profile.Cal350_default;
  Currtip->calADC_At_450 = systemSettings.Profile.Cal450_default;
  __enable_irq();

  setCalState(cal_cold);
  setCurrentMode(mode_run);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  update = update_GUI_Timer();
  update_draw |= update;
  updateAmbientTemp();
  refreshOledDim();
  handleOledDim();

  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
  if (input==LongClick){
    return screen_main;
  }
  if(current_state>=cal_finished){
    if((current_time-screen_timer)>5000 || input==Click){
      return screen_calibration;
    }
  }
  else{
    if((current_time-screen_timer)>180000){   // 3min inactivity
      setCurrentMode(mode_sleep);
      return screen_main;
    }
  }

  if(current_state>cal_cold && GetIronError()){
    error=1;
    return screen_calibration;
  }

  if(tempReady){
    if(current_state>cal_cold && current_state<cal_input_250){
      setCalState(current_state+10);
    }
    else if(current_state<cal_finished){
      if(((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state!=widget_edit){
        measuredTemp*=10;
        if( measuredTemp > (state_temps[current_state-10]+500)){      // Abort if the measured temp is >50ºC than requested
          setCalState(cal_needsAdjust);
        }
        else{
          measured_temps[current_state-10] = measuredTemp - last_NTC_C;
          adcAtTemp[current_state-10] = TIP.last_avg;
          if(current_state<cal_input_450){
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
  setSystemTempUnit(backupTempUnit);
  setUserTemperature(backupTemp);
  setCurrentMode(backupMode);
  Iron.calibrating=0;

  __disable_irq();
  *Currtip = backupTip;
  __enable_irq();
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

    if(current_state<cal_finished){
      u8g2_DrawUTF8(&u8g2, 0, 50, systemSettings.Profile.tip[systemSettings.Profile.currentTip].name);  // Draw current tip name
    }

    if(current_state==cal_cold){
      sprintf(str, "ADC: %u", TIP.last_avg);
      putStrAligned(strings[lang].CAL_InsertColdTip, 0, align_center);
      u8g2_DrawHLine(&u8g2, 0, 13, OledWidth);
      putStrAligned(str, 20, align_center);

    }
    else if(current_state<cal_finished){
      uint8_t s = current_state;
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
      for(uint8_t x=cal_cold;x<(cal_450+1);x++){
        sprintf(str, "%s: %u", state_tempstr[x], adcCal[x]);
        u8g2_DrawUTF8(&u8g2, 20, (x*17), str);
      }
      //u8g2_DrawUTF8(&u8g2, 0, 49, strings[lang].CAL_Success);
    }
    else if(current_state==cal_failed){
      putStrAligned(strings[lang].CAL_Failed, 24, align_center);
    }
    else if(current_state==cal_needsAdjust){
      putStrAligned(strings[lang].CAL_DELTA_HIGH_1, 0, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_2, 17, align_center);
      putStrAligned(strings[lang].CAL_DELTA_HIGH_3, 34, align_center);
    }
  }
  default_screenDraw(scr);
}

static void Cal_Start_create(screen_t *scr) {
  widget_t* w;
  displayOnly_widget_t *dis;
  editable_widget_t* edit;

  newWidget(&w,widget_button,scr);
  Widget_Cal_Button=w;
  w->width = 60;
  w->posX = (OledWidth-1) - w->width;
  w->posY = 48;
  ((button_widget_t*)w->content)->displayString=strings[lang]._START;
  ((button_widget_t*)w->content)->selectable.tab=0;
  ((button_widget_t*)w->content)->action = &startAction;
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
  comboResetIndex(Screen_calibration_settings.widgets);

  updateAmbientTemp();
  setCurrentMode(mode_run);
  backupMode = getCurrentMode();
  backupTemp = getUserTemperature();
  setUserTemperature(0);
  Iron.calibrating=1;

  calAdjust[cal_250] = systemSettings.Profile.Cal250_default;
  calAdjust[cal_350] = systemSettings.Profile.Cal350_default;
  calAdjust[cal_450] = systemSettings.Profile.Cal450_default;

  Currtip = getCurrentTip();
  backupTip= *Currtip;

  __disable_irq();
  Currtip->calADC_At_250 = calAdjust[cal_250];
  Currtip->calADC_At_350 = calAdjust[cal_350];
  Currtip->calADC_At_450 = calAdjust[cal_450];
  __enable_irq();
}

static void Cal_Settings_OnExit(screen_t *scr) {
  Iron.calibrating=0;

  __disable_irq();
  *Currtip = backupTip;
  __enable_irq();

  setUserTemperature(backupTemp);
  setCurrentMode(backupMode);
}

static int Cal_Settings_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  updateAmbientTemp();

  if(GetIronError()){
    error=1;
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
