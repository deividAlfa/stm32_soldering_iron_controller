/*
 * calibration_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "calibration_screen.h"
#include <stdio.h>
#include "oled.h"

typedef enum {cal_200, cal_300, cal_400, cal_end}state_t;
static uint32_t lastUpdateTick;
static int16_t lastTipTemp;
static uint16_t backupTemp;
static uint16_t state_temps[3] = {200, 300, 400};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static state_t current_state = cal_200;
static uint8_t tempReady;
static uint16_t measuredTemp;
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
static void setMeasuredTemp(void *temp) {
	measuredTemp = *(uint16_t*)temp;
}

static void setCalState(state_t s) {
	current_state = s;
	if(current_state != cal_end) {
		Widget_CAL_WaitTemp.posX = 2;
		setCurrentMode(mode_normal);
		setSetTemperature(state_temps[(int)s]);
		sprintf(Widget_CAL_WaitTemp.displayString, "CAL STEP: %3u*C", state_temps[(int)s]);
		measuredTemp = state_temps[(int)s];
	}
	else {
		uint8_t result = processCalibration();
		if(result) {
			Widget_CAL_WaitTemp.posX = 31;
			strcpy(Widget_CAL_WaitTemp.displayString, "SUCCEED!");//8 *8=64
			tipData * Currtip = getCurrentTip();
			Currtip->calADC_At_200 = adcCal[cal_200];
			Currtip->calADC_At_300 = adcCal[cal_300];
			Currtip->calADC_At_400 = adcCal[cal_400];
		}
		else {
			Widget_CAL_WaitTemp.posX = 35;
			strcpy(Widget_CAL_WaitTemp.displayString, "FAILED!");//7 *8=56
		}
	}
}

static int cancelAction(widget_t* w) {
	return screen_settingsmenu;
}

static int okAction(widget_t *w) {
	tempReady = 0;
	if(current_state == cal_end) {
		current_state = cal_200;
		return screen_main;
	}
	measured_temps[(int)current_state] = measuredTemp - (readColdJunctionSensorTemp_x10(mode_Celsius) / 10);
	adcAtTemp[(int)current_state] = TIP.last_avg;
	setCalState(++current_state);
	return screen_edit_calibration_wait;
}

static void waitCalibration_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}

static void inputCalibration_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
	UG_FontSelect(&FONT_8X14_reduced);
	UG_PutString(0,17,"MEASURED:");//12
}

int waitProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
	if(tempReady)
		return screen_edit_calibration_input;
	return default_screenProcessInput(scr, input, s);
}
static void waitOnEnter(screen_t *scr) {
	if(scr != &Screen_edit_calibration_input) {
		UG_FontSetHSpace(0);
		UG_FontSetVSpace(0);
		Iron.calibrating=1;
		tempReady = 0;
		setCurrentMode(mode_normal);
		backupTemp = getSetTemperature();
		setCalState(cal_200);
	}
}
static void waitOnExit(screen_t *scr) {
	if(scr != &Screen_edit_calibration_wait && scr != &Screen_edit_calibration_input) {
		tempReady = 0;
		current_state = cal_200;
		setSetTemperature(backupTemp);
		setCurrentMode(mode_normal);
		Iron.calibrating=0;
	}
	Widget_CAL_Input_OK.buttonWidget.selectable.previous_state=widget_idle;
	Widget_CAL_Input_OK.buttonWidget.selectable.state=widget_idle;

	Widget_CAL_Input_Cancel.buttonWidget.selectable.previous_state=widget_idle;
	Widget_CAL_Input_Cancel.buttonWidget.selectable.state=widget_idle;

	Widget_CAL_Input_MeasuredTemp_edit.editable.selectable.previous_state=widget_selected;
	Widget_CAL_Input_MeasuredTemp_edit.editable.selectable.state=widget_edit;

	Screen_edit_calibration_input.current_widget=&Widget_CAL_Input_MeasuredTemp_edit;

}

void cal_screenUpdate(screen_t *scr){
	if(current_state != cal_end) {
		if((HAL_GetTick()-lastUpdateTick)>systemSettings.settings.guiUpdateDelay){
			lastUpdateTick=HAL_GetTick();
			lastTipTemp = readTipTemperatureCompensated(stored_reading,read_Avg);
			char waitstr[20];
			sprintf(waitstr, "WAIT...   %3u*C",lastTipTemp);
			UG_PutString(2,30,waitstr);
		}
	}
	default_screenUpdate(scr);
}
//-------------------------------------------------------------------------------------------------------------------------------
// Calibration screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void editcalibration_screenDraw(screen_t *scr){
	default_screenDraw(scr);
}
void calibration_screen_setup(screen_t *scr) {

	widget_t* w;
	screen_t* sc;
	screen_setDefaults(scr);
	scr->processInput = &waitProcessInput;
	scr->init = &waitCalibration_screen_init;
	scr->update = &cal_screenUpdate;
	scr->onEnter = &waitOnEnter;
	scr->onExit = &waitOnExit;

	addSetTemperatureReachedCallback(tempReachedCallback);

	w = &Widget_CAL_WaitTemp;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_label);
	static char waitStr[16];
	w->displayString=waitStr;
	w->posX = 2;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;

	w = &Widget_CAL_Cancel;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	w->displayString="BACK";
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelAction;

	sc = &Screen_edit_calibration_input;
	oled_addScreen(&Screen_edit_calibration_input, screen_edit_calibration_input);
	screen_setDefaults(sc);
	sc->draw = &editcalibration_screenDraw;
	sc->init = &inputCalibration_screen_init;
	sc->onExit = &waitOnExit;

	w=&Widget_CAL_Input_MeasuredTemp_edit;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_editable);
	static char measuredTemp[6];
	w->displayString = measuredTemp;
	w->reservedChars = 5;
	w->EndStr = "*C";
	w->posX = 84;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getMeasuredTemp;
	w->editable.setData = &setMeasuredTemp;
	w->editable.selectable.tab = 0;

	w=&Widget_CAL_Input_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->displayString="SAVE";
	w->reservedChars = 4;
	w->posX = 94;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &okAction;


	w=&Widget_CAL_Input_Cancel;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="CANCEL";
	w->reservedChars = 6;
	w->font_size = &FONT_8X14_reduced;
	w->posX = 2;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelAction;


}

static uint8_t processCalibration() {
	  uint16_t delta = state_temps[1] - state_temps[0]; delta >>= 1;
	  uint16_t ambient = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
	  
	  //  Ensure measured temps are valid (200<300<400)
	  if (	(measured_temps[cal_300] <= measured_temps[cal_200]) ||	(measured_temps[cal_400] <= measured_temps[cal_300]) ){
		  return 0;
	  }

	  //  Ensure adc values are valid (200<300<400)
	  if (	(adcAtTemp[cal_300] <=adcAtTemp[cal_200]) ||	(adcAtTemp[cal_400] <= adcAtTemp[cal_300]) ){
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

