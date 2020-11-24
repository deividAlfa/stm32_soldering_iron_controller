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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "setup.h"
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	 __HAL_DBGMCU_FREEZE_TIM17();
	 __HAL_DBGMCU_FREEZE_TIM15();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_SPI2_Init();
  MX_TIM17_Init();
  MX_TIM15_Init();
  /* USER CODE BEGIN 2 */

#ifdef Soft_SPI
  Enable_Soft_SPI();
  ssd1306_init();
#else
  ssd1306_init(&SPI_DEVICE);
#endif
  guiInit(&PWM_TIMER);
  restoreSettings();
  ADC_Init(&ADC_DEVICE);
  RE_Init((RE_State_t *)&RE1_Data, ROT_ENC_L_GPIO_Port, ROT_ENC_L_Pin, ROT_ENC_R_GPIO_Port, ROT_ENC_R_Pin, ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_Pin);
  buzzer_init();
  ironInit(&DELAY_TIMER, &PWM_TIMER,PWM_CHANNEL);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  handle_buzzer();
	  if(Iron.OledUpdate) {
  		Iron.OledUpdate=0;
		RotationData = RE_Get(&RE1_Data);
		oled_processInput(RotationData, &RE1_Data);
		oled_update();
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */







// Called from SysTick IRQ every 1mS
void Program_Handler(void) {
	static uint16_t OledUpdateCount=0,PIDUpdateCount=0;

	if(++PIDUpdateCount>=PID_Refresh_ms){		//Timer for updating PID calculation
		Iron.PIDUpdate=1;
	}

	if(++OledUpdateCount>=Process_Refresh_ms){		// Timer for updating the oled and other routines
		OledUpdateCount=0;
		Iron.OledUpdate = 1;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if((GPIO_Pin == ROT_ENC_BUTTON_Pin) || (GPIO_Pin == ROT_ENC_R_Pin) || (GPIO_Pin == ROT_ENC_L_Pin)) {
		  RE_Process(&RE1_Data);
	}

	if(GPIO_Pin==WAKE_Pin){										// If handle moves
		if(Iron.CurrentMode!=mode_boost){				// Reset idle timer
			Iron.CurrentModeTimer=HAL_GetTick();			// Unless we are in boost mode (let the timer expire)
			if(!Iron.Active){							// Enable active state
				Iron.Active=1;
			}
		}

	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim){
	if(_htim == Iron.Pwm_Timer){									// PWM output low
		if(ADC_Status==ADC_InitTip){								// ADC ready?
			ADC_Status = ADC_SamplingTip;							// Update status
			__HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE);	// Clear Delay Timer flag
			__HAL_TIM_SET_COUNTER(Iron.Delay_Timer,0);				// Clear Delay Timer counter
			__HAL_TIM_ENABLE(Iron.Delay_Timer);						// Enable Delay Timer and start counting
																	// It will trigger the ADC when it overflows and disable by itself (One-pulse mode).
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc){
	if(_hadc == &ADC_DEVICE){
		ADC_Stop_DMA();								// Reset the ADC
		handle_ADC();								// Process the data.
		switch (ADC_Status){
			case ADC_SamplingTip:					// Finished sampling tip. Set new PWM Duty.
				ADC_Status = ADC_StartOthers;		// Set the ADC status
				__HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Duty);
				handleIron();						// Update Iron process
				break;

			case ADC_SamplingOthers:				// Finished sampling secondary channels
				ADC_Status = ADC_StartTip;			// Set the ADC status
				break;

			default:
				Error_Handler();
		}
		ADC_Start_DMA();							// Start ADC with new new status
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
	while(1);
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
