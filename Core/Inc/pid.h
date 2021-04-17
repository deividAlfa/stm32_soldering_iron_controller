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
	uint16_t Kp;
	uint16_t Ki;
	uint16_t Kd;
	int16_t maxI;
	int16_t minI;
} pid_values_t;

typedef struct {
	uint32_t lastTime;
	int32_t lastMeasurement;
	int32_t lastSetpoint;
	/* Controller gains */
	float Kp;
	float Ki;
	float Kd;

	/* Derivative low-pass filter time constant */
	float tau;

	/* Output limits */
	float limMin;
	float limMax;

	/* Integrator limits */
	float limMinInt;
	float limMaxInt;

	/* Controller "memory" */
	float proportional;
	float integrator;
	float derivative;
	float prevError;			    /* Required for integrator */
	float prevMeasurement;		/* Required for derivative */

	/* Controller output */
	float out;

} PIDController_t;

extern PIDController_t pid;


void setupPID(pid_values_t* p);
int32_t calculatePID(int32_t setpoint, int32_t measurement, int32_t baseCalc);
void resetPID();
float getPID_P();
float getPID_I();
float getPID_D();
float getPID_Output();
float getPID_Error();
int32_t getPID_SetPoint();
int32_t getPID_PresentValue();
#endif /* PID_H_ */
