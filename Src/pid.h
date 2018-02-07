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
	double max;
	double min;
	double Kp;
	double Ki;
	double Kd;
	int16_t maxI;
	int16_t minI;
} pid_values_t;

pid_values_t currentPID;

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
void startAutoTune();
uint8_t isAutoTuneFinished();
#endif /* PID_H_ */
