#include "irq.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "init.h"
#include "adc_global.h"
#include "iron.h"
#include "string.h" //memset
#include "filtrai.h"
#include "config.h"
volatile int CNDTR;

volatile uint16_t iron_temp_adc_avg = 0;



#ifdef FLAWLESS_MEAS
	#define CNDTR_IDX  POS( ADC_MEASURES_LEN - hadc1.DMA_Handle->Instance->CNDTR )
	uint8_t adc_measures_mask[ADC_MEASURES_LEN];
	inline void PWM_TIM3_PULSE(TIM_HandleTypeDef *htim){
		CNDTR = hadc1.DMA_Handle->Instance->CNDTR; /* CNDTR has lag? */

		adc_measures_mask[ CNDTR_IDX ] = 2;

	}

	#define IRQ_LAG_IN_SAMPLES 1

	inline void PWM_TIM3_RESET(TIM_HandleTypeDef *htim){
		uint32_t idx = ( CNDTR_IDX -IRQ_LAG_IN_SAMPLES +ADC_MEASURES_LEN ) % ADC_MEASURES_LEN;
		adc_measures_mask[ idx ] = 1;
	}
#endif //FLAWLESS_MEAS

void START_MEASURE(TIM_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		memset((void*)&adc_measures.iron[0], 0,sizeof(adc_measures));
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN );
}



void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim){

		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

#ifdef FLAWLESS_MEAS
		replace_zeros_with_neighbours_u8(&adc_measures_mask[0], ADC_MEASURES_LEN, &adc_measures_mask[0], 2);
		iron_temp_adc_avg = arr_u16_avg_with_u8_ref(&adc_measures.iron[0], ADC_MEASURES_LEN, &adc_measures_mask[0], 2);
		memset(&adc_measures_mask, 0, sizeof(adc_measures_mask));
#else
		iron_temp_adc_avg = arr_u16_avg(&adc_measures.iron[0], ADC_MEASURES_LEN);
#endif

}

void UPDATE_PWM_TURN_ON(void){
	if(getIronOn()){
		update_pwm();
		HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
	}
}
