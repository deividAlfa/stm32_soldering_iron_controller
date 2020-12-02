/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "settings_screen.h"
#include "oled.h"

static widget_t *combo = NULL;
//static widget_t *advancedSettings = NULL;
#ifdef TEST
static uint16_t fGamma;
static uint16_t fBeta1;
static uint16_t fBeta2;
static float 	oldfGamma;
static float 	oldfBeta1;
static float 	oldfBeta2;
#else
static uint16_t KP;
static uint16_t KI;
static uint16_t KD;
#endif
static uint16_t CONTRAST;
static uint16_t MAX_POWER;
static uint16_t oldBTIME, oldBTEMP, oldSLEEPTIME, oldSLEEPTEMP, oldSTANDBYTIME;
static widget_t *tipCombo = NULL;
static widget_t *delTipButton = NULL;
static comboBox_item_t *addNewTipComboItem = NULL;
char str[20];
//static char *tempstr[] = {"*C", "*F", "*K" };
static void edit_iron_tip_screen_init(screen_t *scr) {
	if(strcmp(tipCombo->comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "   ");
		delTipButton->enabled = 0;
	}
	else {
		strcpy(str, tipCombo->comboBoxWidget.currentItem->text);
		delTipButton->enabled = 1;
	}
	default_init(scr);
}
static void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i = tipCombo->comboBoxWidget.items;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		if(x < systemSettings.currentNumberOfTips) {
			strcpy(i->text, systemSettings.ironTips[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	tipCombo->comboBoxWidget.currentItem = tipCombo->comboBoxWidget.items;
	tipCombo->comboBoxWidget.currentScroll = 0;
	if(systemSettings.currentNumberOfTips > sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])) {
		addNewTipComboItem->enabled = 0;
	}
}
static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(tipCombo->comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(systemSettings.ironTips[systemSettings.currentNumberOfTips].name, str);
		++systemSettings.currentNumberOfTips;
	}
	else {
		for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++ x) {
			if(strcmp(tipCombo->comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
				strcpy(systemSettings.ironTips[x].name, str);
				break;
			}
		}
	}
	return screen_edit_iron_tips;
}
static int cancelTip(widget_t *w) {
	return screen_edit_iron_tips;
}
static int delTip(widget_t *w) {
	uint8_t itemIndex = 0;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++ x) {
		if(strcmp(tipCombo->comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
			itemIndex = x;
			break;
		}
	}
	for(int x = itemIndex; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]) - 1; ++ x) {
		systemSettings.ironTips[x] = systemSettings.ironTips[x + 1];
	}
	--systemSettings.currentNumberOfTips;
	return screen_edit_iron_tips;
}

static void * getMaxPower() {
	MAX_POWER = currentPID.max * 100;
	return &MAX_POWER;
}
static void setMaxPower(uint16_t *val) {
	MAX_POWER = *val;
	currentPID.max = (double)MAX_POWER / 100.0;
	setupPIDFromStruct();
}

static void * getContrast_() {
	CONTRAST = getContrast();
	return &CONTRAST;
}
static void setContrast_(uint16_t *val) {
	CONTRAST = *val;
	setContrast(CONTRAST);
}

static void * getBoostTime() {
	return &systemSettings.boost.Time;
}
static void setBoostTime(uint16_t *val) {
	systemSettings.boost.Time = *val;
}
static void * getBoostTemp() {
	return &systemSettings.boost.Temperature;
}
static void setBoostTemp(uint16_t *val) {
	systemSettings.boost.Temperature = *val;
}
static int returnSettingsScreen(widget_t *w) {
	return screen_settings;
}

static void setSleepTime(uint16_t *val) {
	systemSettings.sleep.Time = *val;
}

static void * getSleepTime() {
	return &systemSettings.sleep.Time;
}
static void setStandByTime(uint16_t *val) {
	systemSettings.standby.Time  = *val;
}
static void * getStandByTime() {
	return &systemSettings.standby.Time;
}
static void setSleepTemp(uint16_t *val) {
	systemSettings.sleep.Temperature = *val;
}
static void * getSleepTemp() {
	return &systemSettings.sleep.Temperature;
}
#ifdef TEST
static int saveTC(widget_t *w) {
	//systemSettings.ironTips[systemSettings.currentTip].PID = currentPID;
	//saveSettings();
	return screen_settings;
}
static int cancelTC(widget_t *w) {
	TempControl.fGamma = oldfGamma;
	TempControl.fBeta1 = oldfBeta1;
	TempControl.fBeta2  =oldfBeta2;
	return screen_settings;
}

static void * getfGamma() {
	fGamma = TempControl.fGamma * 100;
	return &fGamma;
}
static void setfGamma(uint16_t *val) {
	TempControl.fGamma = (float)*val/100;
}
static void * getfBeta1() {
	fBeta1 = TempControl.fBeta1 * 100;
	return &fBeta1;
}
static void setfBeta1(uint16_t *val) {
	TempControl.fBeta1 = (float)*val/100;
}
static void * getfBeta2() {
	fBeta2 = TempControl.fBeta2 * 100;
	return &fBeta2;
}
static void setfBeta2(uint16_t *val) {
	TempControl.fBeta2 = (float)*val/100;
}
static void on_Enter(screen_t *scr) {
	oldBTIME = systemSettings.boost.Time;
	oldBTEMP = systemSettings.boost.Temperature;
	oldSLEEPTIME = systemSettings.sleep.Time;
	oldSLEEPTEMP = systemSettings.sleep.Temperature;
	oldSTANDBYTIME = systemSettings.standby.Time;
	oldfGamma = TempControl.fGamma;
	oldfBeta1 = TempControl.fBeta1;
	oldfBeta2 = TempControl.fBeta2;
}
#else

static int okPID(widget_t *w) {
	systemSettings.ironTips[systemSettings.currentTip].PID = currentPID;
	return screen_settings;
}

static void * getKp() {
	KP = currentPID.Kp * 1000000;
	return &KP;
}
static void setKp(uint16_t *val) {
	KP = *val;
	currentPID.Kp = (double)KP / 1000000;
	setupPIDFromStruct();
}
static void * getKi() {
	KI = currentPID.Ki * 1000000;
	return &KI;
}
static void setKi(uint16_t *val) {
	KI = *val;
	currentPID.Ki = (double)KI / 1000000;
	setupPIDFromStruct();
}
static void * getKd() {
	KD = currentPID.Kd * 1000000;
	return &KD;
}
static void setKd(uint16_t *val) {
	KD = *val;
	currentPID.Kd = (double)KD / 1000000;
	setupPIDFromStruct();
}
static void on_Enter(screen_t *scr) {
	oldBTIME = systemSettings.boost.Time;
	oldBTEMP = systemSettings.boost.Temperature;
	oldSLEEPTIME = systemSettings.sleep.Time;
	oldSLEEPTEMP = systemSettings.sleep.Temperature;
	oldSTANDBYTIME = systemSettings.standby.Time;
}
#endif
/*
static void * getPwmPeriod() {
	return &systemSettings.pwmPeriod;
}
static void setPwmPeriod(uint16_t *val) {
	systemSettings.pwmPeriod = *val;
	ApplyPwmSettings();
}
static void * getPwmDelay() {
	return &systemSettings.pwmDelay;
}
static void setPwmDelay(uint16_t *val) {
	systemSettings.pwmDelay = *val;
	ApplyPwmSettings();
}

static void * getTmpUnit() {
	return tempstr[systemSettings.tempUnit];
}
static void setTmpUnit(uint16_t *val) {
	systemSettings.tempUnit = *val;

}

static void * getGuiUpd() {
	return &systemSettings.guiUpdateDelay;
}
static void setGuiUpd(uint16_t *val) {
	systemSettings.guiUpdateDelay = *val;

}

static void * getSavDelay() {
	return &systemSettings.saveSettingsDelay;
}
static void setSavDelay(uint16_t *val) {
	systemSettings.saveSettingsDelay = *val;

}

static void * getNoIronDelay() {
	return &systemSettings.noIronDelay;
}
static void setNoIronDelay(uint16_t *val) {
	systemSettings.noIronDelay = *val;

}

static void * getNoIronADC() {
	return &systemSettings.noIronValue;
}
static void setNoIronADC(uint16_t *val) {
	systemSettings.noIronValue = *val;

}

static int ReturnAdvScreen() {
	return screen_advanced;
}
*/
static void settings_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
	scr->current_widget = combo;
	scr->current_widget->comboBoxWidget.selectable.state = widget_selected;
}

void settings_screen_setup(screen_t *scr) {
	///settings combobox
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &settings_screen_init;
	scr->update = &default_screenUpdate;
	scr->onEnter = &on_Enter;
	widget_t *widget;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_combo);
	widget->posY = 5;
	widget->posX = 0;
	widget->font_size = &FONT_10X16_reduced;
	#ifdef TEST
	comboAddItem(widget, "TEMP CONTROL", screen_edit_pid);
	#else
	comboAddItem(widget, "PID", screen_edit_pid);
	#endif
	comboAddItem(widget, "SCREEN", screen_edit_contrast);
	comboAddItem(widget, "POWER", screen_edit_max_power);
	comboAddItem(widget, "SLEEP", screen_edit_sleep);
	comboAddItem(widget, "BOOST", screen_edit_boost);
	//comboAddItem(widget, "ADVANCED", screen_advanced);
	comboAddItem(widget, "TIPS", screen_edit_iron_tips);
	comboAddItem(widget, "CALIBRATION", screen_edit_calibration_wait);
	comboAddItem(widget, "EXIT", screen_main);
	combo = widget;
	//--------------------------------------------------------------------------------------------
	// Edit Temp Control screen
	screen_t *sc =oled_addScreen(screen_edit_pid);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	widget_t *w;
#ifdef TEST

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Gamma:");
	w->posX = 10;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 6;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 75;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getfGamma;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setfGamma;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Beta1:");
	w->posX = 10;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 6;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 75;
	w->posY = 17;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getfBeta1;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setfBeta1;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Beta2:");
	w->posX = 10;
	w->posY = 34;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 6;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 75;
	w->posY = 34;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getfBeta2;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setfBeta2;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "BACK");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &cancelTC;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	strcpy(w->displayString, "SAVE");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 4;
	w->buttonWidget.action = &saveTC;
#else
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Kp:");
	w->posX = 35;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getKp;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setKp;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Ki:");
	w->posX = 35;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getKi;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setKi;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Kd:");
	w->posX = 35;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getKd;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setKd;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 110;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &okPID;


#endif
	//-------------------------------------------------------------------------------------------------
	// Edit contrast screen
	sc = oled_addScreen(screen_edit_contrast);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "BRIGHTNESS");
	w->posX = 14;
	w->posY = 0;
	w->font_size = &FONT_10X16_reduced;
	w->reservedChars =3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->editable.selectable.state = widget_edit;	// Widget in edit mode by default
	w->editable.selectable.force_state = 1;		// Widget always forced in the initial state
	w->editable.selectable.NoHighlight = 1;		// Don't highlight
	w->editable.selectable.onSelectAction = &returnSettingsScreen;
	w->posX = 31;
	w->posY = 25;
	w->font_size = &FONT_22X36_reduced;
	w->editable.inputData.getData = &getContrast_;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 50;
	w->editable.step = 5;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setContrast_;
	w->editable.max_value = 255;
	w->editable.selectable.state = widget_edit;
	w->editable.min_value = 10;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;

	//--------------------------------------------------------------------------------------------
	// Edit power screen
	sc = oled_addScreen(screen_edit_max_power);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "MAX POWER");
	w->posX = 19;
	w->posY = 0;
	w->font_size = &FONT_10X16_reduced;
	w->reservedChars =3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->editable.selectable.state = widget_edit;	// Widget in edit mode by default
	w->editable.selectable.force_state = 1;		// Widget always forced in the initial state
	w->editable.selectable.NoHighlight = 1;		// Don't highlight
	w->editable.selectable.onSelectAction = &returnSettingsScreen;	// When clicking, do this instead changing to selected mode
	w->posX = 31;
	w->posY = 25;
	w->font_size = &FONT_22X36_reduced;
	w->editable.inputData.getData = &getMaxPower;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 10;
	w->editable.step = 5;
	w->editable.selectable.tab = 0;
	w->editable.selectable.state = widget_edit;
	w->editable.setData = (void (*)(void *))&setMaxPower;
	w->editable.max_value = 100;
	w->editable.min_value = 10;
	w->reservedChars = 3;
	w->displayWidget.justify=justify_right;

	//--------------------------------------------------------------------------------------------------------
	// Edit sleep screen
	sc = oled_addScreen(screen_edit_sleep);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Sleep Time:");
	w->posX = 0;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 1;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getSleepTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setSleepTime;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "s");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Sleep Temp:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getSleepTemp;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setSleepTemp;
	w->reservedChars = 4;
	w->editable.max_value = 480;
	w->editable.min_value = 100;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "C");
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "StdBy Time:");
	w->posX = 0;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 16;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getStandByTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 2;
	w->editable.setData = (void (*)(void *))&setStandByTime;
	w->reservedChars = 4;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "s");
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 110;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &returnSettingsScreen;


	//---------------------------------------------------------------------------------------------
	// Edit boost screen
	sc = oled_addScreen(screen_edit_boost);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Boost Time:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getBoostTime;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setBoostTime;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "s");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Boost Temp:");
	w->posX = 0;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getBoostTemp;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setBoostTemp;
	w->reservedChars = 4;
	w->editable.max_value = 480;
	w->editable.min_value = 100;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "C");
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 110;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &returnSettingsScreen;
/*
	//---------------------------------------------------------------------------------------------
	// Advanced screen
	sc = oled_addScreen(screen_advanced);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_screen_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	advancedSettings = w;
	widgetDefaultsInit(w, widget_combo);
	w->posY = 5;
	w->posX = 0;
	w->font_size = &FONT_10X16_reduced;
	comboAddItem(w, "PWM", screen_adv_pwm);
	comboAddItem(w, "TIP SENSING", screen_adv_tip);
	comboAddItem(w, "MISC", screen_adv_misc);
	comboAddItem(w, "EXIT", screen_settings);
	sc->current_widget = advancedSettings;


	// PWM Screen
	sc = oled_addScreen(screen_adv_pwm);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "PWM Period:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getPwmPeriod;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setPwmPeriod;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "uS");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "PWM Delay:");
	w->posX = 0;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getPwmDelay;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setPwmDelay;
	w->reservedChars = 4;
	w->editable.max_value = 480;
	w->editable.min_value = 100;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "uS");
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &ReturnAdvScreen;


	// Iron detection Screen
	sc = oled_addScreen(screen_adv_tip);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "NoIronADC:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getNoIronADC;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setNoIronADC;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "s");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Resume Delay:");
	w->posX = 0;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getNoIronDelay;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setNoIronDelay;
	w->reservedChars = 4;
	w->editable.max_value = 480;
	w->editable.min_value = 100;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "C");
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &ReturnAdvScreen;


	// MISC Screen
	systemSettings.guiUpdateDelay=200;
	systemSettings.tempUnit=Unit_Celsius;
	systemSettings.saveSettingsDelay=10;
	sc = oled_addScreen(screen_adv_misc);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Save Delay:");
	w->posX = 0;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 0;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getSavDelay;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setSavDelay;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "mS");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "GuiUpdate:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 14;
	//
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 16;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getGuiUpd;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setGuiUpd;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	strcpy(w->endString, "mS");
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "Temp Unit:");
	w->posX = 0;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->reservedChars = 3;



	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 90;
	w->posY = 31;
	w->font_size = &FONT_8X14_reduced;
	w->editable.inputData.getData = &getTmpUnit;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_string;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.selectable.tab = 1;
	w->editable.setData = (void (*)(void *))&setTmpUnit;
	w->reservedChars = 4;
	w->editable.max_value = Unit_Celsius;
	w->editable.min_value = Unit_Farenheit;
	w->displayWidget.justify =justify_right;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "OK");
	w->reservedChars = 2;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &ReturnAdvScreen;
	*/
	//-------------------------------------------------------------------------------------------------------------
	// Edit iron tips screen
	sc = oled_addScreen(screen_edit_iron_tips);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_screen_init;
	sc->update = &default_screenUpdate;

	w = screen_addWidget(sc);
	tipCombo = w;
	widgetDefaultsInit(w, widget_combo);
	w->posY = 5;
	w->posX = 0;
	w->font_size = &FONT_10X16_reduced;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		char *t = malloc(sizeof(systemSettings.ironTips[0].name)/sizeof(systemSettings.ironTips[0].name[0]));
		t[0] = '\0';
		if(!t)
		   Error_Handler();
		comboAddItem(w, t, screen_edit_tip_name);
	}
	addNewTipComboItem = comboAddItem(w, "ADD NEW", screen_edit_tip_name);
	comboAddItem(w, "EXIT", screen_settings);
	sc->current_widget = tipCombo;

	//Screen edit iron tip
	sc = oled_addScreen(screen_edit_tip_name);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;
	w = screen_addWidget(sc);

	widgetDefaultsInit(w, widget_label);
	strcpy(w->displayString, "NAME:");
	w->posX = 0;
	w->posY = 16;
	w->font_size = &FONT_10X16_reduced;
	w->reservedChars = 5;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 50;
	w->posY = 16;
	w->font_size = &FONT_10X16_reduced;
	w->editable.inputData.getData = &getTipStr;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_string;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setTipStr;
	w->editable.max_value = 9999;
	w->reservedChars = 4;

	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 1;
	w->posY = 50;
	strcpy(w->displayString, "SAVE");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveTip;
	w = screen_addWidget(sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "BACK");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelTip;

	w = screen_addWidget(sc);
	delTipButton = w;
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 40;
	w->posY = 50;
	strcpy(w->displayString, "DELETE");
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 2;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             	w->buttonWidget.action = &delTip;
}
