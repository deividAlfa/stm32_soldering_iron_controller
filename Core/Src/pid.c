/*
 * pid.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "pid.h"
#include "tempsensors.h"
#include "settings.h"

PIDController_t pid;


void setupPID(pid_values_t* p) {
	pid.Kp = (float)p->Kp/1000000;
	pid.Ki = (float)p->Ki/1000000;
	pid.Kd = (float)p->Kd/1000000;
	pid.limMinInt = (float)p->minI/100;
	pid.limMaxInt = (float)p->maxI/100;
	pid.limMin = (float)0;
	pid.limMax = (float)1;
	//pid.tau = (float)0.2;	//TODO adjust this from menu? This is not used currently used (For New PID)
}

// New part from Phil: https://github.com/pms67/PID
int32_t calculatePID(int32_t setpoint, int32_t measurement, int32_t baseCalc) {

  float dt = (float)(HAL_GetTick() - pid.lastTime)/1000;
  float error = setpoint - measurement;

  // Proportional term
  pid.proportional = pid.Kp * error;

  // Integral
// pid.integrator = pid.integrator + 0.5f * pid.Ki * dt * (error + pid.prevError);  // New
  pid.integrator = pid.integrator + (pid.Ki*(error*dt));                            // Old

  // Integrator clamping
  if (pid.integrator > pid.limMaxInt) {
    pid.integrator = pid.limMaxInt;
  }
  else if (pid.integrator < pid.limMinInt) {
    pid.integrator = pid.limMinInt;
  }


  // Derivative term
  if(error==pid.prevError) {
    pid.derivative = 0;
  }
  else{
    /*                                                                          // New
    pid.derivative = -(2.0f * pid.Kd * (measurement - pid.prevMeasurement)      // Note: derivative on measurement,
                          + (2.0f * pid.tau - dt) * pid.derivative)             // therefore minus sign in front of equation!
                          / (2.0f * pid.tau + dt);
    */
    pid.derivative = pid.Kd*((error-pid.prevError)/dt);                         // Old
  }

  // Compute output and apply limits
  pid.out = pid.proportional + pid.integrator + pid.derivative;

  if(pid.out > pid.limMax){
      pid.out = pid.limMax;

  } else if (pid.out < pid.limMin) {
      pid.out = pid.limMin;
  }

  // Store error and measurement for later use
  pid.prevMeasurement = measurement;
  pid.lastTime = HAL_GetTick();
  pid.prevError  = error;

  return (pid.out*baseCalc);
}


void resetPID(void){
	pid.integrator = 0;
	pid.derivative = 0;
	pid.prevError = 0;
	pid.lastTime = HAL_GetTick();
}

float getPID_D() {
	return pid.derivative;
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
