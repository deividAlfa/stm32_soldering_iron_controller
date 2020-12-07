/*
 * calibration_screen.c
 *
 *  Created on: Sep 21, 2017
 *      Author: jose
 */

#include "calibration_screen.h"
#include <stdio.h>
#include "oled.h"

typedef enum {cal_200, cal_300, cal_400, cal_end}state_t;
static uint16_t backupTemp;
static uint16_t state_temps[3] = {200, 300, 400};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static state_t current_state = cal_200;
static uint8_t tempReady = 0;
static uint16_t measuredTemp = 0;
static uint16_t adcCal[3];
static uint8_t processCalibration();
const pid_values_t cal_pid = {
		max:	1,
		min:	0,
		Kp:		0.0003,
		Ki:		0.0025,
		Kd:		0,
		maxI:	200,
		minI:	-50,
};


static widget_t Widget_CAL_Wait;
static widget_t Widget_CAL_WaitTemp;
static widget_t Widget_CAL_Cancel;
//static widget_t Widget_CAL_Input_MeasuredTemp_label;
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
		setCurrentMode(mode_normal);
		setSetTemperature(state_temps[(int)s]);
		sprintf(Widget_CAL_WaitTemp.displayString, "HEATING: %d*C", state_temps[(int)s]);
		measuredTemp = state_temps[(int)s];
	}
	else {
		Widget_CAL_WaitTemp.enabled = 0;
		uint8_t result = processCalibration();
		if(result) {
			strcpy(Widget_CAL_Wait.displayString, "SUCEED!");
			tipData * t = getCurrentTip();
			t->calADC_At_200 = adcCal[cal_200];
			t->calADC_At_300 = adcCal[cal_300];
			t->calADC_At_400 = adcCal[cal_400];
		}
		else {
			strcpy(Widget_CAL_Wait.displayString, "FAILED!");
		}
	}
}

static int cancelAction(widget_t* w) {
	return screen_settings;
}

static int okAction(widget_t *w) {
	tempReady = 0;
	if(current_state == cal_end) {
		current_state = cal_200;
		return screen_main;
	}
	measured_temps[(int)current_state] = measuredTemp - (readColdJunctionSensorTemp_x10(Unit_Celsius) / 10);
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
}

int waitProcessInput(struct screen_t *scr, RE_Rotation_t input, RE_State_t *s) {
	if(tempReady)
		return screen_edit_calibration_input;
	return default_screenProcessInput(scr, input, s);
}
static void waitOnEnter(screen_t *scr) {
	if(scr != &Screen_edit_calibration_input) {
		Widget_CAL_WaitTemp.enabled = 1;
		strcpy(Widget_CAL_Wait.displayString, "WAIT...");
		UG_FontSetHSpace(0);
		UG_FontSetVSpace(0);
		Iron.isCalibrating=1;
		tempReady = 0;
		setCurrentMode(mode_normal);
		backupTemp = getSetTemperature();
		currentPID = cal_pid;
		setupPIDFromStruct();
		setCalState(cal_200);
	}
}
static void waitOnExit(screen_t *scr) {
	if(scr != &Screen_edit_calibration_wait && scr != &Screen_edit_calibration_input) {
		tempReady = 0;
		current_state = cal_200;
		setSetTemperature(backupTemp);
		setCurrentMode(mode_normal);
		currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
		setupPIDFromStruct();
		Iron.isCalibrating=0;
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
// Edit Tip screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void editcalibration_screenDraw(screen_t *scr){
	UG_FontSelect(&FONT_8X14_reduced);
	UG_PutString(0,17,"MEASURED:");//12
	default_screenDraw(scr);
}
void calibration_screen_setup(screen_t *scr) {

	screen_t* sc;
	widget_t* w;
	scr->draw = &default_screenDraw;
	scr->processInput = &waitProcessInput;
	scr->init = &waitCalibration_screen_init;
	scr->update = &default_screenUpdate;
	scr->onEnter = &waitOnEnter;
	scr->onExit = &waitOnExit;

	addSetTemperatureReachedCallback(tempReachedCallback);

	w = &Widget_CAL_Wait;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_label);
	w->posX = 0;
	w->posY = 50;
	w->font_size = &FONT_8X14_reduced;

	w = &Widget_CAL_WaitTemp;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_label);
	w->posX = 6;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;

	w = &Widget_CAL_Cancel;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "BACK");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelAction;

	oled_addScreen(&Screen_edit_calibration_input, screen_edit_calibration_input);
	sc = &Screen_edit_calibration_input;
	sc->draw = &editcalibration_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &inputCalibration_screen_init;
	sc->update = &default_screenUpdate;
	sc->onExit = &waitOnExit;
/*
	w=&Widget_CAL_Input_MeasuredTemp_label;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "MEASURED:");
	w->posX = 0;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
*/
	w=&Widget_CAL_Input_MeasuredTemp_edit;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getMeasuredTemp;
	w->editable.setData = &setMeasuredTemp;
	w->reservedChars = 5;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "*C");
	w->editable.selectable.tab = 0;

	w=&Widget_CAL_Input_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "SAVE");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &okAction;


	w=&Widget_CAL_Input_Cancel;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	strcpy(w->displayString, "CANCEL");
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &cancelAction;


}

static uint8_t processCalibration() {
	  uint16_t delta = state_temps[1] - state_temps[0]; delta >>= 1;
	  uint16_t ambient = readColdJunctionSensorTemp_x10(Unit_Celsius) / 10;

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

