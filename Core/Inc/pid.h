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
	double max;
	double min;
	double Kp;
	double Ki;
	double Kd;
	int16_t maxI;
	int16_t minI;
} pid_values_t;

extern volatile pid_values_t currentPID;

void setupPIDFromStruct();
void setupPID(double max, double min, double Kp, double Kd, double Ki, int16_t _minI, int16_t _maxI );
double calculatePID( double setpoint, double pv );
void resetPID();
double getError();
double getIntegral();
double getPID_D();
double getPID_P();
double getPID_I();
double getOutput();
double getPID_SetPoint();
double getPID_PresentValue();
#endif /* PID_H_ */
