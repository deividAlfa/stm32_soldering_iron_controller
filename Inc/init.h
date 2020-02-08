#pragma once
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"



void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_IWDG_Init(void);
void MX_TIM4_Init(void);
void MX_USART3_UART_Init(void);
void MX_TIM3_Init(void);

extern TIM_HandleTypeDef tim3_pwm;
extern TIM_HandleTypeDef tim4_temp_measure;


