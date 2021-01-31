/*
 * iron.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef IRON_H_
#define IRON_H_
#include "pid.h"
#include "settings.h"
typedef void (*setTemperatureReachedCallback)(uint16_t);



typedef void (*currentModeChanged)(uint8_t);


typedef struct {

	TIM_HandleTypeDef 		*Pwm_Timer;					// Pointer to the PWM timer
	uint8_t 				Pwm_Channel;				// PWM channel
	uint16_t				Pwm_Limit;					// Max PWM output value possible
	uint16_t				Pwm_Max;					// Max PWM output based on power limit
	uint16_t				Pwm_Out;					// Last PWM value calculated
	TIM_HandleTypeDef 		*Delay_Timer;				// Pointer to the Delay timer
	int8_t 					CurrentIronPower;			// Last output power
	uint16_t 				CurrentSetTemperature;		// Actual set temperature (Setpoint)
	uint16_t 				Debug_SetTemperature;		// Debug mode temperature
	uint32_t 				LastSysChangeTime;			// Last time a system setting was changed
	uint32_t 				LastModeChangeTime;			// Last time the mode was changed (To provide debouncing)
	uint32_t				LastNoPresentTime;			// last time iron absence was detected
	uint32_t				lastActivityTime;			// last time iron was moved
	uint8_t					CurrentMode;				// Actual working mode (Standby, Sleep, Normal, Boost)
	uint8_t					changeMode;					// change working mode to (Standby, Sleep, Normal, Boost)
	uint32_t 				CurrentModeTimer;			// Time since actual mode was set
	bool 					Cal_TemperatureReachedFlag;	// Flag for temperature calibration
	bool 					DebugMode ;				// Flag to indicate Debug is enabled
	bool 					presence;					// Flag to indicate the presence of the iron
	bool 					calibrating;				// Flag to indicate calibration state (don't save temperature settings)
	bool 					PIDUpdate;					// Flag to indicate PID calculation must be updated
	bool 					updateMode;					// Flag to indicate the mode must be changed
	bool 					newActivity;				// Flag to indicate handle movement
	bool 					FailState;					// Flag to indicate a serious failure, totally disables the PWM until the flag is manually cleared
	uint32_t 				RunawayTimer;				// Runaway timer
	uint8_t 				RunawayLevel;				// Runaway actual level
	uint8_t 				prevRunawayLevel;			// Runaway previous level
	bool 					RunawayStatus;			// Runaway triggered flag
	bool					updatePwm;					// Set when timer values need to be updated
}iron_t;

#define IRON_MIN			2048						// OUT= PWM_MAX/IRON_MIN
extern volatile iron_t Iron;
void IronWake(bool source);
void checkIronPresence(void);
bool GetIronPresence(void);
void SetFailState(bool FailState);
bool GetFailState(void);
void setCurrentMode(uint8_t mode, bool forceMode);
void setModefromStand(uint8_t mode);
void setSetTemperature(uint16_t temperature);
void setCurrentTemperature(uint16_t temperature);
uint8_t getCurrentMode(void);
uint16_t getSetTemperature();
uint16_t getCurrentTemperature();
int8_t getCurrentPower();
void initTimers(void);
bool setPwmPeriod(uint16_t period);
bool setPwmDelay(uint16_t delay);
void setNoIronValue(uint16_t noiron);
void setSystemTempUnit(bool unit);
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback);
void addModeChangedCallback(currentModeChanged callback);
void handleIron(void);
void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel);
uint8_t getIronOn();
void setDebugTemp(uint16_t value);
void setDebugMode(uint8_t value);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim);
#endif /* IRON_H_ */
