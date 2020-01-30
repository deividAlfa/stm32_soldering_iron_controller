#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "adc_global.h"
#include "rotary_encoder.h"
#include "iron.h"
#include "string.h"
#include "config.h"
#include "irq.h"
#include "init.h"


uint32_t started ;
struct{
	int elapsed_cnt;
	int pulse_cnt;
}tim3_stats;

inline void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
		if(htim == &tim3_pwm ){
			tim3_stats.pulse_cnt ++;
			flawless_iron_off_cb(htim);
		}
	}

inline void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == &tim3_pwm ){
		tim3_stats.elapsed_cnt++;
		flawless_iron_on_cb(htim);
	}


	if(htim == &tim4_temp_measure){
		if(iron_temp_measure_state == iron_temp_measure_idle) {
#ifndef FLAWLESS_MEAS
			measure_start_blocking_mode(htim);
#endif
			iron_temp_measure_state = iron_temp_measure_started;

			started = HAL_GetTick();
		}
	}


	TICK_TOCK(Benchmark._02_timer_T);
}



inline void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){

	if(hadc == &hadc1){
		TICK_TOCK(Benchmark._03_sample_period);
		if(iron_temp_measure_state == iron_temp_measure_started){
#ifdef FLAWLESS_MEAS
			flawless_adc_process(hadc);
#else
			blocking_mode_adc_process(hadc);
#endif

			iron_temp_measure_state = iron_temp_measure_ready;
			Benchmark._01_sample_dur = HAL_GetTick() - started;
		}
	}

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if((GPIO_Pin == ROT_ENC_BUTTON_GPIO_Pin) || (GPIO_Pin == ROT_ENC_R_Pin) || (GPIO_Pin == ROT_ENC_L_Pin)) {
		  RE_Process(&RE1_Data);

	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc != &hadc1)
		return;
//	if(iron_temp_measure_state == iron_temp_measure_requested) {
//		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
//		iron_temp_measure_state = iron_temp_measure_pwm_stopped;
//		pwmStoppedSince = HAL_GetTick();
//	}
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}
#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif
