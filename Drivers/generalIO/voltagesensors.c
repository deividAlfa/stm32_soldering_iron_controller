/*
 * voltagesensors.c
 *
 *  Created on: Jul 26, 2017
 *      Author: jose
 */

#include "adc_global.h"

uint16_t getSupplyVoltage_v_x10() {
	uint32_t temp;
	static RollingTypeDef_t data = {
		adc_buffer: 			&adc_measures[0].supply,
		adc_buffer_size: 		Adc_Buffer_Size,
		adc_buffer_elements:	Adc_Buffer_Elements,
		rolling_buffer_size: 	RollingBufferSize,
		rolling_buffer: 		0,
		rolling_buffer_index: 	0,
		last_avg: 				0
	};
	RollingUpdate(&data);	//Update average

	//As we have a 10k/1k divider, the ratio is 1/(10+1)=0.09090909
	// So we previously divide (2<<16)/0.09090909 = 720896
	// Now we can use hardware multiplier and then make bit shifting

	temp = (uint32_t)720896 * ADC_to_mV(data.last_avg);		//
	temp >>= 16; 											// mV corrected
	temp+=50;												// Round number
	return (temp/100);										// Return V*10
}


