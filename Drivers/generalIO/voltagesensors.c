/*
 * voltagesensors.c
 *
 *  Created on: Jul 26, 2017
 *      Author: jose
 */

#include "adc_global.h"

uint16_t getSupplyVoltage_mv() {
	uint32_t ad_sum = 0;
	uint32_t max = 0xFFFFFFFF, min = 0;
	uint32_t avg_data;
	uint16_t measure;
	uint8_t valid_samples = 0;
	uint8_t temp;
	for(temp = 0; temp < (sizeof(adc_measures)/sizeof(adc_measures_t)) - 1; ++temp) {
		measure = adc_measures[temp].supply;
		ad_sum += measure;
		++valid_samples;
		if(measure < min)
			min = measure;
		if(measure > max)
			max = measure;
	}
	ad_sum = ad_sum - max - min;
	avg_data = ad_sum / (valid_samples - 2);
	return 3300 * avg_data / 4096.0;
}
uint16_t getReferenceVoltage_mv() {
	uint32_t ad_sum = 0;
	uint32_t max = 0xFFFFFFFF, min = 0;
	uint32_t avg_data;
	uint16_t measure;
	uint8_t valid_samples = 0;
	uint8_t temp;
	for(temp = 0; temp < (sizeof(adc_measures)/sizeof(adc_measures_t)) - 1; ++temp) {
		measure = adc_measures[temp].reference;
		ad_sum += measure;
		++valid_samples;
		if(measure < min)
			min = measure;
		if(measure > max)
			max = measure;
	}
	ad_sum = ad_sum - max - min;
	avg_data = ad_sum / (valid_samples - 2);
	return 3300 * avg_data / 4096.0;
}
