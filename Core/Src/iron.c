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


volatile iron_t Iron = { 0 };

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


void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel) {
	Iron.Pwm_Timer		= pwmtimer;
	Iron.Delay_Timer	= delaytimer;
	Iron.Pwm_Channel 	= pwmchannel;
	Iron.CurrentMode 	= mode_normal;
	Iron.UserTemperature = systemSettings.UserTemperature;		// Load system temperature setting into userTemperature
	Iron.CurrentTemperature = Iron.UserTemperature;				// Set userTemperature as the CurrentTemperature
    setCurrentTip(systemSettings.currentTip);					// Load TIP

#ifdef	PWM_CHx													// Start PWM
	HAL_TIM_PWM_Start_IT(pwmtimer, pwmchannel);					// CHx Output
#else
	HAL_TIMEx_PWMN_Start_IT(pwmtimer, pwmchannel);				// CHxN Output
#endif
}


void handleIron(void) {

	uint32_t CurrentTime = HAL_GetTick();

	// Update Tip temperature in human readable format
	readTipTemperatureCompensated(New);

	// Check for storing the temperature
	if(CurrTemp_Save_Time_s){
		if( Iron.TemperatureChanged && (CurrentTime - (Iron.LastChangeTemperatureTime) > CurrTemp_Save_Time_s*1000) 	) {
				systemSettings.UserTemperature = Iron.UserTemperature;
				saveSettings();
		}
	}
	switch (Iron.CurrentMode) {
		case mode_boost:
			if(CurrentTime - Iron.CurrentModeTimer > ((uint32_t)systemSettings.boost.Time*1000))
				setCurrentMode(mode_normal);
			break;
		case mode_normal:
			if(systemSettings.sleep.Time && ((CurrentTime - Iron.CurrentModeTimer)> ((uint32_t)systemSettings.sleep.Time*1000)) ) {
				Iron.Active=0;
				setCurrentMode(mode_sleep);
				buzzer_short_beep();
			}
			break;
		case mode_sleep:
			if(Iron.Active) {
				setCurrentMode(mode_normal);
				buzzer_short_beep();
			}
			else if(systemSettings.standby.Time && ((CurrentTime - Iron.CurrentModeTimer) > systemSettings.standby.Time*1000) ) {
				setCurrentMode(mode_standby);
				buzzer_long_beep();
			}
			break;
		case mode_standby:
			if(Iron.Active) {
				setCurrentMode(mode_normal);
				buzzer_short_beep();
			}
			break;
		default:
			break;
	}
	if(!Iron.PIDUpdate){
		return;
	}
	  double set;
	  if(Iron.Debug_Enabled){
		  set = calculatePID(Iron.Debug_Temperature, TIP.last_avg);
	  }
	  else{
		  if(Iron.CurrentTemperature>1){
			  set = calculatePID(human2adc(Iron.CurrentTemperature), TIP.last_avg);
		  }
		  else{
			  set=0;
		  }
	  }
	  if(set==0){ Iron.CurrentIronPower = 0;  }
	  else{ Iron.CurrentIronPower = set * 100; }

	  set = set * (float)(PWM_DUTY);

	  if(set < 0){ set = 0; }
	  Iron.Pwm_Duty = set;											// Set PWM Duty. The ADC will load it after sampling the tip.
	  if(( Iron.CurrentTemperature == readTipTemperatureCompensated(0)) && !Iron.Cal_TemperatureReachedFlag) {
	  		  temperatureReached( Iron.CurrentTemperature);
	  		  Iron.Cal_TemperatureReachedFlag = 1;
	  	  }
}

void setCurrentMode(iron_mode_t mode) {
	Iron.CurrentModeTimer = HAL_GetTick();
	switch (mode) {
		case mode_boost:
			Iron.CurrentTemperature = systemSettings.boost.Temperature;
			break;
		case mode_normal:
			Iron.CurrentTemperature = Iron.UserTemperature;
			break;
		case mode_sleep:
			Iron.Active=0;
			Iron.CurrentTemperature = systemSettings.sleep.Temperature;
			break;
		case mode_standby:
			Iron.Active=0;
			Iron.CurrentTemperature = 0;
			break;
		default:
			break;
	}
	Iron.CurrentMode = mode;
	modeChanged(mode);
}


void DebugSetTemp(uint16_t value) {
	Iron.Debug_Temperature = value;
}

void DebugMode(uint8_t value) {
	Iron.Debug_Enabled = value;
}

void setSetTemperature(uint16_t temperature) {
	if(systemSettings.UserTemperature != temperature){
		Iron.TemperatureChanged=1;
	}
	Iron.CurrentTemperature = temperature;
	Iron.UserTemperature = temperature;
	Iron.Cal_TemperatureReachedFlag = 0;
	Iron.LastChangeTemperatureTime = HAL_GetTick();
	resetPID();
}

iron_mode_t getCurrentMode() {
	return Iron.CurrentMode;
}

uint16_t getSetTemperature() {
	return Iron.CurrentTemperature;
}

uint8_t getCurrentPower() {
	return Iron.CurrentIronPower;
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

