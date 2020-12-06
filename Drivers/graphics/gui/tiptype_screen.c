/*
 * tiptype_screen.c
 *
 *  Created on: 6 dic. 2020
 *      Author: David
 */

#include "tiptype_screen.h"
#include "oled.h"
#include "settings.h"
#include "iron.h"
static uint8_t setup=0;
static uint16_t temp;
static bool FirstBoot=0;
volatile static uint8_t tip;
static char *tipstr[] = {"T12", "JBC" };

static widget_t Widget_tiptype_label;
static widget_t Widget_tiptype_edit;
static widget_t Widget_tiptype_OK;
static widget_t Widget_tiptype_CANCEL;

static void * getTipType() {
	temp = tip;
	return &temp;
}

static void setTipType(uint16_t *val) {
	tip = *val;
}
static int tiptype_OK(widget_t *w) {
	systemSettings.TipType=tip;
	setup=0;
	resetTips();
	saveSettings();
	SetFailState(0);
	if(FirstBoot){
		Widget_tiptype_CANCEL.enabled = 1;
		FirstBoot = 0;
		return screen_main;
	}
	return screen_settings;
}

static int tiptype_CANCEL(widget_t *w) {
	setup=0;
	SetFailState(0);
	return screen_settings;
}

void tiptype_draw(screen_t * scr) {
	switch(setup){
	case 0:
		return;
	case 1:
		ClearBuffer();
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(10,0,"MAKE SURE YOU");//13
		UG_PutString(9,14,"SET THE RIGHT");//13
		UG_PutString(2,28,"IRON TIP MODEL!");//15
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		return;
	case 2:
		return;
	case 3:
		ClearBuffer();
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(2,0,"A WRONG SETTING");//15
		UG_PutString(6,14,"WILL BURN YOUR");//14
		UG_PutString(42,28,"TIP!!");//5
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		if(FirstBoot){ setup+=2; }
		return;
	case 4:
		return;
	case 5:
		ClearBuffer();
		UG_FontSelect(&FONT_8X14_reduced);
		UG_PutString(15,0,"CHANGING TIP");//12
		UG_PutString(2,14,"TYPE WILL ERASE");//15
		UG_PutString(10,28,"THE TIP DATA!");//13
		UG_FontSelect(&FONT_6X8_reduced);
		UG_PutString(0,50,"PUSH BTN TO CONTINUE");//10
		setup++;
		return;
	case 6:
		return;
	case 7:
		ClearBuffer();
		setup++;
		return;
	default:
		default_screenDraw(scr);
	}
}

int tiptype_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	switch(setup){
		case 0:
			return screen_settings;
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

void tiptype_init(screen_t * scr) {
	tip=systemSettings.TipType;
	setup=1;					// 1st state for the setup warning
	SetFailState(1);			// Shut down PWM
	if(tip!=Tip_T12){			// If TIP is already T12, don't change it
		if(tip==Tip_None){
			Widget_tiptype_CANCEL.enabled = 0;
			FirstBoot=1;		//	Tip was set by a factory reset
		}
		tip=Tip_JBC;	// By default take JBC values
	}					// T12 won't get burnt if using JBC settings. But JBC will do if using T12 settings!
	default_init(scr);
}

void tiptype_screen_setup(screen_t *scr) {
	widget_t* w;
	//###################################################################################################################
	// Edit iron type screen
	//###################################################################################################################
	scr->processInput = &tiptype_processInput;
	scr->init = &tiptype_init;
	scr->draw = &tiptype_draw;
	scr->update = &default_screenUpdate;
	w = &Widget_tiptype_label;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "TIP TYPE:");
	w->posX = 0;
	w->posY = 17;
	w->font_size = &FONT_10X16_reduced;
	w->reservedChars =3;


	w = &Widget_tiptype_edit;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 94;
	w->posY = 17;
	w->font_size = &FONT_10X16_reduced;
	w->editable.inputData.getData = &getTipType;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setTipType;
	w->editable.max_value = Tip_JBC;
	w->editable.min_value = Tip_T12;
	w->displayWidget.hasEndStr = 0;
	w->reservedChars = 3;
	w->multiOptionWidget.options = tipstr;
	w->multiOptionWidget.numberOfOptions = 2;

	w = &Widget_tiptype_OK;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 110;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &tiptype_OK;

	w = &Widget_tiptype_CANCEL;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	strcpy(w->displayString, "CANCEL");
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &tiptype_CANCEL;
}
