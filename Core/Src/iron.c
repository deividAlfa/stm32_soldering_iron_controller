/*
 * iron.c
 *
 *  Created on: Sep 14, 2017
 *      Author: jose
 */

#include "iron.h"
#include <stdlib.h>
#include "buzzer.h"
#include "settings.h"
#include "main.h"
#include "tempsensors.h"

volatile iron_t Iron;
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
	Iron.Pwm_Out 		= 0;											// PWM disabled
	Iron.isPresent		= 1;											// Set detected by default (to not show ERROR screen at boot)
	setCurrentMode(mode_normal);										// Set normal mode
	setCurrentTip(systemSettings.currentTip);							// Load TIP
#ifdef	PWM_CHx															// Start PWM
	HAL_TIM_PWM_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);				// PWM output uses CHx channel
#else
	HAL_TIMEx_PWMN_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);			// PWM output uses CHxN channel
#endif
	ApplyPwmSettings();													// Apply PWM settings
	// Now the PWM and ADC are working in the background.


}

void handleIron(void) {
	static uint32_t previouschecksum=0, newchecksum=0, checksumtime=0;
	uint32_t CurrentTime = HAL_GetTick();
	double set;

	// Totally disabled if tip is not defined
	if((systemSettings.TipType!=Tip_T12)&&(systemSettings.TipType!=Tip_JBC)){
		SetFailState(1);
	}
	// Temperature unit change adjustments
	if(Iron.TemperatureUnitChanged){
		Iron.TemperatureUnitChanged=0;
		switchSysTempUnit();
	}

	// No iron detection
	if(TIP.last_RawAvg>systemSettings.noIronValue) { SetIronPresence(0); }
	else{ SetIronPresence(1); }

	// Iron wake signal flag (for gui displaying the pulse icon)
	if(Iron.hasMoved){
		if((CurrentTime-Iron.LastMovedTime)>50){
			Iron.hasMoved = 0;
		}
	}
	// Update Tip temperature in human readable format
	readTipTemperatureCompensated(New);

	// Check changes in system settings
	if(CurrentTime-checksumtime>499 ){								// Check checksum every 500mS
		checksumtime=CurrentTime;
		if(!Iron.isCalibrating && systemSettings.saveSettingsDelay){	// Don't save while in calibration mode, 0=don't dave
			newchecksum=ChecksumSettings(&systemSettings);				// Calculate system checksum
			if(systemSettings.checksum!=newchecksum){					// If latest checksum is not the same as the system settings's checksum
				if(previouschecksum!=newchecksum){						// If different from the previous calculated checksum.
					previouschecksum=newchecksum;						// A change was done recently
					Iron.LastSysChangeTime=CurrentTime;					// Reset timer (we don't save anything until we pass a certain time without changes)
				}
				// If different from the previous calculated checksum, and timer expired (No changes for a long time)
				else if((CurrentTime-Iron.LastSysChangeTime)>((uint32_t)systemSettings.saveSettingsDelay*1000)){
					saveSettings();															// Save
				}
			}
		}
	}

	// Failure flag
	if(Iron.FailState){
		return;			// Do nothing if in failure state (PWM already disabled)
	}

	// Controls inactivity timer and enters low power modes
	switch (Iron.CurrentMode) {
		case mode_boost:
			if(CurrentTime - Iron.CurrentModeTimer > ((uint32_t)systemSettings.boost.Time*1000))
				setCurrentMode(mode_normal);
			break;
		case mode_normal:
			if(!Iron.isCalibrating&&systemSettings.sleep.Time && ((CurrentTime - Iron.CurrentModeTimer)>(uint32_t)systemSettings.sleep.Time*1000) ) {
				setCurrentMode(mode_sleep);
			}
			break;
		case mode_sleep:
			if(systemSettings.standby.Time && ((CurrentTime - Iron.CurrentModeTimer) > (uint32_t)systemSettings.standby.Time*1000) ) {
				setCurrentMode(mode_standby);
			}
			break;
		default:
			break;
	}

	// Disables PID calculation if no iron is detected
	if(!Iron.isPresent){							// If iron not present
		Iron.CurrentIronPower = 0;					// Indicate 0.1% power
		Iron.Pwm_Out = Iron.Pwm_Max/1000;				// Set 0.1% power (to maintain no iron detection)
		return;
	}

	// Only continue if PID update flag is set
	if(!Iron.PIDUpdate){
		return;
	}


	// If in debug mode, use debug setpoint value
	if(Iron.Debug_Enabled){
	  set = calculatePID(Iron.Debug_SetTemperature, TIP.last_avg);
	}
	// Else use current setpoint value
	else{
	  // Disable output if requested temperature is below 100ºC
	  if(Iron.CurrentSetTemperature>99){
		  uint16_t t=human2adc(Iron.CurrentSetTemperature);
		  if(t){
			  set = calculatePID(t, TIP.last_avg);
		  }
		  else{ set=0; }
	  }
	  else{
		  set=0;
	  }
	}
	// If PID output negative, set to 0
	if(set < 0){ set = 0; }
	// If positive PID output, calculate PWM duty and power output.
	if(set){
	  Iron.CurrentIronPower = set*100;
	  Iron.Pwm_Out = set*(float)Iron.Pwm_Max;	// Set PWM Duty. The ADC will load it after sampling the tip.
	}
	// Else, set both to 0
	else{
	  Iron.CurrentIronPower = 0;
	  Iron.Pwm_Out = 0;
	}
	// For calibration process
	if(( Iron.CurrentSetTemperature == readTipTemperatureCompensated(0)) && !Iron.Cal_TemperatureReachedFlag) {
		  temperatureReached( Iron.CurrentSetTemperature);
		  Iron.Cal_TemperatureReachedFlag = 1;
	  }

}
// Round to 10
uint16_t round_10(uint16_t input){
	if((input%10)>5){
		input+=(10-input%10);	// ex. 640°F=337°C->340°C)
	}
	else{
		input-=input%10;		// ex. 300°C=572°F->570°F
	}
	return input;
}
// Changes the system temperature unit
void switchSysTempUnit(void){
	uint16_t tmp;
	if(systemSettings.tempUnit==Unit_Farenheit){
		tmp = TempConversion(systemSettings.UserSetTemperature,toFarenheit);
		systemSettings.UserSetTemperature = round_10(tmp);
		tmp = TempConversion(systemSettings.boost.Temperature,toFarenheit);
		systemSettings.boost.Temperature = round_10(tmp);
		tmp = TempConversion(systemSettings.sleep.Temperature,toFarenheit);
		systemSettings.sleep.Temperature = round_10(tmp);
	}
	else{
		tmp = TempConversion(systemSettings.UserSetTemperature,toCelsius);
		systemSettings.UserSetTemperature = round_10(tmp);
		tmp = TempConversion(systemSettings.boost.Temperature,toCelsius);
		systemSettings.boost.Temperature = round_10(tmp);
		tmp = TempConversion(systemSettings.sleep.Temperature,toCelsius);
		systemSettings.sleep.Temperature = round_10(tmp);
	}
	setCurrentMode(mode_normal);
}
// Sets the temperature unit used (Celsius,Farenheit)
void setTempUnit(TempUnit_t unit){
	if(systemSettings.tempUnit!=unit){
		Iron.TemperatureUnitChanged=1;
		systemSettings.tempUnit = unit;
	}
}

// Applies the PWM settings from the system settings
void ApplyPwmSettings(void){
	__HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer,systemSettings.pwmPeriod);
	__HAL_TIM_SET_AUTORELOAD(Iron.Delay_Timer,systemSettings.pwmDelay);
	Iron.Pwm_Max = systemSettings.pwmPeriod - (systemSettings.pwmDelay+(uint16_t)ADC_MEASURE_TIME);
}
// Sets no Iron detection threshold
void setNoIronValue(uint16_t noiron){
	systemSettings.noIronValue=noiron;
}
// Change the iron operating mode
void setCurrentMode(iron_mode_t mode) {
	Iron.CurrentModeTimer = HAL_GetTick();
	switch (mode) {
		case mode_boost:
			Iron.CurrentSetTemperature = systemSettings.boost.Temperature;
			break;
		case mode_normal:
			if((Iron.CurrentMode==mode_sleep) || (Iron.CurrentMode==mode_standby)){
				buzzer_short_beep();
			}
			Iron.CurrentSetTemperature = systemSettings.UserSetTemperature;
			break;
		case mode_sleep:
			buzzer_short_beep();
			Iron.CurrentSetTemperature = systemSettings.sleep.Temperature;
			break;
		case mode_standby:
			buzzer_long_beep();
			Iron.CurrentSetTemperature = 0;
			break;
		default:
			break;
	}
	Iron.CurrentMode = mode;
	modeChanged(mode);
}
// Called from program timer if WAKE change is detected
void IronWake(void){
	if((Iron.CurrentMode==mode_sleep) || (Iron.CurrentMode==mode_standby)){
		setCurrentMode(mode_normal);				// Back to normal mode
	}
	else if(Iron.CurrentMode==mode_normal){
		Iron.CurrentModeTimer = HAL_GetTick();		// Clear timer to avoid entering sleep mode
	}
	Iron.LastMovedTime = HAL_GetTick();
	Iron.hasMoved=1;
}
// Sets the presence of the iron. Handles alarm output
void SetIronPresence(bool isPresent){
	uint32_t CurrentTime = HAL_GetTick();
	if(Iron.isPresent){								// Was present
		if(!isPresent){								// But no longer
			Iron.LastNoPresentTime = CurrentTime;	// Start alarm and save last detected time
			buzzer_alarm_start();
			Iron.isPresent = 0;
		}
	}
	else{											// Wasn't present
		if(isPresent && (CurrentTime-Iron.LastNoPresentTime)>systemSettings.noIronDelay ){	// But now it is back
			buzzer_alarm_stop();					// Stop alarm							// If enough time passed since last detection
			Iron.isPresent = 1;
		}
		else if(!isPresent){
			Iron.LastNoPresentTime = CurrentTime;	// Still not present, save last detected time
		}
	}
}

// Returns the actual status of the iron presence.
bool GetIronPresence(void){
	return Iron.isPresent;
}

// Sets Failure state
void SetFailState(bool FailState) {
	Iron.FailState = FailState;
	if(FailState){	// Totally disable PWM Output
		__HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(Iron.Pwm_Timer);
	}
	else{			// Enable PWM Output
		__HAL_TIM_MOE_ENABLE(Iron.Pwm_Timer);
	}
}

// Gets Failure state
bool GetFailState() {
	return Iron.FailState;
}


// Sets the debug temperature
void DebugSetTemp(uint16_t value) {
	Iron.Debug_SetTemperature = value;
}
// Handles the debug activation/deactivation
void DebugMode(uint8_t value) {
	Iron.Debug_Enabled = value;
}

// Sets the user temperature
void setSetTemperature(uint16_t temperature) {
	if(Iron.UserSetTemperature != temperature){
		Iron.CurrentSetTemperature = temperature;
		systemSettings.UserSetTemperature = temperature;
		Iron.Cal_TemperatureReachedFlag = 0;
		resetPID();
	}
}

// Returns the actual set temperature
uint16_t getSetTemperature() {
	return Iron.CurrentSetTemperature;
}

// Returns the actual working mode of the iron
iron_mode_t getCurrentMode() {
	return Iron.CurrentMode;
}

// Returns the output power
uint8_t getCurrentPower() {
	return Iron.CurrentIronPower;
}

// Adds a callback to be called when the set temperature is reached
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

// Adds a callback to be called when the iron working mode is changed
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
