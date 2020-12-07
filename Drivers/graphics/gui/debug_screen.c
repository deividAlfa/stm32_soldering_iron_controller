/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "debug_screen.h"
#include "oled.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen variables
//-------------------------------------------------------------------------------------------------------------------------------
int32_t temp;
uint16_t debugTemperature = 0;
static pid_values_t cal_pid;

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
static widget_t Debug_ADC_Val;
static widget_t Debug_ADC_ValRaw;
static widget_t Debug2_ADC_Val;
static widget_t Debug2_ADC_ValRaw;
static widget_t Debug_PID_P;
static widget_t Debug_PID_I;
static widget_t Debug_PID_D;
static widget_t Debug_PID_Err;
static widget_t Debug_PID_Out;
static widget_t Debug_SetPoint_edit;
static widget_t Debug_Cal200_edit;
static widget_t Debug_Cal300_edit;
static widget_t Debug_Cal400_edit;

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------
static void * debug_screen_getADC1() {
	temp = TIP.last_avg;
	return &temp;
}
static void * debug_screen_getADC1_raw() {
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
	temp = getCurrentTip()->calADC_At_200;
	return &temp;
}
static void setCalcAt200(uint16_t *val) {
	getCurrentTip()->calADC_At_200 = *val;
}
static void * getCalcAt300() {
	temp = getCurrentTip()->calADC_At_300;
	return &temp;
}
static void setCalcAt300(uint16_t *val) {
	getCurrentTip()->calADC_At_300 = *val;
}
static void * getCalcAt400() {
	temp = getCurrentTip()->calADC_At_400;
	return &temp;
}
static void setCalcAt400(uint16_t *val) {
	getCurrentTip()->calADC_At_400 = *val;
}

static void * debug_screen_getP() {
	temp = getPID_P()* 1000;
	return &temp;
}
static void * debug_screen_getI() {
	temp = getPID_I()* 1000;
	return &temp;
}
static void * debug_screen_getD() {
	temp = getPID_D()* 1000;
	return &temp;
}
static void * debug_screen_getError() {
	temp = getError();
	return &temp;
}
static void * debug_screen_getOutput() {
	temp = Iron.Pwm_Out;
	return &temp;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen functions
//-------------------------------------------------------------------------------------------------------------------------------
int debug_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	if(input==LongClick){
		return screen_debug2;
	}
	return (default_screenProcessInput(scr, input, state));
}

int debug2_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	if(input==LongClick){
		return screen_main;
	}
	return (default_screenProcessInput(scr, input, state));
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screenDraw(screen_t *scr){
	UG_FontSelect(&FONT_6X8_reduced);
	UG_PutString(0,0,"ADC:       Raw:");//12
	UG_PutString(12,20,"P:      I:      D:");//12
	UG_PutString(0,50,"ERR:       OUT:");//12
	default_screenDraw(scr);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void debug2_screenDraw(screen_t *scr){
	UG_FontSelect(&FONT_6X8_reduced);
	UG_PutString(0,0,"ADC:       Raw:");//12
	UG_PutString(4,20,"SetP C200 C300 C400");//12
	default_screenDraw(scr);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
	scr->draw = &debug_screenDraw;
	scr->processInput = &debug_screenProcessInput;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;
	widget_t *w;

	//ADC1 display, filtered
	w = &Debug_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 24;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	w->reservedChars = 4;

	//ADC1 display, unfiltered
	w = &Debug_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 89;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	w->reservedChars = 4;

	//P TERM
	w = &Debug_PID_P;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 3;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getP;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	w->reservedChars = 6;

	//I TERM
	w = &Debug_PID_I;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 50;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getI;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	//D TERM
	w = &Debug_PID_D;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 91;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getD;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	w->reservedChars = 6;

	//ERROR
	w = &Debug_PID_Err;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 24;
	w->posY = 50;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getError;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	w->reservedChars = 5;

	//OUTPUT
	w = &Debug_PID_Out;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 90;
	w->posY = 50;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getOutput;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	w->reservedChars = 5;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug2_screen_setup(screen_t *scr) {
	scr->draw = &debug2_screenDraw;
	scr->processInput = &debug2_screenProcessInput;
	scr->onEnter = &on_Enter;
	scr->onExit = &on_Exit;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;
	widget_t *w;

	//ADC1 display, filtered
	w = &Debug2_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 24;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	w->reservedChars = 4;

	//ADC1 display, unfiltered
	w = &Debug2_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	w->posX = 89;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	w->reservedChars = 4;

	//Debug setpoint
	w = &Debug_SetPoint_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 4;
	w->posY =33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getDebugTemperature;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 50;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setDebugTemperature;
	w->reservedChars = 4;

	// Cal at 200
	w = &Debug_Cal200_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 34;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt200;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 1;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setCalcAt200;
	w->reservedChars = 4;

	// Cal at 300
	w = &Debug_Cal300_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 64;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt300;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 1;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setCalcAt300;
	w->reservedChars = 4;

	// Cal at 400
	w = &Debug_Cal400_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt400;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 1;
	w->editable.selectable.tab = 3;
	w->editable.setData = (void (*)(void *))&setCalcAt400;
	w->reservedChars = 4;
}
