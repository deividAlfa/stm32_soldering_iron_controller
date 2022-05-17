/*
 * screen_common.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */
#include "screen_common.h"

int32_t temp;
int16_t backupTemp;
uint8_t status, profile, Selected_Tip, lang, backupMode, current_lang=lang_english;
tipData_t backupTip;

struct{
  int8_t step;
  uint8_t min_reached;
  uint32_t timer;
  uint32_t stepTimer;
}dim;

uint8_t newTip;

plotData_t plot;

// Plot graph data update and drawing timer
void updatePlot(void){
  if(getIronErrorFlags().active){
    return;
  }

  if(plot.timeStep<20){ plot.timeStep = 20; }
  if((current_time-plot.timer)>=plot.timeStep){                                          // Only store values if running
    plot.update=plot.enabled;
    plot.timer=current_time;
    plot.d[plot.index] = last_TIP_C;
    if(++plot.index>99){
      plot.index=0;
    }
  }
}

int longClickReturn(widget_t *w){
  selectable_widget_t *sel=NULL;
  if(w->type!=widget_combo){
    extractSelectablePartFromWidget(w);
  }
  else{
    comboBox_item_t *combo =  ((comboBox_widget_t*)w->content)->currentItem;
    if((combo->type == combo_Editable) || (combo->type == combo_MultiOption)){
      sel = &combo->widget->selectable;
    }
  }
  if(!sel || (sel && (sel->state!=widget_edit))){
      return screen_main;
  }
  return -1;
}

uint8_t update_GUI_Timer(void){
  static uint32_t guiTimer=0;
  if((current_time-guiTimer)>=systemSettings.settings.guiUpdateDelay){
    guiTimer=current_time;
    return 1;
  }
  return 0;
}

int autoReturn_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){
  updatePlot();
  wakeOledDim();
  handleOledDim();
  updateScreenTimer(input);

  if(input==Rotate_Decrement_while_click){
    if(scr==&Screen_settings){
      return screen_main;
    }
    else if(scr->current_widget->type==widget_combo){
      comboBox_item_t *item = ((comboBox_widget_t*)scr->current_widget->content)->currentItem;
      if(item->type==combo_Editable || item->type==combo_MultiOption){
        if(item->widget->selectable.state!=widget_edit){
          return screen_settings;
        }
      }
      else{
        return screen_settings;
      }
    }
  }
  if(input==LongClick){
    int x = longClickReturn(scr->current_widget);
    if (x!=-1){
      return x;
    }
  }

  if(checkScreenTimer(15000)){
    return screen_main;
  }
  return default_screenProcessInput(scr, input, state);
}

void resetScreenTimer(void){
  screen_timer=current_time;
}

void updateScreenTimer(RE_Rotation_t input){
  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
}
uint8_t checkScreenTimer(uint32_t time){
  if((current_time-screen_timer)>time){
    return 1;
  }
  return 0;
}
void restore_contrastOrBrightness(void){
#ifndef ST7565
  if(getDisplayContrastOrBrightness() != systemSettings.settings.contrastOrBrightness){
    setDisplayContrastOrBrightness(systemSettings.settings.contrastOrBrightness);
  }
#endif
}

void wakeOledDim(void){
#ifndef ST7565
  dim.timer = current_time;
  if(dim.step<=0 && getDisplayContrastOrBrightness()<systemSettings.settings.contrastOrBrightness ){
    if(getDisplayPower()==disable){
      setDisplayPower(enable);
    }
    dim.step=10;
    dim.min_reached=0;
  }
#endif
}

void handleOledDim(void){
#ifndef ST7565
  uint16_t brightness=getDisplayContrastOrBrightness();
  if(!getDisplayPower() && getCurrentMode()>mode_sleep){                   // If screen turned off and not in sleep mode, wake it.
    wakeOledDim();                                                   		// (Something woke the station from sleep)
  }

  if(dim.step==0){                                                      // If idle
    if(systemSettings.settings.dim_mode==dim_off){                      // Return if dimmer is disabled
      return;
    }
    // If idle timer expired, start decreasing brightness
    if(brightness>5 && ((current_time-dim.timer)>systemSettings.settings.dim_Timeout)){
      dim.step=-5;
    }
    // If min. brightness reached and Oled power is disabled in sleep mode, turn off screen if temp<100ºC or error active
    else if(dim.min_reached && getCurrentMode()==mode_sleep && systemSettings.settings.dim_inSleep==disable && (last_TIP_C<100 || (getIronErrorFlags().active))){
      setDisplayPower(disable);
      dim.min_reached=0;
    }
  }
  // Smooth screen brightness dimming
  else if((current_time-dim.stepTimer)>19){
    dim.stepTimer = current_time;
    brightness+=dim.step;
    if(brightness>4 && brightness<systemSettings.settings.contrastOrBrightness){
      setDisplayContrastOrBrightness(brightness);
    }
    else{
      if(dim.step>0){
        restore_contrastOrBrightness();
      }
      else{
        setDisplayContrastOrBrightness(1);
        dim.min_reached=1;
      }
      dim.timer = current_time;
      dim.step=0;
    }
  }
#endif
}

