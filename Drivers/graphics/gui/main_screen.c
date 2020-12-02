/*
 * main_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "main_screen.h"
#include "oled.h"

static uint16_t m_tip = 0;
static uint16_t m_mode = 0;
static uint16_t m_temp = 0;
static char *modestr[] = {"SBY", "SLP", " ", "BST"};
static char *tipstr[sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])];
static multi_option_widget_t *tipsWidget = NULL;
static widget_t *ironTempWidget;
static widget_t *noIronWidget;
static widget_t *TipSelectWidget;
static widget_t *SetPointWidget;
static widget_t *AmbTempWidget;
static widget_t *ModeWidget;
static widget_t *ShakeIconWidget;
static uint32_t lastUpdateTick=0;
static int8_t UpdateReadings;
static int32_t lastTipTemp;
static int32_t lastAmbTemp;
static int32_t lastPwr;
static uint16_t lastVin;
static char NoIronPrevState=0;
static char ShakePrevState=0;
static char TempUnit=0;
char *s;
/*
const unsigned char therm [8*2] = {
		0x00,0x00,0x00,0xC0,0x20,0xC0,0x00,0x00,
		0x00,0x20,0x70,0xFF,0xFE,0xFF,0x70,0x20
};
const unsigned char therm2 [8*2] = {
		0b00000000,0b00000000,0b11111100,0b00000010,0b00000010,0b11111100,0b00000000,0b00000000,
		0b00000000,0b00000011,0b00000111,0b00001111,0b00001111,0b00000111,0b00000011,0b00000000
};

const unsigned char therm [8*2] = {
		0b00000000,0b11111100,0b00000010,0b11111010,0b00000010,0b11111100,0b01010100,0b01010100,
		0b00000011,0b00000100,0b00001011,0b00001011,0b00001011,0b00000100,0b00000011,0b00000000
};
*/
const unsigned char shake [16*2] = {
		0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000000,0b00111100,0b00000010,
		0b00111100,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
		0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000000,0b00000000,
		0b00000000,0b00000011,0b00111100,0b01000000,0b00111100,0b00000011,0b00000001,0b00000001
};



int boostOn(widget_t *w) {
	setCurrentMode(mode_boost);
	return -1;
}
void setTemp(uint16_t *value) {
	m_temp = *value;
	setSetTemperature(m_temp);
}
void * getTemp() {
	m_temp = getSetTemperature();
	return &m_temp;
}

void setMode(uint16_t *value) {
	m_mode = *value;
	setCurrentMode((iron_mode_t)m_mode);
}

void *getMode() {
	m_mode = getCurrentMode();
	return modestr[m_mode];
}

void setTip(uint16_t *value) {
	m_tip = *value;
	systemSettings.currentTip = m_tip;
	setCurrentTip(m_tip);
}

void * getTip() {
	m_tip = systemSettings.currentTip;
	return &m_tip;
}

static int tempProcessInput(widget_t* w, RE_Rotation_t r, RE_State_t * s) {
	switch (w->editable.selectable.state) {
		case widget_selected:
			if(r == Click && getCurrentMode() != mode_normal)
				setCurrentMode(mode_normal);
			break;
		case widget_edit:
			if(r != Rotate_Nothing && r != LongClick && getCurrentMode() != mode_normal) {
				setCurrentMode(mode_normal);
				return -1;
			}
			break;
		default:
			break;
	}
	return default_widgetProcessInput(w, r, s);
}


static void * main_screen_getIronTemp() {
	if(UpdateReadings){
		lastTipTemp = readTipTemperatureCompensatedRaw(Old);
	}
	return &lastTipTemp;
}

static void * main_screen_getVin() {
	if(UpdateReadings){
		lastVin = getSupplyVoltage_v_x10();
	}
	return &lastVin;
}

static void * main_screen_getAmbTemp() {

	if(UpdateReadings){
		lastAmbTemp = readColdJunctionSensorTemp_C_x10()/10;
			if(lastAmbTemp>99){
				lastAmbTemp=99;		//max 99º or it won't fit
			}
			else if (lastAmbTemp<-9){
				lastAmbTemp=-9;		//min -9º or it won't fit
			}
		}
	return &lastAmbTemp;
}

static void * main_screen_getIronPower() {
	if(UpdateReadings){
		lastPwr = getCurrentPower();
	}
	return &lastPwr;
}


static void main_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	tipsWidget->numberOfOptions = systemSettings.currentNumberOfTips;
	default_init(scr);
}
void main_screenUpdate(screen_t *scr) {
	bool clearScreen=0;
	if(TempUnit != systemSettings.tempUnit){
		TempUnit = systemSettings.tempUnit;
		switch(TempUnit){
		case Unit_Celsius:
			strcpy(ironTempWidget->endString, "*#");	//Uses 16x26 stripped font. # = C
			strcpy(AmbTempWidget->endString, "*C");		// * = ° in all fonts
			break;
		case Unit_Farenheit:
			strcpy(ironTempWidget->endString, "*$");	//Uses 16x26 stripped font. $ = F
			strcpy(AmbTempWidget->endString, "*F");
			break;
		case Unit_Kelvin:
			strcpy(ironTempWidget->endString, "*\%");	//Uses 16x26 stripped font. % = K
			strcpy(AmbTempWidget->endString, "*K");
			break;
		default:
			break;
		}
	}
	if(UpdateReadings){
		UpdateReadings = 0;
	}
	if((HAL_GetTick()-lastUpdateTick)>systemSettings.guiUpdateDelay){
		UpdateReadings=1;						//Update realtime readings slower than the rest of the GUI
		lastUpdateTick=HAL_GetTick();
	}
	if(!GetIronPresence() && (NoIronPrevState==0)){
		NoIronPrevState = 1;
		clearScreen=1;
		ironTempWidget->enabled = 0;		//Hide controls and deselect them
		noIronWidget->enabled = 1;
		TipSelectWidget->enabled = 0;
		SetPointWidget->enabled = 0;
		ModeWidget->enabled = 0;
		if(TipSelectWidget->editable.selectable.state==widget_edit){
			TipSelectWidget->editable.selectable.state=widget_selected;
		}
		if(SetPointWidget->editable.selectable.state==widget_edit){
			SetPointWidget->editable.selectable.state=widget_selected;
		}
	}
	else if(GetIronPresence() && (NoIronPrevState==1)){
		NoIronPrevState = 0;
		clearScreen=1;
		ironTempWidget->enabled = 1;			// Hide NO IRON label and restore the controls
		noIronWidget->enabled = 0;
		TipSelectWidget->enabled = 1;
		SetPointWidget->enabled = 1;
		ModeWidget->enabled = 1;
	}
	if(Iron.hasMoved && (ShakePrevState==0)){
		if(!ironTempWidget->enabled){
			clearScreen=1;
			ShakePrevState=1;
			ShakeIconWidget->enabled=1;
		}
	}
	else if(!Iron.hasMoved && (ShakePrevState==1)){
		clearScreen=1;
		ShakePrevState=0;
		ShakeIconWidget->enabled=0;
	}
	if(clearScreen){
		ClearBuffer();
	}
	default_screenUpdate(scr);
}
void main_screen_draw(screen_t *scr){
	default_screenDraw(scr);
	UG_DrawLine(0, 14, UG_GetXDim(), 14, C_WHITE );
}

void main_screen_setup(screen_t *scr) {
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		tipstr[x] = systemSettings.ironTips[x].name;
	}
	//scr->draw = &default_screenDraw;
	scr->draw = &main_screen_draw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &main_screen_init;
	scr->update = &main_screenUpdate;
	widget_t *widget;

	//V input display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 0;
	widget->posY = 0;
	widget->font_size = &FONT_8X14_reduced;
	widget->displayWidget.getData = &main_screen_getVin;
	widget->displayWidget.number_of_dec = 1;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	widget->displayWidget.justify = justify_right;
	widget->displayWidget.hasEndStr = 1;
	strcpy(widget->endString, "V");

	//Ambient temperature display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 50;
	widget->posY = 0;
	widget->font_size = &FONT_8X14_reduced;
	widget->displayWidget.getData = &main_screen_getAmbTemp;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;
	widget->displayWidget.hasEndStr = 1;
	strcpy(widget->endString, "*C");
	AmbTempWidget = widget;

	//power display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 93;
	widget->posY = 0;
	widget->font_size = &FONT_8X14_reduced;
	widget->displayWidget.getData = &main_screen_getIronPower;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;
	widget->displayWidget.justify = justify_right;
	widget->displayWidget.hasEndStr = 1;
	strcpy(widget->endString, "%");


	//iron tip temperature display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 17;
	widget->posY = 17;
	widget->font_size =  &FONT_22X36_reduced;

	//widget->font_size =  &FONT_24X40;//PRUEBA
	//widget->posX = 0;
	widget->displayWidget.getData = &main_screen_getIronTemp;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 5;
	widget->displayWidget.justify = justify_right;
	widget->displayWidget.hasEndStr = 1;
	strcpy(widget->endString, "*#");
	ironTempWidget = widget;

	// NO IRON widget
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "'))()"); 	//ERROR
	widget->posX = 8;
	widget->posY = 15 ;
	widget->font_size =  &FONT_22X36_reduced;
	widget->reservedChars = 7;
	widget->draw = &default_widgetDraw;
	widget->enabled = 0;
	noIronWidget = widget;


	// mode
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 127-24;
	widget->posY = 63-14;
	widget->font_size = &FONT_8X14_reduced;
	widget->displayWidget.getData = &getMode;
	widget->displayWidget.type = field_string;
	widget->reservedChars = 3;
	ModeWidget = widget;

	// tip temperature setpoint
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->editable.selectable.processInput = (int (*)(widget_t*, RE_Rotation_t, RE_State_t *))&tempProcessInput;
	widget->posX = 50;
	widget->posY = 63-14;
	widget->font_size = &FONT_8X14_reduced;
	widget->editable.inputData.getData = &getTemp;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 5;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setTemp;
	widget->reservedChars = 5;
	widget->editable.selectable.state = widget_selected;
	widget->editable.selectable.longPressAction = &boostOn;
	widget->editable.max_value = 480;
	widget->editable.min_value = 100;
	widget->displayWidget.hasEndStr = 1;
	widget->displayWidget.justify = justify_right;
	strcpy(widget->endString, "*C");
	SetPointWidget=widget;


	// tips
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 1;
	widget->posY = 63-14;
	widget->font_size = &FONT_8X14_reduced;
	widget->multiOptionWidget.editable.inputData.getData = &getTip;
	widget->multiOptionWidget.editable.inputData.number_of_dec = 0;
	widget->multiOptionWidget.editable.inputData.type = field_uinteger16;
	widget->multiOptionWidget.editable.big_step = 0;
	widget->multiOptionWidget.editable.step = 0;
	widget->multiOptionWidget.editable.selectable.tab = 1;
	widget->multiOptionWidget.editable.setData = (void (*)(void *))&setTip;
	widget->reservedChars = 4;
	widget->multiOptionWidget.options = tipstr;
	widget->multiOptionWidget.numberOfOptions = systemSettings.currentNumberOfTips;
	tipsWidget = &widget->multiOptionWidget;
	widget->multiOptionWidget.currentOption = 0;
	widget->multiOptionWidget.defaultOption = 0;
	TipSelectWidget = widget;

	// Wake activity signal
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_bmp);
	widget->posX = 0;
	widget->posY = 24;
	widget->displayBmp.bmp.p = (unsigned char*)shake;
	widget->displayBmp.bmp.width = 16;
	widget->displayBmp.bmp.height = 16;
	widget->displayBmp.bmp.colors = 2;
	widget->displayBmp.bmp.bpp = 8;
	widget->enabled = 0;
	ShakeIconWidget = widget;

}

