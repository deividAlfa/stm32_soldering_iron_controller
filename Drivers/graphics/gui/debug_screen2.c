/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "debug_screen.h"
#include "oled.h"

uint16_t temp;
uint16_t debugTemperature = 0;
static pid_values_t cal_pid;


static void * debug_screen_getADC1_1() {
	temp = TIP.last_avg;
	return &temp;
}
static void * debug_screen_getADC1_1_raw() {
	temp = TIP.last_RawAvg;
	return &temp;
}

static void debug_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}
static void on_Exit(screen_t *scr) {
	DebugMode(RESET);
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
}
static void on_Enter(screen_t *scr) {
	DebugMode(1);
	cal_pid.Kp = 0.002;
	cal_pid.Ki = 0.00025;
	cal_pid.Kd = 0;
	currentPID = cal_pid;
}
static void * getDebugTemperature() {
	return &debugTemperature;
}
static void setDebugTemperature(uint16_t *val) {
	debugTemperature = *val;
	DebugSetTemp(debugTemperature);
}
static void * getCalcAt200() {
	return &getCurrentTip()->calADC_At_200;
}
static void setCalcAt200(uint16_t *val) {
	getCurrentTip()->calADC_At_200 = *val;
}
static void * getCalcAt300() {
	return &getCurrentTip()->calADC_At_300;
}
static void setCalcAt300(uint16_t *val) {
	getCurrentTip()->calADC_At_300 = *val;
}
static void * getCalcAt400() {
	return &getCurrentTip()->calADC_At_400;
}
static void setCalcAt400(uint16_t *val) {
	getCurrentTip()->calADC_At_400 = *val;
}
static int saveDebug(widget_t *w) {
	return screen_main;
}
static int cancelDebug(widget_t *w) {
	return screen_main;
}

//temp_adc
//kp ki kd
//error
//present output

void debug_screen2_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->onEnter = &on_Enter;
	scr->onExit = &on_Exit;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;
	widget_t *widget;

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//ADC iron display

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "ADC:       Raw:");
	widget->posX =0;
	widget->posY = 0;
	widget->font_size = &FONT_6X8_reduced;
	widget->reservedChars = 6;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 24;
	widget->posY = 0;
	widget->font_size = &FONT_6X8_reduced;
	widget->displayWidget.getData = &debug_screen_getADC1_1;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 89;
	widget->posY = 0;
	widget->font_size = &FONT_6X8_reduced;
	widget->displayWidget.getData = &debug_screen_getADC1_1_raw;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;


	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPoint display

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "SetP C200 C300 C400");
	widget->posX =4;
	widget->posY = 20;
	widget->font_size = &FONT_6X8_reduced;
	widget->reservedChars = 19;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 4;
	widget->posY =33;
	widget->font_size = &FONT_6X8_reduced;
	widget->editable.inputData.getData = &getDebugTemperature;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 50;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setDebugTemperature;
	widget->reservedChars = 4;

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Cal at 200

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 34;
	widget->posY = 33;
	widget->font_size = &FONT_6X8_reduced;
	widget->editable.inputData.getData = &getCalcAt200;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 1;
	widget->editable.setData = (void (*)(void *))&setCalcAt200;
	widget->reservedChars = 4;

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Cal at 300

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 64;
	widget->posY = 33;
	widget->font_size = &FONT_6X8_reduced;
	widget->editable.inputData.getData = &getCalcAt300;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 2;
	widget->editable.setData = (void (*)(void *))&setCalcAt300;
	widget->reservedChars = 4;

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Cal at 400


	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->posX = 94;
	widget->posY = 33;
	widget->font_size = &FONT_6X8_reduced;
	widget->editable.inputData.getData = &getCalcAt400;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 100;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 3;
	widget->editable.setData = (void (*)(void *))&setCalcAt400;
	widget->reservedChars = 4;


	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_8X14_reduced;
	widget->posX = 1;
	widget->posY = 50;
	strcpy(widget->displayString, "SAVE");
	widget->reservedChars = 4;
	widget->buttonWidget.selectable.tab = 5;
	widget->buttonWidget.action = &saveDebug;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_button);
	widget->font_size = &FONT_8X14_reduced;
	widget->posX = 94;
	widget->posY = 50;
	strcpy(widget->displayString, "BACK");
	widget->reservedChars = 4;
	widget->buttonWidget.selectable.tab = 4;
	widget->buttonWidget.action = &cancelDebug;

}
