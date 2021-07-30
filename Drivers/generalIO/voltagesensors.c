/*
 * voltagesensors.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "voltagesensors.h"
#ifdef USE_VIN

uint16_t last_VIN;

uint16_t getSupplyVoltage_v_x10() {
uint32_t temp;

  // As we have a R1(10K) / R2(1K) resistor divider, the ratio is 1/(10+1)=0.09090909
  // So we previously divide (2<<16)/0.09090909 = 720896
  // Now we can use hardware multiplier and then make bit shifting
  // avoiding using floats
  temp = (uint32_t)720896 * ADC_to_mV(VIN.last_avg);        // Constant multiply
  temp >>= 16;                                              // Bit shifting (divide by 65536) = mV corrected
  temp+=50;                                                 // Round number
  last_VIN=temp;
  return (temp/100);                                        // Return Supply  V*10
}
#endif

#ifdef USE_VREF
uint16_t getReferenceVoltage_mv_x10() {
  return ADC_to_mV(VREF.last_avg);
}
#endif
