/*
 * adc_global.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "adc_global.h"
#include "buzzer.h"
#include "iron.h"







volatile adc_measures_t ADC_measures[ADC_BFSIZ] = { 0 };
volatile uint16_t Tip_measures[ADC_BFSIZ] = { 0 };
ADC_Status_t ADC_Status = ADC_Idle;


ADCDataTypeDef_t TIP = {
		adc_buffer: &ADC_measures[0].TIP
};

#ifdef USE_VIN
ADCDataTypeDef_t VIN = {
		adc_buffer: &ADC_measures[0].VIN
};
#endif

#ifdef USE_NTC
ADCDataTypeDef_t NTC = {
		adc_buffer: &ADC_measures[0].NTC
};
#endif

#ifdef USE_VREF
ADCDataTypeDef_t VREF = {
		adc_buffer: &ADC_measures[0].VREF
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
		ADC_Status = ADC_Idle;			                                            // Set the ADC status
		buzzer_short_beep();
	}
}

void ADC_Start_DMA(){
	ADC_ChannelConfTypeDef sConfig = {0};

	if(ADC_Status!=ADC_Waiting){
		Error_Handler();
	}
	#ifdef STM32F072xB
		adc_device->Instance->CHSELR &= ~(0x7FFFF);		                          // Disable all regular channels
		sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	#endif

  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
		adc_device->Init.NbrOfConversion = ADC_Num;
  #endif

  adc_device->Init.ExternalTrigConv = ADC_SOFTWARE_START;					      // Set software trigger
  if (HAL_ADC_Init(adc_device) != HAL_OK) { Error_Handler(); }

  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;									    // More sampling time to compensate high input impedances

  #ifdef ADC_CH_1ST
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_1;
    #endif
    sConfig.Channel = ADC_CH_1ST;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_2ND
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_2;
    #endif
    sConfig.Channel = ADC_CH_2ND;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_3RD
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_3;
    #endif
    sConfig.Channel = ADC_CH_3RD;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_4TH
  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
    sConfig.Rank = ADC_REGULAR_RANK_4;
  #endif
  sConfig.Channel = ADC_CH_4TH;
  if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif
  // Start ADC conversion now
  if(HAL_ADC_Start_DMA(adc_device, (uint32_t*)ADC_measures, sizeof(ADC_measures)/ sizeof(uint16_t) )!=HAL_OK){
    Error_Handler();
  }
  ADC_Status=ADC_Sampling;
}


void ADC_Stop_DMA(void){
	HAL_ADC_Stop_DMA(adc_device);
}

/*
 * Some credits: https://kiritchatterjee.wordpress.com/2014/11/10/a-simple-digital-low-pass-filter-in-c/
 */
void DoAverage(ADCDataTypeDef_t* InputData){
	volatile uint16_t *inputBuffer=InputData->adc_buffer;
	uint32_t adc_sum,avg_data;
	uint16_t max=0, min=0xffff;
	uint8_t shift = systemSettings.Profile.filterFactor;						// Set EMA factor setting from system settings

	// Make the average of the ADC buffer
	adc_sum = 0;
	for(uint16_t x = 0; x < ADC_BFSIZ; x++) {
		adc_sum += *inputBuffer;
		if(*inputBuffer > max){
			max = *inputBuffer;
		}
		if(*inputBuffer < min){
			min = *inputBuffer;
		}
		inputBuffer += ADC_Num;
	}
	//Remove highest and lowest values
	adc_sum -=  (min + max);

	// Calculate average
	avg_data = adc_sum / (ADC_BFSIZ -2) ;
	InputData->last_RawAvg = avg_data;
	
	if(systemSettings.Profile.filterMode == filter_ema) {					  // Advanced filtering enabled?

		if(systemSettings.Profile.filterFactor>4){						        // Limit coefficient (3 is already to much in most cases)
			systemSettings.Profile.filterFactor=4;
		}

		// Fixed point shift
		uint32_t RawData = avg_data << 12;

		// Compute EMA of input
		int32_t EMA = InputData->EMA_of_Input>>12;
		int32_t diff = (int32_t)avg_data - EMA;										  // Check difference between stored EMA and last average
		if(abs(diff)>299){															            // If huge (Filtering will delay too much the response)
			InputData->EMA_of_Input = (uint32_t)avg_data<<12;					// Reset stored to last average
		}
		else if(abs(diff)>200){														          // If medium, smoothen the difference
			uint8_t ratio = abs(diff)-100;										        // 1-99%
			// Output: (100-ratio)% of old value + (ratio)% of new value
			InputData->EMA_of_Input = ((uint32_t)(((avg_data*ratio)/100)+((EMA*(100-ratio))/100)))<<12;
		}
		else{
			InputData->EMA_of_Input = ( ((InputData->EMA_of_Input << shift) - InputData->EMA_of_Input) + RawData +(1<<(shift-1)))>>shift;
		}
		InputData->last_avg = InputData->EMA_of_Input>>12;
	}
	else {
		InputData->last_avg=avg_data;
	}
}

uint16_t ADC_to_mV (uint16_t adc){
	/*
	 * Instead running ( ADC*(3300/4095) ),
	 * We previously multiply (3300/4095)*2^20 = 845006
	 * Then we can use the fast hardware multiplier and
	 * divide just with bit rotation.
	 *
	 * So it becomes Vadc = (ADC * 845006) >>20
	 * Max possible input = 20 bit number, more will cause overflow to the 32 bit variable
	 * Calculated to use  12 bit max input from ADC (4095)
	 * Much, much faster than floats!
	 */

	return( ((uint32_t)845006*adc)>>20 );
}


// Don't call this function, only the ADC ISR should use it.
void handle_ADC_Data(void){
  DoAverage(&TIP);
  #ifdef USE_VREF
  DoAverage(&VREF);
  #endif
  #ifdef USE_NTC
  DoAverage(&NTC);
  #endif
  #ifdef USE_VIN
  DoAverage(&VIN);
  #endif
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc){
	if(_hadc == adc_device){
		ADC_Stop_DMA();												                                      // Reset the ADC
		if(ADC_Status!=ADC_Sampling){                                               // Check correct status
      Error_Handler();
    }
		handle_ADC_Data();									                                        // Process the new data.
		handleIron();                                                               // Handle iron
		__HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);	    // Load new calculated PWM Duty
		HAL_IWDG_Refresh(&hiwdg);							                                      // Clear watchdog
		ADC_Status = ADC_Idle;                                                      // Set the ADC status
	}
}
