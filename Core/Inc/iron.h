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

typedef enum {Unit_Celsius,Unit_Farenheit,Unit_Kelvin}TempUnit_t;

typedef struct {

	TIM_HandleTypeDef 		*Pwm_Timer;					// Pointer to the PWM timer
	uint8_t 				Pwm_Channel;				// PWM channel
	uint16_t				Pwm_Out;					// Last PWM value calculated
	uint16_t				Pwm_Max;					// Max PWM output value
	TIM_HandleTypeDef 		*Delay_Timer;				// Pointer to the Delay timer
	uint8_t 				CurrentIronPower;			// Last output power
	uint16_t 				CurrentSetTemperature;		// Actual set temperature (Setpoint)
	uint16_t 				UserSetTemperature;			// Normal mode temperature
	uint16_t 				Debug_SetTemperature;		// Debug mode temperature
	uint32_t 				LastSysChangeTime;			// Last time a system setting was changed
	uint32_t 				StartOfNoActivityTime;		// Last time since no activity in handle sensor
	uint32_t				LastNoPresentTime;			// last time iron absence was detected
	uint32_t				LastMovedTime;				// last time iron was moved
	IRON_measure_state_t 	TempMeasureState;			// Status of ADC measuring (idle/ready)
	iron_mode_t				CurrentMode;				// Actual working mode (Stanby, Sleep, Normal, Boost)
	uint32_t 				CurrentModeTimer;			// Time since actual mode was set
	bool 					Cal_TemperatureReachedFlag;	// Flag for temperature calibration
	bool					TemperatureChanged;			// Flag to indicate UserSetTemperature was changed (For saving purposes)
	bool 					Debug_Enabled ;				// Flag to indicate Debug is enabled
	bool 					isPresent;					// Flag to indicate the presence of the iron
	bool 					PIDUpdate;					// Flag to indicate PID calculation must be updated
	bool 					OledUpdate;					// Flag to indicate OLED screen must be updated
	bool 					hasMoved;					// Flag to indicate handle movement
	bool 					FailState;					// Flag to indicate a serious failure, totally disables the PWM until the flag is manually cleared
}iron_t;

typedef struct
{
	float fSetTemperature;			// Set Temperature (°C)
	float fActualTemperature;		// Actual (measured) Tip Temperature (°C)

	// Improved Take-Back-Half algorithm.
	float fGamma;
	float fBeta1;
	float fBeta2;
	float fTimeStep;

	float fTemperaturePrev;
	float fTemperatureErrorPrevZC;
	float fHeaterControl;
	float fHeaterControlPrevZC;
}	TempControl_t;
extern volatile iron_t Iron;
void IronWake(void);
void SetIronPresence(bool isPresent);
bool GetIronPresence(void);
void SetFailState(bool FailState);
bool getFailState(void);
void setCurrentMode(iron_mode_t mode);
void setSetTemperature(uint16_t temperature);
void setCurrentTemperature(uint16_t temperature);
iron_mode_t getCurrentMode();
uint16_t getSetTemperature();
uint16_t getCurrentTemperature();
uint8_t getCurrentPower();
void ApplyPwmSettings(void);
void setNoIronValue(uint16_t noiron);
void setTempUnit(TempUnit_t unit);
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback);
void addModeChangedCallback(currentModeChanged callback);
void handleIron(void);
void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel);
uint8_t getIronOn();
void DebugSetTemp(uint16_t value);
void DebugMode(uint8_t value);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim);
#endif /* IRON_H_ */
