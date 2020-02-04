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


volatile uint16_t iron_temp_adc_avg = 0;



void flawless_adc_ConvCpltCb(ADC_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

		uint16_t tmp_arr[ADC_HISTORY_LEN];
		memcpy(&tmp_arr[0],&adc_measures.iron[ADC_MEASURES_LEN-ADC_HISTORY_LEN-1], ADC_HISTORY_LEN);

		arr_set_zeros_above_threshold(&adc_measures.iron[0],ADC_MEASURES_LEN+ADC_HISTORY_LEN, 4000, 10, 30 );

		uint16_t new_len = arr_rem_selected_val(0, &adc_measures.iron[ADC_HISTORY_LEN],ADC_MEASURES_LEN);
		iron_temp_adc_avg = arr_u16_avg(&adc_measures.iron[ADC_HISTORY_LEN], new_len);

		memcpy(&adc_measures.iron[0], &tmp_arr[0], ADC_HISTORY_LEN);

		if(getIronOn()){
			update_pwm();
			HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
		}

		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) &adc_measures.iron[ADC_HISTORY_LEN], ADC_MEASURES_LEN ); /* never stop adc */
}








