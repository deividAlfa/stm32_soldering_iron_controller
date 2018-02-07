/*
 * main_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "main_screen.h"
#include "tempsensors.h"
#include "../../../Src/iron.h"
#include "../../../Src/settings.h"
#include "../generalIO/buzzer.h"

#define NO_IRON_ADC 50000
static uint8_t hasIron = 1;
static uint16_t m_tip = 0;
static uint16_t m_mode = 0;
static uint16_t m_temp = 0;
static char *modestr[] = {"STB:", "BOO:", "SLP:", "SET:"};
static char *tipstr[sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])];
static multi_option_widget_t *tipsWidget = NULL;
static widget_t *ironTempWidget;
static widget_t *ironTempLabelWidget;
static widget_t *noIronWidget;
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

void * getMode() {
	m_mode = getCurrentMode();
	return &m_mode;
}

void setTip(uint16_t *value) {
	m_tip = *value;
	systemSettings.currentTip = m_tip;
	saveSettings();
	setCurrentTip(m_tip);
}

void * getTip() {
	m_tip = systemSettings.currentTip;
	return &m_tip;
}

static uint16_t temp;
const unsigned char therm [] = {
		0x00, 0x00, 0x00, 0xC0, 0x20, 0xC0, 0x00, 0x00,
		0x00, 0x20, 0x70, 0xFF, 0xFE, 0xFF, 0x70, 0x20

};

static int tempProcessInput(widget_t* w, RE_Rotation_t r, RE_State_t * s) {
	switch (w->editable.selectable.state) {
		case widget_selected:
			if(r == Click && getCurrentMode() != mode_set)
				setCurrentMode(mode_set);
			break;
		case widget_edit:
			if(r != Rotate_Nothing && r != LongClick && getCurrentMode() != mode_set) {
				setCurrentMode(mode_set);
				return -1;
			}
			break;
		default:
			break;
	}
	return default_widgetProcessInput(w, r, s);
}
static void * main_screen_getIronTemp() {
	temp = readTipTemperatureCompensated(0);
	return &temp;
}

static void * main_screen_getAmbTemp() {
	temp = readColdJunctionSensorTemp_mC() / 100;
	return &temp;
}

static void * main_screen_getIronPower() {
	temp = getCurrentPower();
	return &temp;
}
static void main_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	tipsWidget->numberOfOptions = systemSettings.currentNumberOfTips;
	default_init(scr);
}
void main_screenUpdate(screen_t *scr) {
	uint16_t t = iron_temp_adc_avg;
	if((t > NO_IRON_ADC) && hasIron) {
		UG_FillScreen(C_BLACK);
		ironTempLabelWidget->enabled = 0;
		ironTempWidget->enabled = 0;
		noIronWidget->enabled = 1;
		buzzer_alarm_start();
		hasIron = 0;
	}
	else if((t <= NO_IRON_ADC) && !hasIron){
		UG_FillScreen(C_BLACK);
		ironTempLabelWidget->enabled = 1;
		ironTempWidget->enabled = 1;
		noIronWidget->enabled = 0;
		buzzer_alarm_stop();
		hasIron = 1;
	}
	default_screenUpdate(scr);

}
void main_screen_setup(screen_t *scr) {
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		tipstr[x] = systemSettings.ironTips[x].name;
	}
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &main_screen_init;
	scr->update = &main_screenUpdate;

	//iron tip temperature display
	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 45;
	widget->posY = 25;
	widget->font_size = &FONT_12X20;
	widget->displayWidget.getData = &main_screen_getIronTemp;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 3;
	ironTempWidget = widget;

	//power display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 93;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getIronPower;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 3;

	//power percentage symbol
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "%";
	strcpy(widget->displayString, s);
	widget->posX = 119;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 1;
	widget->draw = &default_widgetDraw;

	//ºC label next to iron tip temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "\247C";
	strcpy(widget->displayString, s);
	widget->posX = 50 + 3 * 12 -5;
	widget->posY = 20 + 5;
	widget->font_size = &FONT_12X20;
	widget->reservedChars = 2;
	widget->draw = &default_widgetDraw;
	ironTempLabelWidget = widget;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "NO IRON");
	widget->posX = 20;
	widget->posY = 20 + 5;
	widget->font_size = &FONT_12X20;
	widget->reservedChars = 7;
	widget->draw = &default_widgetDraw;
	noIronWidget = widget;
	widget->enabled = 0;

	//Thermometer bmp next to Ambient temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_bmp);
	widget->posX = 85;
	widget->posY = 47;
	widget->displayBmp.bmp.p = (unsigned char*)therm;
	widget->displayBmp.bmp.width = 8;
	widget->displayBmp.bmp.height = 16;
	widget->displayBmp.bmp.colors = 2;
	widget->displayBmp.bmp.bpp = 8;

	//Ambient temperature display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 95;
	widget->posY = 50;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getAmbTemp;
	widget->displayWidget.number_of_dec = 1;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;


	// tip temperature setpoint
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->editable.selectable.processInput = (int (*)(widget_t*, RE_Rotation_t, RE_State_t *))&tempProcessInput;
	widget->posX = 36;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->editable.inputData.getData = &getTemp;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 1;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setTemp;
	widget->reservedChars = 3;
	widget->editable.selectable.state = widget_edit;
	widget->editable.selectable.longPressAction = &boostOn;
	scr->current_widget = widget;

	// mode
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 1;
	widget->posY = 1;
	widget->font_size = &FONT_8X14;
	widget->multiOptionWidget.editable.inputData.getData = &getMode;
	widget->multiOptionWidget.editable.inputData.number_of_dec = 0;
	widget->multiOptionWidget.editable.inputData.type = field_uinteger16;
	widget->multiOptionWidget.editable.big_step = 0;
	widget->multiOptionWidget.editable.step = 0;
	widget->multiOptionWidget.editable.selectable.tab = 2;
	widget->multiOptionWidget.editable.setData = (void (*)(void *))&setMode;

	widget->reservedChars = 4;

	widget->multiOptionWidget.options = modestr;
	widget->multiOptionWidget.numberOfOptions = 4;
	widget->multiOptionWidget.currentOption = 0;
	widget->multiOptionWidget.defaultOption = 0;
	// tips
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 1;
	widget->posY = 50;
	widget->font_size = &FONT_8X14;
	widget->multiOptionWidget.editable.inputData.getData = &getTip;
	widget->multiOptionWidget.editable.inputData.number_of_dec = 0;
	widget->multiOptionWidget.editable.inputData.type = field_uinteger16;
	widget->multiOptionWidget.editable.big_step = 0;
	widget->multiOptionWidget.editable.step = 0;
	widget->multiOptionWidget.editable.selectable.tab = 1;
	widget->multiOptionWidget.editable.setData = (void (*)(void *))&setTip;

	widget->reservedChars = 5;

	widget->multiOptionWidget.options = tipstr;
	widget->multiOptionWidget.numberOfOptions = systemSettings.currentNumberOfTips;
	tipsWidget = &widget->multiOptionWidget;
	widget->multiOptionWidget.currentOption = 0;
	widget->multiOptionWidget.defaultOption = 0;
}

