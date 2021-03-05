/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "oled.h"
#include "gui.h"
#include "board.h"
#include "settings.h"
//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static char t[TipSize][TipCharSize];
static int32_t temp;
static uint8_t resStatus, profile;
char str[5];
uint32_t settingsTimer;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets
//-------------------------------------------------------------------------------------------------------------------------------

// SETTINGS SCREEM
static widget_t comboWidget_Settings;
static comboBox_item_t Settings_Combo_PID;
static comboBox_item_t Settings_Combo_IRON;
static comboBox_item_t Settings_Combo_SYSTEM;
static comboBox_item_t Settings_Combo_TIPS;
static comboBox_item_t Settings_Combo_CALIBRATION;
#ifdef ENABLE_DEBUG_SCREEN
static comboBox_item_t Settings_Combo_DEBUG;
#endif
static comboBox_item_t Settings_Combo_EXIT;

// SYSTEM
static widget_t comboWidget_SYSTEM;
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
static comboBox_item_t comboitem_SYSTEM_Reset;
static comboBox_item_t comboitem_SYSTEM_SW;
static comboBox_item_t comboitem_SYSTEM_HW;
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



// PID
static widget_t comboWidget_PID;
static comboBox_item_t comboitem_PID_Time;
static comboBox_item_t comboitem_PID_KP;
static comboBox_item_t comboitem_PID_KI;
static comboBox_item_t comboitem_PID_KD;
static comboBox_item_t comboitem_PID_Back;
static widget_t Widget_PID_Kd;
static widget_t Widget_PID_Ki;
static widget_t Widget_PID_Kp;
static widget_t Widget_PID_Time;

// IRON SETTINGS
static widget_t comboWidget_IRON;
static comboBox_item_t comboitem_IRON_SleepTime;
static comboBox_item_t comboitem_IRON_Power;
static comboBox_item_t comboitem_IRON_Impedance;
static comboBox_item_t comboitem_IRON_PWMPeriod;
static comboBox_item_t comboitem_IRON_ADCDelay;
static comboBox_item_t comboitem_IRON_FilterMode;
static comboBox_item_t comboitem_IRON_filterFactor;
static comboBox_item_t comboitem_IRON_NoIronDelay;
static comboBox_item_t comboitem_IRON_ADCLimit;
static comboBox_item_t comboitem_IRON_Back;

static widget_t Widget_IRON_SleepTime;
static widget_t Widget_IRON_Power;
static widget_t Widget_IRON_Impedance;
static widget_t Widget_IRON_ADCLimit;
static widget_t Widget_IRON_PWMPeriod;
static widget_t Widget_IRON_ADCDelay;
static widget_t Widget_IRON_FilterMode;
static widget_t Widget_IRON_filterFactor;
static widget_t Widget_IRON_NoIronDelay;

// EDIT TIPS SCREEN
static widget_t comboWidget_IRONTIPS;
static comboBox_item_t comboitem_IRONTIPS[TipSize];
static comboBox_item_t comboitem_IRONTIPS_addNewTip;
static comboBox_item_t comboitem_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Save;
static widget_t Widget_IRONTIPS_Delete;
static widget_t Widget_IRONTIPS_Edit;

// RESET SCREEN
static widget_t comboWidget_RESET;
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
static void setSettingsScrTempUnit() {
	widget_t* w;
	if(systemSettings.settings.tempUnit==mode_Farenheit){
		w=&Widget_SYSTEM_TempStep;
		w->endString="\260F";
	}
	else{
		w=&Widget_SYSTEM_TempStep;
		w->endString="\260C";
	}
}

static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(comboWidget_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		for(uint8_t x=0;x<TipSize;x++){
			if(strcmp(systemSettings.Profile.tip[x].name, "    ")!=0){														// Find first tip
				systemSettings.Profile.tip[systemSettings.Profile.currentNumberOfTips] = systemSettings.Profile.tip[x];	// Copy TIP data to new tip
				break;
			}
		}
		strcpy(systemSettings.Profile.tip[systemSettings.Profile.currentNumberOfTips].name, str);						// Copy new string
		++systemSettings.Profile.currentNumberOfTips;

	}
	else {
		for(int x = 0; x < TipSize;x++) {
			if(strcmp(comboWidget_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.Profile.tip[x].name) == 0 ) {
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
		if(strcmp(comboWidget_IRONTIPS.comboBoxWidget.currentItem->text, systemSettings.Profile.tip[x].name) == 0) {
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
	temp = systemSettings.Profile.power;
	return &temp;
}
static void setMaxPower(uint16_t *val) {
	systemSettings.Profile.power = *val;
}

static void * getTipImpedance() {
	temp = systemSettings.Profile.impedance;
	return &temp;
}
static void setTipImpedance(uint16_t *val) {
	systemSettings.Profile.impedance = *val;
}
static void * getContrast_() {
	temp = systemSettings.settings.contrast;
	return &temp;
}
static void setContrast_(uint8_t *val) {
	systemSettings.settings.contrast=*val;
	setContrast(*val);
}

static void setSleepTime(uint8_t *val) {
	systemSettings.Profile.sleepTimeout= *val;
}

static void * getSleepTime() {
	temp = systemSettings.Profile.sleepTimeout;
	return &temp;
}

static void * getKp() {
	temp = currentPID.Kp * 1000000;
	return &temp;
}
static void setKp(int32_t *val) {
	currentPID.Kp = (float)*val / 1000000;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kp=currentPID.Kp ;
	setupPIDFromStruct();
}
static void * getKi() {
	temp = currentPID.Ki * 1000000;
	return &temp;
}
static void setKi(int32_t *val) {
	currentPID.Ki = (float)*val / 1000000;
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Ki=currentPID.Ki;
	setupPIDFromStruct();
}
static void * getKd() {
	temp = currentPID.Kd * 1000000;
	return &temp;
}
static void setKd(int32_t *val) {
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
			Widget_IRON_ADCDelay.editableWidget.max_value = temp-1;
		}
		else{
			Widget_IRON_ADCDelay.editableWidget.max_value = 200;
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
			Widget_IRON_PWMPeriod.editableWidget.min_value = temp+1;
		}
		else{
			Widget_IRON_PWMPeriod.editableWidget.min_value=20;
		}
	}
}

static void * getTmpUnit() {
	temp = systemSettings.settings.tempUnit;
	return &temp;
}
static void setTmpUnit(uint16_t *val) {
	setSystemTempUnit(*val);
	setSettingsScrTempUnit();
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
	if(*val==wakeInputmode_stand){
		comboitem_SYSTEM_ButtonWake.enabled=0;
		systemSettings.settings.wakeOnButton=wakeButton_Off;
	}
	else{
		comboitem_SYSTEM_ButtonWake.enabled=1;
	}
}

static void * getEncoderMode() {

	temp = systemSettings.settings.EncoderMode;
	return &temp;
}
static void setEncoderMode(uint16_t *val) {
	systemSettings.settings.EncoderMode = * val;
	RE_SetMode(&RE1_Data, systemSettings.settings.EncoderMode);
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

static void * getFilterMode() {
	temp = systemSettings.Profile.filterMode;
	return &temp;
}
static void setFilterMode(uint16_t *val) {
	systemSettings.Profile.filterMode = *val;
	if(*val==filter_avg){
		comboitem_IRON_filterFactor.enabled=0;
	}
	else{
		comboitem_IRON_filterFactor.enabled=1;
	}
}

static void * getfilterFactor() {
	temp = systemSettings.Profile.filterFactor;
	return &temp;
}
static void setfilterFactor(uint16_t *val) {
	systemSettings.Profile.filterFactor = *val;
}


static void * getPIDTime() {
	temp = systemSettings.Profile.PIDTime;
	return &temp;
}
static void setPIDTime(uint16_t *val) {
	systemSettings.Profile.PIDTime = *val;
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
	switch(resStatus){
		case 0:																	// Reset system settings
		{
			uint8_t currentProfile=systemSettings.settings.currentProfile;				// Store current profile
			resetSystemSettings();												// Reset system settings
			systemSettings.settings.currentProfile=currentProfile;				// Restore profile
			saveSettings(saveKeepingProfiles);													// Save settings preserving tip data
			NVIC_SystemReset();
			break;
		}
		case 1:																	// Reset current profile
			resetCurrentProfile();												// Set current profile to defaults
			saveSettings(saveKeepingProfiles);													// Save settings preserving tip data
			NVIC_SystemReset();
			break;

		case 2:																	// Reset all Profiles
			systemSettings.settings.currentProfile=profile_None;				// Set factory value
			saveSettings(saveWipingProfiles);													// Save settings, but wiping all tip data
			NVIC_SystemReset();
			break;

		case 3:																	// Reset everything
			resetSystemSettings();												// Reset system settings
			saveSettings(saveWipingProfiles);													// Save settings wiping all tip data
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
//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------

static void settings_screen_init(screen_t *scr) {
	default_init(scr);
	scr->current_widget = &comboWidget_Settings;
	scr->current_widget->comboBoxWidget.selectable.state = widget_selected;
	settingsTimer=HAL_GetTick();
}


static void settings_screen_OnEnter(screen_t *scr) {
	if(scr==&Screen_main){
		comboResetIndex(&comboWidget_Settings);
	}
}

int settings_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	if(input!=Rotate_Nothing){
		settingsTimer=HAL_GetTick();
	}
	if(input==LongClick || (HAL_GetTick()-settingsTimer)>15000){
		return screen_main;
	}
	return default_screenProcessInput(scr, input, state);
}
//-------------------------------------------------------------------------------------------------------------------------------
// Edit Tip screen functions
//-------------------------------------------------------------------------------------------------------------------------------

void edit_tip_screenDraw(screen_t *scr){
	if((strcmp(str, "    ") == 0) || (strcmp(str, comboWidget_IRONTIPS.comboBoxWidget.currentItem->text) == 0) ) {
		if(Widget_IRONTIPS_Save.enabled){
			widgetDisable(&Widget_IRONTIPS_Save);
		}
	}
	else{
		if(!Widget_IRONTIPS_Save.enabled){
			widgetEnable(&Widget_IRONTIPS_Save);
		}
	}
	default_screenDraw(scr);
}


void edit_tip_screen_init(screen_t *scr) {
	if(strcmp(comboWidget_IRONTIPS.comboBoxWidget.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "    ");
		widgetDisable(&Widget_IRONTIPS_Delete);
	}
	else {
		strcpy(str, comboWidget_IRONTIPS.comboBoxWidget.currentItem->text);
		if(systemSettings.Profile.currentNumberOfTips>1){
			widgetEnable(&Widget_IRONTIPS_Delete);
		}
		else{
			widgetDisable(&Widget_IRONTIPS_Delete);
		}
	}
	scr->current_widget=&Widget_IRONTIPS_Edit;
	Widget_IRONTIPS_Edit.editableWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Edit.editableWidget.selectable.state=widget_selected;

	Widget_IRONTIPS_Save.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Save.buttonWidget.selectable.state=widget_idle;

	Widget_IRONTIPS_Back.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Back.buttonWidget.selectable.state=widget_idle;

	Widget_IRONTIPS_Delete.buttonWidget.selectable.previous_state=widget_idle;
	Widget_IRONTIPS_Delete.buttonWidget.selectable.state=widget_idle;

	u8g2_SetFont(&u8g2,default_font);
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,20,19,"NAME:");//12

	default_init(scr);
}
void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i =comboWidget_IRONTIPS.comboBoxWidget.first;
	for(int x = 0; x < TipSize; x++) {
		if(x < systemSettings.Profile.currentNumberOfTips) {
			strcpy(i->text, systemSettings.Profile.tip[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	comboWidget_IRONTIPS.comboBoxWidget.currentItem = comboWidget_IRONTIPS.comboBoxWidget.first;
	comboWidget_IRONTIPS.comboBoxWidget.currentScroll = 0;
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
	if(scr==&Screen_system){
		comboResetIndex(&comboWidget_RESET);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Reset confirmation screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void Reset_confirmation_onEnter(screen_t *scr){
	FillBuffer(BLACK, fill_dma);								// Manually clear the screen
	Screen_reset_confirmation.refresh=screen_blankRefresh;		// Set to already cleared so it doesn't get erased automatically

	u8g2_SetFont(&u8g2,default_font);
	u8g2_SetDrawColor(&u8g2, WHITE);

	switch(resStatus){
	case 0:
		putStrAligned("RESET SYSTEM", 0, align_center);
		putStrAligned("SETTINGS?", 16, align_center);
		break;
	case 1:
		putStrAligned("RESET CURRENT",0 , align_center);
		putStrAligned("PROFILE?", 16, align_center);
		break;
	case 2:
		putStrAligned("RESET ALL",0 , align_center);
		putStrAligned("PROFILES?", 16, align_center);
		break;
	case 3:
		putStrAligned("PERFORM FULL", 0, align_center);
		putStrAligned("SYSTEM RESET?", 16, align_center);
		break;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// PID screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void PID_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_PID);
	Widget_PID_Time.editableWidget.min_value=(systemSettings.Profile.pwmPeriod+1)/100;
	Widget_PID_Time.editableWidget.step=Widget_PID_Time.editableWidget.min_value;
	if(systemSettings.Profile.PIDTime <	Widget_PID_Time.editableWidget.min_value){
		systemSettings.Profile.PIDTime = Widget_PID_Time.editableWidget.min_value;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// IRON screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void IRON_onEnter(screen_t *scr){
	comboResetIndex(&comboWidget_IRON);
	if(systemSettings.Profile.filterMode == filter_avg){
		comboitem_IRON_filterFactor.enabled=0;
	}
	else{
		comboitem_IRON_filterFactor.enabled=1;
	}
}
//-------------------------------------------------------------------------------------------------------------------------------
// SYSTEM screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void SYSTEM_onEnter(screen_t *scr){
	setSettingsScrTempUnit();
	if(scr!=&Screen_reset){
		comboResetIndex(&comboWidget_SYSTEM);
	}
	if(ChecksumProfile(&systemSettings.Profile)!=systemSettings.ProfileChecksum){					// If there's unsaved profile data
		saveSettings(saveKeepingProfiles);															// Save settings
	}
	profile=systemSettings.settings.currentProfile;
}
void SYSTEM_onExit(screen_t *scr){
	if(profile!=systemSettings.settings.currentProfile){	// If profile changed
		loadProfile(profile);
		saveSettings(saveKeepingProfiles);									// Save
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void settings_screen_setup(screen_t *scr) {
	screen_t* sc;
	widget_t* w;
	displayOnly_widget_t* dis;

	screen_setDefaults(scr);
	scr->init = &settings_screen_init;
	scr->onEnter = &settings_screen_OnEnter;
	scr->processInput=&settings_screenProcessInput;


	//#################################### [ SETTINGS MAIN SCREEN ]#######################################
	//
	w = &comboWidget_Settings;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_combo);
	comboAddScreen(&Settings_Combo_PID, w, 			"PID", 			screen_pid);
	comboAddScreen(&Settings_Combo_IRON, w, 		"IRON", 		screen_iron);
	comboAddScreen(&Settings_Combo_SYSTEM, w, 		"SYSTEM", 		screen_system);
	comboAddScreen(&Settings_Combo_TIPS, w, 		"EDIT TIPS", 	screen_edit_iron_tips);
	comboAddScreen(&Settings_Combo_CALIBRATION, w, 	"CALIBRATION", 	screen_edit_calibration_wait);
	#ifdef ENABLE_DEBUG_SCREEN
	comboAddScreen(&Settings_Combo_DEBUG, w, 		"DEBUG", 		screen_debug);
	#endif
	comboAddScreen(&Settings_Combo_EXIT, w, 		"EXIT", 		screen_main);


	//########################################## SYSTEM SCREEN ##########################################
	//
	sc=&Screen_system;
	oled_addScreen(sc,screen_system);
	screen_setDefaults(sc);
	sc->onEnter = &SYSTEM_onEnter;
	sc->onExit = &SYSTEM_onExit;
	sc->processInput=&settings_screenProcessInput;

	//********[ Profile Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Profile;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getProfile;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setProfile;
	w->editableWidget.max_value = ProfileSize-1;
	w->editableWidget.min_value = 0;
	w->multiOptionWidget.options = profileStr;
	w->multiOptionWidget.numberOfOptions = 3;

	//********[ Contrast Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Contrast;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=3;
	dis->getData = &getContrast_;
	w->editableWidget.big_step = 25;
	w->editableWidget.step = 25;
	w->editableWidget.setData = (void (*)(void *))&setContrast_;
	w->editableWidget.max_value = 255;
	w->editableWidget.min_value = 5;

	//********[ Oled Offset Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_OledOffset;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=2;
	dis->getData = &getOledOffset;
	w->editableWidget.big_step = 10;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setOledOffset;
	w->editableWidget.max_value = 15;
	w->editableWidget.min_value = 0;
	//********[ Wake mode Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_WakeMode;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getWakeMode;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setWakeMode;
	w->editableWidget.max_value = wakeInputmode_stand;
	w->editableWidget.min_value = wakeInputmode_shake;
	w->multiOptionWidget.options = wakeMode;
	w->multiOptionWidget.numberOfOptions = 2;


	//********[ Encoder inversion Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_EncoderMode;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getEncoderMode;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setEncoderMode;
	w->editableWidget.max_value = encoder_reverse;
	w->editableWidget.min_value = encoder_normal;
	w->multiOptionWidget.options = encMode;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Save Delay Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_SaveInterval;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=3;
	w->endString="s";
	dis->getData = &getSavDelay;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setSavDelay;
	w->editableWidget.max_value = 60;
	w->editableWidget.min_value = 1;

	//********[ Gui refresh rate Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_GuiUpd;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getGuiUpd_ms;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 10;
	w->editableWidget.setData = (void (*)(void *))&setGuiUpd_ms;
	w->editableWidget.max_value = 500;
	w->editableWidget.min_value = 0;

	//********[ Temp display unit Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_TempUnit;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getTmpUnit;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setTmpUnit;
	w->editableWidget.max_value = mode_Farenheit;
	w->editableWidget.min_value = mode_Celsius;
	w->multiOptionWidget.options = tempUnit;
	w->multiOptionWidget.numberOfOptions = 2;


	//********[ Temp step Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_TempStep;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	dis->getData = &getTmpStep;
	w->editableWidget.big_step = 5;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setTmpStep;
	w->editableWidget.max_value = 50;
	w->editableWidget.min_value = 1;

	//********[ Buzzer Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_Buzzer;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getbuzzerMode;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setbuzzerMode;
	w->editableWidget.max_value = 1;
	w->editableWidget.min_value = 0;
	w->multiOptionWidget.options = OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Init mode Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_InitMode;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getInitMode;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setInitMode;
	w->editableWidget.max_value = mode_run;
	w->editableWidget.min_value = mode_sleep;
	w->multiOptionWidget.options = InitMode;
	w->multiOptionWidget.numberOfOptions = 2;

	//********[ Encoder wake Widget ]***********************************************************
	//
	w = &Widget_SYSTEM_ButtonWake;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getButtonWake;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setButtonWake;
	w->editableWidget.max_value = 1;
	w->editableWidget.min_value = 0;
	w->multiOptionWidget.options =OffOn;
	w->multiOptionWidget.numberOfOptions = 2;

	//========[ SYSTEM COMBO ]===========================================================
	//
	w = &comboWidget_SYSTEM;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_SYSTEM_Profile,w, 		"Profile", 		&Widget_SYSTEM_Profile);
	comboAddOption(&comboitem_SYSTEM_Contrast,w, 		"Contrast", 	&Widget_SYSTEM_Contrast);
	comboAddOption(&comboitem_SYSTEM_OledOffset, w, 	"Offset", 	&Widget_SYSTEM_OledOffset);
	comboAddOption(&comboitem_SYSTEM_EncoderMode, w, 	"Encoder",		&Widget_SYSTEM_EncoderMode);
	comboAddOption(&comboitem_SYSTEM_InitMode, w, 		"Boot", 	&Widget_SYSTEM_InitMode);
	comboAddOption(&comboitem_SYSTEM_WakeMode, w, 		"Wake Mode", 	&Widget_SYSTEM_WakeMode);
	comboAddOption(&comboitem_SYSTEM_ButtonWake, w, 	"Btn. wake", 	&Widget_SYSTEM_ButtonWake);
	comboAddOption(&comboitem_SYSTEM_Buzzer, w, 		"Buzzer", 		&Widget_SYSTEM_Buzzer);
	comboAddOption(&comboitem_SYSTEM_TempUnit, w, 		"Unit", 	&Widget_SYSTEM_TempUnit);
	comboAddOption(&comboitem_SYSTEM_TempStep, w, 		"Step",	&Widget_SYSTEM_TempStep);
	comboAddOption(&comboitem_SYSTEM_GuiUpd, w, 		"Gui time", 	&Widget_SYSTEM_GuiUpd);
	comboAddOption(&comboitem_SYSTEM_SaveInterval, w, 	"Save time",	&Widget_SYSTEM_SaveInterval);
	comboAddScreen(&comboitem_SYSTEM_Reset, w, 			"RESET MENU",	screen_reset);
	comboAddScreen(&comboitem_SYSTEM_SW, w, 			SWSTRING,		-1);
	comboAddScreen(&comboitem_SYSTEM_HW, w, 			HWSTRING,		-1);
	comboAddScreen(&comboitem_SYSTEM_Back, w, 			"BACK", 		screen_settingsmenu);

	//########################################## RESET SCREEN ##########################################
	//
	sc=&Screen_reset;
	oled_addScreen(sc, screen_reset);
	screen_setDefaults(sc);
	sc->onEnter = &Reset_onEnter;
	sc->processInput=&settings_screenProcessInput;

	//========[ RESET OPTIONS COMBO ]===========================================================
	//
	w = &comboWidget_RESET;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddAction(&comboitem_RESET_SETTINGS,w, 	"Reset Settings", 	&goSettingsReset );
	comboAddAction(&comboitem_RESET_TIP,w, 			"Reset Profile", 	&goProfileReset );
	comboAddAction(&comboitem_RESET_ALLTIP,w, 		"Reset Profiles", 	&goAllProfileReset );
	comboAddAction(&comboitem_RESET_EVERYTHING,w, 	"Reset All", 		&goFactoryReset);
	comboAddScreen(&comboitem_RESET_Back,w, 		"BACK", 			screen_system);
	comboitem_RESET_SETTINGS.dispAlign=align_left;
	comboitem_RESET_TIP.dispAlign=align_left;
	comboitem_RESET_ALLTIP.dispAlign=align_left;
	comboitem_RESET_EVERYTHING.dispAlign=align_left;
	//########################################## RESET CONFIRMATION SCREEN ##########################################
	//
	sc=&Screen_reset_confirmation;
	oled_addScreen(sc, screen_reset_confirmation);
	screen_setDefaults(sc);
	sc->onEnter = &Reset_confirmation_onEnter;
	sc->processInput=&settings_screenProcessInput;


	// ********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_Reset_OK;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="RESET";
	w->posX = 0;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &doReset;

	// ********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_Reset_CANCEL;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="CANCEL";
	w->posX = 74;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 0;
	w->buttonWidget.action = &cancelReset;


	//########################################## [ PID SCREEN ] ##########################################
	//
	sc=&Screen_pid;
	oled_addScreen(sc,screen_pid);
	screen_setDefaults(sc);
	sc->onEnter = &PID_onEnter;
	sc->processInput=&settings_screenProcessInput;

	//********[ KP Widget]***********************************************************
	//
	w = &Widget_PID_Kp;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	dis->getData = &getKp;
	dis->number_of_dec = 2;
	w->editableWidget.max_value=99999;
	w->editableWidget.big_step = 1000;
	w->editableWidget.step = 50;
	w->editableWidget.setData =  (void (*)(void *))&setKp;

	//********[ KI Widget ]***********************************************************
	//
	w = &Widget_PID_Ki;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	dis->getData = &getKi;
	dis->number_of_dec = 2;
	w->editableWidget.max_value=99999;
	w->editableWidget.big_step = 1000;
	w->editableWidget.step = 50;
	w->editableWidget.setData = (void (*)(void *))&setKi;

	//********[ KD Widget ]***********************************************************
	//
	w = &Widget_PID_Kd;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=6;
	dis->getData = &getKd;
	dis->number_of_dec = 2;
	w->editableWidget.max_value=99999;
	w->editableWidget.big_step = 1000;
	w->editableWidget.step = 50;
	w->editableWidget.setData = (void (*)(void *))&setKd;

	//********[ PID Time Widget ]***********************************************************
	w = &Widget_PID_Time;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getPIDTime;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 10;
	w->editableWidget.setData = (void (*)(void *))&setPIDTime;
	w->editableWidget.max_value = 1000;
	w->editableWidget.min_value = 0;

	//========[ PID COMBO ]===========================================================
	//
	w = &comboWidget_PID;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_PID_KP, w, 	"Kp", 	&Widget_PID_Kp);
	comboAddOption(&comboitem_PID_KI, w, 	"Ki", 	&Widget_PID_Ki);
	comboAddOption(&comboitem_PID_KD, w, 	"Kd", 	&Widget_PID_Kd);
	comboAddOption(&comboitem_PID_Time, w, 	"Time",	&Widget_PID_Time);
	comboAddScreen(&comboitem_PID_Back, w, 	"BACK", screen_settingsmenu);

	//########################################## IRON SCREEN ##########################################
	//
	sc=&Screen_iron;
	oled_addScreen(sc,screen_iron);
	screen_setDefaults(sc);
	sc->onEnter = &IRON_onEnter;
	sc->processInput=&settings_screenProcessInput;

	//********[ Sleep Time Widget ]***********************************************************
	//
	w = &Widget_IRON_SleepTime;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="min";
	dis->reservedChars=5;
	dis->getData = &getSleepTime;
	w->editableWidget.big_step = 5;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setSleepTime;
	w->editableWidget.max_value = 60;
	w->editableWidget.min_value = 0;

	//********[ Power Widget ]***********************************************************
	//
	w = &Widget_IRON_Power;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	w->endString="W";
	dis->getData = &getMaxPower;
	w->editableWidget.big_step = 20;
	w->editableWidget.step = 5;
	w->editableWidget.setData = (void (*)(void *))&setMaxPower;
	w->editableWidget.max_value = 500;
	w->editableWidget.min_value = 5;

	//********[ Impedance Widget ]***********************************************************
	//
	w = &Widget_IRON_Impedance;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	dis->number_of_dec=1;
	//w->endString="";
	dis->getData = &getTipImpedance;
	w->editableWidget.big_step = 10;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setTipImpedance;
	w->editableWidget.max_value = 160;
	w->editableWidget.min_value = 10;

	//********[ ADC Limit Widget ]***********************************************************
	//
	w = &Widget_IRON_ADCLimit;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=4;
	dis->getData = &getNoIronADC;
	w->editableWidget.big_step = 200;
	w->editableWidget.step = 10;
	w->editableWidget.setData = (void (*)(void *))&setNoIronADC;
	w->editableWidget.max_value = 4100;
	w->editableWidget.min_value = 200;

	//********[ PWM Period Widget ]***********************************************************
	w = &Widget_IRON_PWMPeriod;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getPWMPeriod;
	w->editableWidget.big_step = 20;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setPWMPeriod;
	w->editableWidget.max_value = 500;
	w->editableWidget.min_value = 20;

	//********[ ADC Delay Widget ]***********************************************************
	w = &Widget_IRON_ADCDelay;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getPWMDelay;
	w->editableWidget.big_step = 10;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setPWMDelay;
	w->editableWidget.max_value = 200;
	w->editableWidget.min_value = 1;

	//********[ Filter Mode Widget ]***********************************************************
	//
	w = &Widget_IRON_FilterMode;
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getFilterMode;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setFilterMode;
	w->editableWidget.max_value = filter_dema;
	w->editableWidget.min_value = filter_avg;
	w->multiOptionWidget.options = filterMode;
	w->multiOptionWidget.numberOfOptions = 3;

	//********[ Filter Coefficient Widget ]***********************************************************
	w = &Widget_IRON_filterFactor;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=1;
	dis->getData = &getfilterFactor;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->editableWidget.setData = (void (*)(void *))&setfilterFactor;
	w->editableWidget.max_value = 4;
	w->editableWidget.min_value = 1;

	//********[ No Iron Delay Widget ]***********************************************************
	//
	w = &Widget_IRON_NoIronDelay;
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getNoIronDelay;
	w->editableWidget.big_step = 100;
	w->editableWidget.step = 50;
	w->editableWidget.setData = (void (*)(void *))&setNoIronDelay;
	w->editableWidget.max_value = 950;
	w->editableWidget.min_value = 100;

	//========[ IRON COMBO ]===========================================================
	//
	w = &comboWidget_IRON;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo);
	comboAddOption(&comboitem_IRON_SleepTime, w,	"Sleep", 		&Widget_IRON_SleepTime);
	comboAddOption(&comboitem_IRON_Impedance, w, 	"Heater R.",	&Widget_IRON_Impedance);
	comboAddOption(&comboitem_IRON_Power, w, 		"Power",		&Widget_IRON_Power);
	comboAddOption(&comboitem_IRON_PWMPeriod,w, 	"PWM Time", 	&Widget_IRON_PWMPeriod);
	comboAddOption(&comboitem_IRON_ADCDelay, w, 	"ADC Delay", 	&Widget_IRON_ADCDelay);
	comboAddOption(&comboitem_IRON_FilterMode, w, 	"Filtering", 	&Widget_IRON_FilterMode);
	comboAddOption(&comboitem_IRON_filterFactor, w, 	"Factor",		&Widget_IRON_filterFactor);
	comboAddOption(&comboitem_IRON_ADCLimit, w, 	"No iron",		&Widget_IRON_ADCLimit);
	comboAddOption(&comboitem_IRON_NoIronDelay, w, 	"Detection",	&Widget_IRON_NoIronDelay);
	comboAddScreen(&comboitem_IRON_Back, w, 		"BACK", 		screen_settingsmenu);


	//########################################## IRON TIPS SCREEN ##########################################
	//
	sc=&Screen_edit_iron_tips;
	oled_addScreen(sc, screen_edit_iron_tips);
	screen_setDefaults(sc);
	sc->init = &edit_iron_screen_init;
	sc->processInput=&settings_screenProcessInput;

	//========[ IRON TIPS COMBO ]===========================================================
	//
	w = &comboWidget_IRONTIPS;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_combo);
	for(int x = 0; x < TipSize; x++) {
		t[x][0] = '\0';
		comboAddScreen(&comboitem_IRONTIPS[x],w, &t[x][0], screen_edit_tip_name);
	}
	comboAddScreen(&comboitem_IRONTIPS_addNewTip, w, "ADD NEW", screen_edit_tip_name);
	comboAddScreen(&comboitem_IRONTIPS_Back, w, "BACK", screen_settingsmenu);



	//########################################## IRON TIPS EDIT SCREEN ##########################################
	//
	sc=&Screen_edit_tip_name;
	oled_addScreen(sc, screen_edit_tip_name);
	screen_setDefaults(sc);
	sc->draw = &edit_tip_screenDraw;
	sc->init = &edit_tip_screen_init;

	//********[ Name Edit Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Edit;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=TipCharSize-1;
	w->posX = 75;
	w->posY = 17;
	w->width = 46;
	dis->getData = &getTipStr;
	dis->number_of_dec = 0;
	dis->type = field_string;
	w->editableWidget.big_step = 10;
	w->editableWidget.step = 1;
	w->editableWidget.selectable.tab = 0;
	w->editableWidget.setData = (void (*)(void *))&setTipStr;
	

	//********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Save;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="SAVE";
	w->posX = 0;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 3;
	w->buttonWidget.action = &saveTip;

	//********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Back;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="BACK";
	w->posX = 90;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 1;
	w->buttonWidget.action = &cancelTip;

	//********[ Name Delete Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Delete;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button);
	w->displayString="DEL.";
	w->posX = 46 ;
	w->posY = 48;
	w->buttonWidget.selectable.tab = 2;
	w->buttonWidget.action = &delTip;
}
