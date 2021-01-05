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

#ifdef USE_NTC
const int NTC_table[257] = {
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
#endif
int16_t readColdJunctionSensorTemp_x10(bool tempUnit) {
	int16_t temp;
#ifdef USE_NTC
	int16_t p1, p2;
	int16_t lastavg=NTC.last_avg;
	/* Estimate the interpolating point before and after the ADC value. */
	p1 = NTC_table[(lastavg >> 4)];
	p2 = NTC_table[(lastavg >> 4) + 1];

	/* Interpolate between both points. */
	temp = p1 - ((p1 - p2) * (lastavg & 0x000F)) / 16;
	if(tempUnit==Unit_Farenheit){
		temp=TempConversion(temp,toFarenheit);
		temp+=(320-32);	//TempConversion works in x1, not x10, so subtract 32, add 32x10
	}
	return temp;
#else
	temp=350;				// If no NTC is used, assume 35ºC
	return temp;
#endif
}
// Read tip filtered
uint16_t readTipTemperatureCompensated(bool new) {
	static uint16_t last_value;
	if(new){
		readTipTemperatureCompensatedRaw(New);
		last_value = adc2Human(TIP.last_avg,1,systemSettings.tempUnit);
		if(last_value>530){
			last_value=530;
		}
	}
	return last_value;
}
// Read tip unfiltered
uint16_t readTipTemperatureCompensatedRaw(bool new) {
	static uint16_t last_value;
	if (new){
		last_value = adc2Human(TIP.last_RawAvg,1,systemSettings.tempUnit);
		if(last_value>530){
			last_value=530;
		}
	}
	return last_value;
}
void setCurrentTip(uint8_t tip) {
	currentTipData = &systemSettings.ironTips[tip];
	currentPID = currentTipData->PID;
	setupPIDFromStruct();
}

tipData* getCurrentTip() {
	return currentTipData;
}

// Translate the human readable t into internal value
uint16_t human2adc(int16_t t) {
	volatile int16_t temp = t;
	volatile int16_t tH;
	volatile int16_t ambTemp = readColdJunctionSensorTemp_x10(Unit_Celsius) / 10;

	// If using Farenheit, convert to Celsius
	if(systemSettings.tempUnit==Unit_Farenheit){
		t = TempConversion(t,toCelsius);
	}
	t-=ambTemp;
	if (t < temp_minC){ return 0; } // If requested temp below min, return 0
	else if (t > temp_maxC){ t = temp_maxC; } // If requested over max, apply limit

	// If t>300, map between ADC values Cal_300 - Cal_400
	if (t >= 300){
		temp = map(t, 300, 400, currentTipData->calADC_At_300, currentTipData->calADC_At_400);
	}
	// If t>200, map between ADC values Cal_200 - Cal_300
	else if(t >= 200){
		temp = map(t, 200, 300, currentTipData->calADC_At_200, currentTipData->calADC_At_300);
	}
	// If t<200, map between ADC values ambTemp - Cal_200
	else{
		temp = map(t, ambTemp, 200, 0, currentTipData->calADC_At_200);
	}

	tH = adc2Human(temp,0,Unit_Celsius);
	if (tH < t) {
		while(tH < t){
			tH = adc2Human(++temp,0,Unit_Celsius);
		}
	}
	else if (tH > t) {
		while(tH > t){
			tH = adc2Human(--temp,0,Unit_Celsius);
		}
	}
	return temp;
}

// Translate temperature from internal units to the human readable value
int16_t adc2Human(uint16_t adc_value,bool correction, bool tempUnit) {
	int16_t tempH = 0;
	int16_t ambTemp;
	ambTemp = readColdJunctionSensorTemp_x10(Unit_Celsius) / 10;
	if (adc_value >= currentTipData->calADC_At_300) {
		tempH = map(adc_value, currentTipData->calADC_At_300, currentTipData->calADC_At_400, 300, 400);
	}
	else if(adc_value >= currentTipData->calADC_At_200){
		tempH = map(adc_value, currentTipData->calADC_At_200, currentTipData->calADC_At_300, 200, 300);
	}
	else{
		tempH = map(adc_value, 0, currentTipData->calADC_At_200, ambTemp, 200);
	}
	if(correction){ tempH+= ambTemp; }
	if(tempUnit==Unit_Farenheit){
		tempH=TempConversion(tempH,toFarenheit);
	}
	return tempH;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
	long ret;
	ret = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	if (ret < 0)
		ret = 0;
	return ret;
}

// Fixed point calculation
// 2E20*1.8 = 1887437 , 2E20/1.8 = 582542
// So (temp*1887437)>>20 == temp*1.8 (Real: 1,800000191)
// (temp*582542)>>20 == temp/1.8 (Real: 1,800000687)
// Max input = 1100°C / 3700°F, otherwise we will overflow the signed int32
int16_t TempConversion(int16_t temperature, bool conversion){
	if(conversion==toFarenheit){	// Input==Celsius, Output==Farenheit
		temperature=(((int32_t)temperature*1887437)>>20)+32;// F = (C*1.8)+32
	}
	else{// Input==Farenheit, Output==Celsius
		temperature=(((int32_t)temperature-32)*582542)>>20;// C = (F-32)/1.8
	}
	return temperature;
}
