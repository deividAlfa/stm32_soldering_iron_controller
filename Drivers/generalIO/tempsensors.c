/*
 * tempsensors.c
 *
 *  Created on: Jul 21, 2017
 *      Author: jose
 */

#include "tempsensors.h"
#include "math.h"
#define temp_minC  100                 // Minimum calibration temperature in degrees of Celsius
#define temp_maxC  480                 // Maximum calibration temperature in degrees of Celsius
static tipData *currentTipData;



/* Table of ADC sum value, corresponding to temperature. Starting from higher value to lower.
   Next parameters had been used to build table:
     R1(T1): 10kOhm(25°С)
     R/T characteristics table used: EPCOS R/T:7003; B25/100:3625K
     Scheme: A
     Ra: 10kOhm
     U0/Uref: 3.3V/3.3V
     In the temperature range from -20°C to 125°C the error
     caused by the usage of a table is 0.143°C

	 Source: http://www.sebulli.com/ntc/index.php
*/

int NTC_table[257] = {
  3525, 2945, 2365, 2077, 1891, 1755, 1650,
  1565, 1493, 1431, 1377, 1330, 1287, 1248,
  1213, 1181, 1151, 1123, 1097, 1072, 1050,
  1028, 1007, 988, 969, 952, 935, 919, 903,
  888, 874, 860, 847, 834, 821, 809, 797, 786,
  775, 764, 754, 743, 733, 724, 714, 705, 696,
  687, 678, 670, 662, 653, 645, 638, 630, 622,
  615, 608, 601, 593, 587, 580, 573, 566, 560,
  553, 547, 541, 535, 528, 522, 516, 511, 505,
  499, 493, 488, 482, 477, 471, 466, 461, 455,
  450, 445, 440, 435, 430, 425, 420, 415, 410,
  405, 401, 396, 391, 386, 382, 377, 373, 368,
  364, 359, 355, 350, 346, 341, 337, 333, 328,
  324, 320, 316, 311, 307, 303, 299, 295, 291,
  286, 282, 278, 274, 270, 266, 262, 258, 254,
  250, 246, 242, 238, 234, 230, 226, 222, 218,
  214, 211, 207, 203, 199, 195, 191, 187, 183,
  179, 176, 172, 168, 164, 160, 156, 152, 148,
  144, 141, 137, 133, 129, 125, 121, 117, 113,
  109, 105, 101, 97, 93, 90, 86, 82, 78, 73,
  69, 65, 61, 57, 53, 49, 45, 41, 37, 32, 28,
  24, 20, 15, 11, 7, 2, -2, -6, -11, -15, -20,
  -25, -29, -34, -38, -43, -48, -53, -58, -63,
  -68, -73, -78, -83, -88, -93, -99, -104,
  -109, -115, -121, -126, -132, -138, -144,
  -150, -157, -163, -169, -176, -183, -190,
  -197, -204, -212, -219, -227, -235, -244,
  -252, -261, -270, -280, -290, -301, -311,
  -323, -335, -348, -362, -376, -392, -409,
  -428, -449, -472, -499, -531, -571, -624,
  -710, -796
};



int16_t readColdJunctionSensorTemp_C_x10(void) {
	int16_t p1, p2;
	int16_t temp;
	static RollingTypeDef_t data = {
			adc_buffer: (uint16_t*) &adc_measures[0].NTC,
			adc_buffer_size: Adc_Buffer_Size,
			adc_buffer_elements: Adc_Buffer_Elements,
			rolling_buffer_size: RollingBufferSize,
			rolling_buffer_index: 0,
			last_avg: 0,
			init: 0
	};

	if (!data.init) {
		uint16_t x = data.rolling_buffer_size;
		for (x = 0; x < data.rolling_buffer_size; x++) {
			data.rolling_buffer[x] = 0;
		}
		data.init = 1;
	}

	RollingUpdate(&data);	//Update average

	/* Estimate the interpolating point before and after the ADC value. */
	p1 = NTC_table[(data.last_avg >> 4)];
	p2 = NTC_table[(data.last_avg >> 4) + 1];

	/* Interpolate between both points. */
	temp = p1 - ((p1 - p2) * (data.last_avg & 0x000F)) / 16;

	return (int16_t) temp;
}

uint16_t readTipTemperatureCompensated(uint8_t new) {
	static uint16_t last_value;
	if (!new)
		return last_value;
	last_value = adc2Human(Iron.Temp.Temp_Adc_Avg);
	return last_value;
}

void setCurrentTip(uint8_t tip) {
	currentTipData = &systemSettings.ironTips[tip];
	currentPID = currentTipData->PID;
}

tipData* getCurrentTip() {
	return currentTipData;
}
// Translate the human readable temperature into internal value
uint16_t human2adc(uint16_t t) {
	uint16_t temp = t;
	int16_t ambientTemperature = readColdJunctionSensorTemp_C_x10() / 10;
	if (ambientTemperature > 50)
		ambientTemperature = 50;
	if (t > ambientTemperature)
		t = t - ambientTemperature;
	if (t < temp_minC)
		t = temp_minC;
	if (t > temp_maxC)
		t = temp_maxC;
	if (t >= currentTipData->calADC_At_300)
		temp = map(t, 300, 400, currentTipData->calADC_At_300,
				currentTipData->calADC_At_400);
	else
		temp = map(t, 200, 300, currentTipData->calADC_At_200,
				currentTipData->calADC_At_300);

	uint16_t tH = adc2Human(temp) - ambientTemperature;
	if (tH == t)
		return temp;
	if (tH < t) {
		for (uint16_t x = 0; x < 1000; ++x) {
			++temp;
			tH = adc2Human(temp) - ambientTemperature;
			if (tH >= t)
				return temp;
		}
	}
	if (tH > t) {
		for (uint16_t x = 0; x < 1000; ++x) {
			--temp;
			tH = adc2Human(temp) - ambientTemperature;
			if (tH <= t)
				return temp;
		}
	}
	return temp;
}
// Translate temperature from internal units to the human readable value (Celsius or Farenheit)
uint16_t adc2Human(uint16_t adc_value) {
	int16_t tempH = 0;
	int16_t ambientTemperature;
	ambientTemperature = readColdJunctionSensorTemp_C_x10() / 10;
	if (adc_value < currentTipData->calADC_At_200) {
		tempH = map(adc_value, 0, currentTipData->calADC_At_200,
				ambientTemperature, 200);
	} else if (adc_value >= currentTipData->calADC_At_300) {
		tempH = map(adc_value, currentTipData->calADC_At_300,
				currentTipData->calADC_At_400, 300, 400);
	} else {
		tempH = map(adc_value, currentTipData->calADC_At_200,
				currentTipData->calADC_At_300, 200, 300);
	}
	return tempH + ambientTemperature;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
	long ret;
	ret = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	if (ret < 0)
		ret = 0;
	return ret;
}
