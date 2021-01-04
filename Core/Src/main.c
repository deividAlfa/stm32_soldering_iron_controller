/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "iron.h"
#include "pid.h"
#include "settings.h"
#include "adc_global.h"
#include "buzzer.h"
#include "buzzer.h"
#include "rotary_encoder.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "ssd1306.h"
#include "gui.h"
#include "screen.h"
#include <stdint.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
RE_Rotation_t RotationData;
volatile uint32_t Timing_Start=0, Timing_End=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*//* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Init(void){
	  guiInit();
#ifdef Soft_SPI
	  Enable_Soft_SPI();
	  ssd1306_init();
#elif defined OLED_SPI || defined OLED_I2C
	  ssd1306_init(&OLED_DEVICE, &FILL_DMA);
#endif
	  ADC_Init(&ADC_DEVICE);
	  buzzer_init();
	  restoreSettings();
	  ironInit(&DELAY_TIMER, &PWM_TIMER,PWM_CHANNEL);
	  RE_Init((RE_State_t *)&RE1_Data, ROT_ENC_L_GPIO_Port, ROT_ENC_L_Pin, ROT_ENC_R_GPIO_Port, ROT_ENC_R_Pin, ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_Pin);
	  oled_init(&RE_Get,&RE1_Data);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	DebugOpts();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  Init();
  while (1){
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	oled_handle();
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

// Called from SysTick IRQ every 1mS
void Program_Handler(void) {
	static uint16_t PIDUpdateCount=0;
	handle_buzzer();
	RE_Process(&RE1_Data);
	if(++PIDUpdateCount>=PID_Refresh_ms){		//Timer for updating PID calculation
		PIDUpdateCount=0;
		Iron.PIDUpdate=1;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	/*
	if((GPIO_Pin == ROT_ENC_BUTTON_Pin) || (GPIO_Pin == ROT_ENC_R_Pin) || (GPIO_Pin == ROT_ENC_L_Pin)) {
		RE_Process(&RE1_Data);
	}
	*/
	if(GPIO_Pin==WAKE_Pin){							// If handle moves
		IronWake();
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim){
	if(_htim == Iron.Pwm_Timer){									// PWM output low
		if(ADC_Status==ADC_InitTip){								// ADC ready?
			ADC_Status = ADC_SamplingTip;							// Update status
			__HAL_TIM_SET_COUNTER(Iron.Delay_Timer,0);				// Clear Delay Timer counter
			__HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE);	// Clear Delay Timer flag
			__HAL_TIM_ENABLE(Iron.Delay_Timer);						// Enable Delay Timer and start counting
																	// It will trigger the ADC when it overflows and disable by itself (One-pulse mode).
		}
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
#ifdef DEBUG
	char chars[6];
	SetFailState(1);
	buzzer_fatal_beep();
	FillBuffer(C_BLACK,fill_soft);
	UG_FontSelect(&FONT_6X8_reduced);
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
	UG_PutString(0,0,"ERROR IN:");//11
	UG_PutString(0,10,file);//1
	UG_PutString(0,50,"LINE:");//1
	sprintf(&chars[0],"%d",line);
	UG_PutString(30,50,&chars[0]);//1
	update_display_ErrorHandler();
#endif
	while(1){
		HAL_IWDG_Refresh(&hiwdg);							// Clear watchdog
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
