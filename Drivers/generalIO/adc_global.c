/*
 * adc_global.c
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#include "adc_global.h"
#include "buzzer.h"
#include "iron.h"







volatile adc_measures_t ADC_measures[Adc_Buffer_Size] = { 0 };
volatile uint16_t Tip_measures[Adc_Buffer_Size] = { 0 };
ADC_Status_t ADC_Status = ADC_Idle;

#ifdef ADC_TIP
ADCDataTypeDef_t TIP = {
		adc_buffer: &Tip_measures[0],
		last_avg: 100,
		last_RawAvg: 0,
		EMA_of_EMA:100<<12,
		EMA_of_Input:100<<12,
		Filter:1
};
#endif

#ifdef ADC_VIN
ADCDataTypeDef_t VIN = {
		adc_buffer: &ADC_measures[0].VIN,
		Filter:0
};
#endif

#ifdef ADC_NTC
ADCDataTypeDef_t NTC = {
		adc_buffer: &ADC_measures[0].NTC,
		Filter:0
};
#endif

#ifdef ADC_VREF
ADCDataTypeDef_t VREF = {
		adc_buffer: &ADC_measures[0].VREF,
		Filter:0
};
#endif

static ADC_HandleTypeDef *adc_device;



uint8_t ADC_Cal(void){
	return HAL_ADCEx_Calibration_Start(adc_device);
}


void ADC_Init(ADC_HandleTypeDef *adc){

	adc_device=adc;

	if(ADC_Cal() != HAL_OK ){
		buzzer_alarm_start();
	}
	else{
		ADC_Status = ADC_StartTip;			// Set the ADC status
		ADC_Start_DMA();					// Prepare ADC for next trigger
		buzzer_short_beep();
	}
}

void ADC_Start_DMA(){
	ADC_ChannelConfTypeDef sConfig = {0};

	if( (ADC_Status!=ADC_StartTip) && (ADC_Status!=ADC_StartOthers)){
		return;
	}


	adc_device->Instance->CHSELR &= ~(0x7FFFF);		// Disable all regular channels
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;

	if(ADC_Status == ADC_StartOthers){

			adc_device->Init.ExternalTrigConv = ADC_SOFTWARE_START;						// Set software trigger
			if (HAL_ADC_Init(adc_device) != HAL_OK) { Error_Handler(); }
			ADC_Status = ADC_SamplingOthers;
			sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;							// More sampling time to compensate high input impedances

			#ifdef ADC_VREF
			sConfig.Channel = ADC_VREF;
			if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
			#endif

			#ifdef ADC_NTC
			sConfig.Channel = ADC_NTC;
			if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
			#endif

			#ifdef ADC_VIN
			sConfig.Channel = ADC_VIN;
			if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
			#endif

			// Start ADC (Will be triggered by TIM15), Only Tip measurement
			if(HAL_ADC_Start_DMA(adc_device, (uint32_t*)ADC_measures, sizeof(ADC_measures)/ sizeof(uint16_t) )!=HAL_OK){
				Error_Handler();
			}
	}
	else if(ADC_Status == ADC_StartTip){

			ADC_Status = ADC_InitTip;
			adc_device->Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T15_TRGO;		// Set trigger by Timer15 TRGO
			if (HAL_ADC_Init(adc_device) != HAL_OK) { Error_Handler(); }

			#ifdef ADC_TIP
			sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
			sConfig.Channel = ADC_TIP;
			if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
			#else
			#error ADC_IRON not configured properly
			#endif
			// Start ADC, start conversion, other measurements (non time-critical)
			if(HAL_ADC_Start_DMA(adc_device, (uint32_t*)Tip_measures, sizeof(Tip_measures)/ sizeof(uint16_t) )!=HAL_OK){
				Error_Handler();
			}
	}
}

void ADC_Stop_DMA(void){
	HAL_ADC_Stop_DMA(adc_device);
}
/*
 * Credits: https://kiritchatterjee.wordpress.com/2014/11/10/a-simple-digital-low-pass-filter-in-c/
 *
 */
void DoAverage(ADCDataTypeDef_t* InputData){
	volatile uint16_t *inputBuffer=InputData->adc_buffer;
	uint32_t adc_sum,avg_data;
	uint16_t max=0, min=0xffff;
	uint8_t step;


	if(InputData==&TIP){
		step=1;					// Tip uses its own buffer
	}
	else{
		step=ADC_AuxNum;		// Number of elements in secondary buffer
	}
	// Make the average of the ADC buffer
	adc_sum = 0;
	for(uint16_t x = 0; x < Adc_Buffer_Size; x++) {
		adc_sum += *inputBuffer;
		if(*inputBuffer > max){
			max = *inputBuffer;
		}
		if(*inputBuffer < min){
			min = *inputBuffer;
		}
		inputBuffer += step ;
	}
	//Remove highest and lowest values
	adc_sum -=  (min + max);

	// Calculate average
	avg_data = adc_sum / (Adc_Buffer_Size -2) ;
#ifdef USE_FILTER							// Global filter flag (setup.h)
	if(InputData->Filter){					// Filtering enabled for this data?

		// Fixed point shift
		uint32_t RawData = avg_data<<12;

		// Calc EMA of input
		InputData->EMA_of_Input = ( ((InputData->EMA_of_Input << FILTER_N) - InputData->EMA_of_Input) +RawData )>>FILTER_N;


		if(InputData->Filter==2){
			// DEMA filter
			InputData->EMA_of_EMA = ( ((InputData->EMA_of_EMA << FILTER_N) - InputData->EMA_of_EMA) + InputData->EMA_of_Input  )>>FILTER_N;
			InputData->last_avg = ((2*InputData->EMA_of_Input)-InputData->EMA_of_EMA)>>12;
		}
		else{
			// EMA Filter
			InputData->last_avg = InputData->EMA_of_Input>>12;
		}
	}
	else {
		InputData->last_avg=avg_data;
	}
#else
	InputData->last_avg=avg_data;
#endif
	InputData->last_RawAvg = avg_data;
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


// Don't call this function, only the ADC ISR should use it.
void handle_ADC(void){

	if(ADC_Status == ADC_SamplingTip){
		#ifdef ADC_TIP
		DoAverage(&TIP);
		#endif
	}
	else if(ADC_Status == ADC_SamplingOthers){
		#ifdef ADC_VREF
		DoAverage(&VREF);
		#endif
		#ifdef ADC_NTC
		DoAverage(&NTC);
		#endif
		#ifdef ADC_VIN
		DoAverage(&VIN);
		#endif
	}
}


