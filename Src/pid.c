/*
 * pid.c
 *
 *  Created on: Sep 11, 2017
 *      Author: jose
 */

#include "pid.h"
#include "autoTune_PID.h"
#include "tempsensors.h"

static double max, min, Kp, Kd, Ki, pre_error, integral, mset, mpv, maxI, minI;
static double p, i, d, currentOutput;
uint32_t lastTime;
static uint8_t autoTuneRunning = 0;
static uint8_t autoTuneRequested = 0;

uint8_t isAutoTuneFinished() {
	return (autoTuneRequested == 0 && autoTuneRunning == 0);
}

void startAutoTune() {
	autoTuneRequested = 1;
}
double getError() {
	return pre_error;
}
double getIntegral() {
	return integral;
}

void setupPIDFromStruct() {
	setupPID(currentPID.max, currentPID.min, currentPID.Kp, currentPID.Kd, currentPID.Ki, currentPID.minI, currentPID.maxI);
}
void setupPID(double _max, double _min, double _Kp, double _Kd, double _Ki, int16_t _minI, int16_t _maxI ) {
	max = _max;
	min = _min;
	Kp = _Kp;
	Kd = _Kd;
	Ki = _Ki;
	minI = _minI;
	maxI = _maxI;
	pre_error = 0;
	integral = 0;
}

void resetPID() {
	pre_error = 0;
	integral = 0;
}
double calculatePID( double setpoint, double pv )
{
	if(autoTuneRequested) {
		autoTuneRequested = 0;
		autoTuneRunning = 1;
		autoTuneSetOutputRange(0, 1);
		autoTuneSetZNMode(znModeBasicPID);
		autoTuneSetTargetInputValue(human2adc(300));
		autoTuneStartTuningLoop();
	}
	if(autoTuneRunning) {
		if(autoTuneisFinished()) {
			autoTuneRunning = 0;
		}
		else
			return autoTuneTunePID(pv);
	}
	mset = setpoint;
	mpv = pv;
    double dt = (HAL_GetTick() - lastTime) ;
    dt = dt / 1000;
    // Calculate error
    double error = setpoint - pv;

    // Proportional term
    double Pout = Kp * error;

    // Integral term
    integral += error * dt;
    if(integral > maxI)
    	integral = maxI;
    else if(integral < minI)
        	integral = minI;
    double Iout = Ki * integral;

    // Derivative term
    double derivative;
    	if(error == pre_error)
    		derivative = 0;
    	else
    		derivative = (error - pre_error) / dt;
    double Dout = Kd * derivative;

    // Calculate total output
    double output = Pout + Iout + Dout;
    p = Pout;
    i = Iout;
    d = Dout;
    // Restrict to max/min
    if( output > max )
        output = max;
    else if( output < min )
        output = min;

    // Save error to previous error
    pre_error = error;
    lastTime = HAL_GetTick();
    currentOutput = output;
    return output;
}
double getPID_D() {
	return d * 100;
}
double getPID_P() {
	return p * 100;
}
double getPID_I() {
	return i * 100;
}
double getOutput() {
	return currentOutput * 100;
}
double getPID_SetPoint() {
	return mset;
}
double getPID_PresentValue() {
	return mpv;
}
