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
  data->Mode = getSystemSettings()->EncoderMode;

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
  static uint32_t push_time, halfPointReachedTime, debounce_time, lastStep;
  static bool button_last=1, button_stable=1;
  static bool a_last, a_stable, b_last, b_stable;
  uint32_t current_time = HAL_GetTick();
  uint32_t pressed_time = current_time - push_time;
  uint32_t stable_time = current_time - debounce_time;

  /* Read inputs */
  bool a_now = HAL_GPIO_ReadPin(data->GPIO_A, data->GPIO_PIN_A);
  bool b_now = HAL_GPIO_ReadPin(data->GPIO_B, data->GPIO_PIN_B);
  bool button_now = HAL_GPIO_ReadPin(data->GPIO_BUTTON, data->GPIO_PIN_BUTTON);
  bool skip=0;

  if(button_last != button_now){                      // If different than last reading
    button_last = button_now;                         // Update last reading value
    debounce_time = current_time;                     // Reset debounce timer
    skip = 1;
  }
  else if(button_stable != button_now){               // If button status different than stable status
    if(stable_time>20){                               // If >debounce time
      button_stable = button_now;                     // Update last stable button value
    }
  }

  if(a_last != a_now){                                // If encoder pin changed
    a_last = a_now;                                   // Update last
    skip = 1;                                         // Skip this cycle (1ms deboucing)
  }
  else if(a_stable != a_now){                         // Else, if not same as stable
    a_stable = a_now;                                 // Update stable
  }

  if(b_last != b_now){                                // Same for input b
    b_last = b_now;
    skip = 1;
  }
  else if(b_stable != b_now){
    b_stable = b_now;
  }

  if(skip)                                            // Some pin changed, skip processing
    return;

  if (a_stable && b_stable) {
    if(data->halfPointReached) {

      int8_t add = 1;
      data->halfPointReached = 0;

      if((current_time-lastStep)<30){                 // If last step was less than x time ago, increase twice to trigger big step
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
  else if(a_stable == 0 && b_stable == 0) {
    if(!data->halfPointReached){
      halfPointReachedTime = current_time;
      data->halfPointReached = 1;
      if(button_stable == 0) {//button pressed
        data->pv_click = RE_BT_DRAG;
      }
    }
    else if((current_time-halfPointReachedTime)>500){
      if(button_stable == 0 && data->pv_click == RE_BT_DRAG) {      //button pressed
        data->pv_click = RE_BT_PRESSED;
      }
    }
  }
  else if(!data->halfPointReached) {
    if(a_stable)
      data->direction = 1;
    else if (b_stable)
      data->direction = 0;
  }

  if((data->pv_click == RE_BT_DRAG) && (button_stable == 1))
    data->pv_click = RE_BT_HIDLE;
  else if(data->pv_click != RE_BT_DRAG) {
    if((data->pv_click == RE_BT_HIDLE) && (button_stable == 0)) {
      data->pv_click = RE_BT_PRESSED;
      push_time = current_time;
    }
    else if(data->pv_click == RE_BT_PRESSED) {
      if((button_stable == 0)&&(pressed_time > 500)){
        data->pv_click = RE_BT_LONG_CLICK;
      }
      else if((button_stable == 1)&&(pressed_time > 50)){
        data->pv_click = RE_BT_CLICKED;
      }
    }
    if((data->pv_click == RE_BT_UNRELEASED) && (button_stable == 1) ) {
        data->pv_click = RE_BT_HIDLE;
    }
  }
}
