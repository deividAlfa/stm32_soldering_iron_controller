/*
 * tempsensors.c
 *
 *  Created on: Jul 21, 2017
 *      Author: jose
 */

#include "tempsensors.h"
#include "math.h"
#include "main.h"
const uint16_t temp_minC = 100;                 // Minimum calibration temperature in degrees of Celsius
const uint16_t temp_maxC = 450;                 // Maximum calibration temperature in degrees of Celsius


tipData *currentTipData;

#ifndef FAKE_25_COLD_JUNGTION
static const uint16_t NTC_R = 1013;
uint16_t readColdJunctionSensorTemp_mC(void) {
	static uint32_t rollingAverage[4];
	static uint8_t rIndex = 0;
	uint32_t ad_sum = 0;
	uint32_t max, min;
	uint32_t ad_value, avg_data, slide_data;

	uint8_t gMeas_cnt = sizeof(adc_measures)/sizeof(adc_measures[0]) -1;
	ad_sum = min = max = adc_measures[0].cold_junction;
	while (gMeas_cnt > 0) {
		ad_value = adc_measures[gMeas_cnt].cold_junction;
		ad_sum += ad_value;
		if (ad_value > max)
			max = ad_value;
		if (ad_value < min)
			min = ad_value;
		gMeas_cnt--;
	}
	ad_sum = ad_sum - max - min;
	avg_data = ad_sum / 8;

	rollingAverage[rIndex] = avg_data; //store this result
	rIndex = (rIndex + 1) % 4; //move the index
	slide_data = (rollingAverage[0] + rollingAverage[1] + rollingAverage[2]
			+ rollingAverage[3]) / 4; //get the average
	float NTC_RES = (float)(NTC_R) / ((4096.0 / (float)slide_data) - 1.0);
	float adc_read = log(NTC_RES);
	adc_read = 1000 / (0.001129148 + (0.000234125 * adc_read) + (0.0000000876741 * adc_read * adc_read * adc_read));
	adc_read = adc_read - 273150;  // Convert Kelvin to Celsius
	return adc_read;

}
#else
uint16_t readColdJunctionSensorTemp_mC(void) {
	return 21000;
}
#endif
float last_value;
float readTipTemperatureCompensated(uint8_t new){
	//static float last_value;
	if(!new)
		return last_value;
	last_value = adc2Human(iron_temp_adc_avg);
	return last_value;
}

void setCurrentTip(uint8_t tip) {
	currentTipData = &systemSettings.ironTips[tip];
	currentPID = currentTipData->PID;
	setupPIDFromStruct();
}

tipData * getCurrentTip() {
	return currentTipData;
}
// Translate the human readable temperature into internal value
uint16_t human2adc(uint16_t t) {
  uint16_t temp = t;
  uint16_t ambientTemperature = readColdJunctionSensorTemp_mC() / 1000;
  if(ambientTemperature > 50)
	  ambientTemperature = 50;
  if(t > ambientTemperature)
	  t = t - ambientTemperature;
  if (t < temp_minC) t = temp_minC;
  if (t > temp_maxC) t = temp_maxC;
  if (t >= 300)
    temp = map(t, 300, 400, currentTipData->calADC_At_300, currentTipData->calADC_At_400);
  else
    temp = map(t , 200, 300, currentTipData->calADC_At_200, currentTipData->calADC_At_300);

//  uint16_t tH = adc2Human(temp)- ambientTemperature;
//  if(tH == t)
//	  return temp;
//  if(tH < t) {
//	  for(uint16_t x = 0; x < 1000; ++x) {
//		  ++temp;
//		  tH = adc2Human(temp)- ambientTemperature;
//		  if(tH >= t)
//			  return temp;
//	  }
//  }
//  if(tH > t) {
//	  for(uint16_t x = 0; x < 1000; ++x) {
//		  --temp;
//		  tH = adc2Human(temp)- ambientTemperature;
//		  if(tH <= t)
//			  return temp;
//	  }
//  }
  return temp;
}

float tempH = 0;
// Translate temperature from internal units to the human readable value (Celsius or Farenheit)
float adc2Human(uint16_t adc_value) {

  float ambientTemperature = readColdJunctionSensorTemp_mC() / 1000;
  if (adc_value < currentTipData->calADC_At_200) {
    tempH = map(adc_value, 0, currentTipData->calADC_At_200, 21, 200);
  } else if (adc_value >= currentTipData->calADC_At_300) {
    tempH = map(adc_value, currentTipData->calADC_At_300, currentTipData->calADC_At_400, 300, 400);
  } else {
    tempH = map(adc_value, currentTipData->calADC_At_200, currentTipData->calADC_At_300, 200, 300);
  }
  return tempH + ambientTemperature;
  return tempH;
}

float ret;
float map(float x, float in_min, float in_max, float out_min, float out_max)
{

  ret = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if(ret < 0)
    ret = 0;
  return ret;
}

float map_w_limits(float x, float in_min, float in_max, float out_min, float out_max)
{

  float ret = map(x,in_min,in_max,out_min,out_max);

  if (ret<out_min){
    ret= out_min;
  }

  if (ret>out_max){
    ret= out_max;
  }

  return ret;
}
