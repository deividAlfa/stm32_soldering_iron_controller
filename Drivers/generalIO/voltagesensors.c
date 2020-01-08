/*
 * voltagesensors.c
 *
 *  Created on: Jul 26, 2017
 *      Author: jose
 */

#include "adc_global.h"
#include "main.h"
#define R1 10000
#define R2 1000

#ifndef FAKE_SUPPLY_VOLTAGE_OK
uint16_t getSupplyVoltage_mv() {
	uint32_t ad_sum = 0;
	uint32_t max = 0, min = 0xFFFFFFFF;
	uint32_t avg_data;
	uint16_t measure;
	uint8_t valid_samples;
	for(valid_samples = 0; valid_samples < (sizeof(adc_measures)/sizeof(adc_measures_t)); ++valid_samples) {
		measure = adc_measures[valid_samples].supply;
		ad_sum += measure;
		if(measure < min)
			min = measure;
		if(measure > max)
			max = measure;
	}
	ad_sum = ad_sum - max - min;
	avg_data = (uint16_t)ad_sum / (valid_samples - 2);
	return (3300 * avg_data / 4096.0)*((R1+R2)/R2);
}

#else
uint16_t getSupplyVoltage_mv() {
	return 24000;
}
#endif

