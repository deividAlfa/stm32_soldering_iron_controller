#include "irq.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "init.h"
#include "adc_global.h"
#include "iron.h"
#include "string.h" //memset
#include "filtrai.h"

volatile int CNDTR;

volatile uint16_t iron_temp_adc_avg = 0;


inline void PWM_TIM3_PULSE(TIM_HandleTypeDef *htim){


}

inline void PWM_TIM3_RESET(TIM_HandleTypeDef *htim){

}


void START_MEASURE(TIM_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		memset((void*)&adc_measures.iron[0],0,sizeof(adc_measures));
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN );
}



void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim){

		HAL_ADCEx_MultiModeStop_DMA(&hadc1);
		CNDTR = hadc1.DMA_Handle->Instance->CNDTR;
		iron_temp_adc_avg = arr_u16_avg(&adc_measures.iron[0], ADC_MEASURES_LEN);
}

void UPDATE_PWM_TURN_ON(void){
	if(getIronOn()){
		update_pwm();
		HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
	}
}
