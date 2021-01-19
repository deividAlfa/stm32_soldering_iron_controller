/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "oled.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static char t[TipSize][TipCharSize];
static uint16_t temp;
static uint8_t resStatus, profile;
char str[5];
static char *OffOn[] = {"OFF", " ON" };
static char *tempUnit[] = {"*C", "*F" };
static char *wakeMode[] = {"SHAKE", "STAND" };
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
static comboBox_item_t comboitem_SYSTEM_Profile;
static comboBox_item_t comboitem_SYSTEM_Contrast;
static comboBox_item_t comboitem_SYSTEM_OledOffset;
static comboBox_item_t comboitem_SYSTEM_WakeMode;
static comboBox_item_t comboitem_SYSTEM_EncoderMode;
static comboBox_item_t comboitem_SYSTEM_TempUnit;
static comboBox_item_t comboitem_SYSTEM_TempStep;
static comboBox_item_t comboitem_SYSTEM_GuiUpd;
static comboBox_item_t comboitem_SYSTEM_SaveInterval;
static comboBox_item_t comboitem_SYSTEM_Buzzer;
static comboBox_item_t comboitem_SYSTEM_InitMode;
static comboBox_item_t comboitem_SYSTEM_ButtonWake;
static comboBox_item_t comboitem_SYSTEM_Back;
static widget_t Widget_SYSTEM_Profile;
static widget_t Widget_SYSTEM_Contrast;
static widget_t Widget_SYSTEM_OledOffset;
static widget_t Widget_SYSTEM_WakeMode;
static widget_t Widget_SYSTEM_EncoderMode;
static widget_t Widget_SYSTEM_TempUnit;
static widget_t Widget_SYSTEM_TempStep;
static widget_t Widget_SYSTEM_GuiUpd;
static widget_t Widget_SYSTEM_SaveInterval;
static widget_t Widget_SYSTEM_Buzzer;
static widget_t Widget_SYSTEM_InitMode;
static widget_t Widget_SYSTEM_ButtonWake;

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
static comboBox_item_t comboitem_IRONTIPS[TipSize];
static comboBox_item_t comboitem_IRONTIPS_addNewTip;
static comboBox_item_t comboitem_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Save;
static widget_t Widget_IRONTIPS_Delete;
static widget_t Widget_IRONTIPS_Edit;

// RESET SCREEN

static widget_t comboWidget_Settings_RESET;
static comboBox_item_t comboitem_RESET_SETTINGS;
static comboBox_item_t comboitem_RESET_TIP;
static comboBox_item_t comboitem_RESET_ALLTIP;
static comboBox_item_t comboitem_RESET_EVERYTHING;
static comboBox_item_t comboitem_RESET_Back;
static widget_t Widget_Reset_OK;
static widget_t Widget_Reset_CANCEL;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------

static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(systemSettings.Profile.tip[systemSettings.Profile.currentNumberOfTips].name, str);
		++systemSettings.Profile.currentNumberOfTips;
	}
	else {
		for(int x = 0; x < TipSize;x++) {
			if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.Profile.tip[x].name) == 0 ) {
				strcpy(systemSettings.Profile.tip[x].name, str);
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
	for(int x = 0; x < TipSize; x++) {
		if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.Profile.tip[x].name) == 0) {
			itemIndex = x;
			break;
		}
	}
	for(int x = itemIndex; x < TipSize;x++) {
		systemSettings.Profile.tip[x] = systemSettings.Profile.tip[x+1];
	}
	--systemSettings.Profile.currentNumberOfTips;
	return screen_edit_iron_tips;
}

static void * getMaxPower() {
	temp = currentPID.max * 100;
	return &temp;
}
static void setMaxPower(uint16_t *val) {
	currentPID.max = (float)*val / 100.0;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.max=currentPID.max;
	setupPIDFromStruct();
}

static void * getContrast_() {
	temp = systemSettings.settings.contrast;
	return &temp;
}
static void setContrast_(uint8_t *val) {
	systemSettings.settings.contrast=*val;
	setContrast(*val);
}

static void * getBoostTime() {
	temp = systemSettings.Profile.boost.Time;
	return &temp;
}
static void setBoostTime(uint8_t *val) {
	systemSettings.Profile.boost.Time = *val;
}
static void * getBoostTemp() {
	temp = systemSettings.Profile.boost.Temperature;
	return &temp;

}
static void setBoostTemp(uint16_t *val) {
	systemSettings.Profile.boost.Temperature = *val;
}

static void setSleepTime(uint8_t *val) {
	systemSettings.Profile.sleep.Time = *val;
}

static void * getSleepTime() {
	temp = systemSettings.Profile.sleep.Time;
	return &temp;
}
static void setStandByTime(uint8_t *val) {
	systemSettings.Profile.standby.Time  = *val;
}
static void * getStandByTime() {
	temp = systemSettings.Profile.standby.Time;
	return &temp;
}
static void setSleepTemp(uint16_t *val) {
	systemSettings.Profile.sleep.Temperature = *val;
}
static void * getSleepTemp() {
	temp = systemSettings.Profile.sleep.Temperature;
	return &temp;
}

static void * getKp() {
	temp = currentPID.Kp * 1000000;
	return &temp;
}
static void setKp(uint16_t *val) {
	currentPID.Kp = (float)*val / 1000000;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kp=currentPID.Kp ;
	setupPIDFromStruct();
}
static void * getKi() {
	temp = currentPID.Ki * 1000000;
	return &temp;
}
static void setKi(uint16_t *val) {
	currentPID.Ki = (float)*val / 1000000;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Ki=currentPID.Ki;
	setupPIDFromStruct();
}
static void * getKd() {
	temp = currentPID.Kd * 1000000;
	return &temp;
}
static void setKd(uint16_t *val) {
	currentPID.Kd = (float)*val / 1000000;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kd=currentPID.Kd;
	setupPIDFromStruct();
}

static void * getPWMPeriod() {
	temp=(systemSettings.Profile.pwmPeriod+1)/100;
	return &temp;
}
static void setPWMPeriod(uint16_t *val) {
	uint16_t period=(*val*100)-1;
	if(!setPwmPeriod(period)){
		if(*val<=200){
			Widget_ADVANCED_ADCDelay.editable.max_value = temp-1;
		}
		else{
			Widget_ADVANCED_ADCDelay.editable.max_value = 200;
		}
	}
}

static void * getPWMDelay() {
	temp=(systemSettings.Profile.pwmDelay+1)/100;
	return &temp;
}
static void setPWMDelay(uint16_t *val) {
	uint16_t delay=(*val*100)-1;
	if(!setPwmDelay(delay)){
		if(*val>=20){
			Widget_ADVANCED_PWMPeriod.editable.min_value = temp+1;
		}
		else{
			Widget_ADVANCED_PWMPeriod.editable.min_value=20;
		}
	}
}

static void * getTmpUnit() {
	temp = systemSettings.settings.tempUnit;
	return &temp;
}
static void setTmpUnit(uint16_t *val) {
	switchTempUnit(*val);
}

static void * getTmpStep() {
	temp = systemSettings.settings.tempStep;
	return &temp;
}
static void setTmpStep(uint16_t *val) {
	systemSettings.settings.tempStep = *val;
}

static void * getOledOffset() {
	temp = systemSettings.settings.OledOffset;
	return &temp;
}
static void setOledOffset(uint16_t *val) {
	systemSettings.settings.OledOffset= * val;
}

static void * getWakeMode() {
	temp = systemSettings.settings.WakeInputMode;
	return &temp;
}
static void setWakeMode(uint16_t *val) {
	systemSettings.settings.WakeInputMode= * val;
}

static void * getEncoderMode() {
	temp = systemSettings.settings.EncoderInvert;
	return &temp;
}
static void setEncoderMode(uint16_t *val) {
	systemSettings.settings.EncoderInvert= * val;
}
static void * getGuiUpd_ms() {
	temp = systemSettings.settings.guiUpdateDelay;
	return &temp;
}
static void setGuiUpd_ms(uint16_t *val) {
	systemSettings.settings.guiUpdateDelay = *val;

}

static void * getSavDelay() {
	temp =systemSettings.settings.saveSettingsDelay;
	return &temp;
}
static void setSavDelay(uint16_t *val) {
	systemSettings.settings.saveSettingsDelay = *val;
}

static void * getNoIronDelay() {
	temp = systemSettings.settings.noIronDelay;
	return &temp;
}
static void setNoIronDelay(uint16_t *val) {
	systemSettings.settings.noIronDelay = *val;
}

static void * getNoIronADC() {
	temp = systemSettings.Profile.noIronValue;
	return &temp;
}
static void setNoIronADC(uint16_t *val) {
	systemSettings.Profile.noIronValue = *val;
}
static void * getbuzzerMode() {
	temp = systemSettings.settings.buzzerMode;
	return &temp;
}
static void setbuzzerMode(uint16_t *val) {
	systemSettings.settings.buzzerMode = *val;
}
static void * getInitMode() {
	temp = systemSettings.settings.initMode;
	return &temp;
}
static void setInitMode(uint16_t *val) {
	systemSettings.settings.initMode = *val;
}

static void * getProfile() {
	temp = profile;
	return &temp;
}

static void setProfile(uint16_t *val) {
	profile=*val;
}

static int cancelReset(widget_t *w) {
	return screen_reset;
}
static int doReset(widget_t *w) {
	uint8_t CurrentProfile;
	switch(resStatus){

	case 0:																	// Reset system settings
		CurrentProfile=systemSettings.settings.currentProfile;				// Store current profile
		resetSystemSettings();												// Reset system settings
		systemSettings.settings.currentProfile=CurrentProfile;				// Restore profile
		saveSettings(0);													// Save settings preserving tip data
		HAL_Delay(500);
		NVIC_SystemReset();
		break;

	case 1:																	// Reset current profile
		resetCurrentProfile();												// Set current profile to defaults
		saveSettings(0);													// Save settings preserving tip data
		HAL_Delay(500);
		NVIC_SystemReset();
		break;

	case 2:																	// Reset all Profiles
		systemSettings.settings.currentProfile=profile_None;				// Set factory value
		saveSettings(1);													// Save settings, but wiping all tip data
		HAL_Delay(500);
		NVIC_SystemReset();
		break;

	case 3:																	// Reset everything
		resetSystemSettings();												// Reset system settings
		saveSettings(1);													// Save settings wiping all tip data
		HAL_Delay(500);
		NVIC_SystemReset();
		break;

	default:
		return screen_main;
	}
}

static int goSettingsReset(void) {
	resStatus=0;
	return screen_reset_confirmation;
}
static int goProfileReset(void) {
	resStatus=1;
	return screen_reset_confirmation;
}
static int goAllProfileReset(void) {
	resStatus=2;
	return screen_reset_confirmation;
}
static int goFactoryReset(void) {
	resStatus=3;
	return screen_reset_confirmation;
}

static void setButtonWake(uint16_t *val) {
	systemSettings.settings.wakeOnButton = *val;
}
static void * getButtonWake() {
	temp = systemSettings.settings.wakeOnButton;
	return &temp;
}

static void tempUnitChanged(void) {
	TempUnit = systemSettings.settings.tempUnit;
	if(TempUnit==mode_Farenheit){
		Widget_IRON_BoostTemp.EndStr="*F";
		Widget_IRON_BoostTemp.editable.max_value=900;
		Widget_IRON_BoostTemp.editable.min_value=400;
		Widget_IRON_BoostTemp.editable.big_step = 50;
		Widget_IRON_BoostTemp.editable.step = 10;

		Widget_IRON_SleepTemp.EndStr="*F";
		Widget_IRON_SleepTemp.editable.max_value=750;
		Widget_IRON_SleepTemp.editable.min_value=300;
		Widget_IRON_SleepTemp.editable.big_step = 50;
		Widget_IRON_SleepTemp.editable.step = 10;
		Widget_SYSTEM_TempStep.EndStr="*F";
	}
	else{
		Widget_IRON_BoostTemp.EndStr="*C";
		Widget_IRON_BoostTemp.editable.max_value=480;
		Widget_IRON_BoostTemp.editable.min_value=200;
		Widget_IRON_BoostTemp.editable.big_step = 50;
		Widget_IRON_BoostTemp.editable.step = 10;

		Widget_IRON_SleepTemp.EndStr="*C";
		Widget_IRON_SleepTemp.editable.max_value=400;
		Widget_IRON_SleepTemp.editable.min_value=150;
		Widget_IRON_SleepTemp.editable.big_step = 50;
		Widget_IRON_SleepTemp.editable.step = 10;
		Widget_SYSTEM_TempStep.EndStr="*C";
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void SYSTEM_update(screen_t *scr) {
	if(TempUnit != systemSettings.settings.tempUnit){
		TempUnit=systemSettings.settings.tempUnit;
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

static void settings_screen_OnEnter(screen_t *scr) {
	if(scr==&Screen_main){
		comboResetIndex(&comboWidget_Settings);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Edit Tip screen functions
//-------------------------------------------------------------------------------------------------------------------------------

void edit_tip_screenDraw(screen_t *scr){
	if((strcmp(str, "    ") == 0) || (strcmp(str, comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text) == 0) ) {
		if(Widget_IRONTIPS_Save.enabled){
			Widget_IRONTIPS_Save.enabled=0;
			FillBuffer(C_BLACK,fill_dma);
			UG_FontSelect(&FONT_10X16_reduced);
			UG_PutString(20,17,"NAME:");//12
		}
	}
	else{
		if(!Widget_IRONTIPS_Save.enabled){
			Widget_IRONTIPS_Save.enabled=1;
		}
	}
	default_screenDraw(scr);
}
void edit_tip_onEnter(screen_t *scr){
	Widget_IRONTIPS_Edit.editable.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Edit.editable.selectable.state=widget_selected;

	Widget_IRONTIPS_Save.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Save.buttonWidget.selectable.state=widget_idle;

	Widget_IRONTIPS_Back.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Back.buttonWidget.selectable.state=widget_idle;

	Widget_IRONTIPS_Delete.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Delete.buttonWidget.selectable.state=widget_idle;

	Screen_edit_tip_name.current_widget=&Widget_IRONTIPS_Edit;

	UG_FontSelect(&FONT_10X16_reduced);
	UG_PutString(20,17,"NAME:");//12
}
void edit_tip_screen_init(screen_t *scr) {
	if(strcmp(comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "    ");
		Widget_IRONTIPS_Delete.enabled = 0;
	}
	else {
		strcpy(str, comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem->text);
		if(systemSettings.Profile.currentNumberOfTips>1){
			Widget_IRONTIPS_Delete.enabled = 1;
		}
		else{
			Widget_IRONTIPS_Delete.enabled = 0;
		}
	}
	default_init(scr);
}
void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i =comboWidget_Settings_IRONTIPS.comboBoxWidget.first;
	for(int x = 0; x < TipSize; x++) {
		if(x < systemSettings.Profile.currentNumberOfTips) {
			strcpy(i->text, systemSettings.Profile.tip[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	comboWidget_Settings_IRONTIPS.comboBoxWidget.currentItem = comboWidget_Settings_IRONTIPS.comboBoxWidget.first;
	comboWidget_Settings_IRONTIPS.comboBoxWidget.currentScroll = 0;
	if(systemSettings.Profile.currentNumberOfTips >= TipSize) {
		comboitem_IRONTIPS_addNewTip.enabled = 0;
	}

	else{
		comboitem_IRONTIPS_addNewTip.enabled = 1;
	}
}



//-------------------------------------------------------------------------------------------------------------------------------
// Reset screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void Reset_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_Settings_SYSTEM);
}

//-------------------------------------------------------------------------------------------------------------------------------
// Reset confirmation screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void Reset_confirmation_onEnter(screen_t *scr){

	UG_FontSelect(&FONT_8X14_reduced);
	switch(resStatus){
	case 0:
		UG_PutString(43,0,"RESET");
		UG_PutString(7,17,"SYS. SETTINGS?");//14
		break;
	case 1:
		UG_PutString(43,0,"RESET");
		UG_PutString(7,17,"CURR. PROFILE?");//14
		break;
	case 2:
		UG_PutString(43,0,"RESET");
		UG_PutString(11,17,"ALL PROFILES?");//13
		break;
	case 3:
		UG_PutString(7,17,"DO FULL RESET?");//14
		break;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// PID screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void PID_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_Settings_PID);
	if(scr==&Screen_main){
		comboitem_PID_Back.action_screen=screen_main;
	}
	else if(scr==&Screen_settingsmenu){
		comboitem_PID_Back.action_screen=screen_settingsmenu;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// IRON screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void IRON_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_Settings_IRON);
}
//-------------------------------------------------------------------------------------------------------------------------------
// SYSTEM screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void SYSTEM_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_Settings_SYSTEM);
	if(ChecksumProfile(&systemSettings.Profile)!=systemSettings.ProfileChecksum){	// If there's unsaved profile data
		saveSettings(0);															// Save settings
	}
	profile=systemSettings.settings.currentProfile;
}
void SYSTEM_onExit(screen_t *scr){
	if(profile!=systemSettings.settings.currentProfile){	// If profile changed
		loadProfile(profile);
		saveSettings(0);									// Save
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
// ADVANCED screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void ADVANCED_onEnter(screen_t *scr){
	if(scr==&Screen_settingsmenu){
		comboResetIndex(&comboWidget_Settings_ADVANCED);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void settings_screen_setup(screen_t *scr) {
	screen_t* sc;
	widget_t* w;

	screen_setDefaults(scr);
	scr->processInput = &settings_screenProcessInput;
	scr->init = &settings_screen_init;
	scr->onEnter = &settings_screen_OnEnter;


	//#################################### [ SETTINGS MAIN SCREEN ]#######################################
	//
	w = &comboWidget_Settings;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_combo);
	comboAddScreen(&Settings_Combo_PID, w, 			"PID", 			screen_pid);
	comboAddScreen(&Settings_Combo_IRON, w, 		"IRON", 		screen_iron);
	comboAddScreen(&Settings_Combo_SYSTEM, w, 		"SYSTEM", 		screen_system);
	comboAddScreen(&Settings_Combo_ADVANCED, w, 	"ADVANCED", 	screen_advanced);
	comboAddScreen(&Settings_Combo_TIPS, w, 		"EDIT TIPS", 	screen_edit_iron_tips);
	comboAddScreen(&Settings_Combo_CALIBRATION, w, 	"CALIBRATION", 	screen_edit_calibration_wait);
	comboAddScreen(&Settings_Combo_DEBUG, w, 		"DEBUG", 		screen_debug);
	comboAddScreen(&Settings_Combo_EXIT, w, 		"EXIT", 		screen_main);


	//########################################## [ PID SCREEN ] ##########################################
	//
	sc=&Screen_pid;
	oled_addScreen(sc,screen_pid);
	screen_setDefaults(sc);
	sc->onEnter = &PID_onEnter;

	//********[ KP Widget]***********************************************************
	//
	w = &Widget_PID_Kp;
	static char arr_P[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_P;
	w->reservedChars=6;
	w->posX = 78;
	w->editable.inputData.getData = &getKp;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKp;
	w->displayWidget.justify = justify_right;

	//********[ KI Widget ]***********************************************************
	//
	w = &Widget_PID_Ki;
	static char arr_I[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_I;
	w->reservedChars=6;
	w->posX = 78;
	w->editable.inputData.getData = &getKi;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKi;
	w->displayWidget.justify = justify_right;

	//********[ KD Widget ]***********************************************************
	//
	w = &Widget_PID_Kd;
	static char arr_D[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_D;
	w->reservedChars=6;
	w->posX = 78;
	w->editable.inputData.getData = &getKd;
	w->editable.inputData.number_of_dec = 2;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 200;
	w->editable.step = 100;
	w->editable.setData = (void (*)(void *))&setKd;
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
	screen_setDefaults(sc);
	sc->onEnter = &IRON_onEnter;

	//********[ Sleep Time Widget ]***********************************************************
	//
	w = &Widget_IRON_SleepTime;
	static char arr_SleepTime[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_SleepTime;
	w->EndStr="min";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getSleepTime;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setSleepTime;
	w->editable.max_value = 60;
	w->editable.min_value = 0;
	w->displayWidget.justify =justify_right;


	//********[ Sleep Temp Widget ]***********************************************************
	//
	w = &Widget_IRON_SleepTemp;
	static char arr_SleepTemp[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_SleepTemp;
	w->EndStr="*C";
	w->reservedChars=5;
	w->posX = 86;//94
	w->editable.inputData.getData = &getSleepTemp;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.max_value = 400;
	w->editable.min_value = 150;
	w->editable.setData = (void (*)(void *))&setSleepTemp;
	w->displayWidget.justify =justify_right;

	//********[ Boost Time Widget ]***********************************************************
	//
	w = &Widget_IRON_BoostTime;
	static char arr_BoostTime[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_BoostTime;
	w->EndStr="min";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getBoostTime;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setBoostTime;
	w->editable.max_value = 60;
	w->editable.min_value = 1;
	w->displayWidget.justify =justify_right;

	//********[ Boost Temp Widget ]***********************************************************
	//
	w = &Widget_IRON_BoostTemp;
	static char arr_BoostTemp[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_BoostTemp;
	w->EndStr="*C";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getBoostTemp;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 20;
	w->editable.step = 10;
	w->editable.max_value = 480;
	w->editable.min_value = 300;
	w->editable.setData = (void (*)(void *))&setBoostTemp;
	w->displayWidget.justify =justify_right;

	//********[ Standby Time Widget ]***********************************************************
	//
	w = &Widget_IRON_StbyTime;
	static char arr_StbyTime[7];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_StbyTime;
	w->EndStr="min";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getStandByTime;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setStandByTime;
	w->editable.max_value = 60;
	w->editable.min_value = 0;
	w->displayWidget.justify =justify_right;

	//********[ Power Widget ]***********************************************************
	//
	w = &Widget_IRON_Power;
	static char arr_Pwr[5];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_Pwr;
	w->EndStr="%";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getMaxPower;
	w->editable.big_step = 20;
	w->editable.step = 5;
	w->editable.setData = (void (*)(void *))&setMaxPower;
	w->editable.max_value = 100;
	w->editable.min_value = 5;
	w->displayWidget.justify =justify_right;

	//========[ IRON COMBO ]===========================================================
	//
	w = &comboWidget_Settings_IRON;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_IRON_SleepTime, w,	"Sleep Time", 	&Widget_IRON_SleepTime);
	comboAddOption(&comboitem_IRON_SleepTemp, w, 	"Sleep Temp", 	&Widget_IRON_SleepTemp);
	comboAddOption(&comboitem_IRON_BoostTime, w, 	"Boost Time", 	&Widget_IRON_BoostTime);
	comboAddOption(&comboitem_IRON_BoostTemp, w, 	"Boost Temp", 	&Widget_IRON_BoostTemp);
	comboAddOption(&comboitem_IRON_StbyTime, w, 	"Stdby Time", 	&Widget_IRON_StbyTime);
	comboAddOption(&comboitem_IRON_Power, w, 		"Max. Power",		&Widget_IRON_Power);
	comboAddScreen(&comboitem_IRON_Back, w, 		"BACK", 		screen_settingsmenu);



	//########################################## SYSTEM SCREEN ##########################################
	//
	sc=&Screen_system;
	oled_addScreen(sc,screen_system);
	screen_setDefaults(sc);
	sc->onEnter = &SYSTEM_onEnter;
	sc->onExit = &SYSTEM_onExit;
	sc->update = &SYSTEM_update;

	//********[ Profile Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Profile;
	widgetDefaultsInit(w, widget_multi_option);
	w->posX = 94;
	w->editable.inputData.getData = &getProfile;
	w->reservedChars=4;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setProfile;
	w->editable.max_value = ProfileSize-1;
	w->editable.min_value = 0;
	w->multiOptionWidget.options = profileStr;
	w->multiOptionWidget.numberOfOptions = 3;

	//********[ Contrast Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Contrast;
	static char arr_contrast[4];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_contrast;
	w->reservedChars=3;
	w->posX = 102;
	w->editable.inputData.getData = &getContrast_;
	w->editable.big_step = 25;
	w->editable.step = 25;
	w->editable.setData = (void (*)(void *))&setContrast_;
	w->editable.max_value = 255;
	w->editable.min_value = 5;

	//********[ Oled Offset Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_OledOffset;
	static char oledoffset[3];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=oledoffset;
	w->reservedChars=2;
	w->posX = 110;
	w->editable.inputData.getData = &getOledOffset;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setOledOffset;
	w->editable.max_value = 15;
	w->editable.min_value = 0;
	w->displayWidget.justify =justify_right;

	//********[ Wake mode Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_WakeMode;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getWakeMode;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setWakeMode;
	w->editable.max_value = wakeInputmode_stand;
	w->editable.min_value = wakeInputmode_shake;
	w->multiOptionWidget.options = wakeMode;
	w->multiOptionWidget.numberOfOptions = 2;


	//********[ Encoder inversion Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_EncoderMode;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=3;
	w->posX = 102;
	w->editable.inputData.getData = &getEncoderMode;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setEncoderMode;
	w->editable.max_value = encoder_reverse;
	w->editable.min_value = encoder_normal;
	w->multiOptionWidget.options = OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Save Delay Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_SaveInterval;
	static char arr_savDelay[4];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_savDelay;
	w->reservedChars=3;
	w->EndStr="s";
	w->posX = 102;
	w->editable.inputData.getData = &getSavDelay;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setSavDelay;
	w->editable.max_value = 60;
	w->editable.min_value = 1;
	w->displayWidget.justify =justify_right;

	//********[ Gui refresh rate Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_GuiUpd;
	static char arr_guiUpd[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_guiUpd;
	w->EndStr="mS";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getGuiUpd_ms;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 100;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setGuiUpd_ms;
	w->editable.max_value = 500;
	w->editable.min_value = 0;
	w->displayWidget.justify =justify_right;

	//********[ Temp display unit Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_TempUnit;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=2;
	w->posX = 110;
	w->editable.inputData.getData = &getTmpUnit;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setTmpUnit;
	w->editable.max_value = mode_Farenheit;
	w->editable.min_value = mode_Celsius;
	w->multiOptionWidget.options = tempUnit;
	w->multiOptionWidget.numberOfOptions = 2;


	//********[ Temp step Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_TempStep;
	static char arr_TmpStepP[3];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_TmpStepP;
	w->EndStr="*C";
	w->reservedChars=4;
	w->posX = 94;
	w->editable.inputData.getData = &getTmpStep;
	w->editable.big_step = 5;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setTmpStep;
	w->editable.max_value = 50;
	w->editable.min_value = 1;
	w->displayWidget.justify =justify_right;

	//********[ Buzzer Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Buzzer;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=3;
	w->posX = 102;
	w->editable.inputData.getData = &getbuzzerMode;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setbuzzerMode;
	w->editable.max_value = 1;
	w->editable.min_value = 0;
	w->multiOptionWidget.options = OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Init mode Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_InitMode;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=3;
	w->posX = 102;
	w->editable.inputData.getData = &getInitMode;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setInitMode;
	w->editable.max_value = mode_boost;
	w->editable.min_value = mode_standby;
	w->multiOptionWidget.options = InitMode;
	w->multiOptionWidget.numberOfOptions = 4;

	//********[ Encoder wake Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_ButtonWake;
	widgetDefaultsInit(w, widget_multi_option);
	w->reservedChars=3;
	w->posX = 102;
	w->editable.inputData.getData = &getButtonWake;
	w->editable.big_step = 1;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setButtonWake;
	w->editable.max_value = 1;
	w->editable.min_value = 0;
	w->multiOptionWidget.options =OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//========[ SYSTEM COMBO ]===========================================================
	//
	w = &comboWidget_Settings_SYSTEM;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_SYSTEM_Profile,w, 		"Profile", 		&Widget_SYSTEM_Profile);
	comboAddOption(&comboitem_SYSTEM_Contrast,w, 		"Contrast", 	&Widget_SYSTEM_Contrast);
	comboAddOption(&comboitem_SYSTEM_OledOffset, w, 	"OLED Offset", 	&Widget_SYSTEM_OledOffset);
	comboAddOption(&comboitem_SYSTEM_EncoderMode, w, 	"Encoder inv.",	&Widget_SYSTEM_EncoderMode);
	comboAddOption(&comboitem_SYSTEM_InitMode, w, 		"PowerOn Mode", &Widget_SYSTEM_InitMode);
	comboAddOption(&comboitem_SYSTEM_WakeMode, w, 		"Wake input", 	&Widget_SYSTEM_WakeMode);
	comboAddOption(&comboitem_SYSTEM_ButtonWake, w, 	"Button Wake", 	&Widget_SYSTEM_ButtonWake);
	comboAddOption(&comboitem_SYSTEM_Buzzer, w, 		"Buzzer", 		&Widget_SYSTEM_Buzzer);
	comboAddOption(&comboitem_SYSTEM_TempUnit, w, 		"Temp. Unit", 	&Widget_SYSTEM_TempUnit);
	comboAddOption(&comboitem_SYSTEM_TempStep, w, 		"Temp. step",	&Widget_SYSTEM_TempStep);
	comboAddOption(&comboitem_SYSTEM_GuiUpd, w, 		"GUI update", 	&Widget_SYSTEM_GuiUpd);
	comboAddOption(&comboitem_SYSTEM_SaveInterval, w, 	"Save Delay",	&Widget_SYSTEM_SaveInterval);
	comboAddScreen(&comboitem_SYSTEM_Back, w, 			"BACK", 		screen_settingsmenu);



	//########################################## ADVANCED SCREEN ##########################################
	//
	sc=&Screen_advanced;
	oled_addScreen(sc,screen_advanced);
	screen_setDefaults(sc);
	sc->onEnter = &ADVANCED_onEnter;

	//********[ PWM Period Widget ]***********************************************************
	w = &Widget_ADVANCED_PWMPeriod;
	static char arr_pwmP[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_pwmP;
	w->EndStr="mS";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getPWMPeriod;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setPWMPeriod;
	w->editable.max_value = 500;
	w->editable.min_value = 20;
	w->displayWidget.justify =justify_right;

	//********[ ADC Delay Widget ]***********************************************************
	w = &Widget_ADVANCED_ADCDelay;
	static char arr_adcD[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_adcD;
	w->EndStr="mS";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getPWMDelay;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.setData = (void (*)(void *))&setPWMDelay;
	w->editable.max_value = 200;
	w->editable.min_value = 1;
	w->displayWidget.justify =justify_right;

	//********[ ADC Limit Widget ]***********************************************************
	//
	w = &Widget_ADVANCED_ADCLimit;
	static char arr_adcL[5];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_adcL;
	w->reservedChars=4;
	w->posX = 94;
	w->editable.inputData.getData = &getNoIronADC;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 200;
	w->editable.step = 10;
	w->editable.setData = (void (*)(void *))&setNoIronADC;
	w->editable.max_value = 4100;
	w->editable.min_value = 200;
	w->displayWidget.justify =justify_right;

	//********[ No Iron Delay Widget ]***********************************************************
	//
	w = &Widget_ADVANCED_NoIronDelay;
	static char arr_noIron[6];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_noIron;
	w->EndStr="mS";
	w->reservedChars=5;
	w->posX = 86;
	w->editable.inputData.getData = &getNoIronDelay;
	w->editable.inputData.type = field_uint16;
	w->editable.big_step = 100;
	w->editable.step = 50;
	w->editable.setData = (void (*)(void *))&setNoIronDelay;
	w->editable.max_value = 950;
	w->editable.min_value = 100;
	w->displayWidget.justify =justify_right;

	//========[ ADVANCED COMBO ]===========================================================
	//
	w = &comboWidget_Settings_ADVANCED;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_ADVANCED_PWMPeriod,w, 		"PWM Time", 		&Widget_ADVANCED_PWMPeriod);
	comboAddOption(&comboitem_ADVANCED_ADCDelay, w, 		"ADC Delay", 		&Widget_ADVANCED_ADCDelay);
	comboAddOption(&comboitem_ADVANCED_ADCLimit, w, 		"Det. Limit",		&Widget_ADVANCED_ADCLimit);
	comboAddOption(&comboitem_ADVANCED_NoIronDelay, w, 		"Det. Time",		&Widget_ADVANCED_NoIronDelay);
	comboAddScreen(&comboitem_ADVANCED_Reset, w, 			"Reset Menu",		screen_reset);
	comboAddScreen(&comboitem_ADVANCED_Back, w, 			"BACK", 			screen_settingsmenu);

	//########################################## RESET SCREEN ##########################################
	//
	sc=&Screen_reset;
	oled_addScreen(sc, screen_reset);
	screen_setDefaults(sc);
	sc->onEnter = &Reset_onEnter;

	//========[ RESET OPTIONS COMBO ]===========================================================
	//
	w = &comboWidget_Settings_RESET;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddAction(&comboitem_RESET_SETTINGS,w, 	"RESET: Settings", 	&goSettingsReset );
	comboAddAction(&comboitem_RESET_TIP,w, 			"RESET: Profile ", 	&goProfileReset );
	comboAddAction(&comboitem_RESET_ALLTIP,w, 		"RESET: Profiles", 	&goAllProfileReset );
	comboAddAction(&comboitem_RESET_EVERYTHING,w, 	"RESET: ALL DATA", 	&goFactoryReset);
	comboAddScreen(&comboitem_RESET_Back,w, 		"BACK", 			screen_advanced);



	//########################################## RESET CONFIRMATION SCREEN ##########################################
	//
	sc=&Screen_reset_confirmation;
	oled_addScreen(sc, screen_reset_confirmation);
	screen_setDefaults(sc);
	sc->onEnter = &Reset_confirmation_onEnter;


	// ********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_Reset_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="RESET";
	w->reservedChars=5;
	w->posX = 2;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &doReset;

	// ********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_Reset_CANCEL;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="CANCEL";
	w->reservedChars=6;
	w->posX = 78;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelReset;


	//########################################## IRON TIPS SCREEN ##########################################
	//
	sc=&Screen_edit_iron_tips;
	oled_addScreen(sc, screen_edit_iron_tips);
	screen_setDefaults(sc);
	sc->init = &edit_iron_screen_init;

	//========[ IRON TIPS COMBO ]===========================================================
	//
	w = &comboWidget_Settings_IRONTIPS;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_combo);
	for(int x = 0; x < TipSize; x++) {
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
	screen_setDefaults(sc);
	sc->draw = &edit_tip_screenDraw;
	sc->init = &edit_tip_screen_init;
	sc->onEnter = &edit_tip_onEnter;

	//********[ Name Edit Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Edit;
	screen_addWidget(w,sc);
	static char arr_tipName[TipCharSize];
	widgetDefaultsInit(w, widget_editable);
	w->displayString=arr_tipName;
	w->reservedChars=TipCharSize-1;
	w->posX = 75;
	w->posY = 17;
	w->font_size = &FONT_10X16_reduced;
	w->editable.inputData.getData = &getTipStr;
	w->editable.inputData.number_of_dec = 0;
	w->editable.inputData.type = field_string;
	w->editable.big_step = 10;
	w->editable.step = 1;
	w->editable.selectable.tab = 0;
	w->editable.setData = (void (*)(void *))&setTipStr;
	

	//********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Save;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="SAVE";
	w->reservedChars=4;
	w->posX = 2;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveTip;

	//********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Back;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="BACK";
	w->reservedChars=4;
	w->posX = 94;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelTip;

	//********[ Name Delete Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Delete;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="DEL.";
	w->reservedChars=4;
	w->posX = 48;
	w->posY = 50;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &delTip;
}
