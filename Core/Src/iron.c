/*
 * iron.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
 */

#include "iron.h"
#include "buzzer.h"
#include "settings.h"
#include "main.h"
#include "tempsensors.h"
#include "voltagesensors.h"
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

static void modeChanged(uint8_t newMode) {
  currentModeChangedCallbackStruct_t *s = currentModeChangedCallbacks;
  while(s) {
    s->callback(newMode);
    s = s->next;
  }
}


void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel) {
  Iron.Pwm_Timer      = pwmtimer;
  Iron.Delay_Timer    = delaytimer;
  Iron.Pwm_Channel     = pwmchannel;
  Iron.Error.Flags    = noError;                                                              // Clear all errors
  setCurrentMode(systemSettings.settings.initMode);                                           // Set mode
  initTimers();                                                                               // Initialize timers
                                                                                              // Now the PWM and ADC are working in the background.
}

void handleIron(void) {
  uint32_t CurrentTime = HAL_GetTick();
  uint16_t tipTemp = readTipTemperatureCompensated(update_reading,read_Avg);                // Update Tip temperature in human readable format

  if(!Iron.Error.failState){
    if( (systemSettings.setupMode==setup_On) ||                                             // Ensure not being in setup mode,
      (systemSettings.settings.NotInitialized!=initialized) ||                              // settings and profile are initialized
      (systemSettings.Profile.NotInitialized!=initialized) ||
      (systemSettings.Profile.ID != systemSettings.settings.currentProfile) ||              // Profile ID is the same as the system profile
      (systemSettings.settings.currentProfile>profile_C210)){                               // And it's a known value

      SetFailState(setError);
    }
  }
  checkSettings();                                                                          // Check if settings were modified
  checkIronError();                                                                         // Error detection.
  if( Iron.Error.globalFlag ){                                                            // If any error flag active, stop here
    return;                             // Do nothing else (PWM already disabled)
  }

  // Controls external mode changes (from stand mode changes), this acts as a debouncing timer
  if(Iron.updateMode==needs_update){                                        // If pending mode change
    if((CurrentTime-Iron.LastModeChangeTime)>500){                          // Wait 500mS with steady mode (de-bouncing)
      Iron.updateMode=no_update;                                            // reset flag
      setCurrentMode(Iron.changeMode);                                      // Apply stand mode
    }
  }
  
  // If sleeping, stop here
  if(Iron.CurrentMode==mode_sleep) {
    Iron.Pwm_Out = PWMminOutput;                                            // For safety, update this everytime
    return;
  }
  
  // Controls inactivity timer and enters low power modes
  if(Iron.CurrentMode==mode_run) {
    if((Iron.calibrating==calibration_Off ) && (systemSettings.Profile.sleepTimeout>0) && ((CurrentTime - Iron.CurrentModeTimer)>(uint32_t)systemSettings.Profile.sleepTimeout*60000) ) {
      setCurrentMode(mode_sleep);
    }
  }

  // If there are pending PWM settings to be applied, apply them before new calculation
  if(Iron.updatePwm==needs_update){
    Iron.updatePwm=no_update;
    __HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer,systemSettings.Profile.pwmPeriod);
    __HAL_TIM_SET_AUTORELOAD(Iron.Delay_Timer,systemSettings.Profile.pwmDelay);
    Iron.Pwm_Limit = systemSettings.Profile.pwmPeriod - (systemSettings.Profile.pwmDelay + (uint16_t)ADC_MEASURE_TIME/10);
  }


  updatePowerLimit();                                                       // Update power limit values

  if(Iron.DebugMode==debug_On){                                             // If in debug mode, use debug setpoint value
    Iron.Pwm_Out = calculatePID(Iron.Debug_SetTemperature, TIP.last_avg, Iron.Pwm_Max);
  }
  else{                                                                     // Else, use current setpoint value
    Iron.Pwm_Out = calculatePID(human2adc(Iron.CurrentSetTemperature), TIP.last_avg, Iron.Pwm_Max);
  }
  if(Iron.Pwm_Out<=PWMminOutput){
    Iron.CurrentIronPower = 0;
    Iron.Pwm_Out = PWMminOutput;                                            // Maintain iron detection
  }
  else if(Iron.Pwm_Out == Iron.Pwm_Max){
    Iron.CurrentIronPower = 100;
  }
  else{
    Iron.CurrentIronPower = ((uint32_t)Iron.Pwm_Out*100)/Iron.Pwm_Max;      // Compute new %
  }

  // For calibration process. Add +-2ºC detection margin
  if(  (tipTemp>=(Iron.CurrentSetTemperature-2)) && (tipTemp<=(Iron.CurrentSetTemperature+2)) && !Iron.Cal_TemperatureReachedFlag) {
    temperatureReached( Iron.CurrentSetTemperature);
    Iron.Cal_TemperatureReachedFlag = 1;
  }
  runAwayCheck();                                 // Check runaway condition
}

// Round to closest 10
uint16_t round_10(uint16_t input){
  if((input%10)>5){
    input+=(10-input%10);                         // ex. 640°F=337°C->340°C)
  }
  else{
    input-=input%10;                              // ex. 300°C=572°F->570°F
  }
  return input;
}

// Changes the system temperature unit
void setSystemTempUnit(bool unit){

  if(systemSettings.Profile.tempUnit != unit){
    systemSettings.Profile.tempUnit = unit;
    systemSettings.Profile.UserSetTemperature = round_10(TempConversion(systemSettings.Profile.UserSetTemperature,unit,0));
    systemSettings.Profile.MaxSetTemperature = round_10(TempConversion(systemSettings.Profile.MaxSetTemperature,unit,0));
    systemSettings.Profile.MinSetTemperature = round_10(TempConversion(systemSettings.Profile.MinSetTemperature,unit,0));
  }

  systemSettings.settings.tempUnit = unit;
  Iron.CurrentSetTemperature = systemSettings.Profile.UserSetTemperature;
}

// This function sets the prescaler settings depending on the system, core clock, and loads the stored period
void initTimers(void){
  uint16_t delay, pwm;
  if(systemSettings.settings.currentProfile!=profile_None){
    delay=systemSettings.Profile.pwmDelay;
    pwm=systemSettings.Profile.pwmPeriod;
  }else{
    delay=1999;                                                         // Safe values if profile not initialized
    pwm=19999;
  }
  // Delay timer config
  Iron.Delay_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;        // 10uS input clock
  Iron.Delay_Timer->Init.Period = delay;
  if (HAL_TIM_Base_Init(Iron.Delay_Timer) != HAL_OK){
    Error_Handler();
  }
  // PWM timer config
  Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;        // 10uS input clock
  Iron.Pwm_Timer->Init.Period = pwm;
  if (HAL_TIM_Base_Init(Iron.Pwm_Timer) != HAL_OK){
    Error_Handler();
  }

  __HAL_TIM_CLEAR_FLAG(Iron.Delay_Timer,TIM_FLAG_UPDATE | TIM_FLAG_COM | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 );  // Clear all flags
  __HAL_TIM_ENABLE_IT(Iron.Delay_Timer,TIM_IT_UPDATE);        // Enable Delay timer interrupt

  __HAL_TIM_CLEAR_FLAG(Iron.Pwm_Timer,TIM_FLAG_UPDATE | TIM_FLAG_COM | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 );  // Clear all flags
  #ifdef  PWM_CHx                                                             // Start PWM
    HAL_TIM_PWM_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);                   // PWM output uses CHx channel
  #elif defined PWM_CHxN
    HAL_TIMEx_PWMN_Start_IT(Iron.Pwm_Timer, Iron.Pwm_Channel);                // PWM output uses CHxN channel
  #else
    #error No PWM ouput set (See PWM_CHx / PWM_CHxN in board.h)
  #endif
    __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, 1);               // Set min value into PWM (To enable detection)
    Iron.Pwm_Limit = pwm - (delay + (uint16_t)ADC_MEASURE_TIME/10);
}


// Check iron runaway
void runAwayCheck(void){
  uint16_t TempStep,TempLimit;
  uint32_t CurrentTime = HAL_GetTick();
  uint16_t tipTemp = readTipTemperatureCompensated(stored_reading, read_Avg);

  // If by any means the PWM output is higher than max calculated, generate error
  if(Iron.Pwm_Out > Iron.Pwm_Limit){
    Error_Handler();
  }
  if(systemSettings.settings.tempUnit==mode_Celsius){
    TempStep = 25;
    TempLimit = 500;
  }else{
    TempStep = 45;
    TempLimit = 950;
  }

  if((Iron.Pwm_Out>PWMminOutput) && (Iron.RunawayStatus==runaway_ok)  && (Iron.DebugMode==debug_Off) &&(tipTemp > Iron.CurrentSetTemperature)){

    if(tipTemp>TempLimit){ Iron.RunawayLevel=runaway_500; }        //
    else{
      for(int8_t c=runaway_100; c>=runaway_ok; c--){          // Check temperature diff
        Iron.RunawayLevel=c;
        if(tipTemp > (Iron.CurrentSetTemperature + (TempStep*Iron.RunawayLevel)) ){    // 25ºC steps
          break;                                              // Stop at the highest overrun condition
        }
      }
    }
    if(Iron.RunawayLevel!=runaway_ok){                          // Runaway detected?
      if(Iron.prevRunawayLevel==runaway_ok){                    // First overrun detection?
        Iron.prevRunawayLevel=Iron.RunawayLevel;                // Yes, store in prev level
        Iron.RunawayTimer = CurrentTime;                        // Store time
      }
      else{                                                     // Was already triggered
        switch(Iron.RunawayLevel){
          case runaway_ok:                                      // No problem (<25ºC difference)
            break;                                              // (Never used here)
          case runaway_25:                                      // Temp >25°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>20000){          // 20 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY25);
            }
            break;
          case runaway_50:                                      // Temp >50°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>10000){          // 10 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY50);
            }
            break;
          case runaway_75:                                      // Temp >75°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>3000){           // 3 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY75);
            }
            break;
          case runaway_100:                                     // Temp >100°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>1000){           // 1 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY100);
            }
            break;
          case runaway_500:                                     // Exceed 500ºC!
            if((CurrentTime-Iron.RunawayTimer)>500){            // 0.5 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY500);
            }
            break;
          default:                                              // Unknown overrun state
            Iron.RunawayStatus=runaway_triggered;
            FatalError(error_RUNAWAY_UNKNOWN);
            break;
        }
      }
    }
    return;                                                     // Runaway active, return
  }
  Iron.RunawayTimer = CurrentTime;                              // If no runaway detected, reset values
  Iron.prevRunawayLevel=runaway_ok;
}

// Update PWM max value based on current supply voltage, heater resistance and power limit setting
void updatePowerLimit(void){
  volatile uint32_t volts = getSupplyVoltage_v_x10();                     // Get last voltage reading x10
  volts = (volts*volts)/10;                                               // (Vx10 * Vx10)/10 = (V*V)*10 (x10 for fixed point precision)
  if(volts==0){
    volts=1;                                                              // set minimum value to avoid division by 0
  }
  volatile uint32_t PwmPeriod=systemSettings.Profile.pwmPeriod;           // Read complete PWM period
  volatile uint32_t maxPower = volts/systemSettings.Profile.impedance;    // Compute max power with current voltage and impedance(Impedance stored in x10)
  if(systemSettings.Profile.power >= maxPower){                           // If set power is already higher than the max real power
    Iron.Pwm_Max = Iron.Pwm_Limit;                                        // Set max PWM
  }
  else{                                                                   // Else,
    Iron.Pwm_Max = (PwmPeriod*systemSettings.Profile.power)/maxPower;     // Compute max PWM output for current power limit
    if(Iron.Pwm_Max > Iron.Pwm_Limit){                                    // Ensure it doesn't exceed the limits
      Iron.Pwm_Max = Iron.Pwm_Limit;
    }
  }
}

// Loads the PWM delay
bool setPwmDelay(uint16_t delay){
  if(systemSettings.Profile.pwmPeriod>delay){
    systemSettings.Profile.pwmDelay=delay;
    Iron.updatePwm=needs_update;
    return 0;
  }
  return 1;
}

// Loads the PWM period
bool setPwmPeriod(uint16_t period){
  if(systemSettings.Profile.pwmDelay<period){
    systemSettings.Profile.pwmPeriod=period;
    Iron.updatePwm=needs_update;
    return 0;
  }
  return 1;
}

// Sets no Iron detection threshold
void setNoIronValue(uint16_t noiron){
  systemSettings.Profile.noIronValue=noiron;
}

// Change the iron operating mode in stand mode
void setModefromStand(uint8_t mode){
  if(GetIronError()){ return; }                     // Ignore if error present
  Iron.changeMode = mode;                           // Update mode
  Iron.LastModeChangeTime = HAL_GetTick();          // Reset timer
  Iron.updateMode = needs_update;                   // Set flag
}

// Set the iron operating mode
void setCurrentMode(uint8_t mode){
  Iron.CurrentModeTimer = HAL_GetTick();            // Refresh current mode timer
  if(Iron.CurrentMode != mode){                     // If current mode is different
    if(mode==mode_run){
      resetPID();                                   // Reset PID if returning to run mode
    }
    buzzer_long_beep();
    Iron.CurrentMode = mode;
    Iron.Cal_TemperatureReachedFlag = 0;
    modeChanged(mode);
  }
}

// Called from program timer if WAKE change is detected
void IronWake(bool source){                       // source: handle shake, encoder push button
  if(GetIronError()){ return; }                   // Ignore if error present
  if(Iron.CurrentMode==mode_sleep){
    // If in sleep mode, ignore if wake source disabled
    if( (source==source_wakeButton && !systemSettings.settings.wakeOnButton) || (source==source_wakeInput && !systemSettings.settings.wakeOnShake)){
      return;
    }
  }
  if(source==source_wakeInput){                   // Else, if source wake input
    Iron.newActivity=1;                           // Enable flag for oled pulse icon
    Iron.lastActivityTime = HAL_GetTick();        // Store time for keeping the image on
  }
  setCurrentMode(mode_run);                       // Set run mode. If already in run mode, this will reset the run timer.
}


// Check for changes in system settings.
void checkSettings(void){
  static uint32_t prevSysChecksum=0, newSysChecksum=0, prevTipChecksum=0, newTipChecksum=0, checksumtime=0;
  uint32_t CurrentTime = HAL_GetTick();

  // Don't check if in: Calibration mode, Setup mode, Save delay==0, failure error active
  if( (systemSettings.setupMode==setup_On) || (Iron.calibrating==calibration_On) || (systemSettings.settings.saveSettingsDelay==0) || (Iron.Error.failState!=noError) || (CurrentTime-checksumtime<999)){
    return;
  }
  checksumtime=CurrentTime;                                                                                     // Store current time
  newSysChecksum=ChecksumSettings(&systemSettings.settings);                                                    // Calculate system checksum
  newTipChecksum=ChecksumProfile(&systemSettings.Profile);                                                      // Calculate tip profile checksum
  if((systemSettings.settingsChecksum!=newSysChecksum)||(systemSettings.ProfileChecksum!=newTipChecksum)){      // If anything was changed (Checksum mismatch)
    if((prevSysChecksum!=newSysChecksum)||(prevTipChecksum!=newTipChecksum)){                                   // If different from the previous calculated checksum (settings are being changed quickly, don't save every time).
      prevSysChecksum=newSysChecksum;                                                                           // Store last computed checksum
      prevTipChecksum=newTipChecksum;
      Iron.LastSysChangeTime=CurrentTime;                                                                       // Reset timer (we don't save anything until we pass a certain time without changes)
    }
    else if((CurrentTime-Iron.LastSysChangeTime)>((uint32_t)systemSettings.settings.saveSettingsDelay*1000)){   // If different from the previous calculated checksum, and timer expired (No changes for enough time)
      saveSettings(saveKeepingProfiles);                                                                        // Save settings, this also updates the checksums
    }
  }
}

// Checks for non critical iron errors (Errors that can be cleared)
void checkIronError(void){
  uint32_t CurrentTime = HAL_GetTick();                                       // Get current time
  int16_t ambTemp = readColdJunctionSensorTemp_x10(mode_Celsius);             // Read NTC in Celsius
  IronError_t Err = { 0 };
  Err.failState = Iron.Error.failState;                                       // Get failState flag
  Err.NTC_high = ambTemp > 700 ? 1 : 0;                                       // Check NTC too high (Wrong NTC wiring or overheating, >70ºC)
  Err.NTC_low = ambTemp < -600 ? 1 : 0;                                       // Check NTC too low (If NTC is mounted in the handle, open circuit reports -70ºC or so)
  Err.V_low = getSupplyVoltage_v_x10() < 110 ? 1 : 0;                         // Check supply voltage (Mosfet will not work ok <12V, it will heat up) TODO: maybe set a menu item for this?
  Err.noIron = TIP.last_RawAvg>systemSettings.Profile.noIronValue ? 1 : 0;    // Check tip temperature too high (Wrong connection or not connected)

  if(CurrentTime<1000 || systemSettings.setupMode==setup_On){                 // Don't check sensor errors during first second or in setup mode, wait for readings need to get stable
    Err.Flags &= 0x10;                                                        // Only check failure state
  }
  if(Err.Flags & ErrorMask){                                                  // If there are errors
    Iron.Error.Flags |= Err.Flags;                                            // Update flags
    Iron.LastErrorTime = CurrentTime;                                         // Save time
    if(!Iron.Error.globalFlag){                                               // If first detection
      Iron.Error.globalFlag = 1;                                              // Set global flag
      setCurrentMode(mode_sleep);                                             // Force sleep mode
      Iron.Pwm_Out = PWMminOutput;                                            // Maintain iron detection because it's not critical error
      __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);  // Load now the value into the PWM hardware
      buzzer_alarm_start();                                                   // Start alarm
    }
  }
  else if (Iron.Error.globalFlag && Err.Flags==noError){                      // If global flag set, but there are no errors anymore
    if((CurrentTime-Iron.LastErrorTime)>systemSettings.settings.errorDelay){  // Check enough time has passed
      Iron.Error.Flags = noError;                                             // Reset errors
      buzzer_alarm_stop();                                                    // Stop alarm
      setCurrentMode(mode_run);                                               // Restore run mode
    }
  }
}

// Returns the actual status of the iron error.
bool GetIronError(void){
  return Iron.Error.globalFlag;
}

// Sets Failure state
void SetFailState(bool FailState) {
  if(!FailState && Iron.Error.Flags==0x90){                                   // If only failsafe was active? (This should only happen because it was on first init screen)
    Iron.Error.Flags = 0;                                                     // Reset errors (Ignore timer)
    setCurrentMode(mode_run);                                                 // Resume run mode
  }
  else{
    Iron.Error.failState=FailState;
    checkIronError();                                                         // Update Error
  }
}

// Gets Failure state
bool GetFailState() {
  return Iron.Error.failState;
}


// Sets the debug temperature
void setDebugTemp(uint16_t value) {
  Iron.Debug_SetTemperature = value;
}
// Handles the debug activation/deactivation
void setDebugMode(uint8_t value) {
  Iron.DebugMode = value;
}

// Sets the user temperature
void setSetTemperature(uint16_t temperature) {
  static uint8_t prevProfile=profile_None;
  Iron.Cal_TemperatureReachedFlag = 0;
  if((systemSettings.Profile.UserSetTemperature != temperature)||(prevProfile!=systemSettings.settings.currentProfile)){
    prevProfile=systemSettings.settings.currentProfile;
    systemSettings.Profile.UserSetTemperature = temperature;
    Iron.CurrentSetTemperature=temperature;
    resetPID();
  }
}

// Returns the actual set temperature
uint16_t getSetTemperature() {
  return Iron.CurrentSetTemperature;
}

// Returns the actual working mode of the iron
uint8_t getCurrentMode() {
  return Iron.CurrentMode;
}

// Returns the output power
int8_t getCurrentPower() {
  return Iron.CurrentIronPower;
}

// Adds a callback to be called when the set temperature is reached
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback) {
  setTemperatureReachedCallbackStruct_t *s = malloc(sizeof(setTemperatureReachedCallbackStruct_t));
  if(!s){
    Error_Handler();
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
  if(!s){
    Error_Handler();
  }
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
