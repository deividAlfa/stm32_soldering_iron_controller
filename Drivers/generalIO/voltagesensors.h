/*
 * voltagesensors.h
 *
 *  Created on: Jul 26, 2017
 *      Author: jose
 */

#ifndef GENERALIO_VOLTAGESENSORS_H_
#define GENERALIO_VOLTAGESENSORS_H_

#include "adc_global.h"
#ifdef USE_VIN
uint16_t getSupplyVoltage_v_x10();
#endif
#ifdef USE_VREF
uint16_t getReferenceVoltage_mv_x10();
#endif
#endif /* GENERALIO_VOLTAGESENSORS_H_ */
