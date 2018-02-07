/*
 * autoTune_PID.c
 *
 *  Created on: Nov 26, 2017
 *      Author: jose
 */

//
// Copyright (c) 2016-2017 jackw01
// This code is distrubuted under the MIT License, see LICENSE for details
//

#include "math.h"
#include "autoTune_PID.h"

#define znModeBasicPID 0
#define znModeLessOvershoot 1
#define znModeNoOvershoot 2

double targetInputValue, minOutput, maxOutput;
int cycles;
double targetInputValue = 0;
double minOutput, maxOutput;
uint8_t znMode = znModeBasicPID;
int cycles = 10;

// See startTuningLoop()
int i;
uint8_t output;
double outputValue;
uint32_t microseconds, t1, t2, tHigh, tLow;
double max, min;
double pAverage, iAverage, dAverage;

double kp, ki, kd;

// Set target input for tuning
void autoTuneSetTargetInputValue(double target) {

    targetInputValue = target;
}

// Set output range
void autoTuneSetOutputRange(double min, double max) {
    minOutput = min;
    maxOutput = max;
}

// Set Ziegler-Nichols tuning mode
void autoTuneSetZNMode(uint8_t zn) {
    znMode = zn;
}

// Set tuning cycles
void autoTuneSetTuningCycles(int tuneCycles) {

    cycles = tuneCycles;
}

// Start loop
void autoTuneStartTuningLoop() {

    // Initialize all variables before loop
    i = 0; // Cycle counter
    output = 1; // Current output state
    outputValue = maxOutput;
    t1 = t2 = HAL_GetTick(); // Times used for calculating period
    tHigh = tLow = 0; // More time variables
    microseconds = HAL_GetTick();
    max = -1000000; // Max input
    min = 1000000; // Min input
    pAverage = iAverage = dAverage = 0;
}

// Run one cycle of the loop
double autoTuneTunePID(double input) {

    // Useful information on the algorithm used (Ziegler-Nichols method/Relay method)
    // http://www.processcontrolstuff.net/wp-content/uploads/2015/02/relay_autot-2.pdf
    // https://en.wikipedia.org/wiki/Ziegler%E2%80%93Nichols_method
    // https://www.cds.caltech.edu/~murray/courses/cds101/fa04/caltech/am04_ch8-3nov04.pdf

    // Basic explanation of how this works:
    //  * Turn on the output of the PID controller to full power
    //  * Wait for the output of the system being tuned to reach the target input value
    //      and then turn the controller output off
    //  * Wait for the output of the system being tuned to decrease below the target input
    //      value and turn the controller output back on
    //  * Do this a lot
    //  * Calculate the ultimate gain using the amplitude of the controller output and
    //      system output
    //  * Use this and the period of oscillation to calculate PID gains using the
    //      Ziegler-Nichols method

    // Calculate time delta
    uint32_t prevMicroseconds = microseconds;
    microseconds = HAL_GetTick();
    uint32_t deltaT = microseconds - prevMicroseconds;

    // Calculate max and min
    max = (max > input ? max: input);
    min = (min < input ? min: input);

    // Output is on and input signal has risen to target
    if (output && input > targetInputValue) {

        // Turn output off, record current time as t1, calculate tHigh, and reset maximum
        output = 0;
        outputValue = minOutput;
        t1 = HAL_GetTick();
        tHigh = t1 - t2;
        max = targetInputValue;
    }

    // Output is off and input signal has dropped to target
    if (!output && input < targetInputValue) {

        // Turn output on, record current time as t2, calculate tLow
        output = 1;
        outputValue = maxOutput;
        t2 = HAL_GetTick();
        tLow = t2 - t1;

        // Calculate Ku (ultimate gain)
        // Formula given is Ku = 4d / πa
        // d is the amplitude of the output signal
        // a is the amplitude of the input signal
        double ku = (4.0 * ((maxOutput - minOutput) / 2.0)) / (M_PI * (max - min) * 2.0);

        // Calculate Tu (period of output oscillations)
        double tu = tLow + tHigh;

        // How gains are calculated
        // PID control algorithm needs Kp, Ki, and Kd
        // Ziegler-Nichols tuning method gives Kp, Ti, and Td
        //
        // Kp = 0.6Ku = Kc
        // Ti = 0.5Tu = Kc/Ki
        // Td = 0.125Tu = Kd/Kc
        //
        // Solving these equations for Kp, Ki, and Kd gives this:
        //
        // Kp = 0.6Ku
        // Ki = Kp / (0.5Tu)
        // Kd = 0.125 * Kp * Tu

        // Constants
        // https://en.wikipedia.org/wiki/Ziegler%E2%80%93Nichols_method

        double kpConstant, tiConstant, tdConstant;

        if (znMode == znModeBasicPID) {

            kpConstant = 0.6;
            tiConstant = 0.5;
            tdConstant = 0.125;

        } else if (znMode == znModeLessOvershoot) {

            kpConstant = 0.33;
            tiConstant = 0.5;
            tdConstant = 0.33;

        } else if (znMode == znModeNoOvershoot) {

            kpConstant = 0.2;
            tiConstant = 0.5;
            tdConstant = 0.33;
        }

        // Normal PID
        //double pConstant = 0.6, iConstant = 0.5, dConstant = 0.125;

        // Less overshoot
        //double pConstant = 0.33, iConstant = 0.5, dConstant = 0.33;

        // No overshoot
        //double pConstant = 0.2, iConstant = 0.5, dConstant = 0.33;

        // Calculate gains
        kp = kpConstant * ku;
        ki = (kp / (tiConstant * tu)) * deltaT;
        kd = (tdConstant * kp * tu) / deltaT;

        // Average all gains after the first two cycles
        if (i > 1) {
            pAverage += kp;
            iAverage += ki;
            dAverage += kd;
        }

        // Reset minimum
        min = targetInputValue;

        // Increment cycle count
        i ++;
    }

    // If loop is done, disable output and calculate averages
    if (i >= cycles) {

        output = 0;
        outputValue = minOutput;

        kp = pAverage / (cycles - 2);
        ki = iAverage / (cycles - 2);
        kd = dAverage / (cycles - 2);
    }

    return outputValue;
}

// Get PID constants after tuning
double autoTuneGetKp() { return kp; };
double autoTuneGetKi() { return ki; };
double autoTuneGetKd() { return kd; };

// Is the tuning loop finished?
uint8_t autoTuneisFinished() {
    return (i >= cycles);
}
