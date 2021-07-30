/*
 * voltagesensors.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GENERALIO_VOLTAGESENSORS_H_
#define GENERALIO_VOLTAGESENSORS_H_

#include "adc_global.h"
#ifdef USE_VIN
uint16_t getSupplyVoltage_v_x10();
extern uint16_t last_VIN;
#endif
#ifdef USE_VREF
uint16_t getReferenceVoltage_mv_x10();
#endif
#endif /* GENERALIO_VOLTAGESENSORS_H_ */
