/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "settings_screen.h"
#include "oled.h"
#include "gui.h"

typedef enum {cal_250, cal_350, cal_450, cal_suceed, cal_failed, cal_needsAdjust}state_t;
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
static char* state_tempstr[3] = {"250C", "350C", "450C"};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static state_t current_state = cal_250;
static uint8_t tempReady;
static int32_t measuredTemp;
static uint16_t adcCal[3];
static uint8_t processCalibration();
static tipData * Currtip;

screen_t Screen_edit_calibration;
screen_t Screen_edit_calibration_start;
screen_t Screen_edit_calibration_adjust;
screen_t Screen_edit_calibration_input;

static widget_t comboWidget_Cal;
static comboBox_widget_t comboBox_Cal;
static comboBox_item_t Cal_Combo_Start;
static comboBox_item_t Cal_Combo_Adjust;
static comboBox_item_t Cal_Combo_Exit;

static widget_t Widget_Cal_Start_Cancel;
static button_widget_t button_Cal_Start_Cancel;

static widget_t comboWidget_Cal_Adjust;
static comboBox_widget_t comboBox_Cal_Adjust;
static comboBox_item_t Cal_Combo_Adjust_Target;
static comboBox_item_t Cal_Combo_Adjust_Setpoint;
static comboBox_item_t Cal_Combo_Adjust_Save;
static comboBox_item_t Cal_Combo_Adjust_Cancel;

static widget_t Widget_Cal_Adjust_Select;
static editable_widget_t editable_Cal_Adjust_Select;

static widget_t Widget_Cal_Adjust_Setpoint;
static editable_widget_t editable_Cal_Adjust_Setpoint;

static widget_t Widget_Cal_Input_Measured;
static editable_widget_t editable_Cal_Input_Measured;

static widget_t Widget_Cal_Input_Cancel;
static button_widget_t button_Cal_Input_Cancel;

static widget_t Widget_Cal_Input_OK;
static button_widget_t button_Cal_Input_OK;

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
	return screen_edit_calibration;
}
static int cal_adjust_CancelAction(widget_t* w) {
	return screen_edit_calibration;
}
//***************************************************
static void setCalState(state_t s) {
	current_state = s;
	if(current_state < cal_suceed) {
		setCurrentMode(mode_run);
		setSetTemperature(state_temps[(int)s]);
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

static int okAction(widget_t *w) {
	tempReady = 0;

	// Abort if the measured temp is >50ºC than requested
	if( measuredTemp > (state_temps[current_state]+50)){
		setCalState(cal_needsAdjust);
	}
	else{
		measured_temps[current_state] = measuredTemp - (readColdJunctionSensorTemp_x10(mode_Celsius) / 10);
		adcAtTemp[(int)current_state] = TIP.last_avg;
		setCalState(++current_state);
	}
	return screen_edit_calibration_start;
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
			scr->refresh=screenRefresh_alreadyErased;
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
	if(scr != &Screen_edit_calibration_input) {
		Currtip = getCurrentTip();
		Iron.calibrating=1;
		tempReady = 0;
		setCurrentMode(mode_run);
		backupTemp = getSetTemperature();
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
}

static int Cal_Start_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
	if(GetIronError()){
		return screen_edit_calibration;
	}
	if(tempReady){
		return screen_edit_calibration_input;
	}
	return default_screenProcessInput(scr, input, s);
}

static void Cal_Start_OnExit(screen_t *scr) {
	if((scr != &Screen_edit_calibration_start) && (scr != &Screen_edit_calibration_input)) {
		setSystemTempUnit(backupTempUnit);
		tempReady = 0;
		current_state = cal_250;
		setSetTemperature(backupTemp);
		setCurrentMode(mode_run);
		Iron.calibrating=0;
		Currtip->calADC_At_250 = backupCal250;
		Currtip->calADC_At_350 = backupCal350;
		Currtip->calADC_At_450 = backupCal450;
	}

}
static void Cal_Start_draw(screen_t *scr){
	if((HAL_GetTick()-lastUpdateTick)>200){										// Refresh every 200mS
		lastUpdateTick=HAL_GetTick();
		scr->refresh=screenRefresh_eraseNow;
		default_screenDraw(scr);
		u8g2_SetDrawColor(&u8g2, WHITE);
		char waitstr[6];
		lastTipTemp = readTipTemperatureCompensated(stored_reading,read_Avg);
		switch((int)current_state){
			case cal_250:
			case cal_350:
			case cal_450:
				u8g2_DrawStr(&u8g2, 10, 15, "CAL STEP:");						// Draw current cal state
				u8g2_DrawStr(&u8g2, 90, 15, state_tempstr[(int)current_state]);
				u8g2_DrawStr(&u8g2, 10, 30, "WAIT...");							 // Draw current temp
				sprintf(waitstr, "%3u\260C",lastTipTemp);
				u8g2_DrawStr(&u8g2, 90, 30, waitstr);
				break;
			case cal_suceed:
			{
				char str[20];
				for(uint8_t x=0;x<3;x++){
					sprintf(str, "ADC %s: %u", state_tempstr[x], adcCal[x]);
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
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------
// Calibration input screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void Cal_Input_init(screen_t *scr) {
	default_init(scr);
	u8g2_SetFont(&u8g2,default_font );
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,0,19,"MEASURED:");//12
}

static void Cal_Input_OnExit(screen_t *scr) {
  button_Cal_Input_OK.selectable.previous_state=widget_idle;;
  button_Cal_Input_OK.selectable.state=widget_idle;

  button_Cal_Input_Cancel.selectable.previous_state=widget_idle;
  button_Cal_Input_Cancel.selectable.state=widget_idle;

  editable_Cal_Input_Measured.selectable.previous_state=widget_selected;
  editable_Cal_Input_Measured.selectable.state=widget_edit;

	Screen_edit_calibration_input.current_widget=&Widget_Cal_Input_Measured;
}

static int Cal_Input_ProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
	if(GetIronError()){
		return screen_edit_calibration;
	}
	return default_screenProcessInput(scr, input, s);
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
	comboAddScreen(&Cal_Combo_Start, w, 	"START",		screen_edit_calibration_start);
	comboAddScreen(&Cal_Combo_Adjust, w, 	"ADJUST", 		screen_edit_calibration_adjust);
	comboAddScreen(&Cal_Combo_Exit, w, 		"BACK", 		screen_settingsmenu);
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

	w = &Widget_Cal_Start_Cancel;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_Cal_Start_Cancel);
	button_Cal_Start_Cancel.displayString="BACK";
	w->posX = 86;
	w->posY = 48;
	w->width = 42;
	((button_widget_t*)w->content)->selectable.tab = 0;
	((button_widget_t*)w->content)->action = &cancelAction;

	//########################################## CALIBRATION ADJUST SCREEN ##########################################
	//
	sc = &Screen_edit_calibration_adjust;
	oled_addScreen(sc, screen_edit_calibration_adjust);
	screen_setDefaults(sc);
	sc->init = &Cal_Adjust_init;
	sc->processInput = &Cal_Adjust_ProcessInput;
	sc->onExit = &Cal_Adjust_OnExit;

	w = &Widget_Cal_Adjust_Select;
	widgetDefaultsInit(w, widget_multi_option, &editable_Cal_Adjust_Select);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->getData = &getCalStep;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setCalStep;
	edit->max_value = cal_450;
	edit->min_value = cal_250 ;
	edit->options = state_tempstr;
	edit->numberOfOptions = 3;
	w->posX = 20;
	w->posY = 20;
	w->width = 40;

	w = &Widget_Cal_Adjust_Setpoint;
	widgetDefaultsInit(w, widget_editable, &editable_Cal_Adjust_Setpoint);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=4;
	dis->getData = &getAdjustSetpoint;
	edit->big_step = 100;
	edit->step = 50;
	edit->setData = (void (*)(void *))&setAdjustSetpoint;
	edit->max_value = 4000;
	edit->min_value = 0;
	w->posX = 80;
	w->posY = 20;
	w->width = 40;

	w = &comboWidget_Cal_Adjust;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo, &comboBox_Cal_Adjust);
	comboAddMultiOption(&Cal_Combo_Adjust_Target, w, 	"Cal. Step",		&editable_Cal_Adjust_Select);
	comboAddEditable(&Cal_Combo_Adjust_Setpoint, w, 	"ADC Value", 		&editable_Cal_Adjust_Setpoint);
	comboAddAction(&Cal_Combo_Adjust_Save, w, 		"SAVE", 			&cal_adjust_SaveAction);
	comboAddAction(&Cal_Combo_Adjust_Cancel, w, 	"CANCEL", 			&cal_adjust_CancelAction);


	//########################################## CALIBRATION INPUT SCREEN ##########################################
	//
	sc = &Screen_edit_calibration_input;
	oled_addScreen(sc, screen_edit_calibration_input);
	screen_setDefaults(sc);
	sc->init = &Cal_Input_init;
	sc->onExit = &Cal_Input_OnExit;;
	sc->processInput = &Cal_Input_ProcessInput;

	w=&Widget_Cal_Input_Measured;
	widgetDefaultsInit(w, widget_editable, &editable_Cal_Input_Measured);
	screen_addWidget(w,sc);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars = 5;
	dis->endString = "\260C";
	w->posX = 86;
	w->posY = 17;
	w->width = 42;
	dis->getData = &getMeasuredTemp;
	edit->setData =  (void (*)(void *)) &setMeasuredTemp;
	edit->selectable.tab = 0;

	w=&Widget_Cal_Input_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_Cal_Input_OK);
	button_Cal_Input_OK.displayString="SAVE";
	((button_widget_t*)w->content)->selectable.tab = 1;
	((button_widget_t*)w->content)->action = &okAction;
	w->posX = 86;
	w->posY = 48;
	w->width = 42;

	w=&Widget_Cal_Input_Cancel;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_Cal_Input_Cancel);
	button_Cal_Input_Cancel.displayString="CANCEL";
	((button_widget_t*)w->content)->selectable.tab = 2;
	((button_widget_t*)w->content)->action = &cancelAction;
	w->posX = 0;
	w->posY = 48;
	w->width = 56;
}

static uint8_t processCalibration() {
	  uint16_t delta = (state_temps[cal_350] - state_temps[cal_250])/2;
	  uint16_t ambient = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
	  
	  //  Ensure measured temps are valid (250<350<450)
	  if (	(measured_temps[cal_250] >= measured_temps[cal_350]) ||	(measured_temps[cal_350] >= measured_temps[cal_450]) ){
		  return 0;
	  }

	  //  Ensure adc values are valid (250<350<450)
	  if (	(adcAtTemp[cal_250] >=adcAtTemp[cal_350]) || (adcAtTemp[cal_350] >= adcAtTemp[cal_450]) ){
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
	if(adcCal[cal_250]>4090 || adcCal[cal_350]>4090 || adcCal[cal_450]>4090){		// Check that values don't exceed ADC range
		return 0;
	}
	return 1;

}

