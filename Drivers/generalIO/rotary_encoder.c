/*
 * rotary_encoder.c

 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "rotary_encoder.h"
#include "settings.h"
volatile RE_State_t RE1_Data;
static uint32_t push_time = 0, release_time=0, halfPointReachedTime=0;

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
	static uint32_t last_push=0;
	/* Calculate everything */
	data->Diff = data->RE_Count - data->Absolute;
	data->Absolute += data->Diff;

	/* Check */
	if(data->pv_click == RE_BT_CLICKED) {
		data->pv_click = RE_BT_UNRELEASED;
		if((HAL_GetTick()-last_push)>100){
			last_push=HAL_GetTick();
			RETURN_WITH_STATUS(data, Click);
		}
	}
	else if(data->pv_click == RE_BT_LONG_CLICK) {
		data->pv_click = RE_BT_UNRELEASED;
		if((HAL_GetTick()-last_push)>100){
			last_push=HAL_GetTick();
			RETURN_WITH_STATUS(data, LongClick);
		}
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
	bool now_a;
	bool now_b;
	bool now_button;
	uint32_t pressed_time;
	/* Read inputs */
	now_a = HAL_GPIO_ReadPin(data->GPIO_A, data->GPIO_PIN_A);
	now_b = HAL_GPIO_ReadPin(data->GPIO_B, data->GPIO_PIN_B);
	now_button = HAL_GPIO_ReadPin(data->GPIO_BUTTON, data->GPIO_PIN_BUTTON);
	pressed_time = HAL_GetTick() - push_time;
	if (now_a && now_b) {
		if(data->halfPointReached) {
			data->halfPointReached = 0;
			/* Check mode */
			if (data->Mode == RE_Mode_Zero) {
				if (data->direction) {
					data->RE_Count--;
				} else {
					data->RE_Count++;
				}
			}
			else {
				if (data->direction) {
					data->RE_Count++;
				} else {
					data->RE_Count--;
				}
			}
		}
	}
	else if(now_a == 0 && now_b == 0) {
		if(!data->halfPointReached){
			halfPointReachedTime = HAL_GetTick();
			data->halfPointReached = 1;
			if(now_button == 0) {//button pressed
				data->pv_click = RE_BT_DRAG;
			}
		}
		else if((HAL_GetTick()-halfPointReachedTime)>500){
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
			if((HAL_GetTick()-release_time)>100){
				data->pv_click = RE_BT_PRESSED;
				push_time = HAL_GetTick();
			}
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
				release_time=HAL_GetTick();
		}
	}
}



