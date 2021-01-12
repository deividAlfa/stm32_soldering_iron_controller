/*
 * iron.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "iron.h"
#include "buzzer.h"
#include "settings.h"
#include "main.h"
#include "tempsensors.h"
#include "ssd1306.h"

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
	Iron.Pwm_Timer			= pwmtimer;
	Iron.Delay_Timer		= delaytimer;
	Iron.Pwm_Channel 		= pwmchannel;
	Iron.Pwm_Out 			= 0;										// PWM disabled
	Iron.isPresent			= 1;										// Set detected by default (to not show ERROR screen at boot)
	setCurrentMode(systemSettings.settings.initMode);							// Set mode
	initTimers();														// Initialize timers

#ifdef	PWM_CHx															// Start PWM
	HAL_TIM_PWM_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);				// PWM output uses CHx channel
#elif defined PWM_CHxN
	HAL_TIMEx_PWMN_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);			// PWM output uses CHxN channel
#else
	#error No PWM ouput set (See PWM_CHx / PWM_CHxN in board.h)
#endif
	HAL_TIM_Base_Start_IT(Iron.Delay_Timer);							// Configure and start Delay timer in interrupt mode
	__HAL_TIM_DISABLE(Iron.Delay_Timer);								// Stop timer

	// Now the PWM and ADC are working in the background.
}

void handleIron(void) {
	static uint32_t prevSysChecksum=0, newSysChecksum=0,prevTipChecksum=0, newTipChecksum=0, checksumtime=0;
	uint32_t CurrentTime = HAL_GetTick();
	int16_t ambTemp = readColdJunctionSensorTemp_x10(Unit_Celsius) / 10;
	double set;

	// Totally disabled if tip is not defined
	if(!GetFailState()){
		if((systemSettings.settings.currentProfile!=Profile_T12)&&(systemSettings.settings.currentProfile!=Profile_C245)&&(systemSettings.settings.currentProfile!=Profile_C210)){
			SetFailState(1);
		}
	}

	if(!Iron.RunawayTriggered && Iron.isPresent){							// If overrun not triggered yet and iron detected
		if(Iron.Pwm_Out!=0){												// If PWM is active
			for(int8_t c=runaway_100;c>=runaway_ok;c--){					// Check for overrun
				Iron.RunawayLevel=c;
				if(TIP.last_avg > human2adc(Iron.CurrentSetTemperature+(25*c)) ){
					break;													// Stop at the first overrun condition
				}
			}
			if(Iron.RunawayLevel!=runaway_ok){										// Runaway detected?
				if(Iron.prevRunawayLevel==runaway_ok){								// First overrun detection?
					Iron.prevRunawayLevel=Iron.RunawayLevel;						// Yes, store in prev level
					Iron.RunawayTimer=HAL_GetTick();								// Store time
				}
				else{																// Was already triggered
					switch(Iron.RunawayLevel){
						case runaway_ok:											// No problem (<25ºC difference)
							break;													// (Never used here)
						case runaway_25:											// Temp >25°C over setpoint
							if(HAL_GetTick()>(Iron.RunawayTimer+20000)){			// 20 second limit
								Iron.RunawayTriggered=1;
								FatalError(error_RUNAWAY25);
							}
							break;
						case runaway_50:											// Temp >50°C over setpoint
							if(HAL_GetTick()>(Iron.RunawayTimer+10000)){			// 10 second limit
								Iron.RunawayTriggered=1;
								FatalError(error_RUNAWAY50);
							}
							break;
						case runaway_75:											// Temp >75°C over setpoint
							if(HAL_GetTick()>(Iron.RunawayTimer+3000)){				// 3 second limit
								Iron.RunawayTriggered=1;
								FatalError(error_RUNAWAY75);
							}
							break;
						case runaway_100:											// Temp >100°C over setpoint
							if(HAL_GetTick()>(Iron.RunawayTimer+1000)){				// 3 second limit
								Iron.RunawayTriggered=1;
								FatalError(error_RUNAWAY100);
							}
							break;
						default:													// Unknown overrun state
							Iron.RunawayTriggered=1;
							FatalError(error_RUNAWAY_UNKNOWN);
							break;
					}
				}
			}
			else{
				Iron.prevRunawayLevel=runaway_ok;							// No, clear prev level
			}
		}
	}

	// Check changes in system settings
	if( (!systemSettings.setupMode) && (CurrentTime-checksumtime>999) && (!Iron.isCalibrating) && (systemSettings.settings.saveSettingsDelay) ){

		checksumtime=CurrentTime;														// Store time
		newSysChecksum=ChecksumSettings(&systemSettings.settings);						// Calculate system checksum
		newTipChecksum=ChecksumProfile(&systemSettings.Profile);						// Calculate tips checksum
		if((systemSettings.settingsChecksum!=newSysChecksum)||(systemSettings.ProfileChecksum!=newTipChecksum)){ 	// If any checksum changed
			if((prevSysChecksum!=newSysChecksum)||(prevTipChecksum!=newTipChecksum)){								// If different from the previous calculated checksum.
				prevSysChecksum=newSysChecksum;																		// A change was done recently
				prevTipChecksum=newTipChecksum;
				Iron.LastSysChangeTime=CurrentTime;																	// Reset timer (we don't save anything until we pass a certain time without changes)
			}
			// If different from the previous calculated checksum, and timer expired (No changes for a long time)
			else if((CurrentTime-Iron.LastSysChangeTime)>((uint32_t)systemSettings.settings.saveSettingsDelay*1000)){
				saveSettings(0);																					// Save settings
			}
		}
	}

	// Temperature unit change adjustments
	if(Iron.TemperatureUnitChanged){
		Iron.TemperatureUnitChanged=0;
		switchTempUnit();
	}

	// No iron detection
	if((TIP.last_RawAvg>systemSettings.Profile.noIronValue)||(ambTemp < -60)) {				// NTC disconnected reports -70ºC or so
		SetIronPresence(0);
	}
	else{
		SetIronPresence(1);
	}

	// Iron wake signal flag (for gui displaying the pulse icon)
	if(Iron.hasMoved){
		if((CurrentTime-Iron.LastMovedTime)>50){
			Iron.hasMoved = 0;
		}
	}
	// Update Tip temperature in human readable format
	readTipTemperatureCompensated(New);

	// Failure flag
	if( (Iron.FailState)|| (!Iron.isPresent)){
		return;																		// Do nothing if in failure state (PWM already disabled)
	}
	// Controls inactivity timer and enters low power modes
	switch (Iron.CurrentMode) {
		case mode_boost:
			if(CurrentTime - Iron.CurrentModeTimer > ((uint32_t)systemSettings.Profile.boost.Time*60000))
				setCurrentMode(mode_normal);
			break;
		case mode_normal:
			if(!Iron.isCalibrating&&systemSettings.Profile.sleep.Time && ((CurrentTime - Iron.CurrentModeTimer)>(uint32_t)systemSettings.Profile.sleep.Time*60000) ) {
				setCurrentMode(mode_sleep);
			}
			break;
		case mode_sleep:
			if(systemSettings.Profile.standby.Time && ((CurrentTime - Iron.CurrentModeTimer) > (uint32_t)systemSettings.Profile.standby.Time*60000) ) {
				setCurrentMode(mode_standby);
			}
			break;
		default:
			break;
	}


	// Only continue if PID update flag is set
	// Don't calculate PID for the first second after boot, as the filters haven't got enough data yet
	if(!Iron.PIDUpdate || HAL_GetTick()<1000){
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
	  Iron.Pwm_Out = 0;			// Min 0.x% to keep detecting iron?
	}

	// For calibration process
	if(	  ( (readTipTemperatureCompensated(0)>=(Iron.CurrentSetTemperature-5)) &&	// Add +-3ºC play
			(readTipTemperatureCompensated(0)<=(Iron.CurrentSetTemperature+5))  )
			 && !Iron.Cal_TemperatureReachedFlag) {
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
void switchTempUnit(void){
	uint16_t tmp;
	if(systemSettings.settings.tempUnit==Unit_Farenheit){
		tmp = TempConversion(systemSettings.Profile.UserSetTemperature,toFarenheit,0);
		systemSettings.Profile.UserSetTemperature = round_10(tmp);
		tmp = TempConversion(systemSettings.Profile.boost.Temperature,toFarenheit,0);
		systemSettings.Profile.boost.Temperature = round_10(tmp);
		tmp = TempConversion(systemSettings.Profile.sleep.Temperature,toFarenheit,0);
		systemSettings.Profile.sleep.Temperature = round_10(tmp);
	}
	else{
		tmp = TempConversion(systemSettings.Profile.UserSetTemperature,toCelsius,0);
		systemSettings.Profile.UserSetTemperature = round_10(tmp);
		tmp = TempConversion(systemSettings.Profile.boost.Temperature,toCelsius,0);
		systemSettings.Profile.boost.Temperature = round_10(tmp);
		tmp = TempConversion(systemSettings.Profile.sleep.Temperature,toCelsius,0);
		systemSettings.Profile.sleep.Temperature = round_10(tmp);
	}
	setCurrentMode(mode_normal);
}
// Sets the temperature unit used (Celsius,Farenheit)
void setTempUnit(TempUnit_t unit){
	if(systemSettings.settings.tempUnit!=unit){
		Iron.TemperatureUnitChanged=1;
		systemSettings.settings.tempUnit = unit;
	}
}

// This function sets the prescaler settings depending on the system, core clock
// and loads the stored period
void initTimers(void){
	uint16_t delay, pwm;
	if(systemSettings.settings.currentProfile!=Profile_None){
		delay=systemSettings.Profile.pwmDelay;
		pwm=systemSettings.Profile.pwmPeriod;
	}
	else{
		delay=1999;			// Safe values if tip not initialized
		pwm=19999;
	}
	// Delay timer config
	//
	Iron.Delay_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;				//10uS input clock
	Iron.Delay_Timer->Init.Period = delay;
	if (HAL_TIM_Base_Init(Iron.Delay_Timer) != HAL_OK){
		Error_Handler();
	}

	// PWM timer config
	//
	Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;				//10uS input clock
	Iron.Pwm_Timer->Init.Period = pwm;
	if (HAL_TIM_Base_Init(Iron.Pwm_Timer) != HAL_OK){
		Error_Handler();
	}

	__HAL_TIM_CLEAR_FLAG(Iron.Pwm_Timer,TIM_FLAG_UPDATE | TIM_FLAG_COM | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 );	// Clear all flags
	__HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE | TIM_FLAG_COM | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 );	// Clear all flags
}
// Applies the PWM settings from the system settings
void ApplyPwmSettings(void){
	__HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer,systemSettings.Profile.pwmPeriod);
	__HAL_TIM_SET_AUTORELOAD(Iron.Delay_Timer,systemSettings.Profile.pwmDelay);
	Iron.Pwm_Max = systemSettings.Profile.pwmPeriod - (systemSettings.Profile.pwmDelay+(uint16_t)ADC_MEASURE_TIME);
}
// Sets no Iron detection threshold
void setNoIronValue(uint16_t noiron){
	systemSettings.Profile.noIronValue=noiron;
}
// Change the iron operating mode
void setCurrentMode(iron_mode_t mode) {
	Iron.CurrentModeTimer = HAL_GetTick();
	switch (mode) {
		case mode_boost:
			Iron.CurrentSetTemperature = systemSettings.Profile.boost.Temperature;
			break;
		case mode_normal:
			Iron.CurrentSetTemperature = systemSettings.Profile.UserSetTemperature;
			break;
		case mode_sleep:
			Iron.CurrentSetTemperature = systemSettings.Profile.sleep.Temperature;
			break;
		case mode_standby:
		default:
			mode=mode_standby;
			Iron.CurrentSetTemperature = 0;
			break;
	}
	if(Iron.CurrentMode != mode){
		Iron.CurrentMode = mode;
		buzzer_long_beep();
		Iron.Cal_TemperatureReachedFlag = 0;
	}
	modeChanged(mode);
}
// Called from program timer if WAKE change is detected
void IronWake(bool source){													// source: 0 = handle, 1=encoder
	if((Iron.CurrentMode==mode_sleep) || (Iron.CurrentMode==mode_standby)){
		setCurrentMode(mode_normal);				// Back to normal mode
	}
	else if(Iron.CurrentMode==mode_normal){
		Iron.CurrentModeTimer = HAL_GetTick();		// Clear timer to avoid entering sleep mode
	}
	if(!source){
		Iron.LastMovedTime = HAL_GetTick();
		Iron.hasMoved=1;
	}
}
// Sets the presence of the iron. Handles alarm output
void SetIronPresence(bool isPresent){
	uint32_t CurrentTime = HAL_GetTick();
	if(Iron.isPresent){								// Was present
		if(!isPresent){								// But no longer
			Iron.LastNoPresentTime = CurrentTime;	// Start alarm and save last detected time
			buzzer_alarm_start();
			Iron.isPresent = 0;
			Iron.CurrentIronPower = 0;				// 0% power
			Iron.Pwm_Out = 0;						// Disable pwm
			Iron.prevRunawayLevel=runaway_ok;		// Reset previous overrun
		}
	}
	else{											// Wasn't present
		if(isPresent && (CurrentTime-Iron.LastNoPresentTime)>systemSettings.settings.noIronDelay ){	// But now it is back
			buzzer_alarm_stop();					// Stop alarm							// If enough time passed since last detection
			Iron.isPresent = 1;
			setCurrentMode(mode_normal);
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
	static uint8_t prevIronType;
	if(systemSettings.settings.currentProfile!=Profile_None){
		if((systemSettings.Profile.UserSetTemperature != temperature)||(prevIronType!=systemSettings.settings.currentProfile)){
			prevIronType=systemSettings.settings.currentProfile;
			systemSettings.Profile.UserSetTemperature = temperature;
			Iron.CurrentSetTemperature=temperature;
			Iron.Cal_TemperatureReachedFlag = 0;
			resetPID();
		}
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
