/*
 * irontype_screen.c
 *
 *  Created on: 6 dic. 2020
 *      Author: David
 */

#include "irontype_screen.h"
#include "oled.h"
#include "settings.h"
#include "iron.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Tip Type variables
//-------------------------------------------------------------------------------------------------------------------------------
static uint8_t setup=0;
static uint16_t temp;
static bool FirstBoot=0;
volatile static uint8_t tip;
static char *tipstr[] = {" T12", "C245", "C210" };

//-------------------------------------------------------------------------------------------------------------------------------
// Tip Type widgets
//-------------------------------------------------------------------------------------------------------------------------------
static widget_t Widget_irontype_edit;
static widget_t Widget_irontype_OK;
static widget_t Widget_irontype_CANCEL;

//-------------------------------------------------------------------------------------------------------------------------------
// Tip Type widget functions
//-------------------------------------------------------------------------------------------------------------------------------
static void * getTipType() {
	temp = tip;
	return &temp;
}

static void setTipType(uint16_t *val) {
	tip = *val;
}
static int irontype_OK(widget_t *w) {
	systemSettings.TipType=tip;
	setup=0;
	resetTips();
	saveSettings();
	SetFailState(0);
	currentPID = systemSettings.ironTips[systemSettings.currentTip].PID;
	setupPIDFromStruct();
	if(FirstBoot){
		Widget_irontype_CANCEL.enabled = 1;
		FirstBoot = 0;
		return screen_main;
	}
	return screen_settingsmenu;
}

static int irontype_CANCEL(widget_t *w) {
	setup=0;
	SetFailState(0);
	return screen_settingsmenu;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Tip Type screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void irontype_draw(screen_t * scr) {
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
	switch(setup){
	case 0:
		return;
	case 1:
		FillBuffer(C_BLACK,fill_dma);
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(11,0,"MAKE SURE YOU");//13
		UG_PutString(11,14,"SET THE RIGHT");//13
		UG_PutString(19,28,"IRON MODEL!");//11
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		return;
	case 2:
		return;
	case 3:
		FillBuffer(C_BLACK,fill_dma);
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(2,0,"A WRONG SETTING");//15
		UG_PutString(10,14,"CAN BURN YOUR");//13
		UG_PutString(42,28,"TIP!!");//5
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		if(FirstBoot){ setup+=2; }
		return;
	case 4:
		return;
	case 5:
		FillBuffer(C_BLACK,fill_dma);
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(7,0,"(CHANGING IRON");//12
		UG_PutString(2,14,"TYPE WILL ERASE");//15
		UG_PutString(11,28,"THE TIP DATA)");//13
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		return;
	case 6:
		return;
	case 7:
		FillBuffer(C_BLACK,fill_dma);
		setup++;
		return;
	default:
		UG_FontSelect(&FONT_10X16_reduced);
		UG_PutString(23,0,"IRON TYPE");//9
		default_screenDraw(scr);
	}
}

int irontype_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	switch(setup){
		case 0:
			return screen_settingsmenu;
		case 1:
			return -1;
		case 2:
			if (input==Click){ setup++; }
			return -1;
		case 3:
			return -1;
		case 4:
			if (input==Click){ setup++; }
			return -1;
		case 5:
			return -1;
		case 6:
			if (input==Click){ setup++; }
			return -1;
		case 7:
			return -1;
		default:
			break;
	}
	return (default_screenProcessInput(scr, input, state));
}
void irontype_init(screen_t * scr) {
	tip=systemSettings.TipType;
	setup=1;					// 1st state for the setup warning
	SetFailState(1);			// Shut down PWM
	if(tip!=Tip_T12){			// If TIP is already T12, don't change it
		if(tip==Tip_None){
			Widget_irontype_CANCEL.enabled = 0;
			FirstBoot=1;		//	Tip was set by a factory reset
		}
		tip=Tip_C210;	// By default take C210 values (Lowest TC output, safest option)
	}					// T12 won't get burnt if using C210 settings. But JBC will do if using T12 settings!
	default_init(scr);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Tip type screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void irontype_screen_setup(screen_t *scr) {
	widget_t* w;
	scr->processInput = &irontype_processInput;
	scr->init = &irontype_init;
	scr->draw = &irontype_draw;
	scr->update = &default_screenUpdate;

	// Tip type select
	w = &Widget_irontype_edit;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 48;
	w->posY = 24;
	w->font_size = &FONT_10X16_reduced;
	w->editable.inputData.getData = &getTipType;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setTipType;
	w->editable.max_value = Tip_C210;
	w->editable.min_value = Tip_T12;
	w->displayWidget.hasEndStr = 0;
	w->reservedChars = 4;
	w->multiOptionWidget.options = tipstr;
	w->multiOptionWidget.numberOfOptions = 3;

	// OK Button
	w = &Widget_irontype_OK;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 110;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &irontype_OK;

	// CANCEL Button
	w = &Widget_irontype_CANCEL;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 2;
	w->posY = 50;
	strcpy(w->displayString, "CANCEL");
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &irontype_CANCEL;
}
