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
		adc_buffer: 			&adc_measures[0].V_INPUT,
		adc_buffer_size: 		Adc_Buffer_Size,
		adc_buffer_elements:	Adc_Buffer_Elements,
		rolling_buffer_size: 	RollingBufferSize,
		rolling_buffer_index: 	0,
		last_avg: 				0
	};

	if(!data.init){
		uint16_t x = data.rolling_buffer_size;
		for(x=0;x<data.rolling_buffer_size;x++) {
			data.rolling_buffer[x]=0;
		}
		data.init=1;
	}

	RollingUpdate(&data);	//Update average

	// As we have a R1(10K) / R2(1K) resistor divider, the ratio is 1/(10+1)=0.09090909
	// So we previously divide (2<<16)/0.09090909 = 720896
	// Now we can use hardware multiplier and then make bit shifting
    // avoiding using floats
	temp = (uint32_t)720896 * ADC_to_mV(data.last_avg);		// Constant multiply
	temp >>= 16; 											// Bit shifting (divide by 65536) = mV corrected
	temp+=50;												// Round number
	return (temp/100);										// Return Supply  V*10
}

uint16_t getReferenceVoltage_mv_x10() {
	static RollingTypeDef_t data = {
		adc_buffer: 			&adc_measures[0].VREF,
		adc_buffer_size: 		Adc_Buffer_Size,
		adc_buffer_elements:	Adc_Buffer_Elements,
		rolling_buffer_size: 	RollingBufferSize,
		rolling_buffer_index: 	0,
		last_avg: 				0,
		init: 					0
	};

	if(!data.init){
		uint16_t x = data.rolling_buffer_size;
		for(x=0;x<data.rolling_buffer_size;x++) {
			data.rolling_buffer[x]=0;
		}
		data.init=1;
	}

	RollingUpdate(&data);	//Update average

	return ADC_to_mV(data.last_avg);

}

