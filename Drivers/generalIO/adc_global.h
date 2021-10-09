/*
 * adc_global.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GENERALIO_ADC_GLOBAL_H_
#define GENERALIO_ADC_GLOBAL_H_


#include "main.h"
#include "settings.h"


typedef struct{
  volatile filter_t   filter;
  #ifdef DEBUG_PWM
  volatile int16_t    prev_avg;
  volatile int16_t    prev_raw;
  #endif
  volatile int16_t    last_avg;               // Filtered (EMA calculation)
  volatile int16_t    last_raw;               // Unfiltered, for quick Iron detection
  volatile float      EMA_of_Input;           // Stored filter data (acumulator for EMA)
  volatile uint16_t   *adc_buffer;            // Ptr to ADC buffer data

} ADCDataTypeDef_t;


typedef struct {
  #ifdef ADC_1st
          uint16_t  ADC_1st;
  #endif
  #ifdef ADC_2nd
          uint16_t  ADC_2nd;
  #endif
  #ifdef ADC_3rd
          uint16_t  ADC_3rd;
  #endif
  #ifdef ADC_4th
          uint16_t  ADC_4th;
  #endif
} adc_measures_t;

extern volatile ADCDataTypeDef_t TIP;

#ifdef USE_VIN
        extern volatile ADCDataTypeDef_t VIN;
#endif
#ifdef USE_NTC
        extern volatile ADCDataTypeDef_t NTC;
#endif
#ifdef USE_VREF
        extern volatile ADCDataTypeDef_t VREF;
#endif
#ifdef ENABLE_INT_TEMP
        extern volatile ADCDataTypeDef_t INT_TMP;
#endif


typedef enum { ADC_Idle, ADC_Waiting, ADC_ProbingTip, ADC_Sampling } ADC_Status_t;

extern volatile ADC_Status_t ADC_Status;
extern volatile uint16_t Tip_measures[ADC_BFSIZ];
extern volatile adc_measures_t adc_measures[ADC_BFSIZ];

uint16_t ADC_to_mV (uint16_t adc);
void handle_ADC_Data(void);
void DoAverage(volatile ADCDataTypeDef_t* InputData);
void ADC_Init(ADC_HandleTypeDef *adc);
uint8_t ADC_Cal(void);
void ADC_Reset_measures(void);
void ADC_Stop_DMA(void);
void ADC_Start_DMA(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc);
#endif /* GENERALIO_ADC_GLOBAL_H_ */
