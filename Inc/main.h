/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
#include "stdint.h"
  /* Includes ------------------------------------------------------------------*/
#include "config.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define HW_VER2_1S

#ifndef HW_VER2_1S
	#define OLED_CS_Pin GPIO_PIN_0
	#define OLED_CS_GPIO_Port GPIOA
	#define IRON_TEMP_Pin GPIO_PIN_1
	#define IRON_TEMP_GPIO_Port GPIOA
	#define VREF_Pin GPIO_PIN_2
	#define VREF_GPIO_Port GPIOA
	#define NTC_Pin GPIO_PIN_3
	#define NTC_GPIO_Port GPIOA
	#define V_INPUT_Pin GPIO_PIN_4
	#define V_INPUT_GPIO_Port GPIOA
	#define ROT_ENC_L_Pin GPIO_PIN_1
	#define ROT_ENC_L_GPIO_Port GPIOB
	#define ROT_ENC_R_Pin GPIO_PIN_0
	#define ROT_ENC_R_GPIO_Port GPIOB
	#define ROT_ENC_BUTTON_GPIO_Port GPIOB
	#define ROT_ENC_BUTTON_GPIO_Pin GPIO_PIN_5
	#define JUMPER_Pin GPIO_PIN_2
	#define JUMPER_GPIO_Port GPIOB
	#define OLED_DC_Pin GPIO_PIN_8
	#define OLED_DC_GPIO_Port GPIOA
	#define OLED_RST_Pin GPIO_PIN_9
	#define OLED_RST_GPIO_Port GPIOA
	#define WAKE_Pin GPIO_PIN_6
	#define WAKE_GPIO_Port GPIOB
	#define BUZZER_Pin GPIO_PIN_7
	#define BUZZER_GPIO_Port GPIOB
	#define PWM_CONTROL_Pin GPIO_PIN_8
	#define PWM_CONTROL_GPIO_Port GPIOB
#else
	#define V_INPUT_Pin GPIO_PIN_1
	#define V_INPUT_GPIO_Port GPIOB
	#define EEPROM_SCL_Pin GPIO_PIN_0
	#define EEPROM_SCL_GPIO_Port GPIOA
	#define EEPROM_SDA_Pin GPIO_PIN_1
	#define EEPROM_SDA_GPIO_Port GPIOA
	#define IRON_TEMP_Pin GPIO_PIN_2
	#define IRON_TEMP_GPIO_Port GPIOA
#if 0 // turn off buzzer madness
	#define BUZZER_Pin GPIO_PIN_5
	#define BUZZER_GPIO_Port GPIOA
#else
	#define BUZZER_Pin GPIO_PIN_9	/* NC pin */
	#define BUZZER_GPIO_Port GPIOA
#endif
	#define TEMPAMB_Pin GPIO_PIN_7
	#define TEMPAMB_GPIO_Port GPIOA
	#define WAKE_Pin GPIO_PIN_6
	#define WAKE_GPIO_Port GPIOA
	#define T12PWM_Pin GPIO_PIN_0
	#define T12PWM_GPIO_Port GPIOB
	#define OLED_SCL_Pin GPIO_PIN_13
	#define OLED_SCL_GPIO_Port GPIOB
	#define OLED_SDA_Pin GPIO_PIN_12
	#define OLED_SDA_GPIO_Port GPIOB
	#define ROT_ENC_BUTTON_GPIO_Pin GPIO_PIN_15
	#define ROT_ENC_BUTTON_GPIO_Port GPIOA
	#define ROT_ENC_L_Pin GPIO_PIN_4
	#define ROT_ENC_L_GPIO_Port GPIOB
	#define ROT_ENC_R_Pin GPIO_PIN_3
	#define ROT_ENC_R_GPIO_Port GPIOB
#endif

#define __root __attribute__((used))

typedef struct{
	uint32_t _00_pwm_update_T;
	uint32_t _01_sample_dur;
	uint32_t _02_timer_T;
	uint32_t _03_sample_period;
	uint32_t _04_oled_update_dur;
	uint32_t _05_main_calc_dur;
	uint32_t _06_pid_calc_dur;
	uint32_t _07_ft_integrator;
}Ts_t;

extern Ts_t Benchmark;

#define TICK_TOCK(result) 					\
static uint32_t prev_tick;        \
uint32_t tick_now = HAL_GetTick();      \
result = tick_now - prev_tick;   \
prev_tick = tick_now;


#define TICK						\
{									\
uint32_t tick = HAL_GetTick();

#define TOCK(res)					\
	res = HAL_GetTick() - tick;		\
}

  /* flawless conversions +1 error reduced to +-0.5*/
  //#define UINT_DIV(x,y) (x+y/2)/y
//inline uint16_t UINT_DIV(uint16_t a,uint16_t b){
//	if(b==0){
//		return 0;
//	}
//
//	return (a+b/2)/b;
//
//}


#define UINT_DIV(x,y) ((x+y/2)/y)
#define CONV_TO_UINT(ft) (unsigned int)(ft+0.5)

#define MALLOC_TRACE_LEN 100
extern int malloc_size;
extern int malloc_idx ;
extern int malloc_arr[MALLOC_TRACE_LEN];

#define _malloc(size) 				\
malloc(size);						\
malloc_arr[(malloc_idx++)%MALLOC_TRACE_LEN]=size;	\
malloc_size+=(size)

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
