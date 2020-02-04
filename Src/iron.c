/*
 * iron.c
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#include "iron.h"
#include <stdlib.h>
#include "pid.h"
#include "tempsensors.h"
#include "settings.h"
#include "buzzer.h"
#include "init.h"

static iron_mode_t currentMode = mode_standby;
static uint32_t currentModeTimer = 0;
static uint16_t currentSetTemperature = 300;
static uint16_t user_currentSetTemperature = 300;
static uint16_t tempSetPoint;
static uint8_t currentIronPower = 0;
static uint8_t temperatureReachedFlag = 0;
static TIM_HandleTypeDef *ironPWMTimer;
static uint8_t isIronOn = 0;
static uint32_t lastSetTemperatureTime = 0;
static uint8_t setTemperatureChanged = 0;
static uint8_t debugMode = 0;
static uint16_t debugSetPoint;

typedef struct setTemperatureReachedCallbackStruct_t setTemperatureReachedCallbackStruct_t;

struct setTemperatureReachedCallbackStruct_t {
	setTemperatureReachedCallback callback;
	setTemperatureReachedCallbackStruct_t *next;
};

typedef struct currentModeChangedCallbackStruct_t currentModeChangedCallbackStruct_t;
struct currentModeChangedCallbackStruct_t {
	currentModeChanged callback;
	currentModeChangedCallbackStruct_t *next;
};
static currentModeChangedCallbackStruct_t *currentModeChangedCallbacks = NULL;
static setTemperatureReachedCallbackStruct_t *temperatureReachedCallbacks = NULL;

void setDebugSetPoint(uint16_t value) {
	debugSetPoint = value;
}
void setDebugMode(uint8_t value) {
	debugMode = value;
}
static void temperatureReached(uint16_t temp) {
	setTemperatureReachedCallbackStruct_t *s = temperatureReachedCallbacks;
	while(s) {
		if(s->callback) {
			s->callback(temp);
		}
		s = s->next;
	}
}

static void modeChanged(iron_mode_t newMode) {
	currentModeChangedCallbackStruct_t *s = currentModeChangedCallbacks;
	while(s) {
		s->callback(newMode);
		s = s->next;
	}
}
void turnIronOn() {
	HAL_TIM_PWM_Start(ironPWMTimer, TIM_CHANNEL_3);
	isIronOn = 1;
}
void turnIronOff() {
	HAL_TIM_PWM_Stop(ironPWMTimer, TIM_CHANNEL_3);
	isIronOn = 0;
}
uint8_t getIronOn() {
	return isIronOn;
}
void applyBoostSettings() {
	currentBoostSettings = systemSettings.boost;
}

void applySleepSettings() {
	currentSleepSettings = systemSettings.sleep;
}
void setSetTemperature(uint16_t temperature) {
	user_currentSetTemperature = temperature;
	temperatureReachedFlag = 0;
	setCurrentTemperature(temperature);
	lastSetTemperatureTime = HAL_GetTick();
	setTemperatureChanged = 1;
}
void setCurrentTemperature(uint16_t temperature) {
	currentSetTemperature = temperature;
	tempSetPoint = temperature;
	if((temperature == 0) && getIronOn())
		turnIronOff();
	else if(!getIronOn())
		turnIronOn();
	resetPID();

}
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback) {
	setTemperatureReachedCallbackStruct_t *s = _malloc(sizeof(setTemperatureReachedCallbackStruct_t));
	if(!s)
		while(1){}
	s->callback = callback;
	s->next = NULL;
	setTemperatureReachedCallbackStruct_t *last = temperatureReachedCallbacks;
	if(!last) {
		temperatureReachedCallbacks = s;
		return;
	}
	while(last && last->next != NULL) {
		last = last->next;
	}
	last->next = s;
}
void addModeChangedCallback(currentModeChanged callback) {
	currentModeChangedCallbackStruct_t *s = _malloc(sizeof(currentModeChangedCallbackStruct_t));
	s->callback = callback;
	s->next = NULL;
	currentModeChangedCallbackStruct_t *last = currentModeChangedCallbacks;
	while(last && last->next != NULL) {
		last = last->next;
	}
	if(last)
		last->next = s;
	else
		last = s;
}

void setCurrentMode(iron_mode_t mode) {
	currentModeTimer = HAL_GetTick();
	temperatureReachedFlag = 0;
	switch (mode) {
		case mode_boost:
			setCurrentTemperature(currentBoostSettings.temperature);
			break;
		case mode_set:
			setCurrentTemperature(user_currentSetTemperature);
			break;
		case mode_sleep:
			setCurrentTemperature(currentSleepSettings.sleepTemperature);
			break;
		case mode_standby:
			setCurrentTemperature(0);
			break;
		default:
			break;
	}
	currentMode = mode;
	modeChanged(mode);
}
iron_mode_t getCurrentMode() {
	return currentMode;
}
uint16_t getSetTemperature() {
	return currentSetTemperature;
}



double set=0;
void handleIron(uint8_t activity) {
	uint32_t currentTime = HAL_GetTick();
	if(setTemperatureChanged && (currentTime - lastSetTemperatureTime > 5000)) {
		setTemperatureChanged = 0;
		if(systemSettings.setTemperature != user_currentSetTemperature) {
			systemSettings.setTemperature = user_currentSetTemperature;
			saveSettings();
		}
	}
	switch (currentMode) {
		case mode_boost:
			if(currentTime - currentModeTimer > (currentBoostSettings.time * 1000))
				setCurrentMode(mode_set);
			break;
		case mode_set:
			if(activity)
				currentModeTimer = currentTime;
			else if(currentTime - currentModeTimer > (currentSleepSettings.sleepTime * 1000)) {
				setCurrentMode(mode_sleep);
				buzzer_short_beep();
			}
			else if(currentTime - currentModeTimer > (currentSleepSettings.standbyTime * 1000 * 60)) {
				setCurrentMode(mode_standby);
				buzzer_long_beep();
			}
			break;
		case mode_sleep:
			if(activity) {
				setCurrentMode(mode_set);
				buzzer_short_beep();
			}
			else if(currentTime - currentModeTimer > (currentSleepSettings.standbyTime * 1000 * 60)) {
				setCurrentMode(mode_standby);
				buzzer_long_beep();
			}
			break;
		case mode_standby:
			break;
		default:
			break;
	}



	  if((getSetTemperature() == readTipTemperatureCompensated(0)) && !temperatureReachedFlag) {
		  temperatureReached(getSetTemperature());
		  temperatureReachedFlag = 1;
	  }
}

#ifdef FLAWLESS_MEAS
	#define POWER_LIMIT_PERC 70.0	/* iron is turned on all the time */
#else
	#define POWER_LIMIT_PERC 100.0 /* but iron is turned off ~50 % of time */
#endif

void update_pwm(void){
	  TICK;
	  if(debugMode){
		  set = calculatePID(debugSetPoint, iron_temp_adc_avg);
	  }else{
		  set = calculatePID(human2adc(tempSetPoint), iron_temp_adc_avg);
	  }
	  TOCK(Benchmark._06_pid_calc_dur);
	  set = PWM_TIM_PERDIO * set;
	  //set += 20* (readTipTemperatureCompensated(0)/350.0); // 40 pwm equal to 350C
	  set = (set<1)?1:set;
	  set = (set>PWM_TIM_PERDIO)?PWM_TIM_PERDIO:set;

	  if(isIronOn)
		  currentIronPower = UINT_DIV(set*POWER_LIMIT_PERC, PWM_TIM_PERDIO);
	  else
		  currentIronPower = 0;

	  __HAL_TIM_SET_COMPARE(ironPWMTimer, TIM_CHANNEL_3, CONV_TO_UINT(currentIronPower));
}

void ironInit(TIM_HandleTypeDef *timer) {
	ironPWMTimer = timer;
	user_currentSetTemperature = systemSettings.setTemperature;
	currentSetTemperature = systemSettings.setTemperature;
}

uint8_t getCurrentPower() {
	return currentIronPower;
}
