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
int32_t temp;
int32_t debugTemperature = 0;
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
static widget_t Debug_Cal250_edit;
static widget_t Debug_Cal350_edit;
static widget_t Debug_Cal450_edit;
static widget_t Widget_Power_dbg;
static widget_t Widget_Power_dbg2;
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
static void * getCalcAt250() {
	temp = getCurrentTip()->calADC_At_250;
	return &temp;
}
static void setCalcAt250(uint16_t *val) {
	getCurrentTip()->calADC_At_250 = *val;
}
static void * getCalcAt350() {
	temp = getCurrentTip()->calADC_At_350;
	return &temp;
}
static void setCalcAt350(uint16_t *val) {
	getCurrentTip()->calADC_At_350 = *val;
}
static void * getCalcAt450() {
	temp = getCurrentTip()->calADC_At_450;
	return &temp;
}
static void setCalcAt450(uint16_t *val) {
	getCurrentTip()->calADC_At_450 = *val;
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
	u8g2_DrawStr(&u8g2,65,0,"ADC");
	u8g2_DrawStr(&u8g2,65,16,"RAW");
	u8g2_DrawStr(&u8g2,65,33,"ERR");
	u8g2_DrawStr(&u8g2,60,50,"OUT");

	u8g2_DrawStr(&u8g2,0,0,"P");
	u8g2_DrawStr(&u8g2,0,16,"I");
	u8g2_DrawStr(&u8g2,0,33,"D");
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

	//u8g2_DrawStr(&u8g2,65,0,"ADC");
	//u8g2_DrawStr(&u8g2,65,16,"RAW");
	//u8g2_DrawStr(&u8g2,60,50,"OUT");

	u8g2_DrawStr(&u8g2,0,0,"SetP");
	u8g2_DrawStr(&u8g2,0,16,"C250");
	u8g2_DrawStr(&u8g2,0,33,"C350");
	u8g2_DrawStr(&u8g2,0,50,"C450");
	//u8g2_DrawStr(&u8g2,0,20,"SetP C250 C350 C450");//12
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
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 0;
	w->width = 32;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->textAlign = align_right;

	//ADC1 display, unfiltered
	w = &Debug_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 16;
	w->width = 32;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->textAlign = align_right;

	//P TERM
	w = &Debug_PID_P;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	w->posX = 12;
	w->posY = 0;
	w->displayWidget.getData = &debug_screen_getP;
	w->displayWidget.number_of_dec = 2;
	w->textAlign = align_left;
	

	//I TERM
	w = &Debug_PID_I;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	w->posX = 12;
	w->posY = 16;
	w->displayWidget.getData = &debug_screen_getI;
	w->displayWidget.number_of_dec = 2;
	w->textAlign = align_left;

	//D TERM
	w = &Debug_PID_D;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	w->posX = 12;
	w->posY = 33;
	w->displayWidget.getData = &debug_screen_getD;
	w->displayWidget.number_of_dec = 2;
	w->textAlign = align_left;
	

	//ERROR
	w = &Debug_PID_Err;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 33;
	w->width = 32;
	w->textAlign = align_right;
	w->displayWidget.getData = &debug_screen_getError;
	

	// PWM VALUE OUTPUT
	w = &Debug_PID_Out;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	w->posX = 87;
	w->posY = 50;
	w->width = 40;
	w->displayWidget.getData = &debug_screen_getOutput;
	w->textAlign = align_right;

	// power display
	w=&Widget_Power_dbg;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->endString="%";
	dis->reservedChars=4;
	w->posX = 12;
	w->posY = 50;
	w->displayWidget.getData = &debug_screen_getIronPower;
	w->textAlign = align_right;
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
	w=&Widget_Power_dbg2;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->endString="%";
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 50;
	w->displayWidget.getData = &debug_screen_getIronPower;
	w->textAlign = align_right;

	//ADC1 display, filtered
	w = &Debug2_ADC_Val;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 0;
	w->width = 32;
	w->displayWidget.getData = &debug_screen_getADC1;
	w->textAlign = align_right;

	//ADC1 display, unfiltered
	w = &Debug2_ADC_ValRaw;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 92;
	w->posY = 16;
	w->width = 32;
	w->displayWidget.getData = &debug_screen_getADC1_raw;
	w->textAlign = align_right;

	//Debug setpoint
	w = &Debug_SetPoint_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 36;
	w->posY = 0;
	w->width = 40;
	dis->getData = &getDebugTemperature;
	w->editableWidget.big_step = 200;
	w->editableWidget.step = 50;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 0;
	w->editableWidget.setData = (void (*)(void *))&setDebugTemperature;
	

	// Cal at 250
	w = &Debug_Cal250_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 36;
	w->posY = 16;
	w->width = 40;
	dis->getData = &getCalcAt250;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 1;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt250;
	

	// Cal at 350
	w = &Debug_Cal350_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 36;
	w->posY = 32;
	w->width = 40;
	dis->getData = &getCalcAt350;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 2;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt350;
	

	// Cal at 450
	w = &Debug_Cal450_edit;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->posX = 36;
	w->posY = 48;
	w->width=40;
	dis->getData = &getCalcAt450;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 20;
	w->editableWidget.max_value = 4095;
	w->editableWidget.selectable.tab = 3;
	w->editableWidget.setData = (void (*)(void *))&setCalcAt450;
	
}

#endif
