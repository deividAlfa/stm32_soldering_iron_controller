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
volatile uint16_t new_len;
volatile int good_vals;

void flawless_adc_ConvCpltCb(ADC_HandleTypeDef *htim){

		HAL_TIM_PWM_Stop(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOff();
		HAL_ADCEx_MultiModeStop_DMA(&hadc1);

//		int tmp = 0;
//		for(int i=0; i<ADC_MEASURES_LEN; i++){
//			if(adc_measures.iron[i]<3800 && adc_measures.iron[i]> 30){
//				tmp++;
//			}
//		}
//		good_vals = tmp;

//		uint16_t tmp_arr[];
//		memcpy(&tmp_arr[0],&adc_measures.iron[ADC_MEASURES_LEN-1], *sizeof(tmp_arr[0]));
		arr_set_zeros_above_threshold(&adc_measures.iron[0],ADC_MEASURES_LEN, 4000, 10, 30 );
		uint16_t new_len ;
		iron_temp_adc_avg = arr_u16_avg_ignore_val(0, &adc_measures.iron[30], ADC_MEASURES_LEN-30, &new_len);

		if(new_len <=2){
			iron_temp_adc_avg = 4055;
		}

		//memcpy(&adc_measures.iron[0], &tmp_arr[0], *sizeof(tmp_arr[0]));

		if(getIronOn()){
			update_pwm();
			HAL_TIM_PWM_Start(&tim3_pwm, TIM_CHANNEL_3);  // don't use turnIronOn();
		}

		HAL_ADCEx_MultiModeStart_DMA(&hadc1, (uint32_t*) &adc_measures.iron, ADC_MEASURES_LEN ); /* never stop adc */
}








