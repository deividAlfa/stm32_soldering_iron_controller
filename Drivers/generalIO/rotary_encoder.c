/*
 * rotary_encoder.c

 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "rotary_encoder.h"
#include "settings.h"
volatile RE_State_t RE1_Data;

/* Return with status macro */
#define RETURN_WITH_STATUS(p, s) (p)->Rotation = s; return s

void RE_Init(RE_State_t* data, GPIO_TypeDef* GPIO_A_Port, uint16_t GPIO_A_Pin, GPIO_TypeDef* GPIO_B_Port, uint16_t GPIO_B_Pin, GPIO_TypeDef* GPIO_BUTTON_Port, uint16_t GPIO_BUTTON_Pin) {
  /* Save parameters */
  data->GPIO_A = GPIO_A_Port;
  data->GPIO_B = GPIO_B_Port;
  data->GPIO_PIN_A = GPIO_A_Pin;
  data->GPIO_PIN_B = GPIO_B_Pin;
  data->GPIO_BUTTON = GPIO_BUTTON_Port;
  data->GPIO_PIN_BUTTON = GPIO_BUTTON_Pin;
  data->pv_click = RE_BT_HIDLE;

  /* Set default mode */
  data->Mode = systemSettings.settings.EncoderMode;

  /* Set default */
  data->RE_Count = 0;
  data->Diff = 0;
  data->Absolute = 0;
  data->LastA = 1;
  data->halfPointReached = 0;
}

RE_Rotation_t RE_Get(RE_State_t* data) {
  /* Calculate everything */
  data->Diff = data->RE_Count - data->Absolute;
  data->Absolute += data->Diff;

  /* Check */
  if(data->pv_click == RE_BT_CLICKED) {
    data->pv_click = RE_BT_UNRELEASED;
    RETURN_WITH_STATUS(data, Click);
  }
  else if(data->pv_click == RE_BT_LONG_CLICK) {
    data->pv_click = RE_BT_UNRELEASED;
    RETURN_WITH_STATUS(data, LongClick);
  }
  else if (data->Diff < 0) {
    if(data->pv_click == RE_BT_DRAG) {
      RETURN_WITH_STATUS(data, Rotate_Decrement_while_click);
    }
      RETURN_WITH_STATUS(data, Rotate_Decrement);
  } else if (data->Diff > 0) {
    if(data->pv_click == RE_BT_DRAG) {
      RETURN_WITH_STATUS(data, Rotate_Increment_while_click);
    }
      RETURN_WITH_STATUS(data, Rotate_Increment);
  }
  RETURN_WITH_STATUS(data, Rotate_Nothing);
}

void RE_SetMode(RE_State_t* data, RE_Mode_t mode) {
  /* Set mode */
  data->Mode = mode;
}

void RE_Process(RE_State_t* data) {
  static uint32_t push_time=0, halfPointReachedTime=0, debounce_time=0, lastStep=0;
  static bool last_button_read=1, last_button_stable=1;
  uint32_t current_time = HAL_GetTick();
  uint32_t pressed_time = current_time - push_time;
  uint32_t stable_time = current_time - debounce_time;

  /* Read inputs */
  bool now_a = HAL_GPIO_ReadPin(data->GPIO_A, data->GPIO_PIN_A);
  bool now_b = HAL_GPIO_ReadPin(data->GPIO_B, data->GPIO_PIN_B);
  bool now_button = HAL_GPIO_ReadPin(data->GPIO_BUTTON, data->GPIO_PIN_BUTTON);

  if(last_button_read != now_button){                   // If different than last reading
    last_button_read = now_button;                      // Update last reading value
    if(stable_time<20){                                // If <debounce time
      debounce_time = current_time;                     // Reset debounce timer
    }
  }

  if(last_button_stable!=now_button){                   // If button status different than stable status
    if(stable_time>20){                                // If >debounce time
      last_button_stable = now_button;                  // Update last stable button value
      debounce_time = current_time;                     // Reset debounce timer
    }
    else{
      now_button = last_button_stable;                  // Ignore reading, use last stable button value
    }
  }

  if (now_a && now_b) {
    if(data->halfPointReached) {

      int8_t add = 1;
      data->halfPointReached = 0;

      if((current_time-lastStep)<12){                 // If last step was less than x time ago, increase twice to trigger big step
        add = 2;
      }
      lastStep=current_time;

      /* Check mode */
      if (data->Mode == RE_Mode_Reverse) {
        if (data->direction) {
          data->RE_Count += -add;
        } else {
          data->RE_Count += add;
        }
      }
      else {
        if (data->direction) {
          data->RE_Count += add;
        } else {
          data->RE_Count += -add;
        }
      }
    }
  }
  else if(now_a == 0 && now_b == 0) {
    if(!data->halfPointReached){
      halfPointReachedTime = current_time;
      data->halfPointReached = 1;
      if(now_button == 0) {//button pressed
        data->pv_click = RE_BT_DRAG;
      }
    }
    else if((current_time-halfPointReachedTime)>500){
      if(now_button == 0 && data->pv_click == RE_BT_DRAG) {//button pressed
        data->pv_click = RE_BT_PRESSED;
      }
    }
  }
  else if(!data->halfPointReached) {
    if(now_a)
      data->direction = 1;
    else if (now_b)
      data->direction = 0;
  }

  if((data->pv_click == RE_BT_DRAG) && (now_button == 1))
    data->pv_click = RE_BT_HIDLE;
  else if(data->pv_click != RE_BT_DRAG) {
    if((data->pv_click == RE_BT_HIDLE) && (now_button == 0)) {
      data->pv_click = RE_BT_PRESSED;
      push_time = current_time;
    }
    else if(data->pv_click == RE_BT_PRESSED) {
      if((now_button == 0)&&(pressed_time > 500)){
        data->pv_click = RE_BT_LONG_CLICK;
      }
      else if((now_button == 1)&&(pressed_time > 50)){
        data->pv_click = RE_BT_CLICKED;
      }

    }
    if((data->pv_click == RE_BT_UNRELEASED) && (now_button == 1) ) {
        data->pv_click = RE_BT_HIDLE;
    }
  }
}
