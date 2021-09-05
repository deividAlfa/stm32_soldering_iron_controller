/*
 * pid.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "pid.h"
#include "tempsensors.h"
#include "settings.h"

PIDController_t pid;


void setupPID(pid_values_t* p) {
  pid.Kp =        (float)p->Kp/1000000;
  pid.Ki =        (float)p->Ki/1000000;
  pid.Kd =        (float)p->Kd/1000000;
  pid.limMinInt = (float)p->minI/100;
  pid.limMaxInt = (float)p->maxI/100;
  pid.limMin =    (float)0;
  pid.limMax =    (float)1;
}

int32_t calculatePID(int32_t setpoint, int32_t measurement, int32_t base) {
  float dt = (float)(HAL_GetTick() - pid.lastTime)/1000;
  float error = setpoint - measurement;

  pid.proportional = pid.Kp * error;                                            // Proportional term

  if(pid.reset){
    if(++pid.reset>3){                                                          // If pid resetted, only use proportional for few cycles to avoid spikes
      pid.reset = 0;
    }
    pid.integrator = 0;
    pid.out = pid.proportional;
  }
  else{                                                                         // Else, normal PID calculation
    pid.integrator = pid.integrator + (pid.Ki*(error*dt));                      // Integral

    if (pid.integrator > pid.limMaxInt) {                                       // Integrator clamping
      pid.integrator = pid.limMaxInt;
    }
    else if (pid.integrator < pid.limMinInt) {
      pid.integrator = pid.limMinInt;
    }
    pid.derivative = pid.Kd*((error-pid.prevError)/dt);

    pid.out = pid.proportional + pid.integrator + pid.derivative;               // Compute output
  }

  if(pid.out > pid.limMax){                                                     // Apply limits
      pid.out = pid.limMax;
  } else if (pid.out < pid.limMin) {
      pid.out = pid.limMin;
  }

  pid.prevMeasurement = measurement;                                            // Store data for later use
  pid.lastTime = HAL_GetTick();
  pid.prevError = error;
  return (pid.out*base);
}

void resetPID(void){
  pid.reset = 1;
  pid.proportional=0;         // If not cleared, they'll stay the same is the station enters sleep (or error mode), makign a bit of sense in debug screen
  pid.integrator=0;
  pid.derivative=0;
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
