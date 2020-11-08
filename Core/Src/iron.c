/*
 * iron.c
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#include "iron.h"
#include <stdlib.h>
#include "tempsensors.h"
#include "buzzer.h"
#include "settings.h"
#include "main.h"


volatile iron_t Iron = {

		Pwm_Timer: 					0,
		Pwm_Channel:				0,
		CurrentIronPower: 			0,
	{//Temp
		TemperatureReachedFlag: 	0,
		CurrentSetTemperature: 		300,
		UserCurrentSetTemperature: 	300,
		LastSetTemperatureTime: 	0,
		SetTemperatureChanged: 		0,
		Temp_Adc_Avg: 				0,
		TempSetPoint:				0,
	},
	{//Debug
		Enabled:					0,
		SetPoint:					0,
	},
	{//Status
		isOn:						0,
		Active:						1,
		PwmStoppedSince:			0,
		StartOfNoActivityTime:		0,
		ProcessUpdate:				0,
		TempMeasureState:			iron_temp_measure_idle,
		CurrentMode:				mode_sleep,
		CurrentModeTimer:			0,
	}
};

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
	Iron.Debug.SetPoint = value;
}
void setDebugMode(uint8_t value) {
	Iron.Debug.Enabled = value;
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
#ifdef	PWM_CHx
	HAL_TIM_PWM_Start(Iron.Pwm_Timer, Iron.Pwm_Channel);// CHx Output
#else
	HAL_TIMEx_PWMN_Start(Iron.Pwm_Timer, Iron.Pwm_Channel);// CHxN Output
#endif
	Iron.Status.isOn = 1;
}

void turnIronOff() {
#ifdef	PWM_CHx
	HAL_TIM_PWM_Stop(Iron.Pwm_Timer, Iron.Pwm_Channel);// CHx Output
#else
	HAL_TIMEx_PWMN_Stop(Iron.Pwm_Timer, Iron.Pwm_Channel);// CHxN Output
#endif
	Iron.Status.isOn = 0;
}
uint8_t getIronOn() {
	return Iron.Status.isOn;
}
void applyBoostSettings() {
	Iron.CurrentBoostSettings = systemSettings.boost;
}

void applySleepSettings() {
	Iron.CurrentSleepSettings = systemSettings.sleep;
}
void setSetTemperature(uint16_t temperature) {
	Iron.Temp.UserCurrentSetTemperature = temperature;
	Iron.Temp.TemperatureReachedFlag = 0;
	setCurrentTemperature(temperature);
	Iron.Temp.LastSetTemperatureTime = HAL_GetTick();
	Iron.Temp.SetTemperatureChanged = 1;
}
void setCurrentTemperature(uint16_t temperature) {
	Iron.Temp.CurrentSetTemperature = temperature;
	Iron.Temp.TempSetPoint = temperature;
	if((temperature == 0) && getIronOn()){
		turnIronOff();
	}
	else if(!getIronOn()){
		turnIronOn();
	}
	resetPID();
}
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback) {
	setTemperatureReachedCallbackStruct_t *s = malloc(sizeof(setTemperatureReachedCallbackStruct_t));
	if(!s){
		while(1){;}
	}
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
	currentModeChangedCallbackStruct_t *s = malloc(sizeof(currentModeChangedCallbackStruct_t));
	s->callback = callback;
	s->next = NULL;
	currentModeChangedCallbackStruct_t *last = currentModeChangedCallbacks;
	while(last && last->next != NULL) {
		last = last->next;
	}
	if(last){
		last->next = s;
	}
	else{
		last = s;
	}
}

void setCurrentMode(iron_mode_t mode) {
	Iron.Status.CurrentModeTimer = HAL_GetTick();
	Iron.Temp.TemperatureReachedFlag = 0;
	switch (mode) {
		case mode_boost:
			setCurrentTemperature(Iron.CurrentBoostSettings.temperature);
			break;
		case mode_set:
			setCurrentTemperature(Iron.Temp.UserCurrentSetTemperature);
			break;
		case mode_sleep:
			Iron.Status.Active=0;
			setCurrentTemperature(Iron.CurrentSleepSettings.sleepTemperature);
			break;
		case mode_standby:
			Iron.Status.Active=0;
			setCurrentTemperature(0);
			break;
		default:
			break;
	}
	Iron.Status.CurrentMode = mode;
	modeChanged(mode);
}
iron_mode_t getCurrentMode() {
	return Iron.Status.CurrentMode;
}
uint16_t getSetTemperature() {
	return Iron.Temp.CurrentSetTemperature;
}

void handleIron(void) {
	uint32_t CurrentTime = HAL_GetTick();

	static RollingTypeDef_t data = {
		adc_buffer: 			(uint16_t*)&adc_measures[0].IRON_TEMP,
		adc_buffer_size: 		Adc_Buffer_Size,
		adc_buffer_elements:	Adc_Buffer_Elements,
		rolling_buffer_size: 	RollingBufferSize,
		rolling_buffer_index: 	0,
		last_avg: 				0,
		init: 					0
	};

	if(!data.init){
		uint16_t x = data.rolling_buffer_size;
		for(x=0;x<data.rolling_buffer_size;x++) {
			data.rolling_buffer[x]=0;
		}
		data.init=1;
	}

	if(Iron.Status.TempMeasureState == iron_temp_measure_requested){		//Stop PWM
		#ifdef	PWM_CHx
		HAL_TIM_PWM_Stop(Iron.Pwm_Timer, Iron.Pwm_Channel);					// CHx Output

		#else
		HAL_TIMEx_PWMN_Stop(Iron.Pwm_Timer, Iron.Pwm_Channel);				// CHxN Output

		#endif
		Iron.Status.TempMeasureState = iron_temp_measure_pwm_stopped;
		Iron.Status.PwmStoppedSince = HAL_GetTick();
		return;
	}
	else if(Iron.Status.TempMeasureState == iron_temp_measure_pwm_stopped){	//PWM Stopped enough time to start ADC conversion?
		if(Adc_Measure_Delay){
			if((HAL_GetTick() - Iron.Status.PwmStoppedSince) < Adc_Measure_Delay){
				return;
			}
		}

		if( ADC_Start_DMA() == HAL_OK ){									// Change state when the ADC was triggered successfully
			Iron.Status.TempMeasureState = iron_temp_measure_started;		// Or try again on next call
		}
		return;
	}
	else if(Iron.Status.TempMeasureState != iron_temp_measure_ready){		// Conversion done?
			return;
	}
																			// Yes
	RollingUpdate(&data);													// Calculate average
	Iron.Temp.Temp_Adc_Avg=data.last_avg;									// Store last average

	if(getIronOn()){																																									// Enable PWM if it was active before ADC conversion
		#ifdef	PWM_CHx
			HAL_TIM_PWM_Start(Iron.Pwm_Timer, Iron.Pwm_Channel);			// CHx Output
		#else
			HAL_TIMEx_PWMN_Start(Iron.Pwm_Timer, Iron.Pwm_Channel); 		//CHxN Output
		#endif
	}

	readTipTemperatureCompensated(1);
	Iron.Status.TempMeasureState = iron_temp_measure_idle;
	if(CurrTemp_Save_Time_S){
		if(Iron.Temp.SetTemperatureChanged && (CurrentTime - ((uint32_t)Iron.Temp.LastSetTemperatureTime*1000) > (uint32_t)CurrTemp_Save_Time_S*1000)) {
			Iron.Temp.SetTemperatureChanged = 0;
			if(systemSettings.setTemperature != Iron.Temp.UserCurrentSetTemperature) {
				systemSettings.setTemperature = Iron.Temp.CurrentSetTemperature;
				saveSettings();
			}
		}
	}
	switch (Iron.Status.CurrentMode) {
		case mode_boost:
			if(CurrentTime - Iron.Status.CurrentModeTimer > ((uint32_t)Iron.CurrentBoostSettings.time*1000))
				setCurrentMode(mode_set);
			break;
		case mode_set:
			if(Iron.CurrentSleepSettings.sleepTime && ((CurrentTime - Iron.Status.CurrentModeTimer)> ((uint32_t)Iron.CurrentSleepSettings.sleepTime*1000)) ) {
				Iron.Status.Active=0;
				setCurrentMode(mode_sleep);
				buzzer_short_beep();
			}
			break;
		case mode_sleep:
			if(Iron.Status.Active) {
				setCurrentMode(mode_set);
				buzzer_short_beep();
			}
			else if(Iron.CurrentSleepSettings.standbyTime && ((CurrentTime - Iron.Status.CurrentModeTimer) > Iron.CurrentSleepSettings.standbyTime*1000) ) {
				setCurrentMode(mode_standby);
				buzzer_long_beep();
			}
			break;
		case mode_standby:
			if(Iron.Status.Active) {
				setCurrentMode(mode_set);
				buzzer_short_beep();
			}
			break;
		default:
			break;
	}

	  double set;
	  if(Iron.Debug.Enabled)
		  set = calculatePID(Iron.Debug.SetPoint, Iron.Temp.Temp_Adc_Avg);
	  else
		  set = calculatePID(human2adc(Iron.Temp.TempSetPoint), Iron.Temp.Temp_Adc_Avg);
	  if(Iron.Status.isOn)
		  Iron.CurrentIronPower = set * 100;
	  else
		  Iron.CurrentIronPower = 0;
	  set = 2000.0 *(set * 100.0 -12.0388878376)/102.72647713;
	  if(set < 0)
		  set = 0;
	  __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, set);
	  if((getSetTemperature() == readTipTemperatureCompensated(0)) && !Iron.Temp.TemperatureReachedFlag) {
		  temperatureReached(getSetTemperature());
		  Iron.Temp.TemperatureReachedFlag = 1;
	  }
}

void ironInit(TIM_HandleTypeDef *timer, uint32_t channel) {
	Iron.Pwm_Timer = timer;
	Iron.Pwm_Channel= channel;
    applyBoostSettings();
    applySleepSettings();
    setCurrentTip(systemSettings.currentTip);
	setSetTemperature(systemSettings.setTemperature);
}

uint8_t getCurrentPower() {
	return Iron.CurrentIronPower;
}
