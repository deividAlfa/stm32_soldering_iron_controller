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
static uint32_t screenTimer;
static uint32_t lastUpdateTick;
static int16_t lastTipTemp;
static uint16_t backupTemp;
static uint16_t current_debug_temp;
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

screen_t Screen_calibration;
screen_t Screen_calibration_start;
screen_t Screen_calibration_adjust;

static widget_t *Widget_Cal_Back;
static widget_t *Widget_Cal_Measured;

static comboBox_item_t *Cal_Combo_Adjust_C250;
static comboBox_item_t *Cal_Combo_Adjust_C350;
static comboBox_item_t *Cal_Combo_Adjust_C450;


//=========================================================
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
  current_debug_temp = temp;
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
  adcAtTemp[cal_350] = *val;
  current_debug_temp = temp;
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
  current_debug_temp = temp;
}
//=========================================================
static int cal_adjust_SaveAction(widget_t* w) {
  if( systemSettings.Profile.Cal250_default != adcAtTemp[cal_250] ||
      systemSettings.Profile.Cal350_default != adcAtTemp[cal_350] ||
      systemSettings.Profile.Cal450_default != adcAtTemp[cal_450] ){

    systemSettings.Profile.Cal250_default = adcAtTemp[cal_250];
    systemSettings.Profile.Cal350_default = adcAtTemp[cal_350];
    systemSettings.Profile.Cal450_default = adcAtTemp[cal_450];
    systemSettings.Profile.CalNTC = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
    saveSettingsFromMenu(save_Settings);
  }
  return screen_calibration;
}
//=========================================================
static int cal_adjust_CancelAction(widget_t* w) {
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
  screenTimer = HAL_GetTick();
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
      return screen_settings;
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
static void Cal_create(screen_t *scr) {
  widget_t* w;
  newWidget(&w,widget_combo,scr);
  newComboScreen(w, "START", screen_calibration_start, NULL);
  newComboScreen(w, "ADJUST", screen_calibration_adjust, NULL);
  newComboScreen(w, "BACK", screen_settings, NULL);
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

  Currtip->calADC_At_250 = systemSettings.Profile.Cal250_default;
  Currtip->calADC_At_350 =  systemSettings.Profile.Cal350_default;
  Currtip->calADC_At_450 =  systemSettings.Profile.Cal450_default;

  setCalState(cal_250);
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    return screen_calibration;
  }

  if(tempReady){
    if(current_state<=cal_450){
      setCalState(current_state+10);
      widgetDisable(Widget_Cal_Back);
      ((button_widget_t*)Widget_Cal_Back->content)->selectable.previous_state=widget_idle;
      ((button_widget_t*)Widget_Cal_Back->content)->selectable.state=widget_selected;

      widgetEnable(Widget_Cal_Measured);
      ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.previous_state=widget_selected;
      ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state=widget_edit;
      Screen_calibration_start.current_widget=Widget_Cal_Measured;
    }
    else if(current_state<=cal_input_450){
      if(((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state!=widget_edit){
        if( measuredTemp > (state_temps[current_state-10]+50)){      // Abort if the measured temp is >50ºC than requested
          setCalState(cal_needsAdjust);
        }
        else{
          measured_temps[current_state-10] = measuredTemp - (readColdJunctionSensorTemp_x10(mode_Celsius) / 10);
          adcAtTemp[(int)current_state-10] = TIP.last_avg;
          if(current_state<cal_input_450){
            tempReady = 0;
            setCalState(current_state-9);
          }
          else{
            setCalState(cal_suceed);
          }
          widgetEnable(Widget_Cal_Back);
          ((button_widget_t*)Widget_Cal_Back->content)->selectable.previous_state=widget_selected;
          ((button_widget_t*)Widget_Cal_Back->content)->selectable.state=widget_selected;

          widgetDisable(Widget_Cal_Measured);
          ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.previous_state=widget_idle;
          ((editable_widget_t*)Widget_Cal_Measured->content)->selectable.state=widget_idle;

          Screen_calibration_start.current_widget=Widget_Cal_Back;
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
        setUserTemperature(0);
        break;
      }
      case cal_failed:
          putStrAligned("FAILED!", 15, align_center);
          setUserTemperature(0);
        break;
      case cal_needsAdjust:
          putStrAligned("DELTA TOO HIGH!", 0, align_center);
          putStrAligned("Adjust manually", 15, align_center);
          putStrAligned("and try again", 30, align_center);
          setUserTemperature(0);
        break;

      case cal_input_250:
      case cal_input_350:
      case cal_input_450:
          u8g2_DrawStr(&u8g2, 6, 12, "CAL STEP:");                                 // Draw current cal state
          u8g2_DrawStr(&u8g2, 83, 12, state_tempstr[(int)current_state-10]);
          u8g2_DrawStr(&u8g2, 6, 30, "MEASURED:");//12
      default:
        break;
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
  w->posX = 86;
  w->posY = 48;
  w->width = 42;
  ((button_widget_t*)w->content)->displayString="BACK";
  ((button_widget_t*)w->content)->selectable.tab=0;
  ((button_widget_t*)w->content)->action = &cancelAction;

  newWidget(&w,widget_editable,scr);
  Widget_Cal_Measured=w;
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
}


static void Cal_Adjust_init(screen_t *scr) {
  default_init(scr);
  adcAtTemp[cal_250] = systemSettings.Profile.Cal250_default;
  adcAtTemp[cal_350] = systemSettings.Profile.Cal350_default;
  adcAtTemp[cal_450] = systemSettings.Profile.Cal450_default;
  current_debug_temp = 0;
  setDebugTemp(0);
  setDebugMode(debug_On);
  setCurrentMode(mode_run);
  comboResetIndex(Screen_calibration_adjust.widgets);
}

static void Cal_Adjust_OnExit(screen_t *scr) {
  setDebugMode(debug_Off);
  setCurrentMode(mode_run);
}

static int Cal_Adjust_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
  if(GetIronError()){
    return screen_calibration;
  }
  comboBox_item_t *item=((comboBox_widget_t*)Screen_calibration_adjust.widgets->content)->currentItem;
  if(item->type==combo_Editable && item->widget->selectable.state==widget_edit){
    setDebugTemp(current_debug_temp);
  }
  else{
    setDebugTemp(0);
  }
  return default_screenProcessInput(scr, input, s);
}
static void Cal_Adjust_create(screen_t *scr){
  widget_t* w;
  editable_widget_t* edit;

  // Combo Start
  newWidget(&w,widget_combo,scr);

  newComboEditable(w, "Cal 250", &edit, &Cal_Combo_Adjust_C250);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal250;
  edit->big_step = 100;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setCal250;
  edit->max_value = 4000;
  edit->min_value = 0;

  newComboEditable(w, "Cal 350", &edit, &Cal_Combo_Adjust_C350);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal350;
  edit->big_step = 100;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setCal350;
  edit->max_value = 4000;
  edit->min_value = 0;

  newComboEditable(w, "Cal 450", &edit, &Cal_Combo_Adjust_C450);
  edit->inputData.reservedChars=4;
  edit->inputData.getData = &getCal450;
  edit->big_step = 100;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setCal450;
  edit->max_value = 4000;
  edit->min_value = 0;

  newComboAction(w, "SAVE", &cal_adjust_SaveAction, NULL);
  newComboAction(w, "CANCEL", &cal_adjust_CancelAction, NULL);
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

  sc = &Screen_calibration_adjust;
  oled_addScreen(sc, screen_calibration_adjust);
  sc->init = &Cal_Adjust_init;
  sc->processInput = &Cal_Adjust_ProcessInput;
  sc->onExit = &Cal_Adjust_OnExit;
  sc->create = &Cal_Adjust_create;

  addSetTemperatureReachedCallback(tempReachedCallback);
}
