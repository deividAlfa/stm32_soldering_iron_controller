/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "settings_screen.h"
#include "oled.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static char t[sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])][sizeof(systemSettings.ironTips[0].name)];
static uint16_t temp;
char str[5];
static char *OffOn[] = {"OFF", " ON" };
static char *tempUnit[] = {"*C", "*F" };
static char *InitMode[] = {"SBY","SLP"," ON", "BST"};
bool TempUnit;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets
//-------------------------------------------------------------------------------------------------------------------------------


// SETTINGS SCREEM
static widget_t comboWidget_Settings;
static comboBox_item_t Settings_Combo_PID;
static comboBox_item_t Settings_Combo_IRON;
static comboBox_item_t Settings_Combo_SYSTEM;
static comboBox_item_t Settings_Combo_ADVANCED;
static comboBox_item_t Settings_Combo_TIPTYPE;
static comboBox_item_t Settings_Combo_TIPS;
static comboBox_item_t Settings_Combo_CALIBRATION;
static comboBox_item_t Settings_Combo_DEBUG;
static comboBox_item_t Settings_Combo_EXIT;

//PID
static widget_t comboWidget_Settings_PID;
static comboBox_item_t comboitem_PID_KP;
static comboBox_item_t comboitem_PID_KI;
static comboBox_item_t comboitem_PID_KD;
static comboBox_item_t comboitem_PID_Back;
static widget_t Widget_PID_Kd;
static widget_t Widget_PID_Ki;
static widget_t Widget_PID_Kp;

//IRON
static widget_t comboWidget_Settings_IRON;
static comboBox_item_t comboitem_IRON_BoostTemp;
static comboBox_item_t comboitem_IRON_BoostTime;
static comboBox_item_t comboitem_IRON_StbyTime;
static comboBox_item_t comboitem_IRON_SleepTemp;
static comboBox_item_t comboitem_IRON_SleepTime;
static comboBox_item_t comboitem_IRON_Power;
static comboBox_item_t comboitem_IRON_Back;
static widget_t Widget_IRON_BoostTemp;
static widget_t Widget_IRON_BoostTime;
static widget_t Widget_IRON_StbyTime;
static widget_t Widget_IRON_SleepTemp;
static widget_t Widget_IRON_SleepTime;
static widget_t Widget_IRON_Power;

//SYSTEM
static widget_t comboWidget_Settings_SYSTEM;
static comboBox_item_t comboitem_SYSTEM_Contrast;
static comboBox_item_t comboitem_SYSTEM_OledFix;
static comboBox_item_t comboitem_SYSTEM_TempUnit;
static comboBox_item_t comboitem_SYSTEM_GuiUpd;
static comboBox_item_t comboitem_SYSTEM_SaveInterval;
static comboBox_item_t comboitem_SYSTEM_Buzzer;
static comboBox_item_t comboitem_SYSTEM_InitMode;
static comboBox_item_t comboitem_SYSTEM_EncWake;
static comboBox_item_t comboitem_SYSTEM_Back;
static widget_t Widget_SYSTEM_Contrast;
static widget_t Widget_SYSTEM_OledFix;
static widget_t Widget_SYSTEM_TempUnit;
static widget_t Widget_SYSTEM_GuiUpd;
static widget_t Widget_SYSTEM_SaveInterval;
static widget_t Widget_SYSTEM_Buzzer;
static widget_t Widget_SYSTEM_InitMode;
static widget_t Widget_SYSTEM_EncWake;

//ADVANCED
static widget_t comboWidget_Settings_ADVANCED;
static comboBox_item_t comboitem_ADVANCED_NoIronDelay;
static comboBox_item_t comboitem_ADVANCED_ADCLimit;
static comboBox_item_t comboitem_ADVANCED_ADCDelay;
static comboBox_item_t comboitem_ADVANCED_PWMPeriod;
static comboBox_item_t comboitem_ADVANCED_Reset;
static comboBox_item_t comboitem_ADVANCED_Back;
static widget_t Widget_ADVANCED_NoIronDelay;
static widget_t Widget_ADVANCED_ADCLimit;
static widget_t Widget_ADVANCED_ADCDelay;
static widget_t Widget_ADVANCED_PWMPeriod;

// EDIT TIPS SCREEN
static widget_t comboWidget_Settings_IRONTIPS;
static comboBox_item_t comboitem_IRONTIPS[ sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]) ];
static comboBox_item_t comboitem_IRONTIPS_addNewTip;
static comboBox_item_t comboitem_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Save;
static widget_t Widget_IRONTIPS_Delete;
static widget_t Widget_IRONTIPS_Edit;

// RESET SCREEN
static widget_t Widget_Reset_OK;
static widget_t Widget_Reset_CANCEL;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------
static void edit_iron_tip_screen_init(screen_t *scr) {
	if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "    ");
		Widget_IRONTIPS_Delete.enabled = 0;
	}
	else {
		strcpy(str, comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text);
		if(systemSettings.currentNumberOfTips>1){
			Widget_IRONTIPS_Delete.enabled = 1;
		}
		else{
			Widget_IRONTIPS_Delete.enabled = 0;
		}
	}
	default_init(scr);
}
static void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i =comboWidget_Settings_IRONTIPS.comboBoxWidget.items;
	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		if(x < systemSettings.currentNumberOfTips) {
			strcpy(i->text, systemSettings.ironTips[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem = comboWidget_Settings_IRONTIPS.comboBoxWidget.items;
	comboWidget_Settings_IRONTIPS.comboBoxWidget.currentScroll = 0;
	if(systemSettings.currentNumberOfTips >= sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0])) {
		comboitem_IRONTIPS_addNewTip.enabled = 0;
	}

	else{
		comboitem_IRONTIPS_addNewTip.enabled = 1;
	}
}
static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(systemSettings.ironTips[systemSettings.currentNumberOfTips].name, str);
		++systemSettings.currentNumberOfTips;
	}
	else {
		for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++ x) {
			if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
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
		if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.ironTips[x].name) == 0) {
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
	temp = currentPID.max * 100;
	return &temp;
}
static void setMaxPower(uint16_t *val) {
	currentPID.max = (double)*val / 100.0;
	systemSettings.ironTips[systemSettings.currentTip].PID.max=currentPID.max;
	setupPIDFromStruct();
}

static void * getContrast_() {
	temp = systemSettings.contrast;
	return &temp;
}
static void setContrast_(uint16_t *val) {
	systemSettings.contrast=*val;
	setContrast(*val);
}

static void * getBoostTime() {
	temp = systemSettings.boost.Time;
	return &temp;
}
static void setBoostTime(uint16_t *val) {
	systemSettings.boost.Time = *val;
}
static void * getBoostTemp() {
	temp = systemSettings.boost.Temperature;
	return &temp;

}
static void setBoostTemp(uint16_t *val) {
	systemSettings.boost.Temperature = *val;
}

static void setSleepTime(uint16_t *val) {
	systemSettings.sleep.Time = *val;
}

static void * getSleepTime() {
	temp = systemSettings.sleep.Time;
	return &temp;
}
static void setStandByTime(uint16_t *val) {
	systemSettings.standby.Time  = *val;
}
static void * getStandByTime() {
	temp = systemSettings.standby.Time;
	return &temp;
}
static void setSleepTemp(uint16_t *val) {
	systemSettings.sleep.Temperature = *val;
}
static void * getSleepTemp() {
	temp = systemSettings.sleep.Temperature;
	return &temp;
}

static void * getKp() {
	temp = currentPID.Kp * 1000000;
	return &temp;
}
static void setKp(uint16_t *val) {
	currentPID.Kp = (double)*val / 1000000;
	systemSettings.ironTips[systemSettings.currentTip].PID.Kp=currentPID.Kp ;
	setupPIDFromStruct();
}
static void * getKi() {
	temp = currentPID.Ki * 1000000;
	return &temp;
}
static void setKi(uint16_t *val) {
	currentPID.Ki = (double)*val / 1000000;
	systemSettings.ironTips[systemSettings.currentTip].PID.Ki=currentPID.Ki;
	setupPIDFromStruct();
}
static void * getKd() {
	temp = currentPID.Kd * 1000000;
	return &temp;
}
static void setKd(uint16_t *val) {
	currentPID.Kd = (double)*val / 1000000;
	systemSettings.ironTips[systemSettings.currentTip].PID.Kd=currentPID.Kd;
	setupPIDFromStruct();
}

static void * getPwmPeriod() {
	temp=(systemSettings.pwmPeriod+1)/100;
	if(temp<501){
		Widget_ADVANCED_ADCDelay.editable.max_value = temp-1;
	}
	else{
		Widget_ADVANCED_ADCDelay.editable.max_value = 500;
	}
	return &temp;
}
static void setPwmPeriod(uint16_t *val) {
	systemSettings.pwmPeriod = (*val*100)-1;
	ApplyPwmSettings();
}
static void * getPwmDelay() {
	temp=(systemSettings.pwmDelay+1)/100;


	return &temp;
}
static void setPwmDelay(uint16_t *val) {
	systemSettings.pwmDelay=(*val*100)-1;
	ApplyPwmSettings();
}

static void * getTmpUnit() {
	temp = systemSettings.tempUnit;
	return &temp;
}
static void setTmpUnit(uint16_t *val) {
	setTempUnit(*val);
}
static void * getOledFix() {
	temp = systemSettings.OledFix;
	return &temp;
}
static void setOledFix(uint16_t *val) {
	systemSettings.OledFix= * val;
}

static void * getGuiUpd_ms() {
	temp = systemSettings.guiUpdateDelay;
	return &temp;
}
static void setGuiUpd_ms(uint16_t *val) {
	systemSettings.guiUpdateDelay = *val;

}

static void * getSavDelay() {
	temp =systemSettings.saveSettingsDelay;
	return &temp;
}
static void setSavDelay(uint16_t *val) {
	systemSettings.saveSettingsDelay = *val;
}

static void * getNoIronDelay() {
	temp = systemSettings.noIronDelay;
	return &temp;
}
static void setNoIronDelay(uint16_t *val) {
	systemSettings.noIronDelay = *val;
}

static void * getNoIronADC() {
	temp = systemSettings.noIronValue;
	return &temp;
}
static void setNoIronADC(uint16_t *val) {
	systemSettings.noIronValue = *val;

}
static void * getBuzzEnable() {
	temp = systemSettings.buzzEnable;
	return &temp;
}
static void setBuzzEnable(uint16_t *val) {
	systemSettings.buzzEnable = *val;
}
static void * getInitMode() {
	temp = systemSettings.initMode;
	return &temp;
}
static void setInitMode(uint16_t *val) {
	systemSettings.initMode = *val;
}
static int cancelReset(widget_t *w) {
	return screen_advanced;
}
static int doReset(widget_t *w) {
	resetSettings();
	saveSettings();
	HAL_Delay(500);
	NVIC_SystemReset();
}
static void setEncWake(uint16_t *val) {
	systemSettings.wakeEncoder = *val;
}
static void * getEncWake() {
	temp = systemSettings.wakeEncoder;
	return &temp;
}

static void tempUnitChanged(void) {
	TempUnit = systemSettings.tempUnit;
	if(TempUnit==Unit_Farenheit){
		Widget_IRON_BoostTemp.endString="*F";
		Widget_IRON_BoostTemp.editable.max_value=900;
		Widget_IRON_BoostTemp.editable.min_value=400;
		Widget_IRON_BoostTemp.editable.big_step = 50;
		Widget_IRON_BoostTemp.editable.step = 10;

		Widget_IRON_SleepTemp.endString="*F";
		Widget_IRON_SleepTemp.editable.max_value=750;
		Widget_IRON_SleepTemp.editable.min_value=300;
		Widget_IRON_SleepTemp.editable.big_step = 50;
		Widget_IRON_SleepTemp.editable.step = 10;
	}
	else{
		Widget_IRON_BoostTemp.endString="*C";
		Widget_IRON_BoostTemp.editable.max_value=480;
		Widget_IRON_BoostTemp.editable.min_value=200;
		Widget_IRON_BoostTemp.editable.big_step = 50;
		Widget_IRON_BoostTemp.editable.step = 10;

		Widget_IRON_SleepTemp.endString="*C";
		Widget_IRON_SleepTemp.editable.max_value=400;
		Widget_IRON_SleepTemp.editable.min_value=150;
		Widget_IRON_SleepTemp.editable.big_step = 50;
		Widget_IRON_SleepTemp.editable.step = 10;
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void settings_screenUpdate(screen_t *scr) {
	if(TempUnit != systemSettings.tempUnit){
		tempUnitChanged();
	}
	default_screenUpdate(scr);
}

static void settings_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
	tempUnitChanged();
	scr->current_widget = &comboWidget_Settings;
	scr->current_widget->comboBoxWidget.selectable.state = widget_selected;
}
int settings_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	if(input==LongClick){
		return screen_debug;
	}
	return (default_screenProcessInput(scr, input, state));
}

static void settings_screen_exit(screen_t *scr) {
	//comboResetIndex(&Widget_Settings_combo);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Edit Tip screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void edittipname_screenDraw(screen_t *scr){
	static bool prevState;
	if(strcmp(str, "    ") == 0) {
		if(!prevState){
			prevState=1;
			Widget_IRONTIPS_Save.enabled=0;
			FillBuffer(C_BLACK,fill_dma);
		}
	}
	else{
		if(prevState){
			prevState=0;
			Widget_IRONTIPS_Save.enabled=1;
			FillBuffer(C_BLACK,fill_dma);
		}
	}
	UG_FontSelect(&FONT_10X16_reduced);
	UG_PutString(0,17,"NAME:");//12
	default_screenDraw(scr);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Reset screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void Reset_onEnter(screen_t *scr){
	UG_FontSelect(&FONT_8X14_reduced);
	UG_PutString(0,17,"RESET SETTINGS?");//12
}

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void settings_screen_setup(screen_t *scr) {
	screen_t* sc;
	widget_t* w;

	scr->draw = &default_screenDraw;
	scr->processInput = &settings_screenProcessInput;
	scr->init = &settings_screen_init;
	scr->update = &settings_screenUpdate;
	scr->onExit = &settings_screen_exit;


	//#################################### [ SETTINGS MAIN SCREEN ]#######################################
	//
	w = &comboWidget_Settings;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 0;
	w->posX = 0;
	w->font_size = &FONT_8X14_reduced;
	comboAddScreen(&Settings_Combo_PID, w, 			"PID", 			screen_pid);
	comboAddScreen(&Settings_Combo_IRON, w, 		"IRON", 		screen_iron);
	comboAddScreen(&Settings_Combo_SYSTEM, w, 		"SYSTEM", 		screen_system);
	comboAddScreen(&Settings_Combo_ADVANCED, w, 	"ADVANCED", 	screen_advanced);
	comboAddScreen(&Settings_Combo_TIPTYPE, w, 		"IRON TYPE", 	screen_tiptype);
	comboAddScreen(&Settings_Combo_TIPS, w, 		"EDIT TIPS", 	screen_edit_iron_tips);
	comboAddScreen(&Settings_Combo_CALIBRATION, w, 	"CALIBRATION", 	screen_edit_calibration_wait);
	comboAddScreen(&Settings_Combo_DEBUG, w, 		"DEBUG", 		screen_debug);
	comboAddScreen(&Settings_Combo_EXIT, w, 		"EXIT", 		screen_main);


	//########################################## [ PID SCREEN ] ##########################################
	//
	sc=&Screen_pid;
	oled_addScreen(sc,screen_pid);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	//********[ KP Widget]***********************************************************
	//
	w = &Widget_PID_Kp;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->editable.inputData.getData = &getKp;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKp;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	//********[ KI Widget ]***********************************************************
	//
	w = &Widget_PID_Ki;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->editable.inputData.getData = &getKi;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKi;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	//********[ KD Widget ]***********************************************************
	//
	w = &Widget_PID_Kd;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 78;
	w->editable.inputData.getData = &getKd;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKd;
	w->reservedChars = 6;
	w->displayWidget.justify = justify_right;

	//========[ PID COMBO ]===========================================================
	//
	w = &comboWidget_Settings_PID;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 0;
	w->posX = 0;
	w->font_size = &FONT_8X14_reduced;
	comboAddOption(&comboitem_PID_KP, w, 	"Kp", 	&Widget_PID_Kp);
	comboAddOption(&comboitem_PID_KI, w, 	"Ki", 	&Widget_PID_Ki);
	comboAddOption(&comboitem_PID_KD, w, 	"Kd", 	&Widget_PID_Kd);
	comboAddScreen(&comboitem_PID_Back, w, 	"BACK", screen_settingsmenu);



	//########################################## IRON SCREEN ##########################################
	//
	sc=&Screen_iron;
	oled_addScreen(sc,screen_iron);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	//********[ Sleep Time Widget ]***********************************************************
	//
	w = &Widget_IRON_SleepTime;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->editable.inputData.getData = &getSleepTime;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setSleepTime;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	w->endString="m";
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	//********[ Sleep Temp Widget ]***********************************************************
	//
	w = &Widget_IRON_SleepTemp;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;//94
	w->editable.inputData.getData = &getSleepTemp;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setSleepTemp;
	w->reservedChars = 5;
	w->displayWidget.hasEndStr = 1;
	w->displayWidget.justify =justify_right;

	//********[ Boost Time Widget ]***********************************************************
	//
	w = &Widget_IRON_BoostTime;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->editable.inputData.getData = &getBoostTime;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setBoostTime;
	w->editable.max_value = 600;
	w->editable.min_value = 5;
	w->displayWidget.hasEndStr = 1;
	w->endString="m";
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	//********[ Boost Temp Widget ]***********************************************************
	//
	w = &Widget_IRON_BoostTemp;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->editable.inputData.getData = &getBoostTemp;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setBoostTemp;
	w->reservedChars = 5;
	w->displayWidget.hasEndStr = 1;
	w->displayWidget.justify =justify_right;

	//********[ Standby Time Widget ]***********************************************************
	//
	w = &Widget_IRON_StbyTime;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->editable.inputData.getData = &getStandByTime;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setStandByTime;
	w->reservedChars = 4;
	w->editable.max_value = 600;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	w->endString="m";
	w->displayWidget.justify =justify_right;

	//********[ Power Widget ]***********************************************************
	//
	w = &Widget_IRON_Power;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->editable.inputData.getData = &getMaxPower;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 5;
	w->editable.setData = (void (*)(void *))&setMaxPower;
	w->editable.max_value = 100;
	w->editable.min_value = 5;
	w->displayWidget.hasEndStr = 1;
	w->endString="%";
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	//========[ IRON COMBO ]===========================================================
	//
	w = &comboWidget_Settings_IRON;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 0;
	w->posX = 0;
	w->font_size = &FONT_8X14_reduced;
	comboAddOption(&comboitem_IRON_SleepTime, w,	"Slp Time", 	&Widget_IRON_SleepTime);
	comboAddOption(&comboitem_IRON_SleepTemp, w, 	"Slp Temp", 	&Widget_IRON_SleepTemp);
	comboAddOption(&comboitem_IRON_BoostTime, w, 	"Bst Time", 	&Widget_IRON_BoostTime);
	comboAddOption(&comboitem_IRON_BoostTemp, w, 	"Bst Temp", 	&Widget_IRON_BoostTemp);
	comboAddOption(&comboitem_IRON_StbyTime, w, 	"Sby Time", 	&Widget_IRON_StbyTime);
	comboAddOption(&comboitem_IRON_Power, w, 		"Max Pwr",		&Widget_IRON_Power);
	comboAddScreen(&comboitem_IRON_Back, w, 		"BACK", 		screen_settingsmenu);



	//########################################## SYSTEM SCREEN ##########################################
	//
	sc=&Screen_system;
	oled_addScreen(sc,screen_system);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	//********[ Contrast Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Contrast;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 102;
	w->editable.inputData.getData = &getContrast_;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 20;
	w->editable.step = 5;
	w->editable.setData = (void (*)(void *))&setContrast_;
	w->editable.max_value = 255;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 0;
	w->reservedChars = 3;

	//********[ Oled Fix Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_OledFix;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 102;
	w->editable.inputData.getData = &getOledFix;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setOledFix;
	w->displayWidget.hasEndStr = 0;
	w->reservedChars = 3;
	w->multiOptionWidget.options = OffOn;
	w->multiOptionWidget.numberOfOptions = 2;
	w->editable.max_value = 1;
	w->editable.min_value = 0;

	//********[ Save Delay Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_SaveInterval;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 102;
	w->editable.inputData.getData = &getSavDelay;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setSavDelay;
	w->editable.max_value = 60;
	w->editable.min_value = 1;
	w->displayWidget.hasEndStr = 1;
	w->endString="s";
	w->reservedChars = 3;
	w->displayWidget.justify =justify_right;

	//********[ Gui refresh rate Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_GuiUpd;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->editable.inputData.getData = &getGuiUpd_ms;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setGuiUpd_ms;
	w->editable.max_value = 500;
	w->editable.min_value = 0;
	w->displayWidget.hasEndStr = 1;
	w->endString="mS";
	w->reservedChars = 5;
	w->displayWidget.justify =justify_right;

	//********[ Temp display unit Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_TempUnit;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 110;
	w->editable.inputData.getData = &getTmpUnit;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setTmpUnit;
	w->reservedChars = 2;
	w->editable.max_value = Unit_Farenheit;
	w->editable.min_value = Unit_Celsius;
	w->multiOptionWidget.options = tempUnit;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Buzzer Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Buzzer;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 102;
	w->editable.inputData.getData = &getBuzzEnable;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setBuzzEnable;
	w->reservedChars = 3;
	w->editable.max_value = 1;
	w->editable.min_value = 0;
	w->multiOptionWidget.options = OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Init mode Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_InitMode;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 102;
	w->editable.inputData.getData = &getInitMode;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setInitMode;
	w->reservedChars = 3;
	w->editable.max_value = mode_boost;
	w->editable.min_value = mode_standby;
	w->multiOptionWidget.options = InitMode;
	w->multiOptionWidget.numberOfOptions = 4;

	//********[ Encoder wake Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_EncWake;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 102;
	w->editable.inputData.getData = &getEncWake;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setEncWake;
	w->reservedChars = 3;
	w->editable.max_value = 1;
	w->editable.min_value = 0;
	w->multiOptionWidget.options =OffOn;
	w->multiOptionWidget.numberOfOptions = 4;

	//========[ SYSTEM COMBO ]===========================================================
	//
	w = &comboWidget_Settings_SYSTEM;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 0;
	w->posX = 0;
	w->font_size = &FONT_8X14_reduced;
	comboAddOption(&comboitem_SYSTEM_Contrast,w, 		"Contrast", 	&Widget_SYSTEM_Contrast);
	comboAddOption(&comboitem_SYSTEM_OledFix, w, 		"Oled Fix", 	&Widget_SYSTEM_OledFix);
	comboAddOption(&comboitem_SYSTEM_InitMode, w, 		"Init Mode", 	&Widget_SYSTEM_InitMode);
	comboAddOption(&comboitem_SYSTEM_EncWake, w, 		"Enc. Wake", 	&Widget_SYSTEM_EncWake);
	comboAddOption(&comboitem_SYSTEM_Buzzer, w, 		"Buzzer", 		&Widget_SYSTEM_Buzzer);
	comboAddOption(&comboitem_SYSTEM_TempUnit, w, 		"Temp Unit", 	&Widget_SYSTEM_TempUnit);
	comboAddOption(&comboitem_SYSTEM_GuiUpd, w, 		"Gui Upd", 		&Widget_SYSTEM_GuiUpd);
	comboAddOption(&comboitem_SYSTEM_SaveInterval, w, 	"Save Delay",	&Widget_SYSTEM_SaveInterval);
	comboAddScreen(&comboitem_SYSTEM_Back, w, 			"BACK", 		screen_settingsmenu);



	//########################################## ADVANCED SCREEN ##########################################
	//
	sc=&Screen_advanced;
	oled_addScreen(sc,screen_advanced);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;

	//********[ PWM Period Widget ]***********************************************************
	w = &Widget_ADVANCED_PWMPeriod;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->editable.inputData.getData = &getPwmPeriod;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setPwmPeriod;
	w->editable.max_value = 650;
	w->editable.min_value = 20;
	w->displayWidget.hasEndStr = 1;
	w->endString="mS";
	w->reservedChars = 5;
	w->displayWidget.justify =justify_right;

	//********[ ADC Delay Widget ]***********************************************************
	w = &Widget_ADVANCED_ADCDelay;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->editable.inputData.getData = &getPwmDelay;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setPwmDelay;
	w->reservedChars = 5;
	w->editable.max_value = 500;
	w->editable.min_value = 1;
	w->displayWidget.hasEndStr = 1;
	w->endString="mS";
	w->displayWidget.justify =justify_right;

	//********[ ADC Limit Widget ]***********************************************************
	//
	w = &Widget_ADVANCED_ADCLimit;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 94;
	w->editable.inputData.getData = &getNoIronADC;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 200;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setNoIronADC;
	w->editable.max_value = 4100;
	w->editable.min_value = 200;
	w->reservedChars = 4;
	w->displayWidget.justify =justify_right;

	//********[ No Iron Delay Widget ]***********************************************************
	//
	w = &Widget_ADVANCED_NoIronDelay;
	widgetDefaultsInit(w, widget_editable);
	w->posX = 86;
	w->editable.inputData.getData = &getNoIronDelay;
	w->editable.inputData.type = field_uinteger16;
	w->editable.big_step = 100;
	w->editable.step = 50;
	w->editable.setData = (void (*)(void *))&setNoIronDelay;
	w->reservedChars = 5;
	w->editable.max_value = 1000;
	w->editable.min_value = 100;
	w->displayWidget.hasEndStr = 1;
	w->endString="mS";
	w->displayWidget.justify =justify_right;

	//========[ ADVANCED COMBO ]===========================================================
	//
	w = &comboWidget_Settings_ADVANCED;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 0;
	w->posX = 0;
	w->font_size = &FONT_8X14_reduced;
	comboAddOption(&comboitem_ADVANCED_PWMPeriod,w, 		"PWM Time", 		&Widget_ADVANCED_PWMPeriod);
	comboAddOption(&comboitem_ADVANCED_ADCDelay, w, 		"ADC Delay", 		&Widget_ADVANCED_ADCDelay);
	comboAddOption(&comboitem_ADVANCED_ADCLimit, w, 		"ADC Limit", 		&Widget_ADVANCED_ADCLimit);
	comboAddOption(&comboitem_ADVANCED_NoIronDelay, w, 		"Det. Delay",		&Widget_ADVANCED_NoIronDelay);
	comboAddScreen(&comboitem_ADVANCED_Reset, w, 			"Reset Settings",	screen_reset);
	comboAddScreen(&comboitem_ADVANCED_Back, w, 			"BACK", 			screen_settingsmenu);

	//########################################## RESET SCREEN ##########################################
	//
	sc=&Screen_reset;
	oled_addScreen(sc, screen_reset);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &default_init;
	sc->update = &default_screenUpdate;
	sc->onEnter = &Reset_onEnter;

	//********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_Reset_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 2;
	w->posY = 50;
	strcpy(w->displayString, "RESET");
	w->reservedChars = 5;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &doReset;

	//********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_Reset_CANCEL;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 78;
	w->posY = 50;
	strcpy(w->displayString, "CANCEL");
	w->reservedChars = 6;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelReset;


	//########################################## IRON TIPS SCREEN ##########################################
	//
	sc=&Screen_edit_iron_tips;
	oled_addScreen(sc, screen_edit_iron_tips);
	sc->draw = &default_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_screen_init;
	sc->update = &default_screenUpdate;




	//========[ IRON TIPS COMBO ]===========================================================
	//
	w = &comboWidget_Settings_IRONTIPS;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_combo);
	w->posY = 5;
	w->posX = 0;
	w->font_size = &FONT_10X16_reduced;

	for(int x = 0; x < sizeof(systemSettings.ironTips) / sizeof(systemSettings.ironTips[0]); ++x) {
		t[x][0] = '\0';
		comboAddScreen(&comboitem_IRONTIPS[x],w, &t[x][0], screen_edit_tip_name);
	}
	comboAddScreen(&comboitem_IRONTIPS_addNewTip, w, "ADD NEW", screen_edit_tip_name);
	comboAddScreen(&comboitem_IRONTIPS_Back, w, "BACK", screen_settingsmenu);
	sc->current_widget = w;



	//########################################## IRON TIPS EDIT SCREEN ##########################################
	//
	sc=&Screen_edit_tip_name;
	oled_addScreen(sc, screen_edit_tip_name);
	sc->draw = &edittipname_screenDraw;
	sc->processInput = &default_screenProcessInput;
	sc->init = &edit_iron_tip_screen_init;
	sc->update = &default_screenUpdate;

	//********[ Name Edit Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Edit;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_editable);
	w->posX = 50;
	w->posY = 17;
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

	//********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Save;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 2;
	w->posY = 50;
	strcpy(w->displayString, "SAVE");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveTip;

	//********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Back;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 94;
	w->posY = 50;
	strcpy(w->displayString, "BACK");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelTip;

	//********[ Name Delete Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Delete;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->font_size = &FONT_8X14_reduced;
	w->posX = 48;
	w->posY = 50;
	strcpy(w->displayString, "DEL.");
	w->reservedChars = 4;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &delTip;
	w->buttonWidget.action = &delTip;

}
