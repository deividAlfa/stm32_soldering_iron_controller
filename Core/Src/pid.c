/*
 * pid.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "pid.h"
#include "tempsensors.h"
#include "settings.h"

static float max, min, Kp, Kd, Ki, pre_error, integral, mset, mpv, maxI, minI;
static float p, i, d, currentOutput;
uint32_t lastTime;
volatile pid_values_t currentPID;

float getError() {
	return pre_error;
}
float getIntegral() {
	return integral;
}

void setupPIDFromStruct() {
	setupPID(currentPID.max, currentPID.min, currentPID.Kp, currentPID.Kd, currentPID.Ki, currentPID.minI, currentPID.maxI);
}
void setupPID(float _max, float _min, float _Kp, float _Kd, float _Ki, int16_t _minI, int16_t _maxI ) {
	max = _max;
	min = _min;
	Kp = (float)_Kp/1000000;
	Kd = (float)_Kd/1000000;
	Ki = (float)_Ki/1000000;
	minI = _minI;
	maxI = _maxI;
	pre_error = 0;
	integral = 0;
}

void resetPID() {
	pre_error = 0;
	integral = 0;
}

float calculatePID( float setpoint, float pv )
{
	mset = setpoint;
	mpv = pv;
    float dt = (HAL_GetTick() - lastTime) ;
    dt = dt / 1000;
    // Calculate error
    float error = setpoint - pv;

    // Proportional term
    float Pout = Kp * error;

    // Integral term
    integral += error * dt;
    if(integral > maxI)
    	integral = maxI;
    else if(integral < minI)
        	integral = minI;
    float Iout = Ki * integral;

    // Derivative term
    float derivative;
    	if(error == pre_error)
    		derivative = 0;
    	else
    		derivative = (error - pre_error) / dt;
    float Dout = Kd * derivative;

    // Calculate total output
    float output = Pout + Iout + Dout;
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
float getPID_D() {
	return d;
}
float getPID_P() {
	return p;
}
float getPID_I() {
	return i;
}
float getOutput() {
	return currentOutput;
}
float getPID_SetPoint() {
	return mset;
}
float getPID_PresentValue() {
	return mpv;
}
