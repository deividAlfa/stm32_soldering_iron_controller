/*
 * tempsensors.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GENERALIO_TEMPSENSORS_H_
#define GENERALIO_TEMPSENSORS_H_

#include "main.h"
#include "adc_global.h"
#include "settings.h"
#include "iron.h"

extern volatile int16_t last_TIP_C, last_TIP_F, last_NTC_F, last_TIP_F_Raw, last_NTC_C, last_TIP_C_Raw;

void detectNTC(void);
int16_t readColdJunctionSensorTemp_x10(bool new, bool tempUnit);
int16_t readTipTemperatureCompensated(bool new, bool mode, bool tempUnit);
void    setCurrentTip(uint8_t tip);
tipData_t *getCurrentTip();

int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
int16_t adc2Human_x10(int16_t adc_value,bool correction, bool tempUnit);
int16_t human2adc(int16_t t);
int16_t TempConversion(int16_t temperature, bool conversion, bool x10mode);
int16_t TempIncrementConversion(int16_t temperature, bool conversion);






#endif /* GENERALIO_TEMPSENSORS_H_ */

