/*
 * debug_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
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
static int32_t temp, settingsTimer;
static uint8_t profile, Selected_Tip;
static bool disableTipCopy;
typedef enum resStatus_t{ reset_settings, reset_profile, reset_profiles, reset_all }resStatus_t;
resStatus_t resStatus;
//static char TipName[5];
static tipData tipCfg;

//-------------------------------------------------------------------------------------------------------------------------------
// Settings screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
screen_t Screen_settingsmenu;
screen_t Screen_iron;
screen_t Screen_system;
screen_t Screen_reset;
screen_t Screen_reset_confirmation;
screen_t Screen_iron_tips;
screen_t Screen_edit_tip_settings;

// SETTINGS SCREEM
static widget_t comboWidget_Settings;
static comboBox_widget_t comboBox_Settings;
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
static comboBox_item_t comboitem_SYSTEM_OledDimming;
static comboBox_item_t comboitem_SYSTEM_OledOffset;
static comboBox_item_t comboitem_SYSTEM_WakeMode;
static comboBox_item_t comboitem_SYSTEM_StandMode;
static comboBox_item_t comboitem_SYSTEM_EncoderMode;
static comboBox_item_t comboitem_SYSTEM_TempUnit;
static comboBox_item_t comboitem_SYSTEM_TempStep;
static comboBox_item_t comboitem_SYSTEM_GuiUpd;
static comboBox_item_t comboitem_save_FlagInterval;
static comboBox_item_t comboitem_SYSTEM_Buzzer;
static comboBox_item_t comboitem_SYSTEM_LVP;
static comboBox_item_t comboitem_SYSTEM_InitMode;
static comboBox_item_t comboitem_SYSTEM_ButtonWake;
static comboBox_item_t comboitem_SYSTEM_ShakeWake;
static comboBox_item_t comboitem_SYSTEM_ActiveDetection;
static comboBox_item_t comboitem_SYSTEM_Reset;
static comboBox_item_t comboitem_SYSTEM_SW;
static comboBox_item_t comboitem_SYSTEM_HW;
static comboBox_item_t comboitem_SYSTEM_Back;

static editable_widget_t editable_SYSTEM_Profile;
static editable_widget_t editable_SYSTEM_Contrast;
static editable_widget_t editable_SYSTEM_OledDimming;
static editable_widget_t editable_SYSTEM_OledOffset;
static editable_widget_t editable_SYSTEM_WakeMode;
static editable_widget_t editable_SYSTEM_StandMode;
static editable_widget_t editable_SYSTEM_EncoderMode;
static editable_widget_t editable_SYSTEM_TempUnit;
static editable_widget_t editable_SYSTEM_TempStep;
static editable_widget_t editable_SYSTEM_GuiUpd;
static editable_widget_t editable_save_FlagInterval;
static editable_widget_t editable_SYSTEM_Buzzer;
static editable_widget_t editable_SYSTEM_LVP;
static editable_widget_t editable_SYSTEM_InitMode;
static editable_widget_t editable_SYSTEM_ButtonWake;
static editable_widget_t editable_SYSTEM_ShakeWake;
static editable_widget_t editable_SYSTEM_ActiveDetection;


// IRON SETTINGS
static widget_t comboWidget_IRON;
static comboBox_widget_t comboBox_IRON;
static comboBox_item_t comboitem_IRON_SleepTime;
static comboBox_item_t comboitem_IRON_StandbyTime;
static comboBox_item_t comboitem_IRON_StandbyTemp;
#ifdef USE_VIN
static comboBox_item_t comboitem_IRON_Power;
static comboBox_item_t comboitem_IRON_Impedance;
#endif
static comboBox_item_t comboitem_IRON_MaxTemp;
static comboBox_item_t comboitem_IRON_MinTemp;
static comboBox_item_t comboitem_IRON_PWMPeriod;
static comboBox_item_t comboitem_IRON_ADCDelay;
static comboBox_item_t comboitem_IRON_filterFactor;
static comboBox_item_t comboitem_IRON_errorDelay;
static comboBox_item_t comboitem_IRON_ADCLimit;
static comboBox_item_t comboitem_IRON_Back;

static editable_widget_t editable_IRON_SleepTime;
static editable_widget_t editable_IRON_StandbyTime;
static editable_widget_t editable_IRON_StandbyTemp;
#ifdef USE_VIN
static editable_widget_t editable_IRON_Power;
static editable_widget_t editable_IRON_Impedance;
#endif
static editable_widget_t editable_IRON_MaxTemp;
static editable_widget_t editable_IRON_MinTemp;
static editable_widget_t editable_IRON_ADCLimit;
static editable_widget_t editable_IRON_PWMPeriod;
static editable_widget_t editable_IRON_ADCDelay;
static editable_widget_t editable_IRON_filterFactor;
static editable_widget_t editable_IRON_errorDelay;

// EDIT TIPS SCREEN
static widget_t comboWidget_IRONTIPS;
static comboBox_widget_t comboBox_IRONTIPS;
static comboBox_item_t comboitem_IRONTIPS[TipSize];
static comboBox_item_t comboitem_IRONTIPS_addNewTip;
static comboBox_item_t comboitem_IRONTIPS_Back;

// EDIT TIP SETTINGS SCREEN
static widget_t comboWidget_IRONTIPS_Settings;
static comboBox_widget_t comboBox_IRONTIPS_Settings;
static comboBox_item_t comboitem_IRONTIPS_Settings_TipLabel;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_KP;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_KI;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_KD;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_Imax;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_Imin;
static comboBox_item_t comboitem_IRONTIPS_Settings_PID_tau;
static comboBox_item_t comboitem_IRONTIPS_Settings_Cal250;
static comboBox_item_t comboitem_IRONTIPS_Settings_Cal350;
static comboBox_item_t comboitem_IRONTIPS_Settings_Cal450;
static editable_widget_t editable_IRONTIPS_Settings_TipLabel;
static editable_widget_t editable_IRONTIPS_Settings_PID_Kd;
static editable_widget_t editable_IRONTIPS_Settings_PID_Ki;
static editable_widget_t editable_IRONTIPS_Settings_PID_Kp;
static editable_widget_t editable_IRONTIPS_Settings_PID_Imax;
static editable_widget_t editable_IRONTIPS_Settings_PID_Imin;
static editable_widget_t editable_IRONTIPS_Settings_PID_tau;
static editable_widget_t editable_IRONTIPS_Settings_Cal250;
static editable_widget_t editable_IRONTIPS_Settings_Cal350;
static editable_widget_t editable_IRONTIPS_Settings_Cal450;
static comboBox_item_t comboitem_IRONTIPS_Settings_Save;
static comboBox_item_t comboitem_IRONTIPS_Settings_Copy;
static comboBox_item_t comboitem_IRONTIPS_Settings_Delete;
static comboBox_item_t comboitem_IRONTIPS_Settings_Cancel;

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
    editable_IRON_StandbyTemp.inputData.endString="\260F";
  }
  else{
    editable_SYSTEM_TempStep.inputData.endString="\260C";
    editable_IRON_MaxTemp.inputData.endString="\260C";
    editable_IRON_MinTemp.inputData.endString="\260C";
    editable_IRON_StandbyTemp.inputData.endString="\260C";
  }
}

static void *getTipName() {
  return tipCfg.name;
}

static void setTipName(char *s) {
  strcpy(tipCfg.name, s);
}

#ifdef USE_VIN
static void * getMaxPower() {
  temp = systemSettings.Profile.power;
  return &temp;
}
static void setMaxPower(uint32_t *val) {
  systemSettings.Profile.power = *val;
}

static void * getTipImpedance() {
  temp = systemSettings.Profile.impedance;
  return &temp;
}
static void setTipImpedance(uint32_t *val) {
  systemSettings.Profile.impedance = *val;
}
#endif

static void setSleepTime(uint32_t *val) {
  systemSettings.Profile.sleepTimeout= *val;
}
static void * getSleepTime() {
  temp = systemSettings.Profile.sleepTimeout;
  return &temp;
}


static void setStandbyTime(uint32_t *val) {
  systemSettings.Profile.standbyTimeout= *val;
}
static void * getStandbyTime() {
  temp = systemSettings.Profile.standbyTimeout;
  return &temp;
}

static void setStandbyTemp(uint32_t *val) {
  systemSettings.Profile.standbyTemperature= *val;
}
static void * getStandbyTemp() {
  temp = systemSettings.Profile.standbyTemperature;
  return &temp;
}
static void * getKp() {
  temp = tipCfg.PID.Kp;
  return &temp;
}
static void setKp(int32_t *val) {
  tipCfg.PID.Kp = *val;
}
static void * getKi() {
  temp = tipCfg.PID.Ki;
  return &temp;
}
static void setKi(int32_t *val) {
  tipCfg.PID.Ki = *val;
}
static void * getKd() {
  temp = tipCfg.PID.Kd;
  return &temp;
}
static void setKd(int32_t *val) {
  tipCfg.PID.Kd = *val;
}
static void * getImax() {
  temp = tipCfg.PID.maxI;
  return &temp;
}
static void setImax(int32_t *val) {
  tipCfg.PID.maxI = *val;
}
static void * getImin() {
  temp = tipCfg.PID.minI;
  return &temp;
}
static void setImin(int32_t *val) {
  tipCfg.PID.minI= *val;
}
static void * getTau() {
  temp = tipCfg.PID.tau;
  return &temp;
}
static void setTau(int32_t *val) {
  tipCfg.PID.tau= *val;
}
static void * getCal250() {
  temp = tipCfg.calADC_At_250;
  return &temp;
}
static void setCal250(int32_t *val) {
  tipCfg.calADC_At_250 = *val;
}
static void * getCal350() {
  temp = tipCfg.calADC_At_350;
  editable_IRONTIPS_Settings_Cal350.min_value = tipCfg.calADC_At_250 + 1;
  editable_IRONTIPS_Settings_Cal350.max_value = tipCfg.calADC_At_450 - 1;
  return &temp;
}
static void setCal350(int32_t *val) {
  tipCfg.calADC_At_350 = *val;
}
static void * getCal450() {
  temp = tipCfg.calADC_At_450;
  editable_IRONTIPS_Settings_Cal450.min_value = tipCfg.calADC_At_350 + 1;
  return &temp;
}
static void setCal450(int32_t *val) {
  tipCfg.calADC_At_450 = *val;
}

static int IRONTIPS_Save(widget_t *w) {
  __disable_irq();
  systemSettings.Profile.tip[Selected_Tip] = tipCfg;
  if(Selected_Tip==systemSettings.Profile.currentTip){
    setupPID(&tipCfg.PID);
    resetPID();
  }
  else if(Selected_Tip==systemSettings.Profile.currentNumberOfTips){
    systemSettings.Profile.currentNumberOfTips++;
  }
  __enable_irq();
  return comboitem_IRONTIPS_Settings_Cancel.action_screen;
}
static int IRONTIPS_Delete(widget_t *w) {
  char name[TipCharSize]=_BLANK_TIP;
  systemSettings.Profile.currentNumberOfTips--;                                                                 // Decrease the number of tips in the system


  for(int x = Selected_Tip; x < TipSize-1;x++) {                                                                // Overwrite selected tip and move the rest one position backwards
    systemSettings.Profile.tip[x] = systemSettings.Profile.tip[x+1];
  }

  for(int x = systemSettings.Profile.currentNumberOfTips; x < TipSize;x++) {                                    // Fill the unused tips with blank names
    strcpy(systemSettings.Profile.tip[x].name, name);
  }

  if(systemSettings.Profile.currentTip >= systemSettings.Profile.currentNumberOfTips){                          // Check the system tip is not pointing to a blank slot
    systemSettings.Profile.currentTip = systemSettings.Profile.currentNumberOfTips-1;                           // Select the previous tip in the list if so
  }
                                                                                                                // Skip tip settings (As tip is now deleted)
  return comboitem_IRONTIPS_Settings_Cancel.action_screen;                                                      // And return to main screen or system menu screen
}
static int IRONTIPS_Copy(widget_t *w) {
  Selected_Tip = systemSettings.Profile.currentNumberOfTips;                                                    //
  strcpy(tipCfg.name, _BLANK_TIP);
  comboitem_IRONTIPS_Settings_Delete.enabled=0;
  comboitem_IRONTIPS_Settings_Copy.enabled=0;
  comboitem_IRONTIPS_Settings_Save.enabled=0;
  comboResetIndex(&comboWidget_IRONTIPS_Settings);
  disableTipCopy=1;
  return -1;                                                                                                    // And return to main screen or system menu screen
}


static void * getPWMPeriod() {
  temp=(systemSettings.Profile.pwmPeriod+1)/100;
  return &temp;
}
static void setPWMPeriod(uint32_t *val) {
  uint16_t period=(*val*100)-1;
  if(period>systemSettings.Profile.pwmDelay+(ADC_MEASURE_TIME/10)){
    setPwmPeriod(period);
  }
  else{
    uint16_t t=(systemSettings.Profile.pwmDelay+((ADC_MEASURE_TIME/10)+2));
    setPwmPeriod(t);
  }
}

static void * getPWMDelay() {
  temp=(systemSettings.Profile.pwmDelay+1)/10;
  return &temp;
}
static void setPWMDelay(uint32_t *val) {
  uint16_t delay=(*val*10)-1;
  if(delay<(systemSettings.Profile.pwmPeriod-(ADC_MEASURE_TIME/10))){
    setPwmDelay(delay);
  }
  else{
    setPwmDelay(systemSettings.Profile.pwmPeriod-((ADC_MEASURE_TIME/10)+2));
  }
}

static void * getMaxTemp() {
  temp=systemSettings.Profile.MaxSetTemperature;
  editable_IRON_MinTemp.max_value = temp-1;
  editable_IRON_MaxTemp.min_value = systemSettings.Profile.MinSetTemperature+1;
  return &temp;
}
static void setMaxTemp(uint32_t *val) {
  systemSettings.Profile.MaxSetTemperature=*val;
}

static void * getMinTemp() {
  temp=systemSettings.Profile.MinSetTemperature;
  editable_IRON_MaxTemp.min_value = temp+1;
  editable_IRON_MinTemp.max_value = systemSettings.Profile.MaxSetTemperature-1;
  return &temp;
}

static void setMinTemp(uint32_t *val) {
  systemSettings.Profile.MinSetTemperature=*val;
}

static void * getTmpUnit() {
  temp = systemSettings.settings.tempUnit;
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  setSettingsScrTempUnit();
}

static void * getTmpStep() {
  temp = systemSettings.settings.tempStep;
  return &temp;
}
static void setTmpStep(uint32_t *val) {
  systemSettings.settings.tempStep = *val;
}

static void * getContrast_() {
  temp = systemSettings.settings.contrast;
  return &temp;
}
static void setContrast_(uint32_t *val) {
  systemSettings.settings.contrast=*val;
  setContrast(*val);
}

static void * getOledOffset() {
  temp = systemSettings.settings.OledOffset;
  return &temp;
}
static void setOledOffset(uint32_t *val) {
  systemSettings.settings.OledOffset= * val;
}

static void * getOledDimming() {
  temp = systemSettings.settings.screenDimming;
  return &temp;
}
static void setOledDimming(uint32_t *val) {
  systemSettings.settings.screenDimming = * val;
}


static void * getActiveDetection() {
  temp = systemSettings.settings.activeDetection;
  return &temp;
}
static void setActiveDetection(uint32_t *val) {
  systemSettings.settings.activeDetection = * val;
}

static void * getWakeMode() {
  temp = systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_ButtonWake.enabled = !systemSettings.settings.WakeInputMode;   // 0=shake, 1=stand
  comboitem_SYSTEM_ShakeWake.enabled = !systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_InitMode.enabled = !systemSettings.settings.WakeInputMode;
  comboitem_SYSTEM_StandMode.enabled = systemSettings.settings.WakeInputMode;
  return &temp;
}
static void setWakeMode(uint32_t *val) {
  systemSettings.settings.WakeInputMode = *val;
}

static void * getStandMode() {
  temp = systemSettings.settings.StandMode;
  return &temp;
}
static void setStandMode(uint32_t *val) {
  systemSettings.settings.StandMode = *val;
}

static void * getEncoderMode() {
  temp = systemSettings.settings.EncoderMode;
  RE_SetMode(&RE1_Data, systemSettings.settings.EncoderMode);
  return &temp;
}
static void setEncoderMode(uint32_t *val) {
  systemSettings.settings.EncoderMode = * val;
}

static void * getGuiUpd_ms() {
  temp = systemSettings.settings.guiUpdateDelay;
  return &temp;
}
static void setGuiUpd_ms(uint32_t *val) {
  systemSettings.settings.guiUpdateDelay = *val;

}

static void * getSavDelay() {
  temp =systemSettings.settings.saveSettingsDelay;
  return &temp;
}
static void setSavDelay(uint32_t *val) {
  systemSettings.settings.saveSettingsDelay = *val;
}

static void * getLVP() {
  temp = systemSettings.settings.lvp;
  return &temp;
}
static void setLVP(uint32_t *val) {
  systemSettings.settings.lvp = *val;
}

static void * geterrorDelay() {
  temp = systemSettings.settings.errorDelay;
  return &temp;
}
static void enableDelay(uint32_t *val) {
  systemSettings.settings.errorDelay = *val;
}

static void * getNoIronADC() {
  temp = systemSettings.Profile.noIronValue;
  return &temp;
}
static void setNoIronADC(uint32_t *val) {
  systemSettings.Profile.noIronValue = *val;
}
static void * getbuzzerMode() {
  temp = systemSettings.settings.buzzerMode;
  return &temp;
}
static void setbuzzerMode(uint32_t *val) {
  systemSettings.settings.buzzerMode = *val;
}
static void * getInitMode() {
  temp = systemSettings.settings.initMode;
  return &temp;
}
static void setInitMode(uint32_t *val) {
  systemSettings.settings.initMode = *val;
}

static void * getfilterFactor() {
  temp = systemSettings.Profile.filterFactor;
  return &temp;
}
static void setfilterFactor(uint32_t *val) {
  systemSettings.Profile.filterFactor = *val;
}

static void * getProfile() {
  temp = profile;
  return &temp;
}

static void setProfile(uint32_t *val) {
  profile=*val;
}


static int cancelReset(widget_t *w) {
  return screen_reset;
}
static int doReset(widget_t *w) {
  switch(resStatus){
    case reset_settings:                                                // Reset system settings
      saveSettingsFromMenu(reset_Settings);                        // Save settings preserving tip data
      break;
    case reset_profile:                                                 // Reset current profile
      saveSettingsFromMenu(reset_Profile);                         // Save settings preserving tip data
      break;
    case reset_profiles:                                                // Reset all Profiles
      saveSettingsFromMenu(reset_Profiles);                        // Save settings wiping all tip data
      break;
    case reset_all:                                                     // Reset everything
      saveSettingsFromMenu(reset_All);                             // Save settings wiping all tip data
      break;
    default:
      return screen_main;
    }
    return -1;
}

static int doSettingsReset(void) {
  resStatus=reset_settings;
  return screen_reset_confirmation;
}
static int doProfileReset(void) {
  resStatus=reset_profile;
  return screen_reset_confirmation;
}
static int doProfilesReset(void) {
  resStatus=reset_profiles;
  return screen_reset_confirmation;
}
static int doFactoryReset(void) {
  resStatus=reset_all;
  return screen_reset_confirmation;
}

static void setButtonWake(uint32_t *val) {
  systemSettings.settings.wakeOnButton = *val;
}
static void * getButtonWake() {
  temp = systemSettings.settings.wakeOnButton;
  return &temp;
}

static void setShakeWake(uint32_t *val) {
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

  if((input==LongClick) || (HAL_GetTick()-settingsTimer)>15000){
    return screen_main;
  }

  return default_screenProcessInput(scr, input, state);
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
  FillBuffer(BLACK, fill_dma);                                            // Manually clear the screen
  Screen_reset_confirmation.refresh=screenRefresh_alreadyErased;          // Set to already cleared so it doesn't get erased automatically

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
// Iron tips screen functions
//-------------------------------------------------------------------------------------------------------------------------------

static void iron_tips_screen_init(screen_t *scr) {
  comboBox_item_t *i = comboBox_IRONTIPS.first;
  for(int x = 0; x < TipSize; x++) {
    if(x < systemSettings.Profile.currentNumberOfTips) {
      i->text = systemSettings.Profile.tip[x].name;
      i->enabled = 1;
    }
    else
      i->enabled = 0;
    i = i->next_item;
  }
  comboitem_IRONTIPS_addNewTip.enabled = (systemSettings.Profile.currentNumberOfTips < TipSize);
}

static void iron_tips_screen_onEnter(screen_t *scr) {
  comboResetIndex(&comboWidget_IRONTIPS);
}

//-------------------------------------------------------------------------------------------------------------------------------
// IRONTIPS Settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void IRONTIPS_Settings_onEnter(screen_t *scr){
  settingsTimer=HAL_GetTick();
  bool new=0;
  disableTipCopy=0;
  comboResetIndex(&comboWidget_IRONTIPS_Settings);

  if(scr==&Screen_main){                                                                                // If coming from main, selected tip is current ti`p
    comboitem_IRONTIPS_Settings_Cancel.action_screen = screen_main;
    Selected_Tip = systemSettings.Profile.currentTip;
  }

  else if(scr==&Screen_iron_tips){                                                                 // If coming from tips menu
    if(comboBox_IRONTIPS.currentItem == &comboitem_IRONTIPS_addNewTip) {                                // If was Add New tip option
      for(uint8_t x = 0; x < TipSize; x++) {                                                            // Find first valid tip and store it
        if(strcmp(systemSettings.Profile.tip[x].name, _BLANK_TIP)!=0){
          Selected_Tip=x;
          new=1;
          break;
        }
      }
      if(!new){
        Error_Handler();                                                                                // This shouldn't happen
      }
    }
    else{
      for(uint8_t x = 0; x < TipSize; x++) {                                                            // Else, find the selected tip
        if(strcmp(comboBox_IRONTIPS.currentItem->text, systemSettings.Profile.tip[x].name)==0){
          Selected_Tip = x;
        }
      }
    }
    comboitem_IRONTIPS_Settings_Cancel.action_screen = screen_iron_tips;
  }

  tipCfg = systemSettings.Profile.tip[Selected_Tip];                                                      // Backup selected tip

  if(new){                                                                                                // If new tip selected
    strcpy(tipCfg.name, _BLANK_TIP);                                                                      // Set an empty name
    Selected_Tip=systemSettings.Profile.currentNumberOfTips;                                              // Selected tip is next position
    comboitem_IRONTIPS_Settings_Delete.enabled=0;                                                         // Cannot delete empty tip
    comboitem_IRONTIPS_Settings_Save.enabled=0;                                                           // Disabled until the name is changed
    disableTipCopy=1;                                                                                     // Cannot copy a new tip
  }
  else{
    if(systemSettings.Profile.currentNumberOfTips>1){                                                     // If more than 1 tip in the system, enable delete
      comboitem_IRONTIPS_Settings_Delete.enabled=1;
    }
    else{
      comboitem_IRONTIPS_Settings_Delete.enabled=0;
    }
    if(systemSettings.Profile.currentNumberOfTips<TipSize){                                               // If not full
      comboitem_IRONTIPS_Settings_Copy.enabled=1;                                                         // Existing tip, enable copy button;
    }
    else{
      disableTipCopy=1;
      comboitem_IRONTIPS_Settings_Copy.enabled=0;                                                         // No space left
    }
  }
}

int IRONTIPS_Settings_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  static uint32_t last_update=0;
  uint32_t currentTime=HAL_GetTick();
  bool update=0;
  if((input==LongClick) || (HAL_GetTick()-settingsTimer)>30000){
    return screen_main;
  }
  if(input!=Rotate_Nothing){
    settingsTimer=currentTime;
    if(editable_IRONTIPS_Settings_TipLabel.selectable.state==widget_edit){                                      // If tip name is being edited and there's encoder activity
      update=1;
    }
  }
  else if((currentTime-last_update)>99){
    last_update=currentTime;
    update=1;
  }
 if(update){
    bool enable=1;
    if(strcmp(tipCfg.name, _BLANK_TIP) == 0){                                                                   // Check that the name is not empty
      enable=0;                                                                                                 // If empty, disable save button
    }
    else{
      for(uint8_t x = 0; x < TipSize; x++) {                                                                    // Compare tip names with current edit
        if( (strcmp(tipCfg.name, systemSettings.Profile.tip[x].name) == 0) && x!=Selected_Tip ){                // If match is found, and it's not the tip being edited
          enable=0;                                                                                             // Disable save button
          break;
        }
      }
    }
    comboitem_IRONTIPS_Settings_Save.enabled=enable;
    if(!disableTipCopy){
      comboitem_IRONTIPS_Settings_Copy.enabled=enable;
    }
    else{
      comboitem_IRONTIPS_Settings_Copy.enabled=0;
    }
  }
  return default_screenProcessInput(scr, input, state);
}

//-------------------------------------------------------------------------------------------------------------------------------
// IRON settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void IRON_onEnter(screen_t *scr){
  setSettingsScrTempUnit();
  comboResetIndex(&comboWidget_IRON);

}

//-------------------------------------------------------------------------------------------------------------------------------
// SYSTEM settings screen functions
//-------------------------------------------------------------------------------------------------------------------------------
void SYSTEM_onEnter(screen_t *scr){
  setSettingsScrTempUnit();
  if(scr!=&Screen_reset){
    comboResetIndex(&comboWidget_SYSTEM);
  }
  if(ChecksumProfile(&systemSettings.Profile)!=systemSettings.ProfileChecksum){         // If there's unsaved profile data
    saveSettingsFromMenu(save_Settings);                                                // Save settings
  }
  profile=systemSettings.settings.currentProfile;
}
void SYSTEM_onExit(screen_t *scr){
  if(profile!=systemSettings.settings.currentProfile){                                  // If profile changed
    loadProfile(profile);
    saveSettingsFromMenu(save_Settings);                                                // Save
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

  comboAddScreen(&Settings_Combo_IRON, w,         "IRON",         screen_iron);
  comboAddScreen(&Settings_Combo_SYSTEM, w,       "SYSTEM",       screen_system);
  comboAddScreen(&Settings_Combo_TIPS, w,         "EDIT TIPS",    screen_iron_tips);
  comboAddScreen(&Settings_Combo_CALIBRATION, w,  "CALIBRATION",  screen_edit_calibration);
  #ifdef ENABLE_DEBUG_SCREEN
  comboAddScreen(&Settings_Combo_DEBUG, w,        "DEBUG",        screen_debug);
  #endif
  comboAddScreen(&Settings_Combo_EXIT, w,         "EXIT",         screen_main);


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
  edit->step = 1;
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

  //********[ Oled dimming Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_OledDimming.inputData;
  edit=&editable_SYSTEM_OledDimming;
  editableDefaultsInit(edit,widget_multi_option);
  dis->getData = &getOledDimming;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setOledDimming;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

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


  //********[ Stand mode Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_StandMode.inputData;
  edit=&editable_SYSTEM_StandMode;
  editableDefaultsInit(edit,widget_multi_option);
  dis->getData = &getStandMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandMode;
  edit->max_value = mode_standby;
  edit->min_value = mode_sleep;
  edit->options = InitMode;
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
  dis=&editable_save_FlagInterval.inputData;
  edit=&editable_save_FlagInterval;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=3;
  dis->endString="s";
  dis->getData = &getSavDelay;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setSavDelay;
  edit->max_value = 60;
  edit->min_value = 0;

  //********[ Gui refresh rate Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_GuiUpd.inputData;
  edit=&editable_SYSTEM_GuiUpd;
  editableDefaultsInit(edit,widget_editable);
  dis->endString="mS";
  dis->reservedChars=5;
  dis->getData = &getGuiUpd_ms;
  edit->big_step = 50;
  edit->step = 10;
  edit->setData = (void (*)(void *))&setGuiUpd_ms;
  edit->max_value = 500;
  edit->min_value = 20;

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
  edit->numberOfOptions = 3;

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

  //********[ Active detection Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_ActiveDetection.inputData;
  edit=&editable_SYSTEM_ActiveDetection;
  editableDefaultsInit(edit,widget_multi_option);
  dis->getData = &getActiveDetection;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setActiveDetection;
  edit->max_value = 1;
  edit->min_value = 0;
  edit->options = OffOn;
  edit->numberOfOptions = 2;

  //********[ Low voltage protection Widget ]***********************************************************
  //
  dis=&editable_SYSTEM_LVP.inputData;
  edit=&editable_SYSTEM_LVP;
  editableDefaultsInit(edit,widget_editable);
  dis->endString="V";
  dis->reservedChars=5;
  dis->getData = &getLVP;
  dis->number_of_dec = 1;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setLVP;
  edit->max_value = 240;
  edit->min_value = 90;
  //========[ SYSTEM COMBO ]===========================================================
  //
  w = &comboWidget_SYSTEM;
  screen_addWidget(w, sc);
  widgetDefaultsInit(w, widget_combo, &comboBox_SYSTEM);
  comboAddMultiOption(&comboitem_SYSTEM_Profile,w,          "Profile",    &editable_SYSTEM_Profile);
  comboAddEditable(&comboitem_SYSTEM_Contrast,w,            "Contrast",   &editable_SYSTEM_Contrast);
  comboAddMultiOption(&comboitem_SYSTEM_OledDimming,w,      "Auto dim",   &editable_SYSTEM_OledDimming);
  comboAddEditable(&comboitem_SYSTEM_OledOffset, w,         "Offset",     &editable_SYSTEM_OledOffset);
  comboAddMultiOption(&comboitem_SYSTEM_EncoderMode, w,     "Encoder",    &editable_SYSTEM_EncoderMode);
  comboAddMultiOption(&comboitem_SYSTEM_WakeMode, w,        "Wake Mode",  &editable_SYSTEM_WakeMode);
  comboAddMultiOption(&comboitem_SYSTEM_InitMode, w,        "Boot",       &editable_SYSTEM_InitMode);
  comboAddMultiOption(&comboitem_SYSTEM_StandMode, w,       "Stand idle", &editable_SYSTEM_StandMode);
  comboAddMultiOption(&comboitem_SYSTEM_ButtonWake, w,      "Btn. wake",  &editable_SYSTEM_ButtonWake);
  comboAddMultiOption(&comboitem_SYSTEM_ShakeWake, w,       "Shake wake", &editable_SYSTEM_ShakeWake);
  comboAddMultiOption(&comboitem_SYSTEM_ActiveDetection, w, "Active det.",&editable_SYSTEM_ActiveDetection);
  comboAddMultiOption(&comboitem_SYSTEM_Buzzer, w,          "Buzzer",     &editable_SYSTEM_Buzzer);
  comboAddMultiOption(&comboitem_SYSTEM_TempUnit, w,        "Unit",       &editable_SYSTEM_TempUnit);
  comboAddEditable(&comboitem_SYSTEM_TempStep, w,           "Step",       &editable_SYSTEM_TempStep);
  comboAddEditable(&comboitem_SYSTEM_LVP, w,                "LVP",        &editable_SYSTEM_LVP);
  comboAddEditable(&comboitem_SYSTEM_GuiUpd, w,             "Gui time",   &editable_SYSTEM_GuiUpd);
  comboAddEditable(&comboitem_save_FlagInterval, w,       "Save time",  &editable_save_FlagInterval);
  comboAddScreen(&comboitem_SYSTEM_Reset, w,                "RESET MENU", screen_reset);
  comboAddScreen(&comboitem_SYSTEM_SW, w,                   SWSTRING,     -1);
  comboAddScreen(&comboitem_SYSTEM_HW, w,                   HWSTRING,     -1);
  comboAddScreen(&comboitem_SYSTEM_Back, w,                 "BACK",       screen_settingsmenu);

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
  comboAddAction(&comboitem_RESET_SETTINGS,w,       "Reset Settings", &doSettingsReset );
  comboAddAction(&comboitem_RESET_TIP,w,            "Reset Profile",  &doProfileReset );
  comboAddAction(&comboitem_RESET_ALLTIP,w,         "Reset Profiles", &doProfilesReset );
  comboAddAction(&comboitem_RESET_EVERYTHING,w,     "Reset All",      &doFactoryReset);
  comboAddScreen(&comboitem_RESET_Back,w,           "BACK",           screen_system);
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

  //********[ Stby Time Widget ]***********************************************************
  //
  dis=&editable_IRON_StandbyTime.inputData;
  edit=&editable_IRON_StandbyTime;
  editableDefaultsInit(edit,widget_editable);
  dis->endString="min";
  dis->reservedChars=5;
  dis->getData = &getStandbyTime;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setStandbyTime;
  edit->max_value = 60;
  edit->min_value = 0;

  //********[ Stby Temp Widget ]***********************************************************
  //
  dis=&editable_IRON_StandbyTemp.inputData;
  edit=&editable_IRON_StandbyTemp;
  editableDefaultsInit(edit,widget_editable);
  dis->endString="C";
  dis->reservedChars=5;
  dis->getData = &getStandbyTemp;
  edit->big_step = 10;
  edit->step = 5;
  edit->max_value = 350;
  edit->min_value = 50;
  edit->setData = (void (*)(void *))&setStandbyTemp;

  #ifdef USE_VIN

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

  #endif

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
  edit->min_value = 50;
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
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setPWMPeriod;
  edit->max_value = 200;
  edit->min_value = 20;

  //********[ ADC Delay Widget ]***********************************************************
  //
  dis=&editable_IRON_ADCDelay.inputData;
  edit=&editable_IRON_ADCDelay;
  editableDefaultsInit(edit,widget_editable);
  dis->endString="mS";
  dis->reservedChars=7;
  dis->number_of_dec = 1;
  dis->getData = &getPWMDelay;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setPWMDelay;
  edit->max_value = 500;
  edit->min_value = 1;

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
  edit->max_value = 8;
  edit->min_value = 0;

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
  edit->setData = (void (*)(void *))&enableDelay;
  edit->max_value = 950;
  edit->min_value = 100;

  //========[ IRON COMBO ]===========================================================
  //
  w = &comboWidget_IRON;
  screen_addWidget(w, sc);
  widgetDefaultsInit(w, widget_combo, &comboBox_IRON);
  comboAddEditable(&comboitem_IRON_MaxTemp, w,      "Max temp",   &editable_IRON_MaxTemp);
  comboAddEditable(&comboitem_IRON_MinTemp, w,      "Min temp",   &editable_IRON_MinTemp);
  comboAddEditable(&comboitem_IRON_StandbyTemp, w,  "Stby temp",  &editable_IRON_StandbyTemp);
  comboAddEditable(&comboitem_IRON_StandbyTime, w,  "Stby time",  &editable_IRON_StandbyTime);
  comboAddEditable(&comboitem_IRON_SleepTime, w,    "Sleep time", &editable_IRON_SleepTime);
  #ifdef USE_VIN
  comboAddEditable(&comboitem_IRON_Impedance, w,    "Heater ohm", &editable_IRON_Impedance);
  comboAddEditable(&comboitem_IRON_Power, w,        "Power",      &editable_IRON_Power);
  #endif
  comboAddEditable(&comboitem_IRON_PWMPeriod,w,     "PWM",        &editable_IRON_PWMPeriod);
  comboAddEditable(&comboitem_IRON_ADCDelay, w,     "Delay",      &editable_IRON_ADCDelay);
  comboAddEditable(&comboitem_IRON_filterFactor, w, "Filter",     &editable_IRON_filterFactor);
  comboAddEditable(&comboitem_IRON_ADCLimit, w,     "No iron",    &editable_IRON_ADCLimit);
  comboAddEditable(&comboitem_IRON_errorDelay, w,   "Detect",     &editable_IRON_errorDelay);
  comboAddScreen(&comboitem_IRON_Back, w,           "BACK",       screen_settingsmenu);


  //########################################## IRON TIPS SCREEN ##########################################
  //
  sc=&Screen_iron_tips;
  oled_addScreen(sc, screen_iron_tips);
  screen_setDefaults(sc);
  sc->init = &iron_tips_screen_init;
  sc->processInput=&settings_screenProcessInput;
  sc->onEnter=&iron_tips_screen_onEnter;

  //========[ IRON TIPS COMBO ]===========================================================
  //
  w = &comboWidget_IRONTIPS;
  w->content = &comboBox_IRONTIPS;
  screen_addWidget(w,sc);
  widgetDefaultsInit(w, widget_combo, &comboBox_IRONTIPS);
  for(int x = 0; x < TipSize; x++) {
    comboAddScreen(&comboitem_IRONTIPS[x],w, " ", screen_edit_tip_settings);              // Names filled on menu enter
  }
  comboAddScreen(&comboitem_IRONTIPS_addNewTip, w,  "ADD NEW",  screen_edit_tip_settings);
  comboAddScreen(&comboitem_IRONTIPS_Back, w,       "BACK",     screen_settingsmenu);


  //########################################## [ TIP SETTINGS SCREEN ] ##########################################
  //
  sc=&Screen_edit_tip_settings;
  oled_addScreen(sc,screen_edit_tip_settings);
  screen_setDefaults(sc);
  sc->onEnter = &IRONTIPS_Settings_onEnter;
  sc->processInput=&IRONTIPS_Settings_ProcessInput;

  //********[ TIP label]***********************************************************
  //
  //comboitem_IRONTIPS_Settings_TipLabel.dispAlign = align_center;
  dis = &editable_IRONTIPS_Settings_TipLabel.inputData;
  edit = &editable_IRONTIPS_Settings_TipLabel;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=TipCharSize-1;
  dis->getData = &getTipName;
  dis->type = field_string;
  dis->displayString=tipCfg.name;
  edit->big_step = 10;
  edit->step = 1;
  edit->selectable.tab = 0;
  edit->setData = (void (*)(void *))&setTipName;

  //********[ KP Widget]***********************************************************
  //
  dis = &editable_IRONTIPS_Settings_PID_Kp.inputData;
  edit = &editable_IRONTIPS_Settings_PID_Kp;
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
  dis = &editable_IRONTIPS_Settings_PID_Ki.inputData;
  edit = &editable_IRONTIPS_Settings_PID_Ki;
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
  dis = &editable_IRONTIPS_Settings_PID_Kd.inputData;
  edit = &editable_IRONTIPS_Settings_PID_Kd;
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
  dis = &editable_IRONTIPS_Settings_PID_Imax.inputData;
  edit = &editable_IRONTIPS_Settings_PID_Imax;
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
  dis = &editable_IRONTIPS_Settings_PID_Imin.inputData;
  edit = &editable_IRONTIPS_Settings_PID_Imin;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=6;
  dis->getData = &getImin;
  dis->number_of_dec = 2;
  edit->max_value = 0;
  edit->min_value = -1000;
  edit->big_step = -20;
  edit->step = -1;
  edit->setData = (void (*)(void *))&setImin;


  //********[ Tau Widget ]***********************************************************
  //
  dis = &editable_IRONTIPS_Settings_PID_tau.inputData;
  edit = &editable_IRONTIPS_Settings_PID_tau;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=6;
  dis->getData = &getTau;
  dis->number_of_dec = 2;
  edit->max_value = 200;
  edit->min_value = 0;
  edit->big_step = 10;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setTau;


  //********[ Cal250 Widget ]***********************************************************
  //
  dis = &editable_IRONTIPS_Settings_Cal250.inputData;
  edit = &editable_IRONTIPS_Settings_Cal250;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=4;
  dis->getData = &getCal250;
  edit->max_value = 4095;
  edit->min_value = 0;
  edit->big_step = 50;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal250;


  //********[ Cal350 Widget ]***********************************************************
  //
  dis = &editable_IRONTIPS_Settings_Cal350.inputData;
  edit = &editable_IRONTIPS_Settings_Cal350;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=4;
  dis->getData = &getCal350;
  edit->max_value = 4095;
  edit->min_value = 0;
  edit->big_step = 50;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal350;


  //********[ Cal450 Widget ]***********************************************************
  //
  dis = &editable_IRONTIPS_Settings_Cal450.inputData;
  edit = &editable_IRONTIPS_Settings_Cal450;
  editableDefaultsInit(edit,widget_editable);
  dis->reservedChars=4;
  dis->getData = &getCal450;
  edit->max_value = 4095;
  edit->min_value = 0;
  edit->big_step = 50;
  edit->step = 1;
  edit->setData = (void (*)(void *))&setCal450;

  //========[ PID COMBO ]===========================================================
  //
  w = &comboWidget_IRONTIPS_Settings;
  screen_addWidget(w, sc);
  widgetDefaultsInit(w, widget_combo, &comboBox_IRONTIPS_Settings);
  comboAddEditable(&comboitem_IRONTIPS_Settings_TipLabel, w,  "Name",       &editable_IRONTIPS_Settings_TipLabel);        // Name set automatically on enter
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_KP,   w,  "PID Kp",     &editable_IRONTIPS_Settings_PID_Kp);
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_KI,   w,  "PID Ki",     &editable_IRONTIPS_Settings_PID_Ki);
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_KD,   w,  "PID Kd",     &editable_IRONTIPS_Settings_PID_Kd);
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_Imax, w,  "PID Imax",   &editable_IRONTIPS_Settings_PID_Imax);
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_Imin, w,  "PID Imin",   &editable_IRONTIPS_Settings_PID_Imin);
  comboAddEditable(&comboitem_IRONTIPS_Settings_PID_tau,  w,  "PID tau",    &editable_IRONTIPS_Settings_PID_tau);
  comboAddEditable(&comboitem_IRONTIPS_Settings_Cal250,   w,  "Cal250",     &editable_IRONTIPS_Settings_Cal250);
  comboAddEditable(&comboitem_IRONTIPS_Settings_Cal350,   w,  "Cal350",     &editable_IRONTIPS_Settings_Cal350);
  comboAddEditable(&comboitem_IRONTIPS_Settings_Cal450,   w,  "Cal450",     &editable_IRONTIPS_Settings_Cal450);
  comboAddAction(&comboitem_IRONTIPS_Settings_Save,       w,  "SAVE",       &IRONTIPS_Save);
  comboAddAction(&comboitem_IRONTIPS_Settings_Copy,       w,  "COPY",       &IRONTIPS_Copy);
  comboAddAction(&comboitem_IRONTIPS_Settings_Delete,     w,  "DELETE",     &IRONTIPS_Delete);
  comboAddScreen(&comboitem_IRONTIPS_Settings_Cancel,     w,  "CANCEL",     -1);                                               // Return value set automatically on enter
}
