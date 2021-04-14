/*
 * pid.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "pid.h"
#include "tempsensors.h"
#include "settings.h"
//static float max, min, Kp, Kd, Ki, pre_error, integral, mset, mpv, maxI, minI;
//static float p, i, d, currentOutput;
PIDController_t pid;
//static int32_t lastTime, max, min, Kp, Kd, Ki, pre_error, integral, mset, mpv, maxI, minI,p,i,d,currentOutput;
pid_values_t currentPID;

void setupPID(pid_values_t* p) {
	pid.Kp = (float)p->Kp/10000000;
	pid.Ki = (float)p->Ki/10000000;
	pid.Kd = (float)p->Kd/10000000;
	pid.limMinInt = p->minI;
	pid.limMaxInt = p->maxI;
	pid.limMin = p->min;
	pid.limMax = p->max;
	pid.tau = (float)0.2;	//TODO adjust this from menu?
}

int32_t calculatePID(int32_t setpoint, int32_t measurement, int32_t baseCalc) {
	static uint32_t lastTime=0;
	float timeStep = ((float)HAL_GetTick()-lastTime)/1000;
	pid.lastTime = HAL_GetTick();
	pid.lastSetpoint = setpoint;
	pid.lastMeasurement = measurement;

	/*
	* Error signal
	*/
    float error = setpoint - measurement;


	/*
	* Proportional
	*/
    pid.proportional = pid.Kp * error;

	/*
	* Integral
	*/
	pid.integrator = pid.integrator + 0.5f * pid.Ki * timeStep * (error + pid.prevError);

	/* Anti-wind-up via integrator clamping */
	if (pid.integrator > pid.limMaxInt) {

		pid.integrator = pid.limMaxInt;

	} else if (pid.integrator < pid.limMinInt) {

		pid.integrator = pid.limMinInt;

	}

	/*
	* Derivative (band-limited differentiator)
	*/

    pid.differentiator = -(2.0f * pid.Kd * (measurement - pid.prevMeasurement)	/* Note: derivative on measurement, therefore minus sign in front of equation! */
                        + (2.0f * pid.tau - timeStep) * pid.differentiator)
                        / (2.0f * pid.tau + timeStep);


	/*
	* Compute output and apply limits
	*/
    pid.out = pid.proportional + pid.integrator + pid.differentiator;

    if (pid.out > pid.limMax) {

        pid.out = pid.limMax;

    } else if (pid.out < pid.limMin) {

        pid.out = pid.limMin;

    }

	/* Store error and measurement for later use */
    pid.prevError       = error;
    pid.prevMeasurement = measurement;

	/* Return controller output */
    return (pid.out*baseCalc);

}
void resetPID(void){
	pid.integrator = 0;
	pid.differentiator = 0;
	pid.prevError = 0;
	pid.lastTime = HAL_GetTick();
}

float getPID_D() {
	return pid.differentiator;
}
float getPID_P() {
	return pid.proportional;
}
float getPID_I() {
	return pid.integrator;
}
float getPID_Error() {
	return pid.prevError;
}
float getPID_Output() {
	return pid.out;
}
int32_t getPID_SetPoint() {
	return pid.lastSetpoint;
}
int32_t getPID_PresentValue() {
	return pid.lastMeasurement;
}
