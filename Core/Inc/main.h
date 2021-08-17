/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern IWDG_HandleTypeDef hiwdg;
extern CRC_HandleTypeDef hcrc;
extern struct mallinfo mi;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// This is left here just to have it handy for copying when debugging a specific function
// Don't uncomment!!
//               __attribute__((optimize("O0")))

#define DEBUG_ERROR
#define DEBUG_ALLOC
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// STM32CUBE IDE removed line printing in Error Handler. This macro restores it.
// Credits: https://community.st.com/s/question/0D50X00009XkffVSAR/stm32cubemx-v421-errorhandler-definition-issues-in-mainh
#ifdef DEBUG_ERROR
	#define GET_MACRO( _0, _1, NAME, ... ) NAME
	#define Error_Handler(...) GET_MACRO( _0, ##__VA_ARGS__, Error_Handler1, Error_Handler0 )()
	#define Error_Handler0() _Error_Handler(__BASE_FILE__, __LINE__ )
	#define Error_Handler1(unused) _Error_Handler( char * file, int line )
	void _Error_Handler(char *, int);
#endif
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Program_Handler(void);



/*
 * Macro to enable debugging of the allocated memory
 * max_allocated will hold the max used memory at any time
 *
 */
#ifdef DEBUG_ALLOC
extern uint32_t max_allocated;
#define dbg_mem() mi=mallinfo();                  \
                  if(mi.uordblks>max_allocated){  \
                    max_allocated=mi.uordblks;     \
                  }                               \

#define _malloc(x)    malloc(x); dbg_mem()
#define _calloc(x,y)  calloc(x,y); dbg_mem()
#define _free(x)      free(x); dbg_mem()

#else
#define _malloc     malloc
#define _calloc     calloc
#define _free       free
#endif
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WAKE_Pin GPIO_PIN_0
#define WAKE_GPIO_Port GPIOA
#define BUZ0_Pin GPIO_PIN_1
#define BUZ0_GPIO_Port GPIOA
#define BUZ1_Pin GPIO_PIN_2
#define BUZ1_GPIO_Port GPIOA
#define EE_SCL_Pin GPIO_PIN_3
#define EE_SCL_GPIO_Port GPIOA
#define EE_SDA_Pin GPIO_PIN_4
#define EE_SDA_GPIO_Port GPIOA
#define NTC_Pin GPIO_PIN_5
#define NTC_GPIO_Port GPIOA
#define PWM_Pin GPIO_PIN_6
#define PWM_GPIO_Port GPIOA
#define VIN_Pin GPIO_PIN_0
#define VIN_GPIO_Port GPIOB
#define TIP_Pin GPIO_PIN_1
#define TIP_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_13
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_15
#define OLED_SDA_GPIO_Port GPIOB
#define OLED_RST_Pin GPIO_PIN_8
#define OLED_RST_GPIO_Port GPIOA
#define OLED_DC_Pin GPIO_PIN_9
#define OLED_DC_GPIO_Port GPIOA
#define ENC_SW_Pin GPIO_PIN_15
#define ENC_SW_GPIO_Port GPIOA
#define ENC_L_Pin GPIO_PIN_3
#define ENC_L_GPIO_Port GPIOB
#define ENC_R_Pin GPIO_PIN_4
#define ENC_R_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define WAKE_input()		HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin)
#define BUTTON_input()		HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
