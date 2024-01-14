/*
 * settings_system_screen.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#include "system_screen.h"
#include "screen_common.h"

screen_t Screen_system;

static comboBox_item_t *comboitem_system_ButtonWakeMode;
static comboBox_item_t *comboitem_system_ShakeWakeMode;
static comboBox_item_t *comboitem_system_BootMode;


static editable_widget_t *editable_system_TempStep;
static editable_widget_t *editable_system_bigTempStep;
static editable_widget_t *editable_system_GuiTempDenoise;

#ifndef STM32F072xB
static bool clone_fix;
#endif

void update_System_menu(void){
  bool mode = (getProfileSettings()->WakeInputMode==mode_shake);
  comboitem_system_BootMode->enabled        = mode;
  comboitem_system_ShakeWakeMode->enabled   = mode;
  comboitem_system_ButtonWakeMode->enabled  = mode;
}

void updateTemperatureUnit(void){
  if(getSystemTempUnit()==mode_Farenheit){
    editable_system_TempStep->inputData.endString="\260F";
    editable_system_bigTempStep->inputData.endString="\260F";
    editable_system_GuiTempDenoise->inputData.endString="\260F";
    editable_system_GuiTempDenoise->max_value=70;
  }
  else{
    editable_system_TempStep->inputData.endString="\260C";
    editable_system_bigTempStep->inputData.endString="\260C";
    editable_system_GuiTempDenoise->inputData.endString="\260C";
    editable_system_GuiTempDenoise->max_value=20;
  }
}

//=========================================================
#ifdef ENABLE_DEBUG_SCREEN
static void * getDbgScr() {
  temp = getSystemSettings()->debugEnabled;
  return &temp;
}
static void setDbgScr(uint32_t *val) {
  getSystemSettings()->debugEnabled = *val;
}
#endif

int hwAction(widget_t *w, RE_Rotation_t input){
  static uint32_t last=0;
  if(input==Click){
    if((current_time-last)<500){
      screenSaver.enabled=1;
    }
    last=current_time;
  }
  return -1;
}

static void * getTmpUnit() {
  temp = getSystemTempUnit();
  return &temp;
}
static void setTmpUnit(uint32_t *val) {
  setSystemTempUnit(*val);
  updateTemperatureUnit();
}
//=========================================================
static void * getTmpStep() {
  temp = getSystemSettings()->tempStep;
  return &temp;
}
static void setTmpStep(uint32_t *val) {
  getSystemSettings()->tempStep = *val;
}
//=========================================================

static void * getBigTmpStep() {
  temp = getSystemSettings()->tempBigStep;
  return &temp;
}

static void setBigTmpStep(uint32_t *val) {
  getSystemSettings()->tempBigStep = *val;
}
//=========================================================

static void * getGuiTempDenoise() {
  temp = getSystemSettings()->guiTempDenoise;
  return &temp;
}
static void setGuiTempDenoise(uint32_t *val) {
  getSystemSettings()->guiTempDenoise = *val;
}
//=========================================================
static void * getActiveDetection() {
  temp = getSystemSettings()->activeDetection;
  return &temp;
}
static void setActiveDetection(uint32_t *val) {
  getSystemSettings()->activeDetection = * val;
}
//=========================================================
static void * getEncoderMode() {
  temp = getSystemSettings()->EncoderMode;
  return &temp;
}
static void setEncoderMode(uint32_t *val) {
  getSystemSettings()->EncoderMode = * val;
  RE_SetMode(&RE1_Data, getSystemSettings()->EncoderMode);
}
//=========================================================
static void * getGuiUpd_ms() {
  temp = getSystemSettings()->guiUpdateDelay;
  return &temp;
}
static void setGuiUpd_ms(uint32_t *val) {
  getSystemSettings()->guiUpdateDelay = *val;
}
//=========================================================
static void * getLVP() {
  temp = getSystemSettings()->lvp;
  return &temp;
}
static void setLVP(uint32_t *val) {
  getSystemSettings()->lvp = *val;
}
//=========================================================
static void * getbuzzerMode() {
  temp = getSystemSettings()->buzzerMode;
  return &temp;
}
static void setbuzzerMode(uint32_t *val) {
  getSystemSettings()->buzzerMode = *val;
}
//=========================================================
static void * getInitMode() {
  temp = getSystemSettings()->initMode;
  return &temp;
}
static void setInitMode(uint32_t *val) {
  getSystemSettings()->initMode = *val;
}
//=========================================================
static void * getLanguage() {
  temp = getSystemSettings()->language;
  return &temp;
}
static void setLanguage(uint32_t *val) {
  lang = *val;
  getSystemSettings()->language=*val;
}
//=========================================================
static void * getButtonWakeMode() {
  temp = getSystemSettings()->buttonWakeMode;
  return &temp;
}
static void setButtonWakeMode(uint32_t *val) {
  getSystemSettings()->buttonWakeMode = *val;
}
//=========================================================
static void * getShakeWakeMode() {
  temp = getSystemSettings()->shakeWakeMode;
  return &temp;
}
static void setShakeWakeMode(uint32_t *val) {
  getSystemSettings()->shakeWakeMode = *val;
}
//=========================================================
#ifndef STM32F072xB
static void * getCloneFix() {
  temp = clone_fix;
  return &temp;
}
static void setCloneFix(uint32_t *val) {
  clone_fix = *val;
}
#endif
//=========================================================
static void * getHasBattery() {
  temp = getSystemSettings()->hasBattery;
  return &temp;
}
static void setHasBattery(uint32_t *val) {
  getSystemSettings()->hasBattery = * val;
}
//=========================================================
static void system_onEnter(screen_t *scr){
#ifndef STM32F072xB
  clone_fix = getSystemSettings()->clone_fix;
#endif

  if(scr==&Screen_settings){
    comboResetIndex(Screen_system.current_widget);
    profile=getCurrentProfile();
  }
}

static void system_onExit(screen_t *scr){                                                       // Save when exiting the screen, we have freed up all the screen memory

#if defined ST7565
  if((scr != &Screen_display) && (scr != &Screen_system) && isSystemSettingsChanged()){
#else
  if((scr != &Screen_display) && (scr != &Screen_display_adv) && (scr != &Screen_system) && isSystemSettingsChanged()){    // Going to main screen?
#endif

    if(getSystemSettings()->hasBattery != getFlashSystemSettings()->hasBattery){            // Battery mode changed
      copy_bkp_data((getSystemSettings()->hasBattery) && 1);                                   // 0=ram_to_flash, 1=flash_to_ram
      loadProfile(getCurrentProfile());                                                 // Reload tip from current backup source
    }
#ifndef STM32F072xB
    if(getSystemSettings()->clone_fix != clone_fix){                                           // Clone fix needs rebooting
      getSystemSettings()->clone_fix = clone_fix;
      saveSettings(save_settings, do_reboot);
    }
    else
#endif
      saveSettings(save_settings, no_reboot);                                                          // Other settings changed, not requiring rebooting
  }
}

static void system_create(screen_t *scr){
  widget_t* w;
  displayOnly_widget_t* dis;
  editable_widget_t* edit;

  current_lang = lang;

  //  [ SYSTEM COMBO ]
  //
  newWidget(&w,widget_combo,scr,NULL);

  //  [ Language Widget ]
  //
  newComboMultiOption(w, strings[lang]._Language, &edit, NULL);
  edit->inputData.getData = &getLanguage;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setLanguage;
  edit->options = Langs;
  edit->numberOfOptions = LANGUAGE_COUNT;

  //  [ Boot mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Boot, &edit, &comboitem_system_BootMode);
  dis=&edit->inputData;
  dis->getData = &getInitMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setInitMode;
  edit->options = strings[lang].InitMode;
  edit->numberOfOptions = 3;

  //  [ Encoder wake mode  Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Button_Wake, &edit,&comboitem_system_ButtonWakeMode);
  dis=&edit->inputData;
  dis->getData = &getButtonWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setButtonWakeMode;
  edit->options = strings[lang].WakeModes;
  edit->numberOfOptions = 4;

  //  [ Shake wake mode Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Shake_Wake, &edit,&comboitem_system_ShakeWakeMode);
  dis=&edit->inputData;
  dis->getData = &getShakeWakeMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setShakeWakeMode;
  edit->options = strings[lang].WakeModes;
  edit->numberOfOptions = 4;

  //  [ Encoder inversion Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Encoder, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getEncoderMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setEncoderMode;
  edit->options = strings[lang].encMode;
  edit->numberOfOptions = 2;

  //  [ Buzzer Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Buzzer, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getbuzzerMode;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setbuzzerMode;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Temp display unit Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Temperature, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getTmpUnit;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setTmpUnit;
  edit->options = tempUnit;
  edit->numberOfOptions = 2;

  //  [ Temp step Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM__Step, &edit, NULL);
  editable_system_TempStep=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getTmpStep;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (setterFn)&setTmpStep;
  edit->max_value = 50;
  edit->min_value = 1;

  // [ Temp big step Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM__Big_Step, &edit, NULL);
  editable_system_bigTempStep=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getBigTmpStep;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (setterFn)&setBigTmpStep;
  edit->max_value = 50;
  edit->min_value = 1;

  // [ De-noise threshold Widget ]
  //
  newComboEditable(w, strings[lang].FILTER__Threshold, &edit, NULL);
  editable_system_GuiTempDenoise=edit;
  dis=&edit->inputData;
  dis->reservedChars=4;
  dis->getData = &getGuiTempDenoise;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (setterFn)&setGuiTempDenoise;
  edit->max_value = 50;
  edit->min_value = 0;

  //  [ Active detection Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Active_Detection,&edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getActiveDetection;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setActiveDetection;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

  //  [ Low voltage protection Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_LVP, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="V";
  dis->reservedChars=5;
  dis->getData = &getLVP;
  dis->number_of_dec = 1;
  edit->big_step = 5;
  edit->step = 1;
  edit->setData = (setterFn)&setLVP;
  edit->max_value = 250;
  edit->min_value = 90;

  //  [ Gui refresh rate Widget ]
  //
  newComboEditable(w, strings[lang].SYSTEM_Gui_Time, &edit, NULL);
  dis=&edit->inputData;
  dis->endString="ms";
  dis->reservedChars=5;
  dis->getData = &getGuiUpd_ms;
  edit->big_step = 20;
  edit->step = 10;
  edit->setData = (setterFn)&setGuiUpd_ms;
  edit->max_value = 250;
  edit->min_value = 20;

  //  [ Battery Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_Battery,&edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getHasBattery;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setHasBattery;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;

#ifdef ENABLE_DEBUG_SCREEN
  //  [ Debug enable Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_DEBUG, &edit, NULL);
  dis=&edit->inputData;
  dis->getData = &getDbgScr;
  dis->reservedChars=3;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setDbgScr;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif

#ifndef STM32F072xB
  //  [ Clone fix Widget ]
  //
  newComboMultiOption(w, strings[lang].SYSTEM_CLONE_FIX, &edit, NULL);
  edit->inputData.getData = &getCloneFix;
  edit->big_step = 1;
  edit->step = 1;
  edit->setData = (setterFn)&setCloneFix;
  edit->options = strings[lang].OffOn;
  edit->numberOfOptions = 2;
#endif

  newComboScreen(w, strings[lang].SYSTEM_RESET_MENU, screen_reset, NULL);
  newComboScreen(w, strings[lang].SYSTEM_DISPLAY_MENU, screen_display, NULL);
  newComboScreen(w, SWSTRING, -1, NULL);
  newComboAction(w, HWSTRING, &hwAction, NULL);
  newComboScreen(w, strings[lang]._BACK, screen_settings, NULL);

  updateTemperatureUnit();
  update_System_menu();
}

int system_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){

  if(current_lang!=lang){                                                       // If language changed
    selectable_widget_t *sel = &((comboBox_widget_t*)scr->current_widget->content)->currentItem->widget->selectable;
    current_lang=lang;
    oled_backup_comboStatus(scr);
    oled_destroy_screen(scr);                                                   // Destroy and create the screen
    system_create(scr);
    oled_restore_comboStatus(scr);
    sel->state=widget_edit;
    sel->previous_state=widget_edit;
    scr->state = refresh_triggered;
  }
  return autoReturn_ProcessInput(scr, input, state);
}



void system_screen_setup(screen_t *scr){

  scr->onEnter = &system_onEnter;
  scr->onExit = &system_onExit;
  scr->processInput=&system_ProcessInput;
  scr->create = &system_create;
}
