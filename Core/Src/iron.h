/*
 * iron.h
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#ifndef IRON_H_
#define IRON_H_

#include "stm32f0xx_hal.h"
#include "pid.h"

typedef void (*setTemperatureReachedCallback)(uint16_t);
typedef enum {iron_temp_measure_idle, iron_temp_measure_pwm_stopped, iron_temp_measure_requested, iron_temp_measure_started, iron_temp_measure_ready} iron_temp_measure_state_t;


typedef enum {mode_standby, mode_sleep, mode_set, mode_boost} iron_mode_t;
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


typedef struct {

	TIM_HandleTypeDef *Pwm_Timer;
	uint32_t Pwm_Channel;
	uint8_t CurrentIronPower;

	struct{
		uint8_t TemperatureReachedFlag;
		uint16_t CurrentSetTemperature;
		uint16_t UserCurrentSetTemperature;
		uint32_t LastSetTemperatureTime;
		uint8_t SetTemperatureChanged;
		uint16_t Temp_Adc_Avg;
		uint16_t TempSetPoint;
	}Temp;
	struct{
		uint8_t Mode ;
		uint16_t SetPoint;
	}Debug;
	struct{
		uint8_t isOn;
		uint8_t Active;
		uint32_t PwmStoppedSince;
		uint32_t StartOfNoActivityTime;
		uint8_t ProcessUpdate;
		iron_temp_measure_state_t TempMeasureState;
		iron_mode_t CurrentMode;
		uint32_t CurrentModeTimer;
	} Status;
	ironSleep_t CurrentSleepSettings;
	ironBoost_t CurrentBoostSettings;

}iron_t;

extern volatile iron_t Iron;
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
void handleIron(void);
void ironInit(TIM_HandleTypeDef *timer, uint32_t channel);
void turnIronOn();
void turnIronOff();
uint8_t getIronOn();
void setDebugSetPoint(uint16_t value);
void setDebugMode(uint8_t value);
#endif /* IRON_H_ */
