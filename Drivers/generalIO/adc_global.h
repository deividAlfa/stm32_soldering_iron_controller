/*
 * adc_global.h
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#ifndef GENERALIO_ADC_GLOBAL_H_
#define GENERALIO_ADC_GLOBAL_H_


#include "main.h"







typedef struct{
	volatile uint16_t	*adc_buffer;	// Ptr to ADC buffer data
	volatile uint32_t	EMA_of_Input;	// EMA of Input averages
	volatile uint32_t	EMA_of_EMA;		// EMA of EMA (To calculate DEMA)
	volatile uint16_t	last_avg;		// Filtered (DEMA calculation)
	volatile uint16_t	last_RawAvg; 	// Unfiltered, for quick Iron detection
	volatile uint8_t	Filter;			// 0=No filter,  1= EMA filter(some undershoot), 2=DEMA filter(some overshoot, might cause oscillation)
} ADCDataTypeDef_t;




typedef struct {
	#ifdef ADC_1st
					uint16_t ADC_1st;
	#endif
	#ifdef ADC_2nd
					uint16_t ADC_2nd;
	#endif
	#ifdef ADC_3rd
					uint16_t ADC_3rd;
	#endif
	#ifdef ADC_4th
					uint16_t ADC_4th;
	#endif
} adc_measures_t;

#ifdef ADC_TIP
				extern ADCDataTypeDef_t TIP;
#endif
#ifdef ADC_VIN
				extern ADCDataTypeDef_t VIN;
#endif
#ifdef ADC_NTC
				extern ADCDataTypeDef_t NTC;
#endif
#ifdef ADC_VREF
				extern ADCDataTypeDef_t VREF;
#endif



typedef enum { ADC_Idle, ADC_StartTip, ADC_InitTip, ADC_SamplingTip, ADC_StartOthers, ADC_SamplingOthers } ADC_Status_t;

extern ADC_Status_t ADC_Status;
//extern adc_measures_t adc_measures[ADC_BFSIZ];

uint16_t ADC_to_mV (uint16_t adc);
void handle_ADC(void);
void DoAverage(ADCDataTypeDef_t* InputData);
void DEMA_Filter(ADCDataTypeDef_t* InputData);
void ADC_Init(ADC_HandleTypeDef *adc);
uint8_t ADC_Cal(void);
void ADC_Stop_DMA(void);
void ADC_Start_DMA(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc);
#endif /* GENERALIO_ADC_GLOBAL_H_ */
