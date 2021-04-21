    /*
 * tempsensors.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "tempsensors.h"
#include "math.h"
#define temp_minC  100                 // Minimum calibration temperature in degrees of Celsius
#define temp_maxC  480                 // Maximum calibration temperature in degrees of Celsius
static tipData *currentTipData;
uint16_t last_TIP_Raw;
uint16_t last_TIP;
int16_t last_NTC;

#ifdef USE_NTC
const int NTC_TABLE;					// Defined in board.h
#endif

int16_t readColdJunctionSensorTemp_x10(bool tempUnit) {
#ifdef USE_NTC
	int16_t temp;
	int16_t p1, p2;
	int16_t lastavg=NTC.last_avg;
	/* Estimate the interpolating point before and after the ADC value. */
	p1 = NTC_Table[(lastavg >> 4)];
	p2 = NTC_Table[(lastavg >> 4) + 1];

	/* Interpolate between both points. */
	temp = p1 - ((p1 - p2) * (lastavg & 0x000F)) / 16;
	if(temp>999){
		temp=999;								// Put some limits (99.9ºC)
	}													// Negative limit is around -77ºC, we need that to detect NTC disconnected
	if(tempUnit==mode_Farenheit){
		temp=TempConversion(temp, mode_Farenheit, 1);
	}
	last_NTC = temp;
	return last_NTC;
#else
	last_NTC=350;				// If no NTC is used, assume 35ºC
	return last_NTC;
#endif
}
// Read tip temperature
uint16_t readTipTemperatureCompensated(bool update, bool ReadRaw){
	uint16_t temp, temp_Raw;
	if(systemSettings.setupMode==setup_On){
		return 0;
	}
	if(update){
		temp = adc2Human(TIP.last_avg,1,systemSettings.settings.tempUnit);
		temp_Raw = adc2Human(TIP.last_RawAvg,1,systemSettings.settings.tempUnit);

		// Limit output values
		if(temp>999){
			temp=999;
		}
		else if(temp<0){
			temp = 0;
		}
		if(temp_Raw>999){
			temp_Raw=999;
		}
		else if(temp_Raw<0){
			temp_Raw = 0;
		}
		last_TIP = temp;
		last_TIP_Raw = temp_Raw;
	}
	if(ReadRaw){
		return last_TIP_Raw;
	}
	else{
		return last_TIP;
	}
}

void setCurrentTip(uint8_t tip) {
	currentTipData = &systemSettings.Profile.tip[tip];
	setupPID(&currentTipData->PID);
}

tipData* getCurrentTip() {
	return currentTipData;
}

// Translate the human readable t into internal value
uint16_t human2adc(int16_t t) {
	volatile int16_t temp = t;
	volatile int16_t tH;
	volatile int16_t ambTemp = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;

	// If using Farenheit, convert to Celsius
	if(systemSettings.settings.tempUnit==mode_Farenheit){
		t = TempConversion(t,mode_Celsius,0);
	}
	t-=ambTemp;
	if (t < temp_minC){ return 0; }                                 // If requested temp below min, return 0
	else if (t > temp_maxC){ t = temp_maxC; }                       // If requested over max, apply limit

	// If t>350, map between ADC values Cal_350 - Cal_450
	if (t >= 350){
		temp = map(t, 350, 450, currentTipData->calADC_At_350, currentTipData->calADC_At_450);
	}
	// Else, map between ADC values Cal_250 - Cal_350
	else{
 		temp = map(t, 250, 350, currentTipData->calADC_At_250, currentTipData->calADC_At_350);
	}
	tH = adc2Human(temp,0,mode_Celsius);
	if (tH < t) {
		while(tH < t){
			tH = adc2Human(++temp,0,mode_Celsius);
		}
	}
	else if (tH > t) {
		while(tH > t){
			tH = adc2Human(--temp,0,mode_Celsius);
		}
	}
	return temp;
}

// Translate temperature from internal units to the human readable value
int16_t adc2Human(uint16_t adc_value,bool correction, bool tempUnit) {
	int16_t tempH = 0;
	int16_t ambTemp;
	ambTemp = readColdJunctionSensorTemp_x10(mode_Celsius) / 10;
	if (adc_value >= currentTipData->calADC_At_350) {
		tempH = map(adc_value, currentTipData->calADC_At_350, currentTipData->calADC_At_450, 350, 450);
	}
	else{
		tempH = map(adc_value, currentTipData->calADC_At_250, currentTipData->calADC_At_350, 250, 350);
	}
	if(correction){ tempH+= ambTemp; }
	if(tempUnit==mode_Farenheit){
		tempH=TempConversion(tempH,mode_Farenheit,0);
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
int16_t TempConversion(int16_t temperature, bool conversion, bool x10mode){
	if(conversion==mode_Farenheit){	// Input==Celsius, Output==Farenheit
		temperature=(((int32_t)temperature*1887437)>>20);// F = (C*1.8)+32
		if(x10mode){
			temperature += 320;
		}
		else{
			temperature += 32;
		}
	}
	else{// Input==Farenheit, Output==Celsius
		if(x10mode){
			temperature -= 320;
		}
		else{
			temperature -= 32;
		}
		temperature=((int32_t)temperature*582542)>>20;// C = (F-32)/1.8
	}
	return temperature;
}
