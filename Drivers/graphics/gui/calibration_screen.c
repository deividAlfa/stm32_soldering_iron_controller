/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include "oled.h"
#include "gui.h"

typedef enum {cal_200, cal_300, cal_400, cal_suceed, cal_failed}state_t;
static uint32_t lastUpdateTick;
static int16_t lastTipTemp;
static uint16_t backupTemp;
static bool backupTempUnit;
static uint16_t backupCal200;
static uint16_t backupCal300;
static uint16_t backupCal400;
const static uint16_t state_temps[3] = {200, 300, 400};
const static char* state_tempstr[3] = {"200C", "300C", "400C"};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static state_t current_state = cal_200;
static uint8_t tempReady;
static int32_t measuredTemp;
static uint16_t adcCal[3];
static uint8_t processCalibration();

static widget_t Widget_CAL_WaitTemp;
static widget_t Widget_CAL_Cancel;
static widget_t Widget_CAL_Input_MeasuredTemp_edit;
static widget_t Widget_CAL_Input_Cancel;
static widget_t Widget_CAL_Input_OK;

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

static void setCalState(state_t s) {
	current_state = s;
	if(current_state < cal_suceed) {
		Widget_CAL_WaitTemp.posX = 2;
		setCurrentMode(mode_run,forceMode);
		setSetTemperature(state_temps[(int)s]);
		measuredTemp = state_temps[(int)s];
	}
	else {
		tipData * Currtip = getCurrentTip();
		uint8_t result = processCalibration();
		if(result) {
			current_state = cal_suceed;
			Currtip->calADC_At_200 = adcCal[cal_200];
			Currtip->calADC_At_300 = adcCal[cal_300];
			Currtip->calADC_At_400 = adcCal[cal_400];
		}
		else {
			current_state = cal_failed;
		}
	}
}

static int cancelAction(widget_t* w) {
	setSystemTempUnit(backupTempUnit);
	tipData * Currtip = getCurrentTip();
	if(current_state != cal_suceed){
		Currtip->calADC_At_200 = backupCal200;
		Currtip->calADC_At_300 = backupCal300;
		Currtip->calADC_At_400 = backupCal400;
	}
	return screen_settingsmenu;
}

static int okAction(widget_t *w) {
	tempReady = 0;
	measured_temps[(int)current_state] = measuredTemp - (readColdJunctionSensorTemp_x10(mode_Celsius) / 10);
	adcAtTemp[(int)current_state] = TIP.last_avg;
	setCalState(++current_state);
	return screen_edit_calibration_wait;
}

static void waitCalibration_screen_onEnter(screen_t *scr) {
	if(scr != &Screen_edit_calibration_input) {
		tipData * Currtip = getCurrentTip();
		Iron.calibrating=1;
		tempReady = 0;
		setCurrentMode(mode_run,forceMode);
		backupTemp = getSetTemperature();
		backupTempUnit=systemSettings.settings.tempUnit;

		backupCal200 = Currtip->calADC_At_200;
		backupCal300 = Currtip->calADC_At_300;
		backupCal400 = Currtip->calADC_At_400;

		switch(systemSettings.settings.currentProfile){
			case profile_T12:
				Currtip->calADC_At_200=T12_Cal200;
				Currtip->calADC_At_300=T12_Cal300;
				Currtip->calADC_At_400=T12_Cal400;
				break;
			case profile_C210:
				Currtip->calADC_At_200=C210_Cal200;
				Currtip->calADC_At_300=C210_Cal300;
				Currtip->calADC_At_400=C210_Cal400;
				break;
			case profile_C245:
				Currtip->calADC_At_200=C245_Cal200;
				Currtip->calADC_At_300=C245_Cal300;
				Currtip->calADC_At_400=C245_Cal400;
				break;
		}
		setSystemTempUnit(mode_Celsius);
		setCalState(cal_200);
	}
}

static void inputCalibration_screen_init(screen_t *scr) {
	default_init(scr);
	u8g2_SetFont(&u8g2,default_font );
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,0,19,"MEASURED:");//12
}

int waitProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
	if(tempReady)
		return screen_edit_calibration_input;
	return default_screenProcessInput(scr, input, s);
}

static void waitOnExit(screen_t *scr) {
	if(scr != &Screen_edit_calibration_wait && scr != &Screen_edit_calibration_input) {
		tempReady = 0;
		current_state = cal_200;
		setSetTemperature(backupTemp);
		setCurrentMode(mode_run,forceMode);
		Iron.calibrating=0;
	}
	Widget_CAL_Input_OK.buttonWidget.selectable.previous_state=widget_idle;
	Widget_CAL_Input_OK.buttonWidget.selectable.state=widget_idle;

	Widget_CAL_Input_Cancel.buttonWidget.selectable.previous_state=widget_idle;
	Widget_CAL_Input_Cancel.buttonWidget.selectable.state=widget_idle;

	Widget_CAL_Input_MeasuredTemp_edit.editableWidget.selectable.previous_state=widget_selected;
	Widget_CAL_Input_MeasuredTemp_edit.editableWidget.selectable.state=widget_edit;

	Screen_edit_calibration_input.current_widget=&Widget_CAL_Input_MeasuredTemp_edit;

}

//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void waitCalibration_screen_draw(screen_t *scr){
	if((HAL_GetTick()-lastUpdateTick)>200){										// Refresh every 200mS
		lastUpdateTick=HAL_GetTick();
		scr->refresh=screen_eraseAndRefresh;
		default_screenDraw(scr);
		u8g2_SetDrawColor(&u8g2, WHITE);
		char waitstr[6];
		lastTipTemp = readTipTemperatureCompensated(stored_reading,read_Avg);
		switch((int)current_state){
			case cal_200:
			case cal_300:
			case cal_400:
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
		}
	}
}
void calibration_screen_setup(screen_t *scr) {

	widget_t* w;
	displayOnly_widget_t *dis;
	screen_t* sc;
	screen_setDefaults(scr);
	scr->processInput = &waitProcessInput;
	scr->draw = &waitCalibration_screen_draw;
	scr->onExit = &waitOnExit;
	scr->onEnter = &waitCalibration_screen_onEnter;

	addSetTemperatureReachedCallback(tempReachedCallback);

	w = &Widget_CAL_Cancel;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->displayString="BACK";
	w->posX = 90;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelAction;

	sc = &Screen_edit_calibration_input;
	oled_addScreen(&Screen_edit_calibration_input, screen_edit_calibration_input);
	screen_setDefaults(sc);
	sc->init = &inputCalibration_screen_init;
	sc->onExit = &waitOnExit;

	w=&Widget_CAL_Input_MeasuredTemp_edit;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	static char measuredTemp[6];
	w->displayString = measuredTemp;
	dis->reservedChars = 5;
	w->endString = "\260C";
	w->posX = 84;
	w->posY = 17;
	dis->getData = &getMeasuredTemp;
	w->editableWidget.setData =  (void (*)(void *)) &setMeasuredTemp;
	w->editableWidget.selectable.tab = 0;

	w=&Widget_CAL_Input_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="SAVE";
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &okAction;
	w->posX = 90;
	w->posY = 48;


	w=&Widget_CAL_Input_Cancel;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="CANCEL";
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelAction;
	w->posX = 0;
	w->posY = 48;


}

static uint8_t processCalibration() {
	  uint16_t delta = state_temps[1] - state_temps[0]; delta >>= 1;
	  uint16_t ambient = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
	  
	  //  Ensure measured temps are valid (200<300<400)
	  if (	(measured_temps[cal_300] <= measured_temps[cal_200]) ||	(measured_temps[cal_400] <= measured_temps[cal_300]) ){
		  return 0;
	  }

	  //  Ensure adc values are valid (200<300<400)
	  if (	(adcAtTemp[cal_300] <=adcAtTemp[cal_200]) ||(adcAtTemp[cal_400] <= adcAtTemp[cal_300]) ){
		  return 0;
	  }

	  if ((measured_temps[cal_300] > measured_temps[cal_200]) && ((measured_temps[cal_200] + delta) < measured_temps[cal_300]))
	    adcCal[cal_300] = map(state_temps[cal_300], measured_temps[cal_200], measured_temps[cal_300], adcAtTemp[cal_200], adcAtTemp[cal_300]);
	  else
	    adcCal[1] = map(state_temps[1], ambient, measured_temps[1], 0, adcAtTemp[1]);
	  adcCal[cal_300] += map(state_temps[cal_300], measured_temps[cal_200], measured_temps[cal_400], adcAtTemp[cal_200], adcAtTemp[cal_400]) + 1;
	  adcCal[1] >>= 1;

	  if ((measured_temps[1] > measured_temps[0]) && ((measured_temps[0] + delta) < measured_temps[1]))
	    adcCal[0] = map(state_temps[0], measured_temps[0], state_temps[1], adcAtTemp[0], adcCal[1]);
	  else
	    adcCal[0] = map(state_temps[0], ambient, state_temps[1], 0, adcCal[1]);

	  if ((measured_temps[2] > measured_temps[1]) && ((measured_temps[1] + delta) < measured_temps[2]))
	    adcCal[2] = map(state_temps[2], state_temps[1], measured_temps[2], adcCal[1], adcAtTemp[2]);
	  else
	    adcCal[2] = map(state_temps[2], state_temps[0], measured_temps[2], adcCal[1], adcAtTemp[2]);

	  if (((adcCal[0] + delta) > adcCal[1]) || ((adcCal[1] + delta) > adcCal[2])) {
	    adcCal[0] = map(state_temps[0], measured_temps[0], measured_temps[2], adcAtTemp[0], adcAtTemp[2]);
	    adcCal[1] = map(state_temps[1], measured_temps[0], measured_temps[2], adcAtTemp[0], adcAtTemp[2]);
	    adcCal[2] = map(state_temps[2], measured_temps[0], measured_temps[2], adcAtTemp[0], adcAtTemp[2]);
	  }
	return 1;

}

