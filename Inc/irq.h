#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

__attribute__((weak)) void flawless_iron_on_cb(TIM_HandleTypeDef *htim);
__attribute__((weak)) void flawless_iron_off_cb(TIM_HandleTypeDef *htim);

void flawless_adc_process(ADC_HandleTypeDef *htim);

__weak void measure_start_blocking_mode(TIM_HandleTypeDef *htim);
void blocking_mode_adc_process(ADC_HandleTypeDef *htim);
void periodic_pwm_update(void);
