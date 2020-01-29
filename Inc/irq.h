#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

__attribute__((weak)) void PWM_TIM3_RESET(TIM_HandleTypeDef *htim);
__attribute__((weak)) void PWM_TIM3_PULSE(TIM_HandleTypeDef *htim);

void START_MEASURE(TIM_HandleTypeDef *htim);
void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim);
void UPDATE_PWM_TURN_ON(void);
