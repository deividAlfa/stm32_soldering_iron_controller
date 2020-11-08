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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
void Enable_Soft_SPI_SPI(void);
void CheckReset(void);
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

#ifdef Soft_SPI
  Enable_Soft_SPI_SPI();
#endif




  HAL_TIM_Base_Start_IT(&BASE_TIMER);
  #ifdef Soft_SPI
  ssd1306_init();
  #else
  ssd1306_init(&SPI_DEVICE);
  #endif
  guiInit(&PWM_TIMER);
  CheckReset();
  restoreSettings();
  setupPIDFromStruct();
  ADC_Init(&ADC_DEVICE);
  RE_Init(&RE1_Data, ROT_ENC_L_GPIO_Port, ROT_ENC_L_Pin, ROT_ENC_R_GPIO_Port, ROT_ENC_R_Pin, ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_Pin);
  buzzer_init();
  ironInit(&PWM_TIMER,PWM_CHANNEL);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  handleIron();
	  handle_buzzer();
	  if(Iron.Status.ProcessUpdate) {
  		Iron.Status.ProcessUpdate=0;
		RE_Rotation_t r = RE_Get(&RE1_Data);
		oled_processInput(r, &RE1_Data);
		oled_update();
  		oled_draw();
	  }
  }
  /* USER CODE END 3 */
}



/* USER CODE BEGIN 4 */

void Enable_Soft_SPI_SPI(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_SPI_MspDeInit(&SPI_DEVICE);
	 /*Configure GPIO pins : OLED_DC_Pin OLED_RST_Pin */
	 GPIO_InitStruct.Pin = OLED_DC_Pin|OLED_RST_Pin|SCK_Pin|SDO_Pin;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	 HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	static uint8_t OledRow=0;

	if(hspi != &SPI_DEVICE){		return; 	}
	if(OledRow>7){															//We sent the last row of the OLED buffer data. Return without retriggering DMA.
				OledRow=0;
				oled_status=oled_idle;
				//HAL_SPI_DMAStop(&SPI_DEVICE);
				return;
		}
	oled_status=oled_sending_cmd;
	write_cmd(0xB0|OledRow);
	#ifdef SH1106_FIX
	write_cmd(0x02);
	#else
	write_cmd(0x00);
	#endif
	write_cmd(0x10);
	oled_status=oled_sending_data;
	Oled_Clear_CS();
	Oled_Set_DC();
	if(HAL_SPI_Transmit_DMA(&SPI_DEVICE,(uint8_t*)OledDmaBf+ OledRow++ * 128, 128) != HAL_OK){
		Error_Handler();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	static uint16_t UpdateCount=0,PIDUpdateCount=0;
	//Timer set every 1mS

	if(htim == &BASE_TIMER)  {

		if(++PIDUpdateCount>=PID_Refresh_ms){		//Timer for updating Tip temperature and PID calculation
			PIDUpdateCount=0;
			if(Iron.Status.TempMeasureState == iron_temp_measure_idle){
				Iron.Status.TempMeasureState = iron_temp_measure_requested;
			}
		}

		if(++UpdateCount>=Process_Refresh_ms){		// Timer for updating the oled and other routines
			UpdateCount=0;
			Iron.Status.ProcessUpdate = 1;
		}
	}
}


void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc != ADC_DEVICE){
		return;
	}
	// DMA callback is called every 300uS or less.
	// No sense of keeping the DMA in circular mode.
	// We trigger the ADC DMA conversion every PID set in the TMR3 callback
	ADC_Stop_DMA();
	adc_dma_status = adc_dma_idle;
	if(Iron.Status.TempMeasureState == iron_temp_measure_started){
		Iron.Status.TempMeasureState = iron_temp_measure_ready;
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if((GPIO_Pin == ROT_ENC_BUTTON_Pin) || (GPIO_Pin == ROT_ENC_R_Pin) || (GPIO_Pin == ROT_ENC_L_Pin)) {
		  RE_Process(&RE1_Data);
	}

	if(GPIO_Pin==WAKE_Pin){										// If handle moves
		if(Iron.Status.CurrentMode!=mode_boost){				// Reset idle timer
			Iron.Status.CurrentModeTimer=HAL_GetTick();			// Unless we are in boost mode (let the timer expire)
			if(!Iron.Status.Active){							// Enable active state
				Iron.Status.Active=1;
			}
		}

	}
}


void CheckReset(void){
	if(!BUTTON_input){
	 setContrast(255);
	 UG_FillScreen(C_BLACK);
	 UG_FontSelect(&FONT_8X14);
	 UG_SetForecolor(C_WHITE);
	 UG_SetBackcolor(C_BLACK);
	 UG_PutString(10,20,"HOLD BUTTON");
	 UG_PutString(18,32,"TO RESET");
	 UG_PutString(16,44,"DEFAULTS!!");
	 update_display();

	 uint16_t ResetTimer= HAL_GetTick();
	 while(!BUTTON_input){
		 if((HAL_GetTick()-ResetTimer)>5000){
			 resetSettings();
			 UG_FillScreen(C_BLACK);
			 UG_PutString(40,15,"RESET!");
			 UG_PutString(0,40,"RELEASE BUTTON");
			 UG_Update();
			 update_display();
			 while(!BUTTON_input){;}
			 ResetTimer= HAL_GetTick();
			 while((HAL_GetTick()-ResetTimer)<1000){;}
			 NVIC_SystemReset();
		 }
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
