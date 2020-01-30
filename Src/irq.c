#include "irq.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "init.h"
#include "adc_global.h"
#include "iron.h"
#include "string.h" //memset
#include "filtrai.h"
#include "config.h"


volatile uint16_t iron_temp_adc_avg = 0;
extern volatile int CNDTR;


#ifdef FLAWLESS_MEAS
	#define CNDTR_IDX  POS( ADC_MEASURES_LEN - CNDTR )
	static int8_t iron_state_ref[ADC_MEASURES_LEN] = { [0 ... ADC_MEASURES_LEN-1] = -1};
	inline void PWM_TIM3_PULSE(TIM_HandleTypeDef *htim){

		iron_state_ref[ CNDTR_IDX ] = GPIO_PIN_RESET;

	}

	#define IRQ_LAG_IN_SAMPLES 1

	inline void PWM_TIM3_RESET(TIM_HandleTypeDef *htim){
		static uint32_t prev_idx;
		uint32_t idx = ( CNDTR_IDX -IRQ_LAG_IN_SAMPLES +ADC_MEASURES_LEN ) % ADC_MEASURES_LEN;

		if( prev_idx > idx){
			/* fifo overflow */
			memset(&iron_state_ref, GPIO_PIN_SET, idx);
		}

		prev_idx = idx;
		iron_state_ref[ idx ] = GPIO_PIN_SET;
	}

#else //FLAWLESS_MEAS

void START_MEASURE(TIM_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		memset((void*)&adc_measures.iron[0], 0,sizeof(adc_measures));
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN );
}

#endif //FLAWLESS_MEAS


void PROCESS_MEASUREMENT_DATA(ADC_HandleTypeDef *htim){

		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

#ifdef FLAWLESS_MEAS
		if(iron_state_ref[0] == -1 ){
			iron_state_ref[0]= GPIO_PIN_RESET;}
		replace_ignored_val_by_neighbours(-1, &iron_state_ref[0], ADC_MEASURES_LEN, &iron_state_ref[0]);
		iron_temp_adc_avg = arr_calc_avgU16_when_ref_value_is(&adc_measures.iron[0], ADC_MEASURES_LEN, &iron_state_ref[0], GPIO_PIN_RESET);
		memset(&iron_state_ref, -1, sizeof(iron_state_ref));
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN ); /* never stop adc */
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
