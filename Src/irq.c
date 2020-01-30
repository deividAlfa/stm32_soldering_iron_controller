#include "irq.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#include "init.h"
#include "adc_global.h"
#include "iron.h"
#include "string.h" //memset
#include "filtrai.h"
#include "config.h"

#define IRQ_CB_LAG 1
#define CNDTR_IDX  POS( ADC_MEASURES_LEN - CNDTR )

static int8_t flawless_iron_states[ADC_MEASURES_LEN] = { [0 ... ADC_MEASURES_LEN-1] = -1};
volatile uint16_t iron_temp_adc_avg = 0;
extern volatile int CNDTR;

inline void flawless_iron_off_cb(TIM_HandleTypeDef *htim){

	flawless_iron_states[ CNDTR_IDX ] = GPIO_PIN_RESET;

}

inline void flawless_iron_on_cb(TIM_HandleTypeDef *htim){
	static uint32_t prev_idx;
	uint32_t idx = ( CNDTR_IDX - IRQ_CB_LAG +ADC_MEASURES_LEN ) % ADC_MEASURES_LEN;

	if( prev_idx > idx){
		/* fifo overflow */
		memset(&flawless_iron_states, GPIO_PIN_SET, idx);
	}

	prev_idx = idx;
	flawless_iron_states[ idx ] = GPIO_PIN_SET;
}

void flawless_adc_process(ADC_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

		if(flawless_iron_states[0] == -1 ){
			flawless_iron_states[0]= GPIO_PIN_SET;}
		replace_ignored_val_by_neighbours(-1, &flawless_iron_states[0], ADC_MEASURES_LEN, &flawless_iron_states[0]);
		iron_temp_adc_avg = arr_calc_avgU16_when_ref_value_is(&adc_measures.iron[0], ADC_MEASURES_LEN, &flawless_iron_states[0], GPIO_PIN_RESET);
		memset(&flawless_iron_states, -1, sizeof(flawless_iron_states));


		if(getIronOn()){
			update_pwm();
			HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
		}
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN ); /* never stop adc */


}

void blocking_mode_adc_start(TIM_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		memset((void*)&adc_measures.iron[0], 0,sizeof(adc_measures));
		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) adc_measures.iron, ADC_MEASURES_LEN );
}


void blocking_mode_adc_process(ADC_HandleTypeDef *htim){

		HAL_ADCEx_MultiModeStop_DMA(&hadc1);
		iron_temp_adc_avg = arr_u16_avg(&adc_measures.iron[0], ADC_MEASURES_LEN);
		if(getIronOn()){
			update_pwm();
			HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
		}

}






