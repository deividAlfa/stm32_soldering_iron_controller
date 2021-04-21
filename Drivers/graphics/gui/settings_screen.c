/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "settings_screen.h"
#include "main_screen.h"
#include "debug_screen.h"
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
static uint32_t settingsTimer;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
screen_t Screen_settingsmenu;
screen_t Screen_pid;
screen_t Screen_iron;
screen_t Screen_system;
screen_t Screen_reset;
screen_t Screen_reset_confirmation;
screen_t Screen_edit_iron_tips;
screen_t Screen_edit_tip_name;

// SETTINGS SCREEM
static widget_t comboWidget_Settings;
static comboBox_widget_t comboBox_Settings;
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
static comboBox_widget_t comboBox_SYSTEM;
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
static comboBox_item_t comboitem_SYSTEM_ShakeWake;
static comboBox_item_t comboitem_SYSTEM_Reset;
static comboBox_item_t comboitem_SYSTEM_SW;
static comboBox_item_t comboitem_SYSTEM_HW;
static comboBox_item_t comboitem_SYSTEM_Back;

static editable_widget_t editable_SYSTEM_Profile;
static editable_widget_t editable_SYSTEM_Contrast;
static editable_widget_t editable_SYSTEM_OledOffset;
static editable_widget_t editable_SYSTEM_WakeMode;
static editable_widget_t editable_SYSTEM_EncoderMode;
static editable_widget_t editable_SYSTEM_TempUnit;
static editable_widget_t editable_SYSTEM_TempStep;
static editable_widget_t editable_SYSTEM_GuiUpd;
static editable_widget_t editable_SYSTEM_SaveInterval;
static editable_widget_t editable_SYSTEM_Buzzer;
static editable_widget_t editable_SYSTEM_InitMode;
static editable_widget_t editable_SYSTEM_ButtonWake;
static editable_widget_t editable_SYSTEM_ShakeWake;



// PID
static widget_t comboWidget_PID;
static comboBox_widget_t comboBox_PID;
static comboBox_item_t comboitem_PID_KP;
static comboBox_item_t comboitem_PID_KI;
static comboBox_item_t comboitem_PID_KD;
static comboBox_item_t comboitem_PID_Imax;
static comboBox_item_t comboitem_PID_Imin;
static comboBox_item_t comboitem_PID_Back;

static editable_widget_t editable_PID_Kd;
static editable_widget_t editable_PID_Ki;
static editable_widget_t editable_PID_Kp;
static editable_widget_t editable_PID_Imax;
static editable_widget_t editable_PID_Imin;

// IRON SETTINGS
static widget_t comboWidget_IRON;
static comboBox_widget_t comboBox_IRON;
static comboBox_item_t comboitem_IRON_SleepTime;
static comboBox_item_t comboitem_IRON_Power;
static comboBox_item_t comboitem_IRON_Impedance;
static comboBox_item_t comboitem_IRON_MaxTemp;
static comboBox_item_t comboitem_IRON_MinTemp;
static comboBox_item_t comboitem_IRON_PWMPeriod;
static comboBox_item_t comboitem_IRON_ADCDelay;
static comboBox_item_t comboitem_IRON_FilterMode;
static comboBox_item_t comboitem_IRON_filterFactor;
static comboBox_item_t comboitem_IRON_errorDelay;
static comboBox_item_t comboitem_IRON_ADCLimit;
static comboBox_item_t comboitem_IRON_Back;

static editable_widget_t editable_IRON_SleepTime;
static editable_widget_t editable_IRON_Power;
static editable_widget_t editable_IRON_Impedance;
static editable_widget_t editable_IRON_MaxTemp;
static editable_widget_t editable_IRON_MinTemp;
static editable_widget_t editable_IRON_ADCLimit;
static editable_widget_t editable_IRON_PWMPeriod;
static editable_widget_t editable_IRON_ADCDelay;
static editable_widget_t editable_IRON_FilterMode;
static editable_widget_t editable_IRON_filterFactor;
static editable_widget_t editable_IRON_errorDelay;

// EDIT TIPS SCREEN
static widget_t comboWidget_IRONTIPS;
static comboBox_widget_t comboBox_IRONTIPS;
static comboBox_item_t comboitem_IRONTIPS[TipSize];
static comboBox_item_t comboitem_IRONTIPS_addNewTip;
static comboBox_item_t comboitem_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Back;
static button_widget_t button_IRONTIPS_Back;
static widget_t Widget_IRONTIPS_Save;
static button_widget_t button_IRONTIPS_Save;
static widget_t Widget_IRONTIPS_Delete;
static button_widget_t button_IRONTIPS_Delete;
static widget_t Widget_IRONTIPS_Edit;
static editable_widget_t editable_IRONTIPS_Edit;

// RESET SCREEN
static widget_t comboWidget_RESET;
static comboBox_widget_t comboBox_RESET;
static comboBox_item_t comboitem_RESET_SETTINGS;
static comboBox_item_t comboitem_RESET_TIP;
static comboBox_item_t comboitem_RESET_ALLTIP;
static comboBox_item_t comboitem_RESET_EVERYTHING;
static comboBox_item_t comboitem_RESET_Back;
static widget_t Widget_Reset_OK;
static button_widget_t button_Reset_OK;
static widget_t Widget_Reset_CANCEL;
static button_widget_t button_Reset_CANCEL;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------
static void setSettingsScrTempUnit() {
	if(systemSettings.settings.tempUnit==mode_Farenheit){
    editable_SYSTEM_TempStep.inputData.endString="\260F";
    editable_IRON_MaxTemp.inputData.endString="\260F";
    editable_IRON_MinTemp.inputData.endString="\260F";
	}
	else{
	  editable_SYSTEM_TempStep.inputData.endString="\260C";
	  editable_IRON_MaxTemp.inputData.endString="\260C";
	  editable_IRON_MinTemp.inputData.endString="\260C";
	}
}

static void *getTipStr() {
	return str;
}

static void setTipStr(char *s) {
	strcpy(str, s);
}
static int saveTip(widget_t *w) {
	if(strcmp(comboBox_IRONTIPS.currentItem->text, "ADD NEW") == 0) {
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
			if(strcmp(comboBox_IRONTIPS.currentItem->text, systemSettings.Profile.tip[x].name) == 0 ) {
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
		if(strcmp(comboBox_IRONTIPS.currentItem->text, systemSettings.Profile.tip[x].name) == 0) {
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
	temp = systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kp;
	return &temp;
}
static void setKp(int32_t *val) {
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kp=*val;
	setupPID(&systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID);
	resetPID();
}
static void * getKi() {
	temp = systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Ki;
	return &temp;
}
static void setKi(int32_t *val) {
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Ki=*val;
	setupPID(&systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID);
	resetPID();
}
static void * getKd() {
	temp = systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kd;
	return &temp;
}
static void setKd(int32_t *val) {
	systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.Kd=*val;
	setupPID(&systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID);
	resetPID();
}
static void * getImax() {
  temp = systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.maxI;
  return &temp;
}
static void setImax(int32_t *val) {
  systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.maxI=*val;
  setupPID(&systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID);
  resetPID();
}
static void * getImin() {
  temp = systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.minI;
  return &temp;
}
static void setImin(int32_t *val) {
  systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID.minI=*val;
  setupPID(&systemSettings.Profile.tip[systemSettings.Profile.currentTip].PID);
  resetPID();
}
static void * getPWMPeriod() {
	temp=(systemSettings.Profile.pwmPeriod+1)/100;
	return &temp;
}
static void setPWMPeriod(uint16_t *val) {
	uint16_t period=(*val*100)-1;
	if(!setPwmPeriod(period)){
		if(*val<=200){
		  editable_IRON_ADCDelay.max_value = temp-1;
		}
		else{
		  editable_IRON_ADCDelay.max_value = 200;
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
			editable_IRON_PWMPeriod.min_value = temp+1;
		}
		else{
		  editable_IRON_PWMPeriod.min_value=20;
		}
	}
}

static void * getMaxTemp() {
  temp=systemSettings.Profile.MaxSetTemperature;
  return &temp;
}
static void setMaxTemp(uint16_t *val) {
  if(*val<=systemSettings.Profile.MinSetTemperature){
    *val=systemSettings.Profile.MinSetTemperature+1;
  }
  editable_IRON_MinTemp.max_value = *val-1;
  systemSettings.Profile.MaxSetTemperature=*val;
}

static void * getMinTemp() {
  temp=systemSettings.Profile.MinSetTemperature;
  return &temp;
}

static void setMinTemp(uint16_t *val) {
  if(*val>=systemSettings.Profile.MaxSetTemperature){
    *val=systemSettings.Profile.MaxSetTemperature-1;
  }
  editable_IRON_MaxTemp.min_value = *val+1;
  systemSettings.Profile.MinSetTemperature=*val;
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

static void * geterrorDelay() {
	temp = systemSettings.settings.errorDelay;
	return &temp;
}
static void seterrorDelay(uint16_t *val) {
	systemSettings.settings.errorDelay = *val;
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
		case 0:																	                                // Reset system settings
		{
			uint8_t currentProfile=systemSettings.settings.currentProfile;				// Store current profile
			resetSystemSettings();												                        // Reset system settings
			systemSettings.settings.currentProfile=currentProfile;				        // Restore profile
			saveSettings(saveKeepingProfiles);													          // Save settings preserving tip data
			NVIC_SystemReset();
			break;
		}
		case 1:																	                                // Reset current profile
			resetCurrentProfile();												                        // Set current profile to defaults
			saveSettings(saveKeepingProfiles);													          // Save settings preserving tip data
			NVIC_SystemReset();
			break;

		case 2:																	                                // Reset all Profiles
			systemSettings.settings.currentProfile=profile_None;				          // Set factory value
			saveSettings(saveWipingProfiles);													            // Save settings, but wiping all tip data
			NVIC_SystemReset();
			break;

		case 3:																	                                // Reset everything
			resetSystemSettings();												                        // Reset system settings
			saveSettings(saveWipingProfiles);													            // Save settings wiping all tip data
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

static void setShakeWake(uint16_t *val) {
  systemSettings.settings.wakeOnShake = *val;
}
static void * getShakeWake() {
  temp = systemSettings.settings.wakeOnShake;
  return &temp;
}
//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------

static void settings_screen_init(screen_t *scr) {
	default_init(scr);
	scr->current_widget = &comboWidget_Settings;
	comboBox_IRONTIPS.selectable.state = widget_selected;
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
	if((strcmp(str, "    ") == 0) || (strcmp(str, comboBox_IRONTIPS.currentItem->text) == 0) ) {
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
	if(strcmp(comboBox_IRONTIPS.currentItem->text, "ADD NEW") == 0) {
		strcpy(str, "    ");
		widgetDisable(&Widget_IRONTIPS_Delete);
	}
	else {
		strcpy(str, comboBox_IRONTIPS.currentItem->text);
		if(systemSettings.Profile.currentNumberOfTips>1){
			widgetEnable(&Widget_IRONTIPS_Delete);
		}
		else{
			widgetDisable(&Widget_IRONTIPS_Delete);
		}
	}
	scr->current_widget=&Widget_IRONTIPS_Edit;
	editable_IRONTIPS_Edit.selectable.previous_state=widget_idle;
	editable_IRONTIPS_Edit.selectable.state=widget_selected;

	button_IRONTIPS_Save.selectable.previous_state=widget_idle;
	button_IRONTIPS_Save.selectable.state=widget_idle;

	button_IRONTIPS_Back.selectable.previous_state=widget_idle;
	button_IRONTIPS_Back.selectable.state=widget_idle;

	button_IRONTIPS_Delete.selectable.previous_state=widget_idle;
	button_IRONTIPS_Delete.selectable.state=widget_idle;

	u8g2_SetFont(&u8g2,default_font);
	u8g2_SetDrawColor(&u8g2, WHITE);
	u8g2_DrawStr(&u8g2,20,19,"NAME:");

	default_init(scr);
}
void edit_iron_screen_init(screen_t *scr) {
	comboBox_item_t *i = comboBox_IRONTIPS.first;
	for(int x = 0; x < TipSize; x++) {
		if(x < systemSettings.Profile.currentNumberOfTips) {
			strcpy(i->text, systemSettings.Profile.tip[x].name);
			i->enabled = 1;
		}
		else
			i->enabled = 0;
		i = i->next_item;
	}
	comboBox_IRONTIPS.currentItem = comboBox_IRONTIPS.first;
	comboBox_IRONTIPS.currentScroll = 0;
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
	FillBuffer(BLACK, fill_dma);								                    // Manually clear the screen
	Screen_reset_confirmation.refresh=screenRefresh_alreadyErased;		      // Set to already cleared so it doesn't get erased automatically

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
}

//-------------------------------------------------------------------------------------------------------------------------------
// IRON screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void IRON_onEnter(screen_t *scr){
  setSettingsScrTempUnit();
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
		saveSettings(saveKeepingProfiles);															                    // Save settings
	}
	profile=systemSettings.settings.currentProfile;
}
void SYSTEM_onExit(screen_t *scr){
	if(profile!=systemSettings.settings.currentProfile){	                                // If profile changed
		loadProfile(profile);
		saveSettings(saveKeepingProfiles);									                                // Save
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void settings_screen_setup(screen_t *scr) {
	screen_t* sc;
	widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

	screen_setDefaults(scr);
	scr->init = &settings_screen_init;
	scr->onEnter = &settings_screen_OnEnter;
	scr->processInput=&settings_screenProcessInput;


	//#################################### [ SETTINGS MAIN SCREEN ]#######################################
	//
	w = &comboWidget_Settings;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_combo, &comboBox_Settings);
	comboAddScreen(&Settings_Combo_PID, w, 			  "PID", 			  screen_pid);
	comboAddScreen(&Settings_Combo_IRON, w, 		  "IRON", 		  screen_iron);
	comboAddScreen(&Settings_Combo_SYSTEM, w, 		"SYSTEM", 		screen_system);
	comboAddScreen(&Settings_Combo_TIPS, w, 		  "EDIT TIPS", 	screen_edit_iron_tips);
	comboAddScreen(&Settings_Combo_CALIBRATION, w,"CALIBRATION",screen_edit_calibration);
	#ifdef ENABLE_DEBUG_SCREEN
	comboAddScreen(&Settings_Combo_DEBUG, w, 		  "DEBUG", 		  screen_debug);
	#endif
	comboAddScreen(&Settings_Combo_EXIT, w, 		  "EXIT", 		  screen_main);


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
  dis=&editable_SYSTEM_Profile.inputData;
  edit=&editable_SYSTEM_Profile;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getProfile;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setProfile;
	edit->max_value = ProfileSize-1;
	edit->min_value = 0;
	edit->options = profileStr;
	edit->numberOfOptions = ProfileSize;

	//********[ Contrast Widget ]***********************************************************
	//
	dis=&editable_SYSTEM_Contrast.inputData;
  edit=&editable_SYSTEM_Contrast;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=3;
	dis->getData = &getContrast_;
	edit->big_step = 25;
	edit->step = 25;
	edit->setData = (void (*)(void *))&setContrast_;
	edit->max_value = 255;
	edit->min_value = 5;

	//********[ Oled Offset Widget ]***********************************************************
	//
	dis=&editable_SYSTEM_OledOffset.inputData;
	edit=&editable_SYSTEM_OledOffset;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=2;
	dis->getData = &getOledOffset;
	edit->big_step = 10;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setOledOffset;
	edit->max_value = 15;
	edit->min_value = 0;
	//********[ Wake mode Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_WakeMode.inputData;
  edit=&editable_SYSTEM_WakeMode;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getWakeMode;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setWakeMode;
	edit->max_value = wakeInputmode_stand;
	edit->min_value = wakeInputmode_shake;
	edit->options = wakeMode;
	edit->numberOfOptions = 2;


	//********[ Encoder inversion Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_EncoderMode.inputData;
  edit=&editable_SYSTEM_EncoderMode;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getEncoderMode;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setEncoderMode;
	edit->max_value = encoder_reverse;
	edit->min_value = encoder_normal;
	edit->options = encMode;
	edit->numberOfOptions = 2;

	//********[ Save Delay Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_SaveInterval.inputData;
  edit=&editable_SYSTEM_SaveInterval;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=3;
	dis->endString="s";
	dis->getData = &getSavDelay;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setSavDelay;
	edit->max_value = 60;
	edit->min_value = 1;

	//********[ Gui refresh rate Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_GuiUpd.inputData;
  edit=&editable_SYSTEM_GuiUpd;
  editableDefaultsInit(edit,widget_editable);
	dis->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getGuiUpd_ms;
	edit->big_step = 100;
	edit->step = 10;
	edit->setData = (void (*)(void *))&setGuiUpd_ms;
	edit->max_value = 500;
	edit->min_value = 0;

	//********[ Temp display unit Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_TempUnit.inputData;
  edit=&editable_SYSTEM_TempUnit;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getTmpUnit;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setTmpUnit;
	edit->max_value = mode_Farenheit;
	edit->min_value = mode_Celsius;
	edit->options = tempUnit;
	edit->numberOfOptions = 2;


	//********[ Temp step Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_TempStep.inputData;
  edit=&editable_SYSTEM_TempStep;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=4;
	dis->getData = &getTmpStep;
	edit->big_step = 5;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setTmpStep;
	edit->max_value = 50;
	edit->min_value = 1;

	//********[ Buzzer Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_Buzzer.inputData;
  edit=&editable_SYSTEM_Buzzer;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getbuzzerMode;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setbuzzerMode;
	edit->max_value = 1;
	edit->min_value = 0;
	edit->options = OffOn;
	edit->numberOfOptions = 2;

	//********[ Init mode Widget ]***********************************************************
	//
  dis=&editable_SYSTEM_InitMode.inputData;
  edit=&editable_SYSTEM_InitMode;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getInitMode;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setInitMode;
	edit->max_value = mode_run;
	edit->min_value = mode_sleep;
	edit->options = InitMode;
	edit->numberOfOptions = 2;

  //********[ Encoder wake Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_ButtonWake.inputData;
  edit=&editable_SYSTEM_ButtonWake;
  editableDefaultsInit(edit,widget_multi_option);
  dis->getData = &getButtonWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setButtonWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options =OffOn;
  edit->numberOfOptions = 2;

  //********[ Shake wake Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_ShakeWake.inputData;
  edit=&editable_SYSTEM_ShakeWake;
  editableDefaultsInit(edit,widget_multi_option);
  dis->getData = &getShakeWake;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setShakeWake;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

	//========[ SYSTEM COMBO ]===========================================================
	//
	w = &comboWidget_SYSTEM;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo, &comboBox_SYSTEM);
	comboAddMultiOption(&comboitem_SYSTEM_Profile,w,      "Profile", 		&editable_SYSTEM_Profile);
	comboAddEditable(&comboitem_SYSTEM_Contrast,w, 		    "Contrast", 	&editable_SYSTEM_Contrast);
	comboAddEditable(&comboitem_SYSTEM_OledOffset, w,     "Offset", 	  &editable_SYSTEM_OledOffset);
	comboAddMultiOption(&comboitem_SYSTEM_EncoderMode, w, "Encoder",		&editable_SYSTEM_EncoderMode);
	comboAddMultiOption(&comboitem_SYSTEM_InitMode, w, 		"Boot", 	    &editable_SYSTEM_InitMode);
	comboAddMultiOption(&comboitem_SYSTEM_WakeMode, w, 		"Wake Mode", 	&editable_SYSTEM_WakeMode);
  comboAddMultiOption(&comboitem_SYSTEM_ButtonWake, w,  "Btn. wake",  &editable_SYSTEM_ButtonWake);
  comboAddMultiOption(&comboitem_SYSTEM_ShakeWake, w,   "Shake wake", &editable_SYSTEM_ShakeWake);
	comboAddMultiOption(&comboitem_SYSTEM_Buzzer, w, 		  "Buzzer", 		&editable_SYSTEM_Buzzer);
	comboAddMultiOption(&comboitem_SYSTEM_TempUnit, w, 		"Unit", 	    &editable_SYSTEM_TempUnit);
	comboAddEditable(&comboitem_SYSTEM_TempStep, w, 	    "Step",	      &editable_SYSTEM_TempStep);
	comboAddEditable(&comboitem_SYSTEM_GuiUpd, w, 		    "Gui time", 	&editable_SYSTEM_GuiUpd);
	comboAddEditable(&comboitem_SYSTEM_SaveInterval, w,   "Save time",	&editable_SYSTEM_SaveInterval);
	comboAddScreen(&comboitem_SYSTEM_Reset, w, 			      "RESET MENU",	screen_reset);
	comboAddScreen(&comboitem_SYSTEM_SW, w, 			        SWSTRING,		  -1);
	comboAddScreen(&comboitem_SYSTEM_HW, w, 			        HWSTRING,		  -1);
	comboAddScreen(&comboitem_SYSTEM_Back, w, 			      "BACK", 		  screen_settingsmenu);

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
	widgetDefaultsInit(w, widget_combo, &comboBox_RESET);
	comboAddAction(&comboitem_RESET_SETTINGS,w, 	    "Reset Settings", &goSettingsReset );
	comboAddAction(&comboitem_RESET_TIP,w, 			      "Reset Profile",  &goProfileReset );
	comboAddAction(&comboitem_RESET_ALLTIP,w, 		    "Reset Profiles", &goAllProfileReset );
	comboAddAction(&comboitem_RESET_EVERYTHING,w,     "Reset All", 	  	&goFactoryReset);
	comboAddScreen(&comboitem_RESET_Back,w, 		      "BACK", 			    screen_system);
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
	widgetDefaultsInit(w, widget_button, &button_Reset_OK);
	button_Reset_OK.displayString="RESET";
	w->posX = 0;
	w->posY = 48;
	w->width = 50;
	((button_widget_t*)w->content)->selectable.tab = 1;
	((button_widget_t*)w->content)->action = &doReset;

	// ********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_Reset_CANCEL;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_Reset_CANCEL);
	button_Reset_CANCEL.displayString="CANCEL";
	w->posX = 72;
	w->posY = 48;
	w->width = 56;
	((button_widget_t*)w->content)->selectable.tab = 0;
	((button_widget_t*)w->content)->action = &cancelReset;


	//########################################## [ PID SCREEN ] ##########################################
	//
	sc=&Screen_pid;
	oled_addScreen(sc,screen_pid);
	screen_setDefaults(sc);
	sc->onEnter = &PID_onEnter;
	sc->processInput=&settings_screenProcessInput;

	//********[ KP Widget]***********************************************************
	//
//	w = &Widget_PID_Kp;
	//widgetDefaultsInit(w, widget_editable, &editable_PID_Kp);



	dis = &editable_PID_Kp.inputData;
	edit = &editable_PID_Kp;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=6;
	dis->getData = &getKp;
	dis->number_of_dec = 2;
	edit->max_value=65000;
	edit->big_step = 500;
	edit->step = 50;
	edit->setData =  (void (*)(void *))&setKp;

	//********[ KI Widget ]***********************************************************
	//
  dis = &editable_PID_Ki.inputData;
  edit = &editable_PID_Ki;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=6;
	dis->getData = &getKi;
	dis->number_of_dec = 2;
	edit->max_value=65000;
	edit->big_step = 500;
	edit->step = 50;
	edit->setData = (void (*)(void *))&setKi;

  //********[ KD Widget ]***********************************************************
  //
  dis = &editable_PID_Kd.inputData;
  edit = &editable_PID_Kd;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=6;
  dis->getData = &getKd;
  dis->number_of_dec = 2;
  edit->max_value=65000;
  edit->big_step = 500;
  edit->step = 50;
  edit->setData = (void (*)(void *))&setKd;

  //********[ Imax Widget ]***********************************************************
  //
  dis = &editable_PID_Imax.inputData;
  edit = &editable_PID_Imax;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=6;
  dis->getData = &getImax;
  dis->number_of_dec = 2;
  edit->max_value=1000;
  edit->big_step = 20;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setImax;


  //********[ Imin Widget ]***********************************************************
  //
  dis = &editable_PID_Imin.inputData;
  edit = &editable_PID_Imin;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=6;
  dis->getData = &getImin;
  dis->number_of_dec = 2;
  edit->max_value = 0;
  edit->min_value = -1000;
  edit->big_step = -20;
  edit->step = -1;
  edit->setData = (void (*)(void *))&setImin;

	//========[ PID COMBO ]===========================================================
	//
	w = &comboWidget_PID;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo, &comboBox_PID);
	comboAddEditable(&comboitem_PID_KP, w, 	    "Kp", 	&editable_PID_Kp);
	comboAddEditable(&comboitem_PID_KI, w,  	  "Ki", 	&editable_PID_Ki);
  comboAddEditable(&comboitem_PID_KD, w,      "Kd",   &editable_PID_Kd);
  comboAddEditable(&comboitem_PID_Imax, w,    "Imax", &editable_PID_Imax);
  comboAddEditable(&comboitem_PID_Imin, w,    "Imin", &editable_PID_Imin);
	comboAddScreen(&comboitem_PID_Back, w, 	    "BACK", screen_settingsmenu);

	//########################################## IRON SCREEN ##########################################
	//
	sc=&Screen_iron;
	oled_addScreen(sc,screen_iron);
	screen_setDefaults(sc);
	sc->onEnter = &IRON_onEnter;
	sc->processInput=&settings_screenProcessInput;

	//********[ Sleep Time Widget ]***********************************************************
	//
  dis=&editable_IRON_SleepTime.inputData;
  edit=&editable_IRON_SleepTime;
  editableDefaultsInit(edit,widget_editable);
	dis->endString="min";
	dis->reservedChars=5;
	dis->getData = &getSleepTime;
	edit->big_step = 5;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setSleepTime;
	edit->max_value = 60;
	edit->min_value = 0;

	//********[ Power Widget ]***********************************************************
	//
  dis=&editable_IRON_Power.inputData;
  edit=&editable_IRON_Power;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=4;
	dis->endString="W";
	dis->getData = &getMaxPower;
	edit->big_step = 20;
	edit->step = 5;
	edit->setData = (void (*)(void *))&setMaxPower;
	edit->max_value = 500;
	edit->min_value = 5;

	//********[ Impedance Widget ]***********************************************************
	//
  dis=&editable_IRON_Impedance.inputData;
  edit=&editable_IRON_Impedance;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=4;
	dis->number_of_dec=1;
	//dis->endString="";
	dis->getData = &getTipImpedance;
	edit->big_step = 10;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setTipImpedance;
	edit->max_value = 160;
	edit->min_value = 10;

  //********[ Max Temp Widget ]***********************************************************
  //
  dis=&editable_IRON_MaxTemp.inputData;
  edit=&editable_IRON_MaxTemp;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=5;
  dis->getData = &getMaxTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 480;
  edit->setData = (void (*)(void *))&setMaxTemp;

  //********[ Min Temp Widget ]***********************************************************
  //
  dis=&editable_IRON_MinTemp.inputData;
  edit=&editable_IRON_MinTemp;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=5;
  dis->getData = &getMinTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 480;
  edit->setData = (void (*)(void *))&setMinTemp;

	//********[ ADC Limit Widget ]***********************************************************
	//
  dis=&editable_IRON_ADCLimit.inputData;
  edit=&editable_IRON_ADCLimit;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=4;
	dis->getData = &getNoIronADC;
	edit->big_step = 200;
	edit->step = 10;
	edit->setData = (void (*)(void *))&setNoIronADC;
	edit->max_value = 4100;
	edit->min_value = 200;

	//********[ PWM Period Widget ]***********************************************************
	//
  dis=&editable_IRON_PWMPeriod.inputData;
  edit=&editable_IRON_PWMPeriod;
  editableDefaultsInit(edit,widget_editable);
	dis->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getPWMPeriod;
	edit->big_step = 20;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setPWMPeriod;
	edit->max_value = 500;
	edit->min_value = 20;

	//********[ ADC Delay Widget ]***********************************************************
	//
  dis=&editable_IRON_ADCDelay.inputData;
  edit=&editable_IRON_ADCDelay;
  editableDefaultsInit(edit,widget_editable);
	dis->endString="mS";
	dis->reservedChars=5;
	dis->getData = &getPWMDelay;
	edit->big_step = 10;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setPWMDelay;
	edit->max_value = 200;
	edit->min_value = 1;

	//********[ Filter Mode Widget ]***********************************************************
	//
  dis=&editable_IRON_FilterMode.inputData;
  edit=&editable_IRON_FilterMode;
  editableDefaultsInit(edit,widget_multi_option);
	dis->getData = &getFilterMode;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setFilterMode;
	edit->max_value = filter_ema;
	edit->min_value = filter_avg;
	edit->options = filterMode;
	edit->numberOfOptions = 2;

	//********[ Filter Coefficient Widget ]***********************************************************
	//
  dis=&editable_IRON_filterFactor.inputData;
  edit=&editable_IRON_filterFactor;
  editableDefaultsInit(edit,widget_editable);
	dis->reservedChars=1;
	dis->getData = &getfilterFactor;
	edit->big_step = 1;
	edit->step = 1;
	edit->setData = (void (*)(void *))&setfilterFactor;
	edit->max_value = 4;
	edit->min_value = 1;

	//********[ No Iron Delay Widget ]***********************************************************
	//
  dis=&editable_IRON_errorDelay.inputData;
  edit=&editable_IRON_errorDelay;
  editableDefaultsInit(edit,widget_editable);
	dis->endString="mS";
	dis->reservedChars=5;
	dis->getData = &geterrorDelay;
	edit->big_step = 100;
	edit->step = 50;
	edit->setData = (void (*)(void *))&seterrorDelay;
	edit->max_value = 950;
	edit->min_value = 100;

	//========[ IRON COMBO ]===========================================================
	//
	w = &comboWidget_IRON;
	screen_addWidget(w, sc);
	widgetDefaultsInit(w, widget_combo, &comboBox_IRON);
  comboAddEditable(&comboitem_IRON_MaxTemp, w,        "Max temp",   &editable_IRON_MaxTemp);
  comboAddEditable(&comboitem_IRON_MinTemp, w,        "Min temp",   &editable_IRON_MinTemp);
	comboAddEditable(&comboitem_IRON_SleepTime, w,	    "Sleep", 		  &editable_IRON_SleepTime);
  comboAddEditable(&comboitem_IRON_Impedance, w,      "Heater ohm", &editable_IRON_Impedance);
	comboAddEditable(&comboitem_IRON_Power, w, 		      "Power",		  &editable_IRON_Power);
	comboAddEditable(&comboitem_IRON_PWMPeriod,w, 	    "PWM Time", 	&editable_IRON_PWMPeriod);
	comboAddEditable(&comboitem_IRON_ADCDelay, w,   	  "ADC Delay", 	&editable_IRON_ADCDelay);
	comboAddMultiOption(&comboitem_IRON_FilterMode, w,  "Filtering", 	&editable_IRON_FilterMode);
	comboAddEditable(&comboitem_IRON_filterFactor, w,   "Factor",		  &editable_IRON_filterFactor);
	comboAddEditable(&comboitem_IRON_ADCLimit, w, 	    "No iron",		&editable_IRON_ADCLimit);
	comboAddEditable(&comboitem_IRON_errorDelay, w, 	  "Detection",	&editable_IRON_errorDelay);
	comboAddScreen(&comboitem_IRON_Back, w, 		        "BACK", 		  screen_settingsmenu);


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
  w->content = &comboBox_IRONTIPS;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_combo, &comboBox_IRONTIPS);
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
	widgetDefaultsInit(w, widget_editable, &editable_IRONTIPS_Edit);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=TipCharSize-1;
	w->posX = 75;
	w->posY = 17;
	w->width = 46;
	dis->getData = &getTipStr;
	dis->number_of_dec = 0;
	dis->type = field_string;
	edit->big_step = 10;
	edit->step = 1;
	edit->selectable.tab = 0;
	edit->setData = (void (*)(void *))&setTipStr;
	

	//********[ Name Save Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Save;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_IRONTIPS_Save);
	button_IRONTIPS_Save.displayString="SAVE";
	w->posX = 0;
	w->posY = 48;
	w->width = 42;
  ((button_widget_t*)w->content)->selectable.tab = 3;
  ((button_widget_t*)w->content)->action = &saveTip;

	//********[ Name Back Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Back;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_IRONTIPS_Back);
	button_IRONTIPS_Back.displayString="BACK";
	w->posX = 86;
	w->posY = 48;
	w->width = 42;
	((button_widget_t*)w->content)->selectable.tab = 1;
	((button_widget_t*)w->content)->action = &cancelTip;

	//********[ Name Delete Button Widget ]***********************************************************
	//
	w = &Widget_IRONTIPS_Delete;
	screen_addWidget(w,sc);
	widgetDefaultsInit(w, widget_button, &button_IRONTIPS_Delete);
	button_IRONTIPS_Delete.displayString="DEL.";
	w->posX = 43 ;
	w->posY = 48;
	w->width = 42;
	((button_widget_t*)w->content)->selectable.tab = 2;
	((button_widget_t*)w->content)->action = &delTip;
}
