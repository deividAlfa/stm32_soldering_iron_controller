/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "debug_screen.h"
#include "adc_global.h"
#include "tempsensors.h"

uint16_t temp;
static TIM_HandleTypeDef *htim4;
uint16_t debugSetPoint = 0;
static pid_values_t cal_pid;

void setPWM_tim(TIM_HandleTypeDef *tim){
	htim4 = tim;
}

static void * debug_screen_getADC1_1() {
	temp = iron_temp_adc_avg;
	return &temp;
}
static void * debug_screen_getP() {
	temp = getPID_P();
	return &temp;
}
static void * debug_screen_getI() {
	temp = getPID_I();
	return &temp;
}
static void * debug_screen_getD() {
	temp = getPID_D();
	return &temp;
}
static void * debug_screen_getError() {
	temp = getError();
	return &temp;
}
static void * debug_screen_getOutput() {
	temp = getOutput();
	return &temp;
}
static void debug_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}
static void debug_screen2_on_exit(screen_t *scr) {
	setDebugMode(0);
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
}
static void debug_screen2_on_enter(screen_t *scr) {
	setDebugMode(1);
	cal_pid.Kp = 0.002;
	cal_pid.Ki = 0.00025;
	cal_pid.Kd = 0;
	currentPID = cal_pid;
}
static void * debug_screen2_getDebugSetpoint() {
	return &debugSetPoint;
}
static void debug_screen2_setDebugSetpoint(uint16_t *val) {
	debugSetPoint = *val;
	setDebugSetPoint(debugSetPoint);
}
static void * debug_screen2_getCalcAt200() {
	return &getCurrentTip()->calADC_At_200;
}
static void debug_screen2_setCalcAt200(uint16_t *val) {
	getCurrentTip()->calADC_At_200 = *val;
}
static void * debug_screen2_getCalcAt300() {
	return &getCurrentTip()->calADC_At_300;
}
static void debug_screen2_setCalcAt300(uint16_t *val) {
	getCurrentTip()->calADC_At_300 = *val;
}
static void * debug_screen2_getCalcAt400() {
	return &getCurrentTip()->calADC_At_400;
}
static void debug_screen2_setCalcAt400(uint16_t *val) {
	getCurrentTip()->calADC_At_400 = *val;
}
static int saveDebug2(widget_t *w) {
	saveSettings();
	return screen_main;
}
static int cancelDebug2(widget_t *w) {
	return screen_main;
}

//temp_adc
//kp ki kd
//error
//present output
void debug_screen_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;

	widget_t *widget = screen_addWidget(scr);
	//ADC_1_1 display
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 1;
	widget->posY = 0;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getADC1_1;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	widget = screen_addWidget(scr);
	//P TERM
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 1;
	widget->posY = 20;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getP;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	//I TERM
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 30;
	widget->posY = 20;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getI;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	//D TERM
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 60;
	widget->posY = 20;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getD;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 1;
	widget->posY = 30;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getError;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 1;
	widget->posY = 40;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getOutput;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
}

void debug_screen2_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->onEnter = &debug_screen2_on_enter;
	scr->onExit = &debug_screen2_on_exit;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;

	widget_t *widget = screen_addWidget(scr);
	//ADC_1_1 display
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 1;
	widget->posY = 0;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getADC1_1;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	widget = screen_addWidget(scr);

	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 1;
	widget->posY = 10;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &debug_screen2_getDebugSetpoint;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&debug_screen2_setDebugSetpoint;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 1;
	widget->posY = 20;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &debug_screen2_getCalcAt200;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 1;
	widget->editable.setData = (void (*)(void *))&debug_screen2_setCalcAt200;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 1;
	widget->posY = 30;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &debug_screen2_getCalcAt300;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 2;
	widget->editable.setData = (void (*)(void *))&debug_screen2_setCalcAt300;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 1;
	widget->posY = 40;
	widget->font_size = &FONT_6X8;
	widget->editable.inputData.getData = &debug_screen2_getCalcAt400;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 3;
	widget->editable.setData = (void (*)(void *))&debug_screen2_setCalcAt400;
	widget->reservedChars = 5;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_6X8;
	widget->posX = 2;
	widget->posY = 56;
	char *s = "SAVE";
	strcpy(widget->displayString, s);
	widget->reservedChars = 4;
	widget->buttonWidget.selectable.tab = 4;
	widget->buttonWidget.action = &saveDebug2;
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_6X8;
	widget->posX = 90;
	widget->posY = 56;
	s = "CANCEL";
	strcpy(widget->displayString, s);
	widget->reservedChars = 6;
	widget->buttonWidget.selectable.tab = 5;
	widget->buttonWidget.action = &cancelDebug2;

}
