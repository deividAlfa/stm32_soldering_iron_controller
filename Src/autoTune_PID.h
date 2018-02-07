/*
 * autoTune_PID.h
 *
 *  Created on: Nov 27, 2017
 *      Author: jose
 */

#ifndef AUTOTUNE_PID_H_
#define AUTOTUNE_PID_H_

#include "stm32f1xx_hal.h"
#define znModeBasicPID 0
#define znModeLessOvershoot 1
#define znModeNoOvershoot 2

uint8_t autoTuneisFinished();
double autoTuneGetKp();
double autoTuneGetKi();
double autoTuneGetKd();
void autoTuneSetTargetInputValue(double target);
void autoTuneSetOutputRange(double min, double max);
void autoTuneSetZNMode(uint8_t zn);
void autoTuneSetTuningCycles(int tuneCycles);
void autoTuneStartTuningLoop();
double autoTuneTunePID(double input);

#endif /* AUTOTUNE_PID_H_ */
