/*
 * adc_global.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GENERALIO_ADC_GLOBAL_H_
#define GENERALIO_ADC_GLOBAL_H_


#include "main.h"







typedef struct{
	volatile uint16_t	*adc_buffer;	        // Ptr to ADC buffer data
	volatile uint16_t	last_avg;		          // Filtered (DEMA calculation)
	volatile uint16_t	last_RawAvg; 	        // Unfiltered, for quick Iron detection
	volatile uint32_t	EMA_of_Input;	        // EMA of Input averages
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


				extern ADCDataTypeDef_t TIP;

#ifdef USE_VIN
				extern ADCDataTypeDef_t VIN;
#endif
#ifdef USE_NTC
				extern ADCDataTypeDef_t NTC;
#endif
#ifdef USE_VREF
				extern ADCDataTypeDef_t VREF;
#endif



typedef enum { ADC_Idle, ADC_StartTip, ADC_InitTip, ADC_SamplingTip, ADC_StartOthers, ADC_SamplingOthers } ADC_Status_t;

extern ADC_Status_t ADC_Status;
extern volatile uint16_t Tip_measures[ADC_BFSIZ];
extern adc_measures_t adc_measures[ADC_BFSIZ];

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
