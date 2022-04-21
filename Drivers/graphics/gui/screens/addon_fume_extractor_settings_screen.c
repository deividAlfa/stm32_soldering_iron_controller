/*
 * addons_screen.c
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#include "addon_fume_extractor_settings_screen.h"
#include "gui_strings.h"
#include "screen_common.h"

#ifdef ENABLE_ADDON_FUME_EXTRACTOR

screen_t Screen_fume_extractor_settings;

static comboBox_item_t* comboitem_fume_extractor_after_run_widget;

static void* getFumeExtractorMode();
static void  setFumeExtractorMode(uint32_t *val);
static void* getAfterRunDelay();
static void  setAfterRunDelay(uint32_t *val);
static void  update_addons_screen_fume_extractor(void);
static void  addons_screen_fume_extractor_create(screen_t *scr);
static void  addons_screen_fume_extractor_init(screen_t *scr);

static void* getFumeExtractorMode()
{
  temp = systemSettings.addonSettings.fumeExtractorMode;
  return &temp;
}

static void setFumeExtractorMode(uint32_t *val)
{
  systemSettings.addonSettings.fumeExtractorMode = *val;
  update_addons_screen_fume_extractor();
}

static void* getAfterRunDelay()
{
  temp = systemSettings.addonSettings.fumeExtractorAfterrun * 5u;
  return &temp;
}

static void setAfterRunDelay(uint32_t *val)
{
  systemSettings.addonSettings.fumeExtractorAfterrun = (*val) / 5u;
}

static void update_addons_screen_fume_extractor(void)
{
  comboitem_fume_extractor_after_run_widget->enabled = systemSettings.addonSettings.fumeExtractorMode == fume_extractor_mode_auto;
}

static void addons_screen_fume_extractor_create(screen_t *scr)
{
  widget_t* w;
  editable_widget_t* edit;

  newWidget(&w,widget_combo,scr);

  // Mode
  newComboMultiOption(w, strings[lang].FUME_EXTRACTOR_Mode, &edit, NULL);
  edit->inputData.getData       = &getFumeExtractorMode;
  edit->big_step                = 1;
  edit->step                    = 1;
  edit->setData                 = (setterFn)&setFumeExtractorMode;
  edit->options                 = strings[lang].FUME_EXTRACTOR_Modes;
  edit->numberOfOptions         = sizeof(strings[0].FUME_EXTRACTOR_Modes) / sizeof(char*);

  // After Run delay
  newComboEditable(w, strings[lang].FUME_EXTRACTOR_AfterRun, &edit, &comboitem_fume_extractor_after_run_widget);
  edit->inputData.reservedChars = 4u;
  edit->inputData.endString     = strings[lang].FUME_EXTRACTOR_AfterRunUnit;
  edit->inputData.getData       = &getAfterRunDelay;
  edit->big_step                = 30;
  edit->step                    = 5;
  edit->setData                 = (setterFn)&setAfterRunDelay;
  edit->max_value               = 600;
  edit->min_value               = 0;

  // Back
  newComboScreen(w, strings[lang]._BACK, screen_addons, NULL);

  update_addons_screen_fume_extractor();
}

static void addons_screen_fume_extractor_init(screen_t *scr)
{
  default_init(scr);
  comboResetIndex(Screen_fume_extractor_settings.current_widget);
}

void addons_screen_fume_extractor_setup(screen_t *scr)
{
  scr->create       = &addons_screen_fume_extractor_create;
  scr->init         = &addons_screen_fume_extractor_init;
  scr->processInput = &autoReturn_ProcessInput;
}

#endif
