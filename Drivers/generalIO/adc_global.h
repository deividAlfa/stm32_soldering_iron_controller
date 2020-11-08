/*
 * adc_global.h
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#ifndef GENERALIO_ADC_GLOBAL_H_
#define GENERALIO_ADC_GLOBAL_H_


#include "main.h"

typedef struct {
	uint16_t ADC_CH1;
	uint16_t ADC_CH2;
	uint16_t ADC_CH3;
	uint16_t ADC_CH4;
	//uint16_t int_temp;
	//uint16_t int_ref;
} adc_measures_t;


typedef struct{
	  uint16_t	*adc_buffer;
	  uint16_t 	adc_buffer_size;
	  uint16_t  adc_buffer_elements;
	  uint16_t	rolling_buffer_size;
	  uint16_t  rolling_buffer[RollingBufferSize];		//from Setup.h
	  uint16_t	rolling_buffer_index;
	  uint16_t	last_avg;
	  uint8_t	init;
} RollingTypeDef_t;

typedef enum { adc_dma_active, adc_dma_idle } adc_dma_status_t;

extern adc_dma_status_t adc_dma_status;
extern adc_measures_t adc_measures[Adc_Buffer_Size];

uint16_t ADC_to_mV (uint16_t adc);
void RollingUpdate(RollingTypeDef_t* Data);
void ADC_Init(ADC_HandleTypeDef *adc);
uint8_t ADC_Cal(void);
uint8_t ADC_Stop_DMA(void);
uint8_t  ADC_Start_DMA(void);


#endif /* GENERALIO_ADC_GLOBAL_H_ */
