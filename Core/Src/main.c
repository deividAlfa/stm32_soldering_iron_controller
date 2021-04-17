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
#include "rotary_encoder.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "ssd1306.h"
#include "gui.h"
#include "screen.h"

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
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Init(void){
#if defined OLED_SOFT_SPI || defined OLED_SOFT_I2C
	  ssd1306_init(&FILL_DMA);
#elif defined OLED_SPI || defined OLED_I2C
	  ssd1306_init(&OLED_DEVICE, &FILL_DMA);
#endif
	  guiInit();
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
  HAL_IWDG_Refresh(&hiwdg);             // Wait 500mS for voltage to stabilize? (Before calibrating ADC)
  HAL_Delay(500);

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
	handle_buzzer();
	RE_Process(&RE1_Data);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if(GPIO_Pin==WAKE_Pin){													// If wake sensor input changed
		if(systemSettings.settings.WakeInputMode==wakeInputmode_stand){		// In stand mode
			if(WAKE_input()){
				setModefromStand(mode_run);
			}
			else{
				setModefromStand(mode_sleep);
			}
		}
		else{																// In shake mode
			IronWake(source_wakeInput);
		}
	}
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim){
	if(_htim == Iron.Pwm_Timer){									// PWM output low
		if(ADC_Status==ADC_InitTip){								// ADC ready?
			ADC_Status = ADC_SamplingTip;							// Update status
			__HAL_TIM_ENABLE(Iron.Delay_Timer);						// Enable Delay Timer and start counting
																	// It will trigger the ADC when it overflows and disable by itself (One-pulse mode).
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *_htim){
	if(_htim == Iron.Delay_Timer){									// Delay Timer?
		if(ADC_Status==ADC_SamplingTip){							// ADC ready?
			__HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE);	// Clear Delay Timer flag
			if(HAL_ADC_Start_DMA(&ADC_DEVICE, (uint32_t*)Tip_measures, sizeof(Tip_measures)/ sizeof(uint16_t) )!=HAL_OK){	// Start ADC conversion
				Error_Handler();
			}
		}
		else{
			Error_Handler();										// If ADC_Status!=ADC_SamplingTip, lose of ADC_Status control happened somewhere
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

#ifdef DEBUG_ERROR
	#if defined OLED_I2C || defined OLED_SPI
	display_abort();
	#endif
	SetFailState(setError);
	buzzer_fatal_beep();
	Diag_init();

	char strOut[16];
	uint8_t outPos=0;
	uint8_t inPos=0;
	uint8_t ypos=12;
	sprintf(strOut,"ERR!! LINE:%u",line);
	u8g2_DrawStr(&u8g2, 0, 0, strOut);
	while(1){												// Divide string in chuncks that fit teh screen width
		strOut[outPos] = file[inPos];						// Copy char
		strOut[outPos+1] = 0;								// Set out string null terminator
		uint8_t currentWidth = u8g2_GetStrWidth(&u8g2, strOut);	// Get width
		if(currentWidth<OledWidth){									// If less than oled width, increase input string pos
			inPos++;
		}
		if( (currentWidth>OledWidth) || (strOut[outPos]==0) ){		// If width bigger than oled width or current char null(We reached end of input string)
			char current = strOut[outPos];					// Store current char
			strOut[outPos]=0;								// Set current out char to null
			u8g2_DrawStr(&u8g2, 0, ypos, strOut);			// Draw string
			if(current==0){									// If current is null, we reached end
				break;										// Break
			}
			outPos=0;										// Else, reset output position
			ypos += 12;										// Increase Y position and continue copying the input string
		}
		else{
			outPos++;										// Ouput string not filled yet, increase position
		}
	};

	#if defined OLED_I2C || defined OLED_SPI
	update_display_ErrorHandler();

	#elif defined OLED_SOFT_I2C || defined OLED_SOFT_SPI
	update_display();
	#endif
	Reset_onError();
#endif
	while(1){
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
