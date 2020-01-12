/*
 * pid.c
 *
 *  Created on: Sep 11, 2017
 *      Author: jose
 */

#include "pid.h"
#include "tempsensors.h"
#include "filtrai.h"

static double max, min, Kp, Kd, Ki, pre_error, mset, mpv, maxI, minI;
float integral;
static double p, i, d, currentOutput;
uint32_t lastTime;

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
	min = 0;
	Kp = _Kp;
	Kd = _Kd;
	Ki = _Ki;
	minI = _minI;
	maxI = _maxI*5;
	pre_error = 0;
	integral = 0;
}

void resetPID() {
	pre_error = 0;
	integral = 0;
}


__attribute__((used))  INTEGRATOR_FT Ti_buf = INIT_INTEGRATOR(400, float); /* equal to ms */
__attribute__((used))  INTEGRATOR_FT Td_buf = INIT_INTEGRATOR(10, float); /* equal to ms */
__attribute__((used)) float integrator_rlt;

double output;
double Iout;
double Dout;
double derivative;
double Pout;
float perc_avg;
int err4=0;

#define POS(x) ((x<0)?-x:x )
double Ts;
double calculatePID( double setpoint, double pv )
{
	mset = setpoint;
	mpv = pv;

    uint32_t tick_start = HAL_GetTick();
    Ts = (tick_start - lastTime) ;
    lastTime = tick_start;
    Ts = Ts / 1000;

    // Calculate error
    double error = setpoint - pv;

    // Proportional term
    Pout = Kp * error;

    // Integral term
    float integral_tmp = error * Ts;

#if 0/* lets try proper integrator */
    integral = integrator_ft(error, &Ti_buf );
#else
    integral += error * Ts;
    if(integral > maxI)
    	integral = maxI;
    else if(integral < minI)
        	integral = minI;
#endif
    Iout = Ki * integral;


    // Derivative term
    derivative;
    	if(error == pre_error)
    		derivative = 0;
    	else
    		derivative = (error - pre_error) / Ts;
#if 0
    derivative = integrator_ft(derivative, &Td_buf );
    double Dout = Kd * derivative;
#else
    Dout = Kd * derivative;
#endif


    // Calculate total output
    output = Pout + Iout;// + Dout;
    float res = POS(Iout)/(POS(Iout)+POS(Pout))*100.0;
    err4 +=(res<0)?1:0;
    perc_avg = integrator_ft(res, &Ti_buf );

    p = Pout;
    i = Iout;
    d = Dout;
    // Restrict to max/min
    if( output > max )
        output = max;
    else if( output < min )
        output = min;


#if 0
    pre_error = integrator_ft(error, &Td_buf );
#else
    // Save error to previous error
    pre_error = error;
#endif

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
