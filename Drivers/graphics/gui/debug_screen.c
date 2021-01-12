/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "debug_screen.h"
#include "oled.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen variables
//-------------------------------------------------------------------------------------------------------------------------------
int32_t temp;
uint16_t debugTemperature = 0;
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
static widget_t Debug_ADC_Val;
static widget_t Debug_ADC_ValRaw;
static widget_t Debug2_ADC_Val;
static widget_t Debug2_ADC_ValRaw;
static char adcVal[5];
static char adcRaw[5];
static widget_t Debug_PID_P;
static widget_t Debug_PID_I;
static widget_t Debug_PID_D;
static widget_t Debug_PID_Err;
static widget_t Debug_PID_Out;
static widget_t Debug_SetPoint_edit;
static widget_t Debug_Cal200_edit;
static widget_t Debug_Cal300_edit;
static widget_t Debug_Cal400_edit;
static widget_t Widget_Power;
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

static void * debug_screen_getIronPower() {
	//if(UpdateReadings){
		temp = getCurrentPower();
	//}
	return &temp;
}

static void debug_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}
static void on_Exit(screen_t *scr) {
	DebugMode(RESET);
}
static void on_Enter(screen_t *scr) {
	DebugMode(SET);
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
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
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
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
	UG_FontSelect(&FONT_6X8_reduced);
	UG_PutString(0,0,"ADC:       Raw:");//12
	UG_PutString(4,20,"SetP C200 C300 C400");//12
	default_screenDraw(scr);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
	screen_setDefaults(scr);
	scr->draw = &debug_screenDraw;
	scr->processInput = &debug_screenProcessInput;
	scr->init = &debug_screen_init;
	widget_t *w;

	//ADC1 display, filtered
	w = &Debug_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display, adcVal, 4);
	w->posX = 24;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	

	//ADC1 display, unfiltered
	w = &Debug_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display,adcRaw,4);
	w->posX = 89;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	

	//P TERM
	w = &Debug_PID_P;
	screen_addWidget(w, scr);
	static char p[7];
	widgetDefaultsInit(w, widget_display,p,6);
	w->posX = 3;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getP;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	

	//I TERM
	w = &Debug_PID_I;
	screen_addWidget(w, scr);
	static char i[7];
	widgetDefaultsInit(w, widget_display,i,6);
	w->posX = 50;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getI;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	
	w->displayWidget.justify = justify_right;

	//D TERM
	w = &Debug_PID_D;
	screen_addWidget(w, scr);
	static char d[7];
	widgetDefaultsInit(w, widget_display,d,6);
	w->posX = 91;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getD;
	w->displayWidget.number_of_dec = 2;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	

	//ERROR
	w = &Debug_PID_Err;
	screen_addWidget(w, scr);
	static char e[6];
	widgetDefaultsInit(w, widget_display,e,5);
	w->posX = 24;
	w->posY = 50;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getError;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	

	//OUTPUT
	w = &Debug_PID_Out;
	screen_addWidget(w, scr);
	static char o[6];
	widgetDefaultsInit(w, widget_display,o,5);
	w->posX = 90;
	w->posY = 50;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getOutput;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_int32;
	w->displayWidget.justify = justify_right;
	
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug2_screen_setup(screen_t *scr) {
	screen_setDefaults(scr);
	scr->draw = &debug2_screenDraw;
	scr->processInput = &debug2_screenProcessInput;
	scr->onEnter = &on_Enter;
	scr->onExit = &on_Exit;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;
	widget_t *w;

	//power display
	w=&Widget_Power;
	screen_addWidget(w,scr);
	static char o[5];
	widgetDefaultsInit(w, widget_display,o,4);
	w->posX = 93;
	w->posY = 50;
	w->font_size = &FONT_8X14_reduced;
	w->displayWidget.getData = &debug_screen_getIronPower;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	
	w->displayWidget.justify = justify_right;
	w->displayWidget.hasEndStr = 1;
	w->EndStr = "%";

	//ADC1 display, filtered
	w = &Debug2_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display,adcVal,4);
	w->posX = 24;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	

	//ADC1 display, unfiltered
	w = &Debug2_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display,adcRaw,4);
	w->posX = 89;
	w->posY = 0;
	w->font_size = &FONT_6X8_reduced;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uinteger16;
	

	//Debug setpoint
	w = &Debug_SetPoint_edit;
	screen_addWidget(w, scr);
	static char setP[5];
	widgetDefaultsInit(w, widget_editable,setP,4);
	w->posX = 4;
	w->posY =33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getDebugTemperature;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 50;
	w->editable.max_value = 4095;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setDebugTemperature;
	

	// Cal at 200
	w = &Debug_Cal200_edit;
	screen_addWidget(w, scr);
	static char c200[5];
	widgetDefaultsInit(w, widget_editable,c200,4);
	w->posX = 34;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt200;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 20;
	w->editable.max_value = 4095;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setCalcAt200;
	

	// Cal at 300
	w = &Debug_Cal300_edit;
	screen_addWidget(w, scr);
	static char c300[5];
	widgetDefaultsInit(w, widget_editable,c300,4);
	w->posX = 64;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt300;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 20;
	w->editable.max_value = 4095;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setCalcAt300;
	

	// Cal at 400
	w = &Debug_Cal400_edit;
	screen_addWidget(w, scr);
	static char c400[5];
	widgetDefaultsInit(w, widget_editable,c400,4);
	w->posX = 94;
	w->posY = 33;
	w->font_size = &FONT_6X8_reduced;
	w->editable.inputData.getData = &getCalcAt400;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 20;
	w->editable.max_value = 4095;
	w->editable.selectable.tab = 3;
	w->editable.setData = (void (*)(void *))&setCalcAt400;
	
}
