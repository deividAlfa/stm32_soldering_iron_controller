/*
 * tempsensors.h
 *
 *  Created on: Jul 21, 2017
 *      Author: jose
 */

#ifndef GENERALIO_TEMPSENSORS_H_
#define GENERALIO_TEMPSENSORS_H_

#include "stm32f1xx_hal.h"
#include  "adc_global.h"
#include "../../../Src/settings.h"

uint16_t readColdJunctionSensorTemp_mC(void);
uint16_t coldJunctionTemp_mC_To_uV(int tempX10);
uint16_t readTipSensorADC_Avg(void);
uint16_t readTipTemperatureCompensated(uint8_t new);
uint16_t realTempToADC(uint16_t real);
void setCurrentTip(uint8_t tip);
tipData * getCurrentTip();

long map(long x, long in_min, long in_max, long out_min, long out_max);
uint16_t adc2Human(uint16_t);
uint16_t human2adc(uint16_t);
#endif /* GENERALIO_TEMPSENSORS_H_ */
