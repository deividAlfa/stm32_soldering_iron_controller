/*
 * pid.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef PID_H_
#define PID_H_

#include "main.h"

typedef struct pid_values {
	uint16_t max;
	uint16_t min;
	uint16_t Kp;
	uint16_t Ki;
	uint16_t Kd;
	int16_t maxI;
	int16_t minI;
} pid_values_t;

extern volatile pid_values_t currentPID;

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
