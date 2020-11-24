/*
 * iron.h
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#ifndef IRON_H_
#define IRON_H_


#include "pid.h"

typedef void (*setTemperatureReachedCallback)(uint16_t);
typedef enum {IRON_measure_idle, IRON_measure_ready} IRON_measure_state_t;


typedef enum {mode_standby=0, mode_sleep, mode_normal, mode_boost} iron_mode_t;
typedef void (*currentModeChanged)(iron_mode_t);

typedef struct {
	uint16_t Temperature;
	uint16_t Time;
} ironSettings_t;

typedef struct tipData {
	uint16_t calADC_At_200;
	uint16_t calADC_At_300;
	uint16_t calADC_At_400;
	char name[5];
	pid_values_t PID;
} tipData;


typedef struct {

	TIM_HandleTypeDef 		*Pwm_Timer;
	uint32_t 				Pwm_Channel;
	uint16_t				Pwm_Duty;
	TIM_HandleTypeDef 		*Delay_Timer;
	uint8_t 				CurrentIronPower;

	uint16_t 				CurrentTemperature;
	uint16_t 				UserTemperature;
	uint32_t 				LastChangeTemperatureTime;
	bool					TemperatureChanged;

	bool 					Debug_Enabled ;
	uint16_t 				Debug_Temperature;

	uint8_t 				Cal_TemperatureReachedFlag;

	bool 					Active;
	uint32_t 				StartOfNoActivityTime;
	bool 					PIDUpdate;
	bool 					OledUpdate;
	IRON_measure_state_t 	TempMeasureState;
	iron_mode_t				CurrentMode;
	uint32_t 				CurrentModeTimer;

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
void handleIron(void);
void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel);
uint8_t getIronOn();
void DebugSetTemp(uint16_t value);
void DebugMode(uint8_t value);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim);
#endif /* IRON_H_ */
