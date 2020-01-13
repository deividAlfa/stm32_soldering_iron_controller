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
#include "settings.h"

uint16_t readColdJunctionSensorTemp_mC(void);
uint16_t coldJunctionTemp_mC_To_uV(int tempX10);
uint16_t readTipSensorADC_Avg(void);
float readTipTemperatureCompensated(uint8_t new);
uint16_t realTempToADC(uint16_t real);
void setCurrentTip(uint8_t tip);
tipData * getCurrentTip();

float map(float x, float in_min, float in_max, float out_min, float out_max);
float adc2Human(uint16_t);
uint16_t human2adc(uint16_t);
#endif /* GENERALIO_TEMPSENSORS_H_ */
