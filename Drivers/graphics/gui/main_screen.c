/*
 * main_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "main_screen.h"
#include "oled.h"

#define NO_IRON_ADC 4000
static uint8_t hasIron = 1;
static uint16_t m_tip = 0;
static uint16_t m_mode = 0;
static uint16_t m_temp = 0;
static char *modestr[] = {"STB:", "SLP:", "SET:", "BOO:"};
static char *tipstr[sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])];
static multi_option_widget_t *tipsWidget = NULL;
static widget_t *ironTempWidget;
static widget_t *ironTempLabelWidget;
static widget_t *noIronWidget;
static widget_t *TipSelectWidget;
static widget_t *SetPointWidget;
static widget_t *SetModeWidget;
static int16_t temp;
static int8_t UpdateReadings;
static uint16_t lastTipTemp;
static uint16_t lastAmbTemp;
static uint16_t lastPwr;
static uint16_t lastVin;
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
*/
const unsigned char therm [8*2] = {
		0b00000000,0b11111100,0b00000010,0b11111010,0b00000010,0b11111100,0b01010100,0b01010100,
		0b00000011,0b00000100,0b00001011,0b00001011,0b00001011,0b00000100,0b00000011,0b00000000
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
	if(UpdateReadings){
		lastTipTemp = readTipTemperatureCompensated(0);
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
			else if (temp<-9){
				lastAmbTemp=-9;		//min -9º or it won't fit
			}
		}
	return &lastAmbTemp;
}

static void * main_screen_getIronPower() {

	if(!hasIron){
		lastPwr=0;
		return &lastPwr;
	}
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
	static uint32_t lastNoIron=0,lastUpdateTick=0;
	uint16_t t = Iron.Temp.Temp_Adc_Avg;
	if(UpdateReadings){
		UpdateReadings = 0;
	}
	if((HAL_GetTick()-lastUpdateTick)>GUI_Update_ms){
		UpdateReadings=1;						//Update temperature readings slower than the rest of the gui
		lastUpdateTick=HAL_GetTick();
	}
	if(t > NO_IRON_ADC){
		if(!hasIron) {
			lastNoIron=HAL_GetTick();
		}
		else{
			hasIron = 0;
			UG_FillScreen(C_BLACK);
			ironTempLabelWidget->enabled = 0;
			ironTempWidget->enabled = 0;
			noIronWidget->enabled = 1;
			//TipSelectWidget->enabled = 0;
			//SetPointWidget->enabled = 0;
			//SetModeWidget->enabled = 0;
			buzzer_alarm_start();
		}
	}
	else if((t < NO_IRON_ADC) && !hasIron && ((HAL_GetTick()-lastNoIron)>No_Iron_Delay_mS) ){
		UG_FillScreen(C_BLACK);
		ironTempLabelWidget->enabled = 1;
		ironTempWidget->enabled = 1;
		noIronWidget->enabled = 0;
		//TipSelectWidget->enabled = 1;
		//SetPointWidget->enabled = 1;
		//SetModeWidget->enabled = 1;
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
	widget->posX = 30;
	widget->posY = 18;
	widget->font_size = &FONT_16X26;
	widget->displayWidget.getData = &main_screen_getIronTemp;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 3;
	widget->displayWidget.justify = justify_right;
	ironTempWidget = widget;

	//ºC label next to iron tip temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "\247C");
	widget->posX = 50 + 3 * 12 -5 + 3;
	widget->posY = 18;
	widget->font_size = &FONT_16X26;
	widget->reservedChars = 2;
	widget->draw = &default_widgetDraw;
	ironTempLabelWidget = widget;

	//power display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 93;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getIronPower;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 3;
	widget->displayWidget.justify = justify_right;

	//power percentage symbol
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "%");
	widget->posX = 119;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 1;
	widget->draw = &default_widgetDraw;

	// NO IRON widget
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "NO IRON");
	widget->posX = 7;
	widget->posY = 18 ;
	widget->font_size = &FONT_16X26;
	widget->reservedChars = 7;
	widget->draw = &default_widgetDraw;
	noIronWidget = widget;
	widget->enabled = 0;

	//Thermometer bmp next to Ambient temperature
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_bmp);
	widget->posX = 56;
	widget->posY = 0;
	widget->displayBmp.bmp.p = (unsigned char*)therm;
	widget->displayBmp.bmp.width = 8;
	widget->displayBmp.bmp.height = 16;
	widget->displayBmp.bmp.colors = 2;
	widget->displayBmp.bmp.bpp = 8;
	widget->enabled = 1;

	//Ambient temperature display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 64;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getAmbTemp;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 2;

	//V input display
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 0;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->displayWidget.getData = &main_screen_getVin;
	widget->displayWidget.number_of_dec = 1;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;
	widget->displayWidget.justify = justify_right;

	//V input display "V"
	//power percentage symbol
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	strcpy(widget->displayString, "V");
	widget->posX = 33;
	widget->posY = 0;
	widget->font_size = &FONT_8X14;
	widget->reservedChars = 1;
	widget->draw = &default_widgetDraw;

	// tip temperature setpoint
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_editable);
	widget->editable.selectable.processInput = (int (*)(widget_t*, RE_Rotation_t, RE_State_t *))&tempProcessInput;
	widget->posX = 36;
	widget->posY = 50;
	widget->font_size = &FONT_8X14;
	widget->editable.inputData.getData = &getTemp;
	widget->editable.inputData.number_of_dec = 0;
	widget->editable.inputData.type = field_uinteger16;
	widget->editable.big_step = 10;
	widget->editable.step = 5;
	widget->editable.selectable.tab = 0;
	widget->editable.setData = (void (*)(void *))&setTemp;
	widget->reservedChars = 3;
	widget->editable.selectable.state = widget_selected;
	widget->editable.selectable.longPressAction = &boostOn;
	//scr->current_widget = widget;
	widget->editable.max_value = 480;
	widget->editable.min_value = 100;
	SetPointWidget=widget;

	// mode
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 1;
	widget->posY = 50;
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
	widget->multiOptionWidget.currentOption = 2;
	widget->multiOptionWidget.defaultOption = 2;
	SetModeWidget = widget;

	// tips
	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_multi_option);
	widget->posX = 93;
	widget->posY = 50;
	widget->font_size = &FONT_8X14;
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


}

