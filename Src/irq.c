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


void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim){
	static uint16_t ironTempADCRollingAverage[2] = {0};
	static int rindex = 0;
	static int doOnce = 0;
	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t temp;


		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

		//__HAL_ADC_DISABLE(&hadc1);
		//__HAL_ADC_DISABLE(&hadc2);


	//		__HAL_ADC_ENABLE
		//__HAL_ADC_DISABLE(&hadc1);
		CNDTR = hadc1.DMA_Handle->Instance->CNDTR;

		for(int x = 0; x < sizeof(adc_measures)/sizeof(adc_measures[0].iron); ++x) {
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


}

void UPDATE_PWM_TURN_ON(void){
	if(getIronOn()){
		update_pwm();
		HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
	}
}
