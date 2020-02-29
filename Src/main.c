
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dwt_stm32_delay.h"
#include "buzzer.h"
#include "rotary_encoder.h"
#include "gui.h"
#include "debug_screen.h"
#include "settings.h"
#include "iron.h"
#include "init.h"
#include "config.h"

adc_measures_t adc_measures;
volatile iron_temp_measure_state_t iron_temp_measure_state = iron_temp_measure_idle;


ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

int malloc_size=0;
int malloc_idx = 0;
int malloc_arr[MALLOC_TRACE_LEN];

/* Private variables ---------------------------------------------------------*/

Ts_t Benchmark;
//uint32_t Ts[5];

static uint8_t activity = 1;
static uint32_t startOfNoActivityTime = 0;
static uint8_t startOfNoActivity = 0;

volatile uint32_t adc = 0;
volatile uint32_t dma = 0;

void SystemClock_Config(void);



RE_State_t RE1_Data;


extern TIM_HandleTypeDef tim4_temp_measure;
extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef tim3_pwm;


int main(void)
{
  HAL_Init();
  //resetSettings();

  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init(&hadc2);
  MX_ADC1_Init(&hadc1);
  HAL_Delay(1000);
  buzzer_init();
  if(HAL_ADCEx_Calibration_Start(&hadc2) != HAL_OK)
	  buzzer_alarm_start();
  else if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
	  buzzer_alarm_start();
  else
	  buzzer_short_beep();

  ironInit(&tim3_pwm);
  MX_IWDG_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  /* Enable ADC slave */
  if (HAL_ADC_Start(&hadc2) != HAL_OK)
  {
	  buzzer_alarm_start();
	  Error_Handler();
  }
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) &adc_measures.iron[0], ADC_MEASURES_LEN );

  HAL_TIM_Base_Start_IT(&tim4_temp_measure);
  restoreSettings();

  DWT_Delay_Init(); // Important for I2C communication
  UG_GUI gui;
  ssd1306_init();
  UG_Init(&gui, pset, 128, 64);
  guiInit();
  oled_init();
  oled_draw();
  UG_Update();
  update_display();

  HAL_IWDG_Start(&hiwdg);
  RE_Init(&RE1_Data, ROT_ENC_L_GPIO_Port, ROT_ENC_L_Pin, ROT_ENC_R_GPIO_Port, ROT_ENC_R_Pin, ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_GPIO_Pin);

  uint32_t lastTimeDisplay = HAL_GetTick();

  setPWM_tim(&tim3_pwm);
  iron_temp_measure_state = iron_temp_measure_idle;
  setContrast(systemSettings.contrast);
  currentBoostSettings = systemSettings.boost;
  currentSleepSettings = systemSettings.sleep;
  applyBoostSettings();
  applySleepSettings();
  setCurrentTip(systemSettings.currentTip);
  setupPIDFromStruct();


  if(HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin) == GPIO_PIN_RESET) {
	  activity = 0;
	  setCurrentMode(mode_sleep);
  }

  while (1)
  {


	  HAL_IWDG_Refresh(&hiwdg);
	  if(iron_temp_measure_state == iron_temp_measure_ready) {
		  TICK_TOCK(Benchmark._00_pwm_update_T);
		  TICK;
		  readTipTemperatureCompensated(1);

		  if(HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin) == GPIO_PIN_RESET) {
			  if(startOfNoActivity == 0)
				  startOfNoActivityTime = HAL_GetTick();
			  startOfNoActivity = 1;
			  if((HAL_GetTick() - startOfNoActivityTime) > 500)
				  activity = 0;
		  }
		  else {
			  activity = 1;
			  startOfNoActivity = 0;
		  }
		  handleIron(activity);
		  iron_temp_measure_state = iron_temp_measure_idle;
		  Benchmark._05_main_calc_dur = TOCK;
	  }

	  if(HAL_GetTick() - lastTimeDisplay > 50) {
		  TICK;
		  handle_buzzer();
		  RE_Rotation_t r = RE_Get(&RE1_Data);
		  oled_update();
		  oled_processInput(r, &RE1_Data);
		  oled_draw();
		  lastTimeDisplay = HAL_GetTick();
		  Benchmark._04_oled_update_dur = TOCK;
	  }
  }

}
