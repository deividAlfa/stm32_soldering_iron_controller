/*
 * calibration_screen.c
 *
 *  Created on: Sep 21, 2017
 *      Author: jose
 */

#include "calibration_screen.h"
#include "../../../Src/iron.h"
#include "../../../Src/pid.h"
#include "../../generalIO/adc_global.h"
#include "../../generalIO/tempsensors.h"
#include "../../graphics/gui/oled.h"

typedef enum {cal_200, cal_300, cal_400, cal_end}state_t;
static uint16_t state_temps[3] = {200, 300, 400};
static uint16_t measured_temps[3];
static uint16_t adcAtTemp[3];
static state_t current_state = cal_200;
static uint8_t tempReady = 0;
static char *waitStr;
static uint16_t measuredTemp = 0;
static widget_t *waitWidget = NULL;
static widget_t *cancelButton = NULL;
static widget_t *waitTemp = NULL;
static iron_mode_t ironModeBackup;
static uint16_t tempSetBackup;
static uint16_t adcCal[3];
static uint8_t processCalibration();
static pid_values_t cal_pid;
static screen_t *calInputScreen;
static screen_t *calWaitScreen;

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
		setSetTemperature(state_temps[(int)s]);
		setCurrentMode(mode_set);
		sprintf(waitStr, "Setting temp.to %dC", state_temps[(int)s]);
		measuredTemp = state_temps[(int)s];
	}
	else {
		waitTemp->enabled = 0;
		strcpy(cancelButton->displayString, "Finish");
		uint8_t result = processCalibration();
		waitWidget->posX = 20;
		if(result) {
			strcpy(waitWidget->displayString, "Cal succeed");
			tipData * t = getCurrentTip();
			t->calADC_At_200 = adcCal[cal_200];
			t->calADC_At_300 = adcCal[cal_300];
			t->calADC_At_400 = adcCal[cal_400];
			saveSettings();
		}
		else {
			strcpy(waitWidget->displayString, "Cal failed");
		}
	}
}

static int cancelAction(widget_t* w) {
	return screen_main;
}

static int okAction(widget_t *w) {
	tempReady = 0;
	if(current_state == cal_end) {
		current_state = cal_200;
		return screen_main;
	}
	measured_temps[(int)current_state] = measuredTemp - (readColdJunctionSensorTemp_mC() / 1000);
	adcAtTemp[(int)current_state] = iron_temp_adc_avg;
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
	if(scr != calInputScreen) {
		UG_FontSetHSpace(0);
		UG_FontSetVSpace(0);
		waitTemp->enabled = 1;
		tempReady = 0;
		strcpy(waitWidget->displayString, "Please wait!");
		waitWidget->posX = 10;
		ironModeBackup = getCurrentMode();
		tempSetBackup = getSetTemperature();
		setCurrentMode(mode_set);
		currentPID = cal_pid;
		setupPIDFromStruct();
		setCalState(cal_200);
	}
}
static void waitOnExit(screen_t *scr) {
	if(scr != calWaitScreen && scr != calInputScreen) {
		tempReady = 0;
		current_state = cal_200;
		setSetTemperature(tempSetBackup);
		setCurrentMode(ironModeBackup);
		currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
		setupPIDFromStruct();
	}
}
void calibration_screen_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &waitProcessInput;
	scr->init = &waitCalibration_screen_init;
	scr->update = &default_screenUpdate;
	scr->onEnter = &waitOnEnter;
	scr->onExit = &waitOnExit;
	calWaitScreen = scr;
	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "Please wait!";
	strcpy(widget->displayString, s);
	widget->posX = 10;
	widget->posY = 16;
	widget->font_size = &FONT_8X14;
	waitWidget = widget;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	waitStr = widget->displayString;
	widget->posX = 0;
	widget->posY = 30;
	widget->font_size = &FONT_6X8;
	waitTemp = widget;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_6X8;
	widget->posX = 90;
	widget->posY = 56;
	s = "CANCEL";
	strcpy(widget->displayString, s);
	widget->reservedChars = 6;
	widget->buttonWidget.selectable.tab = 0;
	widget->buttonWidget.action = &cancelAction;
	cancelButton = widget;

	screen_t *sc = oled_addScreen(screen_edit_calibration_input);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &inputCalibration_screen_init;
	sc->update = &default_screenUpdate;
	sc->onExit = &waitOnExit;
	calInputScreen = sc;
	widget = screen_addWidget(sc);
	widgetDefaultsInit(widget, widget_label);
	s = "Set measured temp.";
	strcpy(widget->displayString, s);
	widget->posX = 10;
	widget->posY = 16;
	widget->font_size = &FONT_6X8;

	widget = screen_addWidget(sc);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 55;
	widget->posY = 30;
	widget->font_size = &FONT_8X14;
	widget->editable.inputData.getData = &getMeasuredTemp;
	widget->editable.setData = &setMeasuredTemp;
	widget->reservedChars = 3;
	widget->editable.selectable.tab = 0;
	widget = screen_addWidget(sc);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_6X8;
	widget->posX = 90;
	widget->posY = 56;
	s = "CANCEL";
	strcpy(widget->displayString, s);
	widget->reservedChars = 6;
	widget->buttonWidget.selectable.tab = 2;
	widget->buttonWidget.action = &cancelAction;

	widget = screen_addWidget(sc);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_6X8;
	widget->posX = 20;
	widget->posY = 56;
	s = "OK";
	strcpy(widget->displayString, s);
	widget->reservedChars = 6;
	widget->buttonWidget.selectable.tab = 1;
	widget->buttonWidget.action = &okAction;
	addSetTemperatureReachedCallback(tempReachedCallback);
	cal_pid.Kp = 0.01;
	cal_pid.Ki = 0.0025;
	cal_pid.Kd = 0;
	cal_pid.min = 0;
	cal_pid.max = 1;
	cal_pid.maxI = 200;
	cal_pid.minI = -50;
}

static uint8_t processCalibration() {
	  uint16_t delta = state_temps[1] - state_temps[0]; delta >>= 1;
	  uint16_t ambient = readColdJunctionSensorTemp_mC() / 1000;

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
