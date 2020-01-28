/*
 * pid.h
 *
 *  Created on: Sep 11, 2017
 *      Author: jose
 */

#ifndef PID_H_
#define PID_H_

#include "stm32f1xx_hal.h"
typedef struct pid_values {
	float Kp;
	float Ki;
	float Kd;
	float max;
	float min;
	int16_t maxI;
	int16_t minI;

} pid_values_t;

pid_values_t currentPID;

void setupPIDFromStruct();
void setupPID(float max, float min, float Kp, float Kd, float Ki, int16_t _minI, int16_t _maxI );
float calculatePID( float setpoint, float pv );
void resetPID();
float getError();
float getIntegral();
float getPID_D();
float getPID_P();
float getPID_I();
float getOutput();
float getPID_SetPoint();
float getPID_PresentValue();
#endif /* PID_H_ */
