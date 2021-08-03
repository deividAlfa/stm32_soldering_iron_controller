/*
 * screen_common.c
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */
#include "screen_common.h"

int32_t temp, temp2, temp3;
uint8_t status, profile, Selected_Tip;
char *tipName;
bool disableTipCopy;
bool newTip;

plotData_t plot;

// Plot graph data update and drawing timer
void updatePlot(void){
  if(Iron.Error.Flags & _ACTIVE){
    return;
  }

  int16_t current_temp = readTipTemperatureCompensated(stored_reading,read_Avg);

  if(systemSettings.settings.tempUnit==mode_Farenheit){
    current_temp = TempConversion(current_temp, mode_Celsius, 0);
  }
  if(plot.timeStep<20){ plot.timeStep = 20; }
  if((current_time-plot.timer)>=plot.timeStep){                                          // Only store values if running
    plot.update=plot.enabled;
    plot.timer=current_time;
    plot.d[plot.index] = current_temp;
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

int autoReturn_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state){
  updatePlot();
  if(input!=Rotate_Nothing){
    screen_timer=current_time;
  }
  if(input==LongClick){
    int x = longClickReturn(scr->current_widget);
    if (x!=-1){
      return x;
    }
  }

  if((current_time-screen_timer)>15000){
    return screen_main;
  }
  return default_screenProcessInput(scr, input, state);
}
