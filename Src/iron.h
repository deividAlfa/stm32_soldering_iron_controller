/*
 * iron.h
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#ifndef IRON_H_
#define IRON_H_

#include "stm32f1xx_hal.h"
#include "pid.h"

typedef void (*setTemperatureReachedCallback)(uint16_t);

typedef enum iron_mode_t {mode_standby, mode_boost, mode_sleep, mode_set} iron_mode_t;
typedef void (*currentModeChanged)(iron_mode_t);

typedef struct ironBoost_t {
	uint16_t temperature;
	uint16_t time;
} ironBoost_t;

typedef struct ironSleep_t {
	uint16_t sleepTime;
	uint16_t standbyTime;
	uint16_t sleepTemperature;
} ironSleep_t;

typedef struct tipData {
	uint16_t calADC_At_200;
	uint16_t calADC_At_300;
	uint16_t calADC_At_400;
	char name[5];
	pid_values_t PID;
} tipData;

ironSleep_t currentSleepSettings;
ironBoost_t currentBoostSettings;

void setCurrentMode(iron_mode_t mode);
void setSetTemperature(uint16_t temperature);
void setCurrentTemperature(uint16_t temperature);
iron_mode_t getCurrentMode();
uint16_t getSetTemperature();
uint16_t getCurrentTemperature();
uint8_t getCurrentPower();
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback);
void addModeChangedCallback(currentModeChanged callback);
void applyBoostSettings();
void applySleepSettings();
void handleIron(uint8_t activity);
void ironInit(TIM_HandleTypeDef *timer);
void turnIronOn();
void turnIronOff();
uint8_t getIronOn();
void setDebugSetPoint(uint16_t value);
void setDebugMode(uint8_t value);
#endif /* IRON_H_ */
