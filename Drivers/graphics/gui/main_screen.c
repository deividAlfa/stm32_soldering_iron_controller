/*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "main_screen.h"
#include "oled.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static uint16_t temp;
static char *modestr[] = {"SBY", "SLP", "   ", "BST"};
static char *tipName[TipSize];
#define toSystem	0
#define fromSystem	1

const uint8_t pulseBMP[] ={
	0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b11000000,0b00111100,0b00000010,
	0b00111100,0b11000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
	0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000001,0b00000000,0b00000000,
	0b00000000,0b00000011,0b00111100,0b01000000,0b00111100,0b00000011,0b00000001,0b00000001
};
UG_BMP_MONO pulse = {
	p:pulseBMP,
	width: 16,
	height: 16
};

const uint8_t settingsBMP[] ={
	0b01100000,0b01100110,0b11111110,0b11111100,0b10011100,0b00001111,0b00001111,0b10011100,0b11111100,0b11111110,0b01100110,0b01100000,
	0b00000000,0b00000110,0b00000111,0b00000011,0b00000011,0b00001111,0b00001111,0b00000011,0b00000011,0b00000111,0b00000110,0b00000000
};
UG_BMP_MONO settingsBtn = {
	p:settingsBMP,
	width: 12,
	height: 12
};
//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_NTC
static widget_t Widget_AmbTemp;
#endif
#ifdef USE_VIN
static widget_t Widget_Vsupply;
#endif
static widget_t Widget_IronTemp;
static widget_t Widget_Power;
static widget_t Widget_noIron;
static widget_t Widget_TipSelect;
static widget_t Widget_SetPoint;
static widget_t Widget_Mode;
static widget_t Widget_PulseIcon;
static widget_t Widget_SettingsBtn;
static uint32_t lastUpdateTick;
static bool UpdateReadings;
static int16_t lastTipTemp;
static int8_t lastPwr;
#ifdef USE_NTC
static int8_t lastAmbTemp;
#endif
#ifdef USE_VIN
static uint16_t lastVin;
#endif
static bool NoIronPrevState=0, PulsePrevState=0;
static bool TempUnit;

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------

static void setTemp(uint16_t *value) {
	setSetTemperature(*value);
}
static void * getTemp() {
	temp = getSetTemperature();
	return &temp;
}
static void *getMode() {
	temp = getCurrentMode();
	return modestr[temp];
}

static void setTip(uint16_t *value) {
	systemSettings.Profile.currentTip = *value;
	setCurrentTip(*value);
}
static void * getTip() {
	temp = systemSettings.Profile.currentTip;
	return &temp;
}

static void * main_screen_getIronTemp() {
	if(UpdateReadings){
		lastTipTemp = readTipTemperatureCompensated(stored_reading,read_Avg);
	}
	return &lastTipTemp;
}

#ifdef USE_VIN
static void * main_screen_getVin() {
	if(UpdateReadings){
		lastVin = getSupplyVoltage_v_x10();
	}
	return &lastVin;
}
#endif
#ifdef USE_NTC
static void * main_screen_getAmbTemp() {
	if(UpdateReadings){
		lastAmbTemp = readColdJunctionSensorTemp_x10(TempUnit)/10;
	}
	return &lastAmbTemp;
}
#endif

static void * main_screen_getIronPower() {
	if(UpdateReadings){
		lastPwr = getCurrentPower();
	}
	return &lastPwr;
}

static int enterSettings(widget_t *w) {
	return screen_settingsmenu;
}

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void tempUnitChanged(void) {
	TempUnit = systemSettings.settings.tempUnit;
	if(TempUnit==mode_Farenheit){
		Widget_IronTemp.EndStr="*F";
		#ifdef USE_NTC
		Widget_AmbTemp.EndStr="*F";						// '*' shown as '°' in all fonts
		#endif
		Widget_SetPoint.EndStr="*F";
		Widget_SetPoint.editable.max_value=900;
		Widget_SetPoint.editable.min_value=350;
		Widget_SetPoint.editable.big_step = 20;
		Widget_SetPoint.editable.step = 10;
	}
	else{
		Widget_IronTemp.EndStr="*C";
		#ifdef USE_NTC
		Widget_AmbTemp.EndStr="*C";					// '*' shown as '°' in all fonts
		#endif
		Widget_SetPoint.EndStr="*C";
		Widget_SetPoint.editable.max_value=480;
		Widget_SetPoint.editable.min_value=180;
		Widget_SetPoint.editable.big_step = 10;
		Widget_SetPoint.editable.step = 5;
	}
}

static int tempProcessInput(widget_t* w, RE_Rotation_t r, RE_State_t * s) {
	if((r==Click) && (getCurrentMode() != mode_normal)){
		setCurrentMode(mode_normal);
	}
	else if((w->editable.selectable.state==widget_selected) && (r == LongClick)){
			setCurrentMode(mode_boost);
			return -1;
	}
	return default_widgetProcessInput(w, r, s);
}
static int TipProcessInput(widget_t* w, RE_Rotation_t r, RE_State_t * s) {
	if((w->editable.selectable.state==widget_selected) && (r == LongClick)){
			return screen_pid;
	}
	return default_widgetProcessInput(w, r, s);
}
static void main_screen_init(screen_t *scr) {
	tempUnitChanged();
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	Widget_TipSelect.multiOptionWidget.numberOfOptions = systemSettings.Profile.currentNumberOfTips;
	Widget_SetPoint.editable.step = systemSettings.settings.tempStep;
	Widget_SetPoint.editable.big_step = systemSettings.settings.tempStep;
	default_init(scr);
}

void main_screenUpdate(screen_t *scr) {
	bool clearScreen=0;
	//Widget_TipSelect.multiOptionWidget.numberOfOptions  = systemSettings.Profile.currentNumberOfTips;
	if(TempUnit != systemSettings.settings.tempUnit){
		tempUnitChanged();
	}
	if(UpdateReadings){
		UpdateReadings = 0;
	}
	if((HAL_GetTick()-lastUpdateTick)>systemSettings.settings.guiUpdateDelay){
		UpdateReadings=1;						//Update realtime readings slower than the rest of the GUI
		lastUpdateTick=HAL_GetTick();
	}
	if(!GetIronPresence() && !NoIronPrevState){
		NoIronPrevState = 1;
		clearScreen=1;
		Widget_IronTemp.enabled = 0;		//Hide controls and deselects them
		Widget_noIron.enabled = 1;
		Widget_TipSelect.enabled = 0;
		Widget_SetPoint.enabled = 0;
		Widget_Mode.enabled = 0;
		Widget_PulseIcon.enabled=0;

		Widget_TipSelect.editable.selectable.previous_state=widget_idle;
		Widget_TipSelect.editable.selectable.state=widget_idle;

		Widget_SetPoint.editable.selectable.previous_state=widget_idle;
		Widget_SetPoint.editable.selectable.state=widget_idle;

		Widget_SettingsBtn.buttonWidget.selectable.previous_state=widget_idle;
		Widget_SettingsBtn.buttonWidget.selectable.state=widget_selected;

		scr->current_widget=&Widget_SettingsBtn;
	}
	else if(GetIronPresence() && NoIronPrevState){
		NoIronPrevState = 0;
		clearScreen=1;
		Widget_IronTemp.enabled = 1;			// Hide NO IRON label and restore the controls
		Widget_noIron.enabled = 0;
		Widget_TipSelect.enabled = 1;
		Widget_SetPoint.enabled = 1;
		Widget_Mode.enabled = 1;
	}
	if(Iron.newActivity && !PulsePrevState){
		if(Widget_IronTemp.enabled){
			clearScreen=1;
			PulsePrevState=1;
			Widget_PulseIcon.enabled=1;
		}
	}
	else if(Iron.newActivity && PulsePrevState){
		if((HAL_GetTick()-Iron.lastActivityTime)>50){
			clearScreen=1;
			PulsePrevState=0;
			Iron.newActivity=0;
			Widget_PulseIcon.enabled=0;
		}
	}
	if(clearScreen){
		FillBuffer(C_BLACK,fill_dma);
	}

	Widget_SetPoint.editable.step = systemSettings.settings.tempStep;
	Widget_SetPoint.editable.big_step = systemSettings.settings.tempStep;

	default_screenUpdate(scr);
}
void main_screen_draw(screen_t *scr){
	#ifdef USE_NTC
	uint8_t x = strlen(Widget_AmbTemp.displayString);
	x*=Widget_AmbTemp.font_size->char_width;
	Widget_AmbTemp.posX = ( ((UG_GetXDim()-x)/2)-1+4 );	// Center the AmbTemp widget
	#endif
	default_screenDraw(scr);
	//UG_DrawLine(0, 0, UG_GetXDim()-1, 0, C_WHITE );
	//UG_DrawLine(0, 15, UG_GetXDim()-1, 15, C_WHITE );
}


//-------------------------------------------------------------------------------------------------------------------------------
// Main screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void main_screen_setup(screen_t *scr) {

	//
	for(int x = 0; x < TipSize; x++) {
		tipName[x] = systemSettings.Profile.tip[x].name;
	}

	screen_setDefaults(scr);
	scr->draw = &main_screen_draw;
	scr->init = &main_screen_init;
	scr->update = &main_screenUpdate;
	widget_t *w;

	#ifdef USE_VIN
	//V input display
	w = &Widget_Vsupply;
	screen_addWidget(w, scr);
	static char vin[6];
	widgetDefaultsInit(w, widget_display);
	w->displayString=vin;
	w->EndStr="V";
	w->reservedChars=5;
	w->posX = 0;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->displayWidget.getData = &main_screen_getVin;
	w->displayWidget.number_of_dec = 1;
	w->displayWidget.type = field_uint16;
	w->displayWidget.justify = justify_right;
	#endif

	#ifdef USE_NTC
	//Ambient temperature display
	w=&Widget_AmbTemp;
	screen_addWidget(w,scr);
	static char ntc[6];
	widgetDefaultsInit(w, widget_display);
	w->displayString=ntc;
	w->EndStr="*C";
	w->reservedChars=5;
	w->posX = 55;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->displayWidget.getData = &main_screen_getAmbTemp;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_int8;
	#endif

	//power display
	w=&Widget_Power;
	screen_addWidget(w,scr);
	static char pwr[5];
	widgetDefaultsInit(w, widget_display);
	w->displayString=pwr;
	w->EndStr="%";
	w->reservedChars=4;
	w->posX = 95;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->displayWidget.getData = &main_screen_getIronPower;
	w->displayWidget.type = field_int8;
	w->displayWidget.justify = justify_right;


	//iron tip temperature display
	w=&Widget_IronTemp;
	screen_addWidget(w,scr);
	static char irontmp[6];
	widgetDefaultsInit(w, widget_display);
	w->displayString=irontmp;
	w->EndStr="*C";
	w->reservedChars=5;
	w->posX = 17;
	w->posY = 16;
	w->font_size =  &font_iron_temp;
	w->displayWidget.getData = &main_screen_getIronTemp;
	w->displayWidget.number_of_dec = 0;
	w->displayWidget.type = field_uint16;
	w->displayWidget.justify = justify_right;

	// NO IRON widget
	w=&Widget_noIron;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_label);
	w->displayString="NO IRON";
	w->reservedChars=7;
	w->posX = 7;
	w->posY = 17 ;
	w->font_size =  &font_no_iron;
	w->draw = &default_widgetDraw;
	w->enabled = 0;

	// Settings Button
	w=&Widget_SettingsBtn;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_bmp_button);
	w->posX = 2;
	w->posY = 50;
	w->buttonWidget.bmp = &settingsBtn;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &enterSettings;

	// Tips
	w=&Widget_TipSelect;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=TipCharSize-1;
	w->posX = 64;
	w->posY = 50;
	w->font_size = &FONT_8X14_reduced;
	w->multiOptionWidget.editable.inputData.getData = &getTip;
	w->multiOptionWidget.editable.inputData.number_of_dec = 0;
	w->multiOptionWidget.editable.inputData.type = field_uint8;
	w->multiOptionWidget.editable.big_step = 0;
	w->multiOptionWidget.editable.step = 0;
	w->multiOptionWidget.editable.selectable.tab = 2;
	w->multiOptionWidget.editable.setData = (void (*)(void *))&setTip;
	w->multiOptionWidget.options = tipName;
	w->multiOptionWidget.currentOption = 0;
	w->multiOptionWidget.defaultOption = 0;
	w->multiOptionWidget.editable.selectable.processInput=&TipProcessInput;

	// Tip temperature setpoint
	w=&Widget_SetPoint;
	screen_addWidget(w,scr);
	static char SetP[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=SetP;
	w->EndStr="*C";
	w->reservedChars=5;
	w->editable.selectable.processInput = &tempProcessInput;
	w->posX = 19;
	w->posY = 50;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getTemp;
	w->editable.inputData.type = field_uint16;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setTemp;
	//w->editable.selectable.longPressAction = &boostOn;
	w->displayWidget.justify = justify_right;

	// Iron mode label
	w=&Widget_Mode;
	screen_addWidget(w,scr);
	char IronMode[4];
	widgetDefaultsInit(w, widget_display);
	w->displayString=IronMode;
	w->reservedChars=3;
	w->posX = 103;
	w->posY = 50;
	w->font_size = &FONT_8X14_reduced;
	w->displayWidget.getData = &getMode;
	w->displayWidget.type = field_string;

	// Wake activity signal icon
	w=&Widget_PulseIcon;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_bmp);
	w->posX = 0;
	w->posY = 22;
	w->displayBmp.bmp = &pulse;
	w->enabled = 0;
}

