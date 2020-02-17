/*
 * pid.c
 *
 *  Created on: Sep 11, 2017
 *      Author: jose
 */

#include "pid.h"
#include "tempsensors.h"
#include "filtrai.h"

static float max, min, Kp, Kd, Ki, pre_error, mset, mpv, maxI, minI;
double integral;
static float p, i, d, currentOutput;
uint32_t lastTime;

float getError() {
	return pre_error;
}
float getIntegral() {
	return integral;
}

void setupPIDFromStruct() {
	setupPID(currentPID.max, currentPID.min, currentPID.Kp, currentPID.Kd, currentPID.Ki, currentPID.minI, currentPID.maxI);
}
int stabilize=0;
float Ki_df;
float Kp_df;
void setupPID(float _max, float _min, float _Kp, float _Kd, float _Ki, int16_t _minI, int16_t _maxI ) {
	max = _max;
	min = 0;
	Kp = _Kp;
	Kd = _Kd;
	Ki = _Ki;
	Ki_df = Ki;
	Kp_df = Kp;
	minI = _minI;
	maxI = _maxI*5;
	pre_error = 0;
	integral = 0;
}

void resetPID() {
	pre_error = 0;
	integral = 0;
}

__attribute__((used))  INTEGRATOR_FT err_buf = INIT_INTEGRATOR(50, float, 3); /* equal to ms */
__attribute__((used))  INTEGRATOR_FT Ti_buf = INIT_INTEGRATOR(400, float, 100); /* equal to ms */
//__attribute__((used))  INTEGRATOR_FT Td_buf = INIT_INTEGRATOR(10, float, 10); /* equal to ms */
__attribute__((used)) float integrator_rlt;

float output;
float Iout;
float Dout;
float Pout;
float perc_avg;
int err4=0;
float error;

uint8_t fallback = 0;
float fallback_integer;
uint8_t zone = 0;
extern tipData *currentTipData;
float calculatePID( float setpoint, float pv )
{
	mset = setpoint;
	mpv = pv;

    uint32_t tick_start = HAL_GetTick();
    float Ts = (tick_start - lastTime) ;
    lastTime = tick_start;
    Ts = Ts / 1000;

    // Calculate error
    error = setpoint - pv;

    // Proportional term
    Pout = POS(Kp * error);

    static float prev_setpoint = 0;
    if(setpoint != prev_setpoint){
    	Ki = map_w_limits(setpoint, currentTipData->calADC_At_200, currentTipData->calADC_At_300, Ki_df*0.3, Ki_df);
    	Kp = map_w_limits(setpoint, currentTipData->calADC_At_200, currentTipData->calADC_At_300, Kp_df*0.2, Kp_df);
        prev_setpoint = setpoint;
    }


    integral += error*Ki * Ts;
    if(integral > maxI){
    	integral = maxI;
    }else if(integral < minI){
        	integral = minI;
    }

	if( pv > (setpoint + 0.03*setpoint)){
		integral = 0;
	}


	if( ABS(setpoint-pv) > 200 ){
		//Kp = Kp_df*4;
		integral = 0;
	}

	if(integral<0){
		integral = 0;
	}

    Iout = integral;


    // Derivative term
    float derivative;
    	if(error == pre_error)
    		derivative = 0;
    	else
    		derivative = (error - pre_error) / Ts;
#if 0
    derivative = integrator_ft(derivative, &Td_buf );
    float Dout = Kd * derivative;
#else
    Dout = Kd * derivative;
#endif


    // Calculate total output
    output = Pout + Iout + Dout;
    float res = ABS(Iout)/(ABS(Iout)+ABS(Pout))*100.0;
    err4 +=(res<0)?1:0;

    TICK;
    perc_avg = integrator_ft(res, &Ti_buf );
    Benchmark._07_ft_integrator = TOCK;

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
float getPID_D() {
	return d * 100;
}
float getPID_P() {
	return p * 100;
}
float getPID_I() {
	return i * 100;
}
float getOutput() {
	return currentOutput * 100;
}
float getPID_SetPoint() {
	return mset;
}
float getPID_PresentValue() {
	return mpv;
}
