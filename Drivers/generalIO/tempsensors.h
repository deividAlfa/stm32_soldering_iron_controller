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

extern int16_t    last_TIP_Raw;
extern int16_t    last_TIP;


void detectNTC(void);
int16_t   readColdJunctionSensorTemp_x10(bool new, bool tempUnit);
int16_t   readTipTemperatureCompensated(bool new, bool mode);
void      setCurrentTip(uint8_t tip);
tipData_t *getCurrentTip();

long      map(long x, long in_min, long in_max, long out_min, long out_max);
int16_t   adc2Human(uint16_t adc_value,bool correction, bool tempUnit);
uint16_t  human2adc(int16_t t);
int16_t   TempConversion(int16_t temperature, bool conversion, bool x10mode);






#endif /* GENERALIO_TEMPSENSORS_H_ */

