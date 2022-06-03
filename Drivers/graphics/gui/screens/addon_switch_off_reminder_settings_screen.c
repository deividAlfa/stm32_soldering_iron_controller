/*
 * addons_screen.c
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#include "addon_SWITCH_OFF_REMINDER_settings_screen.h"
#include "gui_strings.h"
#include "screen_common.h"

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER

screen_t Screen_switch_off_reminder_settings;

static comboBox_item_t* comboitem_switch_off_reminder_inactivity_delay_widget;
static comboBox_item_t* comboitem_switch_off_reminder_reminder_period_widget;
static comboBox_item_t* comboitem_switch_off_reminder_beep_type_widget;

static void* getSwitchOffReminderEnabledState();
static void  setSwitchOffReminderEnabledState(uint32_t *val);
static void  addons_screen_switch_off_reminder_create(screen_t *scr);
static void  update_addons_screen_switch_off_reminder(void);
static void* getInactivityDelay();
static void  setInactivityDelay(uint32_t *val);
static void* getReminderPeriod();
static void  setReminderPeriod(uint32_t *val);

static void* getSwitchOffReminderEnabledState()
{
  temp = systemSettings.addonSettings.swOffReminderEnabled;
  return &temp;
}

static void setSwitchOffReminderEnabledState(uint32_t *val)
{
  systemSettings.addonSettings.swOffReminderEnabled=*val;
  update_addons_screen_switch_off_reminder();
}

static void* getInactivityDelay()
{
  temp = systemSettings.addonSettings.swOffReminderInactivityDelay;
  return &temp;
}

static void setInactivityDelay(uint32_t *val)
{
  systemSettings.addonSettings.swOffReminderInactivityDelay = *val;
}

static void* getReminderPeriod()
{
  temp = systemSettings.addonSettings.swOffReminderPeriod;
  return &temp;
}

static void setReminderPeriod(uint32_t *val)
{
  systemSettings.addonSettings.swOffReminderPeriod = *val;
}

static void* getBeepType()
{
  temp = systemSettings.addonSettings.swOffReminderBeepType;
  return &temp;
}

static void setBeepType(uint32_t *val)
{
  systemSettings.addonSettings.swOffReminderBeepType = *val;
}

static void update_addons_screen_switch_off_reminder(void)
{
  uint8_t const enabled = systemSettings.addonSettings.swOffReminderEnabled ? true : false;
  comboitem_switch_off_reminder_inactivity_delay_widget->enabled = enabled;
  comboitem_switch_off_reminder_reminder_period_widget ->enabled = enabled;
  comboitem_switch_off_reminder_beep_type_widget       ->enabled = enabled;
}

static void addons_screen_switch_off_reminder_create(screen_t *scr)
{
  widget_t* w;
  editable_widget_t* edit;

  newWidget(&w,widget_combo,scr);

  // On/off
  newComboMultiOption(w, strings[lang].SWITCH_OFF_REMINDER_EnableDisableOption, &edit, NULL);
  edit->inputData.getData       = &getSwitchOffReminderEnabledState;
  edit->big_step                = 1;
  edit->step                    = 1;
  edit->setData                 = (setterFn)&setSwitchOffReminderEnabledState;
  edit->options                 = strings[lang].OffOn;
  edit->numberOfOptions         = sizeof(strings[0].OffOn) / sizeof(char*);

  // Inactivity delay
  newComboEditable(w, strings[lang].SWITCH_OFF_REMINDER_InactivityDelay, &edit, &comboitem_switch_off_reminder_inactivity_delay_widget);
  edit->inputData.reservedChars = 4u;
  edit->inputData.endString     = strings[lang].SWITCH_OFF_REMINDER_TimeUnit;
  edit->inputData.getData       = &getInactivityDelay;
  edit->big_step                = 5;
  edit->step                    = 1;
  edit->setData                 = (setterFn)&setInactivityDelay;
  edit->max_value               = UINT8_MAX;
  edit->min_value               = 1;

  // Reminder period
  newComboEditable(w, strings[lang].SWITCH_OFF_REMINDER_ReminderPeriod, &edit, &comboitem_switch_off_reminder_reminder_period_widget);
  edit->inputData.reservedChars = 4u;
  edit->inputData.endString     = strings[lang].SWITCH_OFF_REMINDER_TimeUnit;
  edit->inputData.getData       = &getReminderPeriod;
  edit->big_step                = 5;
  edit->step                    = 1;
  edit->setData                 = (setterFn)&setReminderPeriod;
  edit->max_value               = UINT8_MAX;
  edit->min_value               = 1;

  // Beep type
  newComboMultiOption(w, strings[lang].SWITCH_OFF_REMINDER_BeepType, &edit, &comboitem_switch_off_reminder_beep_type_widget);
  edit->inputData.getData       = &getBeepType;
  edit->big_step                = 1;
  edit->step                    = 1;
  edit->setData                 = (setterFn)&setBeepType;
  edit->options                 = strings[lang].SWITCH_OFF_REMINDER_BeepTypes;
  edit->numberOfOptions         = sizeof(strings[0].SWITCH_OFF_REMINDER_BeepTypes) / sizeof(char*);

  // Back
  newComboScreen(w, strings[lang]._BACK, screen_addons, NULL);

  update_addons_screen_switch_off_reminder();
}

static void addons_screen_switch_off_reminder_init(screen_t *scr)
{
  default_init(scr);
  comboResetIndex(Screen_switch_off_reminder_settings.current_widget);
}

void addons_screen_switch_off_reminder_setup(screen_t *scr)
{
  scr->create       = &addons_screen_switch_off_reminder_create;
  scr->init         = &addons_screen_switch_off_reminder_init;
  scr->processInput = &autoReturn_ProcessInput;
}

#endif
