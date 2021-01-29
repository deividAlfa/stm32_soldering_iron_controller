/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */


#include "debug_screen.h"
#include "oled.h"
#include "gui.h"

#ifdef ENABLE_DEBUG_SCREEN
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen variables
//-------------------------------------------------------------------------------------------------------------------------------
int16_t temp;
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
static void * getDebugTemperature() {
	return &debugTemperature;
}
static void setDebugTemperature(uint16_t *val) {
	debugTemperature = *val;
	resetPID();
	setDebugTemp(debugTemperature);
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
	default_screenDraw(scr);
}
void debug_onEnter(screen_t *scr){
	u8g2_SetFont(&u8g2,default_font );
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,0,0,"ADC:       Raw:");//12
	u8g2_DrawStr(&u8g2,12,20,"P:      I:      D:");//12
	u8g2_DrawStr(&u8g2,0,50,"ERR:       OUT:");//12
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void debug2_onExit(screen_t *scr) {
	setDebugMode(RESET);
}
static void debug2_onEnter(screen_t *scr) {
	setDebugMode(SET);
	u8g2_SetFont(&u8g2,default_font );
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,0,0,"ADC:       Raw:");//12
	u8g2_DrawStr(&u8g2,4,20,"SetP C200 C300 C400");//12
	u8g2_DrawStr(&u8g2,0,55,SYSTEM_VERSION);//12
}
//-------------------------------------------------------------------------------------------------------------------------------
// Debug screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug_screen_setup(screen_t *scr) {
	screen_setDefaults(scr);
	scr->processInput = &debug_screenProcessInput;
	scr->onEnter= &debug_onEnter;
	widget_t *w;
	displayOnly_widget_t* dis;

	//ADC1 display, filtered
	w = &Debug_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=adcVal;
	dis->reservedChars=4;
	w->posX = 04;
	w->posY = 0;
	w->displayWidget.getData = &debug_screen_getADC1;
	

	//ADC1 display, unfiltered
	w = &Debug_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=adcRaw;
	dis->reservedChars=4;
	w->posX = 89;
	w->posY = 0;
	w->displayWidget.getData = &debug_screen_getADC1_raw;

	//P TERM
	w = &Debug_PID_P;
	screen_addWidget(w, scr);
	static char p[7];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=p;
	dis->reservedChars=6;
	w->posX = 3;
	w->posY = 33;
	w->displayWidget.getData = &debug_screen_getP;
	w->displayWidget.number_of_dec = 2;
	

	//I TERM
	w = &Debug_PID_I;
	screen_addWidget(w, scr);
	static char i[7];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=i;
	dis->reservedChars=6;
	w->posX = 50;
	w->posY = 33;
	w->displayWidget.getData = &debug_screen_getI;
	w->displayWidget.number_of_dec = 2;

	//D TERM
	w = &Debug_PID_D;
	screen_addWidget(w, scr);
	static char d[7];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=d;
	dis->reservedChars=6;
	w->posX = 91;
	w->posY = 33;
	w->displayWidget.getData = &debug_screen_getD;
	w->displayWidget.number_of_dec = 2;
	

	//ERROR
	w = &Debug_PID_Err;
	screen_addWidget(w, scr);
	static char e[6];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=e;
	dis->reservedChars=5;
	w->posX = 04;
	w->posY = 50;
	w->displayWidget.getData = &debug_screen_getError;
	

	//OUTPUT
	w = &Debug_PID_Out;
	screen_addWidget(w, scr);
	static char out1[6];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=out1;
	dis->reservedChars=5;
	w->posX = 90;
	w->posY = 50;
	w->displayWidget.getData = &debug_screen_getOutput;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Debug2 screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void debug2_screen_setup(screen_t *scr) {
	screen_setDefaults(scr);
	scr->processInput = &debug2_screenProcessInput;
	scr->onEnter = &debug2_onEnter;
	scr->onExit = &debug2_onExit;
	widget_t *w;
	displayOnly_widget_t* dis;

	//power display
	w=&Widget_Power;
	screen_addWidget(w,scr);
	static char out2[5];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=out2;
	w->endString="%";
	dis->reservedChars=4;
	w->posX = 103;
	w->posY = 55;
	w->displayWidget.getData = &debug_screen_getIronPower;
	//ADC1 display, filtered
	w = &Debug2_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=adcVal;
	dis->reservedChars=4;
	w->posX = 04;
	w->posY = 0;
	w->displayWidget.getData = &debug_screen_getADC1;

	//ADC1 display, unfiltered
	w = &Debug2_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=adcRaw;
	dis->reservedChars=4;
	w->posX = 89;
	w->posY = 0;
	w->displayWidget.getData = &debug_screen_getADC1_raw;

	//Debug setpoint
	w = &Debug_SetPoint_edit;
	screen_addWidget(w, scr);
	static char setP[5];
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=setP;
	dis->reservedChars=4;
	w->posX = 4;
	w->posY = 33;
	dis->getData = &getDebugTemperature;
	w->editableWidget.big_step = 200;
	w->editableWidget.step = 50;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 0;
	w->editableWidget.setData = (void (*)(void *))&setDebugTemperature;
	

	// Cal at 200
	w = &Debug_Cal200_edit;
	screen_addWidget(w, scr);
	static char c200[5];
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=c200;
	dis->reservedChars=4;
	w->posX = 34;
	w->posY = 33;
	dis->getData = &getCalcAt200;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 1;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt200;
	

	// Cal at 300
	w = &Debug_Cal300_edit;
	screen_addWidget(w, scr);
	static char c300[5];
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=c300;
	dis->reservedChars=4;
	w->posX = 64;
	w->posY = 33;
	dis->getData = &getCalcAt300;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 2;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt300;
	

	// Cal at 400
	w = &Debug_Cal400_edit;
	screen_addWidget(w, scr);
	static char c400[5];
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=c400;
	dis->reservedChars=4;
	w->posX = 94;
	w->posY = 33;
	dis->getData = &getCalcAt400;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 3;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt400;
	
}

#endif
