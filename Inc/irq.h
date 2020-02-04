#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"

__attribute__((weak)) void tim3_PeriodicElapsedCb(TIM_HandleTypeDef *htim);
__attribute__((weak)) void tim3_pulseFinishCb(TIM_HandleTypeDef *htim);
__attribute__((weak)) void tim4_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void flawless_adc_ConvCpltCb(ADC_HandleTypeDef *htim);
