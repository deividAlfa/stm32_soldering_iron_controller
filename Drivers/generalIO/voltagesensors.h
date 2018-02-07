/*
 * voltagesensors.h
 *
 *  Created on: Jul 26, 2017
 *      Author: jose
 */

#ifndef GENERALIO_VOLTAGESENSORS_H_
#define GENERALIO_VOLTAGESENSORS_H_

#include "adc_global.h"

uint16_t getSupplyVoltage_mv();
uint16_t getReferenceVoltage_mv();

#endif /* GENERALIO_VOLTAGESENSORS_H_ */
