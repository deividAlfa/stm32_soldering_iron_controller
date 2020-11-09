/*
 * adc_global.c
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#include "adc_global.h"
#include "buzzer.h"

//From setup.h
adc_measures_t adc_measures[Adc_Buffer_Size];
adc_dma_status_t adc_dma_status = adc_dma_idle;
volatile uint16_t vref_adc_avg = 0;
volatile uint16_t ntc_adc_avg = 0;
volatile uint16_t vinput_adc_avg = 0;
static ADC_HandleTypeDef *hadc_device;

uint8_t ADC_Cal(void){
	return HAL_ADCEx_Calibration_Start(hadc_device);
}


void ADC_Init(ADC_HandleTypeDef *adc){
	hadc_device=adc;
	if(ADC_Cal() != HAL_OK ){
		buzzer_alarm_start();
	}
	else{
		buzzer_short_beep();
	}
}

uint8_t ADC_Start_DMA(void){
	if(	adc_dma_status == adc_dma_idle ){		// If DMA still busy, return
		adc_dma_status = adc_dma_active;
		return( HAL_ADC_Start_DMA(hadc_device, (uint32_t*) adc_measures, sizeof(adc_measures)/ sizeof(uint16_t)) );
	}
	return (HAL_BUSY);
}

uint8_t ADC_Stop_DMA(void){
	uint8_t t;
	t = HAL_ADC_Stop_DMA(hadc_device);
	return t;
}

void RollingUpdate(RollingTypeDef_t* InputData){
	uint16_t *inputBuffer=InputData->adc_buffer;
	volatile uint32_t adc_sum;
	volatile uint16_t avg_data, adc_data ,max=0, min=0xffff;

	// Average of the ADC buffer
	adc_sum = 0;
	for(uint16_t x = 0; x < InputData->adc_buffer_size; x++) {
		adc_data = *inputBuffer;
		adc_sum += adc_data;
		inputBuffer += InputData->adc_buffer_elements;
		if(adc_data > max){
			max = adc_data;
		}
		if(adc_data < min){
			min = adc_data;
		}
	}

	//Remove highest and lowest values
	adc_sum -=  (min + max);

	// Add ADC average to the rolling buffer
	avg_data = adc_sum / (InputData->adc_buffer_size - 2) ;
	InputData->rolling_buffer[InputData->rolling_buffer_index] = avg_data;
	InputData->rolling_buffer_index++;
	InputData->rolling_buffer_index %= InputData->rolling_buffer_size;

	// Average of the rolling buffer
	adc_sum = 0;
	for(uint8_t x = 0; x < InputData->rolling_buffer_size; x++) {
		adc_data = InputData->rolling_buffer[x];
		adc_sum += adc_data;
	}
	InputData->last_avg=adc_sum / InputData->rolling_buffer_size;
}

uint16_t ADC_to_mV (uint16_t adc){
	/*
	 * Instead running ( ADC*(3300/4095) ),
	 * We previously multiply (3300/4095)*2^20 = 845006
	 * Then we can use the fast hardware multiplier and
	 * divide just with bit rotation.
	 *
	 * So it becomes Vadc = (ADC * 845006) >>20
	 * Max 20 bits, more will cause overflow to the 32 bit variable
	 * Much, much faster than floats!
	 */

	return( ((uint32_t)845006*adc)>>20 );
}
