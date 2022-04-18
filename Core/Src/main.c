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
#include "myTest.h"
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
#include "addon_fume_extractor.h"
#endif


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "main.c"
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
#if defined DEBUG_PWM && defined SWO_PRINT
volatile uint16_t dbg_prev_TIP_Raw, dbg_prev_TIP, dbg_prev_VIN, dbg_prev_PWR;
volatile int16_t dbg_prev_NTC;
volatile bool dbg_newData;
#endif


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if defined DEBUG_PWM && defined SWO_PRINT
int _write(int32_t file, uint8_t *ptr, int32_t len)
{
  for (int i = 0; i < len; i++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
#endif


#ifdef DEBUG_ALLOC
struct mallinfo mi;
uint32_t max_allocated;
#endif
// Allocate max possible ram, then release it. This fill the malloc pool and avoids internal fragmentation due (ST's?) poor malloc implementation.
void malloc_fragmentation_fix(void){
  uint32_t *ptr = NULL;
  uint32_t try=17408;
  while(!ptr && try){
    ptr = _malloc(try);
    try-=16;
  }
  _free(ptr);
  #ifdef DEBUG_ALLOC
  max_allocated=0; //Clear max allocated result
  #endif
}



void Init(void){
#if (defined OLED_SPI || defined OLED_I2C) && defined OLED_DEVICE
  ssd1306_init(&OLED_DEVICE, &FILL_DMA);
#elif defined OLED_SPI || defined OLED_I2C
  ssd1306_init(&FILL_DMA);
#endif

    guiInit();
    ADC_Init(&ADC_DEVICE);
    buzzer_init();
    restoreSettings();
    setContrast(systemSettings.settings.contrast);
    ironInit(&READ_TIMER, &PWM_TIMER,PWM_CHANNEL);
    RE_Init((RE_State_t *)&RE1_Data, ENC_L_GPIO_Port, ENC_L_Pin, ENC_R_GPIO_Port, ENC_R_Pin, ENC_SW_GPIO_Port, ENC_SW_Pin);
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

  malloc_fragmentation_fix();


  #if defined DEBUG && !defined STM32F072xB
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
  configurePWMpin(output_Low);

  for(uint32_t t=HAL_GetTick();(HAL_GetTick()-t)<500; ){  // Wait 500ms for voltage to stabilize? (Before calibrating ADC)
    HAL_IWDG_Refresh(&hiwdg);
  }

  Init();

  #ifdef RUN_MY_TEST
  myTest();
  #endif

  while (1){
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#if defined DEBUG_PWM && defined SWO_PRINT
    if(dbg_newData){
      dbg_newData=0;

      printf("     LAST  VAL    RAW   VAL      PREV  VAL    RAW   VAL\n"
             "TIP: %4u  %3u\260C  %4u  %3u\260C    %4u  %3u\260C  %4u  %3u\260C \n"
             "PID: %3u%%                        %3u%%\n\n",
            TIP.last_avg, last_TIP, TIP.last_raw, last_TIP_Raw, TIP.prev_avg, dbg_prev_TIP, TIP.prev_raw, dbg_prev_TIP_Raw,
            Iron.CurrentIronPower, dbg_prev_PWR);

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
  handle_buzzer();                                                    // Handle buzzer state
  RE_Process(&RE1_Data);                                              // Handle Encoder
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  handleAddonFumeExtractor();
#endif
  if(systemSettings.Profile.WakeInputMode!=mode_stand){
    readWake();
  }
}


/*
 *
 *  Timer working in load preload mode! The value loaded now is loaded on next event. That's why the values are reversed!
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *_htim){
  if(_htim == Iron.Read_Timer){
    __HAL_TIM_CLEAR_FLAG(Iron.Read_Timer,TIM_FLAG_UPDATE);

    if(ADC_Status==ADC_Idle){
      __HAL_TIM_SET_AUTORELOAD(Iron.Read_Timer,systemSettings.Profile.readPeriod-(systemSettings.Profile.readDelay+1)); // load (period-delay) time

      if(systemSettings.settings.activeDetection && !Iron.Error.safeMode){
        configurePWMpin(output_High);                                                   // Force PWM high for a few uS (typically 5-10uS)
        while(__HAL_TIM_GET_COUNTER(Iron.Read_Timer)<(PWM_DETECT_TIME/5));
      }
      configurePWMpin(output_Low);                                                      // Force PWM low
      ADC_Status = ADC_Waiting;
    }
    else if(ADC_Status==ADC_Waiting){
      __HAL_TIM_SET_AUTORELOAD(Iron.Read_Timer,systemSettings.Profile.readDelay);       // Load Delay time
      ADC_Start_DMA();
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
  #if (defined OLED_I2C || defined OLED_SPI) && defined OLED_DEVICE
  if(!oled.use_sw){
    display_abort();
  }
  #endif
  setSafeMode(enable);
  buzzer_fatal_beep();
  Oled_error_init();

  char strOut[16];
  uint8_t outPos=0;
  uint8_t inPos=0;
  uint8_t ypos=16;

  sprintf(strOut,"Line %u",line);
  u8g2_DrawStr(&u8g2, 0, 0, strOut);

  while(1){                                                  // Divide string in chuncks that fit teh screen width
    strOut[outPos] = file[inPos];                            // Copy char
    strOut[outPos+1] = 0;                                    // Set out string null terminator
    uint8_t currentWidth = u8g2_GetStrWidth(&u8g2, strOut);  // Get width
    if(currentWidth<OledWidth){                              // If less than oled width, increase input string pos
      inPos++;
    }
    if( (currentWidth>OledWidth) || (strOut[outPos]==0) ){  // If width bigger than oled width or current char null(We reached end of input string)
      char current = strOut[outPos];                        // Store current char
      strOut[outPos]=0;                                     // Set current out char to null
      u8g2_DrawStr(&u8g2, 0, ypos, strOut);                 // Draw string
      if(current==0){                                       // If current is null, we reached end
        break;                                              // Break
      }
      outPos=0;                                             // Else, reset output position
      ypos += 12;                                           // Increase Y position and continue copying the input string
    }
    else{
      outPos++;                                              // Output buffer not filled yet, increase position
    }
  };

  #if (defined OLED_I2C || defined OLED_SPI) && defined OLED_DEVICE

  #ifdef I2C_TRY_HW
  if(oled.use_sw){
    update_display();
  }
  else{
    update_display_ErrorHandler();
  }
  #else
  update_display_ErrorHandler();
  #endif

  #else
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
