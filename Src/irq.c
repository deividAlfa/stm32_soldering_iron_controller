#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "adc_global.h"
#include "rotary_encoder.h"
#include "iron.h"
#include "string.h"
#include "config.h"

volatile uint16_t iron_temp_adc_avg = 0;
static uint32_t pwmStoppedSince = 0;

#define ADC_MEASURE_DELAY	0


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef tim3_pwm;
extern TIM_HandleTypeDef tim4_temp_measure;
extern RE_State_t RE1_Data;


struct{
	int elapsed_cnt;
	int pulse_cnt;
}tim3_stats;
int err2=0;
volatile int samples_dumped, CNDTR;
uint32_t tick_dt;
uint32_t started ;


void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim){
		tim3_stats.pulse_cnt ++;
		if(htim == &tim3_pwm ){
			tim3_stats.pulse_cnt ++;
		}
	}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == &tim3_pwm ){
		tim3_stats.elapsed_cnt++;
	}
	if(htim != &tim4_temp_measure)
		return;
	TICK_TOCK(Benchmark._02_timer_T);

	if(iron_temp_measure_state == iron_temp_measure_idle) {
		iron_temp_measure_state = iron_temp_measure_requested;
		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		iron_temp_measure_state = iron_temp_measure_pwm_stopped;
		pwmStoppedSince = HAL_GetTick();		

		memset((void*)&adc_measures[0],0,sizeof(adc_measures));


//		int idx = hadc1.DMA_Handle->Instance->CNDTR;
//		idx = (idx>ADC_MEASURES_LEN)?ADC_MEASURES_LEN:idx;
//		idx = (idx==0)?ADC_MEASURES_LEN:idx;
//		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) &adc_measures.iron[samples_dumped], idx);
		iron_temp_measure_state = iron_temp_measure_started;
		//__HAL_ADC_ENABLE(&hadc1);
		//__HAL_ADC_ENABLE(&hadc2);
		started = HAL_GetTick();
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures, sizeof(adc_measures)/ sizeof(uint32_t));

	}else{

		err2++;
	}
}


#define FIFO_LEN 			(sizeof(adc_measures)/sizeof(adc_measures[0]))
#define ROLLING_AVG_LEN 	(sizeof(ironTempADCRollingAverage)/sizeof(ironTempADCRollingAverage[0]))
int aleliuja;
int err=0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){

	TICK_TOCK(Benchmark._03_sample_period);
	static uint16_t ironTempADCRollingAverage[2] = {0};
	static int rindex = 0;
	static int doOnce = 0;
	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t temp;
	if(hadc != &hadc1)
		return;
	if((iron_temp_measure_state == iron_temp_measure_pwm_stopped) && (HAL_GetTick() - pwmStoppedSince > ADC_MEASURE_DELAY)) {
		iron_temp_measure_state = iron_temp_measure_started;
		return;
	} else if(iron_temp_measure_state == iron_temp_measure_started) {

#ifdef TEST_ALLTIME_SAMPLING
		 HAL_DMA_Abort(&hadc1.DMA_Handle);
#else
		HAL_ADCEx_MultiModeStop_DMA(&hadc1);
#endif
		Benchmark._01_sample_dur = HAL_GetTick() - started;
		//__HAL_ADC_DISABLE(&hadc1);
		//__HAL_ADC_DISABLE(&hadc2);


//		__HAL_ADC_ENABLE
		//__HAL_ADC_DISABLE(&hadc1);
		CNDTR = hadc1.DMA_Handle->Instance->CNDTR;

		for(int x = 0; x < sizeof(adc_measures)/sizeof(adc_measures[0]); ++x) {
			temp = adc_measures[x].iron;
			if( adc_measures[x].iron==0) err++;

			acc += temp;
			if(temp > max)
				max = temp;
			if(temp < min)
				min = temp;
		}
//		acc = acc - min - max;
//		uint16_t last = acc / ((sizeof(adc_measures)/sizeof(adc_measures[0])) -2);
		iron_temp_measure_state = UINT_DIV( (acc - min - max) , FIFO_LEN-2);
		uint16_t last = UINT_DIV( (acc - min - max) , FIFO_LEN-2);
		ironTempADCRollingAverage[rindex] = last;
		if(doOnce) {
			for(uint8_t x = 0; x < sizeof(ironTempADCRollingAverage)/sizeof(ironTempADCRollingAverage[0]); ++x) {
				ironTempADCRollingAverage[x] = last;
			}
			doOnce =0;
		}
		rindex = (rindex + 1) % ROLLING_AVG_LEN;
		acc = 0;
		for(uint8_t x = 0; x < sizeof(ironTempADCRollingAverage)/sizeof(ironTempADCRollingAverage[0]); ++x) {
			acc += ironTempADCRollingAverage[x];
		}
		iron_temp_adc_avg = UINT_DIV(acc, ROLLING_AVG_LEN); //minimize divide error from +-1 to +- 0.5
		iron_temp_measure_state = iron_temp_measure_ready;

		if(getIronOn())
			update_pwm();
			HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
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
