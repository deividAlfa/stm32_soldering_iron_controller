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
#ifdef DEBUG
uint16_t dbg_prev_TIP_Raw, dbg_prev_TIP, dbg_prev_VIN, dbg_prev_PWR;
int16_t dbg_prev_NTC;
bool dbg_NewData;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef DEBUG
int _write(int32_t file, uint8_t *ptr, int32_t len)
{
  for (int i = 0; i < len; i++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
#endif
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
  #ifdef DEBUG
	  DebugOpts();          // Enable debug options in Debug build
	  DWT->CTRL |= 1 ; // enable the counter
	  DWT->CYCCNT = 0; // reset the counter
	  // Now CPU cycles can be read in DWT->CYCCNT;
  #endif
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
#ifdef DEBUG
    if(NewData){
      NewData=0;
      Iron.lastActivityTime=HAL_GetTick();
      Iron.newActivity=1;
      __disable_irq();

      printf("     LAST  VAL    RAW   VAL      PREV  VAL    RAW   VAL\n"
             "TIP: %4u  %3u\260C  %4u  %3u\260C    %4u  %3u\260C  %4u  %3u\260C \n"
             "PID: %3u%%                        %3u%%\n\n",
            TIP.last_avg, dbg_last_TIP, TIP.last_raw, dbg_last_TIP_Raw, TIP.prev_avg, dbg_prev_TIP, TIP.prev_raw, dbg_prev_TIP_Raw,
            Iron.CurrentIronPower, dbg_prev_PWR);
      __enable_irq();
    }
#endif
    checkSettings();                                                                          // Check if settings were modified
    oled_handle();                                                                            // Handle oled drawing
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

// Called from SysTick IRQ every 1mS
void Program_Handler(void) {
  static bool last_wake=0;
  bool now_wake = WAKE_input();

  if(last_wake!=now_wake){                                              // If wake sensor input changed
    last_wake=now_wake;
    if(systemSettings.settings.WakeInputMode==wakeInputmode_stand){     // In stand mode
      if(now_wake){
        setModefromStand(mode_run);
      }
      else{
        setModefromStand(systemSettings.settings.StandMode);            // Set sleep or standby mode depending on system setting
      }
    }
    else{
      IronWake(source_wakeInput);
    }
  }

	handle_buzzer();                                                    // Handle buzzer state
	RE_Process(&RE1_Data);                                              // Handle Encoder

}


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim){
	if(_htim == Iron.Pwm_Timer){																			// PWM output low
	  __HAL_TIM_CLEAR_FLAG(Iron.Pwm_Timer,TIM_FLAG_CC1|TIM_FLAG_CC2|TIM_FLAG_CC3|TIM_FLAG_CC4);   // Clear compare flags
	  // If there are pending PWM settings to be applied, apply them before new calculation
		if(ADC_Status==ADC_Idle){																		    // ADC idle
			ADC_Status = ADC_Waiting;																	    // Update status to waiting
			__HAL_TIM_ENABLE(Iron.Delay_Timer);														// Enable Delay Timer and start counting (One-pulse mode)
		}
		else if(ADC_Status==ADC_Sampling){                              // ADC busy?
	    ADC_Stop_DMA();                                               // Stop ADC. Skip conversion.
	    ADC_Status = ADC_Idle;                                        // Set the ADC status
		}
		else if(ADC_Status==ADC_Waiting){                               // ADC waiting(Delay timer running)
		  if(Iron.Delay_Timer->Instance->CR1 & TIM_CR1_CEN){            // Delay timer running?
        __disable_irq();
        __HAL_TIM_DISABLE(Iron.Delay_Timer);                          // Stop timer
        __HAL_TIM_SET_COUNTER(Iron.Delay_Timer,0);                    // Clear counter
        __HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE);       // Clear flag
        __HAL_TIM_ENABLE(Iron.Delay_Timer);                           // Re-enable
        __enable_irq();
		  }
	  }
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *_htim){
  if(_htim == Iron.Pwm_Timer){                                    // Delay Timer?
    __HAL_TIM_CLEAR_FLAG(Iron.Pwm_Timer,TIM_FLAG_UPDATE);         // Clear PWM Timer flag
    if(Iron.updatePwm==needs_update){
      Iron.updatePwm=no_update;
      __HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer,systemSettings.Profile.pwmPeriod);
      __HAL_TIM_SET_AUTORELOAD(Iron.Delay_Timer,systemSettings.Profile.pwmDelay);
    }
    __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);      // Load new calculated PWM Duty

  }
	if(_htim == Iron.Delay_Timer){																		// Delay Timer?
	  __HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE);				  // Clear Delay Timer flag
    ADC_Start_DMA();                                                // Start ADC conversion
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
	while(1){																									// Divide string in chuncks that fit teh screen width
		strOut[outPos] = file[inPos];														// Copy char
		strOut[outPos+1] = 0;																		// Set out string null terminator
		uint8_t currentWidth = u8g2_GetStrWidth(&u8g2, strOut);	// Get width
		if(currentWidth<OledWidth){															// If less than oled width, increase input string pos
			inPos++;
		}
		if( (currentWidth>OledWidth) || (strOut[outPos]==0) ){	// If width bigger than oled width or current char null(We reached end of input string)
			char current = strOut[outPos];												// Store current char
			strOut[outPos]=0;																			// Set current out char to null
			u8g2_DrawStr(&u8g2, 0, ypos, strOut);									// Draw string
			if(current==0){																				// If current is null, we reached end
				break;																							// Break
			}
			outPos=0;																							// Else, reset output position
			ypos += 12;																						// Increase Y position and continue copying the input string
		}
		else{
			outPos++;																							// Output buffer not filled yet, increase position
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
