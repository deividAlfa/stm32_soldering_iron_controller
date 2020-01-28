#include "irq.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "init.h"
#include "adc_global.h"
#include "iron.h"
#include "string.h"
#include "config.h"

volatile int samples_dumped, CNDTR;
uint32_t tick_dt;
volatile uint16_t iron_temp_adc_avg = 0;

#define ADC_MEASURE_DELAY	0

inline void PWM_TIM3_PULSE(TIM_HandleTypeDef *htim){


}

inline void PWM_TIM3_RESET(TIM_HandleTypeDef *htim){

}


void START_MEASURE(TIM_HandleTypeDef *htim){
		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();

		memset((void*)&adc_measures[0],0,sizeof(adc_measures));


//		int idx = hadc1.DMA_Handle->Instance->CNDTR;
//		idx = (idx>ADC_MEASURES_LEN)?ADC_MEASURES_LEN:idx;
//		idx = (idx==0)?ADC_MEASURES_LEN:idx;
//		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) &adc_measures.iron[samples_dumped], idx);
		//__HAL_ADC_ENABLE(&hadc1);
		//__HAL_ADC_ENABLE(&hadc2);
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures, ADC_MEASURES_LEN );
}


#define FIFO_LEN 			(sizeof(adc_measures)/sizeof(adc_measures[0]))
#define ROLLING_AVG_LEN 	(sizeof(ironTempADCRollingAverage)/sizeof(ironTempADCRollingAverage[0]))
int aleliuja;
int err=0;


uint16_t arr_u16_avg(uint16_t* arr, uint16_t len){
	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t temp;
	uint16_t result;
	for(int x = 0; x < len; x++) {
		temp = *arr++;
		acc += temp;
		if(temp > max)
			max = temp;
		if(temp < min)
			min = temp;
	}

	result = UINT_DIV( (acc - min - max) , len-2);
	return result;
}

void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim){

		HAL_ADCEx_MultiModeStop_DMA(&hadc1);
		CNDTR = hadc1.DMA_Handle->Instance->CNDTR;
		iron_temp_adc_avg = arr_u16_avg(&adc_measures[0].iron, ADC_MEASURES_LEN);
}

void UPDATE_PWM_TURN_ON(void){
	if(getIronOn()){
		update_pwm();
		HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
	}
}
