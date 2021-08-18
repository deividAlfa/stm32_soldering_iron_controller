/*
 * iron.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "iron.h"
#include "buzzer.h"
#include "settings.h"
#include "main.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "ssd1306.h"
#include "screens.h"
#include "oled.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "iron.c"
#endif

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
  Iron.Read_Timer    = delaytimer;
  Iron.Pwm_Channel    = pwmchannel;
  Iron.Error.Flags    = _NOERROR;

  if(systemSettings.settings.WakeInputMode == mode_shake){
    setCurrentMode(systemSettings.settings.initMode);
  }
  else{
    if(WAKE_input()){
      setCurrentMode(mode_run);
    }
    else{
      setCurrentMode(systemSettings.settings.StandMode);
    }
  }
  initTimers();
  detectNTC();
}

void handleIron(void) {
  uint32_t CurrentTime = HAL_GetTick();

  readTipTemperatureCompensated(new_reading, read_average);     // Update readings
  readColdJunctionSensorTemp_x10(new_reading, mode_Celsius);

  if(!Iron.Error.safeMode){
    if( (systemSettings.setupMode==enable) || systemSettings.settings.NotInitialized || systemSettings.Profile.NotInitialized!=initialized ||
        (systemSettings.Profile.ID != systemSettings.settings.currentProfile) || (systemSettings.settings.currentProfile>profile_C210)){

      Iron.Error.safeMode=1;
    }
  }

  checkIronError();

  // Controls external mode changes (from stand mode changes), this acts as a debouncing timer
  if(Iron.updateStandMode==needs_update){
    if(Iron.Error.active){
      Iron.updateStandMode=no_update;
    }
    else if((CurrentTime-Iron.LastModeChangeTime)>100){                             // Wait 100mS with no changes (de-bouncing)
      Iron.updateStandMode=no_update;
      setCurrentMode(Iron.changeMode);
    }
  }

  // If sleeping or error, stop here
  if(Iron.CurrentMode==mode_sleep || Iron.Error.active) {                           // For safety, force PWM low everytime
    Iron.Pwm_Out=0;
    __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, 0);
    Iron.CurrentIronPower=0;
    return;
  }
  

  // Controls inactivity timer and enters low power modes
  uint32_t mode_time = CurrentTime - Iron.CurrentModeTimer;
  uint32_t boost_time = (uint32_t)systemSettings.Profile.boostTimeout*1000;
  uint32_t sleep_time = (uint32_t)systemSettings.Profile.sleepTimeout*60000;
  uint32_t standby_time = (uint32_t)systemSettings.Profile.standbyTimeout*60000;

  if(Iron.calibrating==disable){                                                      // Don't enter low power states while calibrating. Calibration always forces run mode
    if((Iron.CurrentMode==mode_boost) && (mode_time>boost_time)){                             // If boost mode and time expired
      setCurrentMode(mode_run);
    }
    else if(Iron.CurrentMode==mode_run){                                                      // If running
      if(standby_time){                                                                       // If standby timer enabled
        if(mode_time>standby_time){                                                           // Check timeout
          setCurrentMode(mode_standby);
        }
      }
      else{                                                                                   // Otherwise, check sleep timeout
        if(mode_time>sleep_time){                                                             //
          setCurrentMode(mode_sleep);
        }
      }
    }
    else if(Iron.CurrentMode==mode_standby){                                                  // If in standby
      if(mode_time>sleep_time){                                                               // Check sleep timeout
        setCurrentMode(mode_sleep);
      }
    }
  }


  if(Iron.updatePwm){
    Iron.Pwm_Period = ((systemSettings.Profile.readPeriod+1)/systemSettings.Profile.pwmMul)-1;
  }

  #ifdef USE_VIN
  updatePowerLimit();                                                                         // Update power limit values
  #endif

  // Update PID
  volatile uint16_t PID_temp;
  if(Iron.DebugMode==enable){                                                               // If in debug mode, use debug setpoint value
    Iron.Pwm_Out = calculatePID(Iron.Debug_SetTemperature, TIP.last_avg, Iron.Pwm_Max);
  }
  else{                                                                                       // Else, use current setpoint value
    PID_temp = (human2adc(Iron.CurrentSetTemperature) + human2adc(Iron.CurrentSetTemperature+1))/2;   // +0.5C for better display stability
    Iron.Pwm_Out = calculatePID(PID_temp, TIP.last_avg, Iron.Pwm_Max);
  }

  if(!Iron.Pwm_Out){
    Iron.CurrentIronPower = 0;
  }
  else if(Iron.Pwm_Out == Iron.Pwm_Max){
    Iron.CurrentIronPower = 100;
  }
  else if(Iron.Pwm_Out < Iron.Pwm_Max){
    Iron.CurrentIronPower = ((uint32_t)Iron.Pwm_Out*100)/Iron.Pwm_Max;                        // Compute new %
  }
  else{
    Error_Handler();
  }
  if(Iron.updatePwm){
    Iron.updatePwm=0;
    __HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer, Iron.Pwm_Period);
  }
  __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);                      // Load new calculated PWM Duty

  // For calibration process. Add +-2ºC detection margin
  if( !Iron.temperatureReached && (last_TIP>=(Iron.CurrentSetTemperature-2)) && (last_TIP<=(Iron.CurrentSetTemperature+2))) {
    temperatureReached( Iron.CurrentSetTemperature);
    Iron.temperatureReached = 1;
  }
}

// Round to closest 10
uint16_t round_10(uint16_t input){
  if((input%10)>5){
    input+=(10-input%10);                                                                     // ex. 640°F=337°C->340°C)
  }
  else{
    input-=input%10;                                                                          // ex. 300°C=572°F->570°F
  }
  return input;
}

// Changes the system temperature unit
void setSystemTempUnit(bool unit){

  if(systemSettings.Profile.tempUnit != unit){
    systemSettings.Profile.tempUnit = unit;
    systemSettings.Profile.UserSetTemperature = round_10(TempConversion(systemSettings.Profile.UserSetTemperature,unit,0));
    systemSettings.Profile.standbyTemperature = round_10(TempConversion(systemSettings.Profile.standbyTemperature,unit,0));
    systemSettings.Profile.MaxSetTemperature = round_10(TempConversion(systemSettings.Profile.MaxSetTemperature,unit,0));
    systemSettings.Profile.MinSetTemperature = round_10(TempConversion(systemSettings.Profile.MinSetTemperature,unit,0));
  }

  systemSettings.settings.tempUnit = unit;
  setCurrentMode(Iron.CurrentMode);     // Reload temps
}

// This function inits the timers and sets the prescaler settings depending on the system core clock
// The final PWM settings are applied by LoadProfile
void initTimers(void){
  // Delay timer config
  #ifdef READ_TIMER_HALFCLOCK
  Iron.Read_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;                               // 5uS input clock
  #else
  Iron.Read_Timer->Init.Prescaler = (SystemCoreClock/200000)-1;
  #endif

  Iron.Read_Timer->Init.Period = systemSettings.Profile.readPeriod-(systemSettings.Profile.readDelay+1);
  if (HAL_TIM_Base_Init(Iron.Read_Timer) != HAL_OK){
    Error_Handler();
  }

  #ifdef PWM_TIMER_HALFCLOCK
  Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;                               // 5uS input clock
  #else
  Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/200000)-1;
  #endif
  Iron.Pwm_Period = ((systemSettings.Profile.readPeriod+1)/ systemSettings.Profile.pwmMul)-1;
  Iron.Pwm_Timer->Init.Period = Iron.Pwm_Period;
  if (HAL_TIM_Base_Init(Iron.Pwm_Timer) != HAL_OK){
    Error_Handler();
  }


  Iron.Pwm_Out = 0;

  __HAL_TIM_SET_COUNTER(Iron.Read_Timer,0);
  __HAL_TIM_SET_COUNTER(Iron.Pwm_Timer,0);
  __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);                      // Set min value into PWM

  __HAL_TIM_CLEAR_FLAG(Iron.Read_Timer,TIM_FLAG_UPDATE | TIM_FLAG_COM | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 );
  HAL_TIM_Base_Start_IT(Iron.Read_Timer);                                                     // Start Read Timer

  #ifdef  PWM_CHx
  HAL_TIM_PWM_Start(Iron.Pwm_Timer, Iron.Pwm_Channel);                                        // Start PWM, output uses CHx channel
  #elif defined PWM_CHxN
  HAL_TIMEx_PWMN_Start(Iron.Pwm_Timer, Iron.Pwm_Channel);                                     // Start PWM, output uses CHxN channel
  #else
  #error No PWM ouput set (See PWM_CHx / PWM_CHxN in board.h)
  #endif


}

void setReadDelay(uint16_t delay){
 systemSettings.Profile.readDelay=delay;
}


void setReadPeriod(uint16_t period){
 systemSettings.Profile.readPeriod=period;
 Iron.updatePwm=1;
}

void setPwmMul(uint16_t mult){
  systemSettings.Profile.pwmMul=mult;
  Iron.updatePwm=1;
}

void configurePWMpin(uint8_t mode){
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(mode==output_PWM){
    GPIO_InitStruct.Mode =  GPIO_MODE_AF_PP;
  }
  else if(mode==output_Low){
    PWM_GPIO_Port->BSRR = PWM_Pin<<16;
    GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
  }
  else if(mode==output_High){
    PWM_GPIO_Port->BSRR = PWM_Pin;
    GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
  }

  #ifdef PWM_ALT_PIN
  GPIO_InitStruct.Alternate = PWM_ALT_PIN;
  #endif

  GPIO_InitStruct.Pin =   PWM_Pin;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWM_GPIO_Port, &GPIO_InitStruct);
}

// Check iron runaway
void runAwayCheck(void){
  uint16_t TempStep,TempLimit;
  uint32_t CurrentTime = HAL_GetTick();
  uint16_t tipTemp = readTipTemperatureCompensated(old_reading, read_average);
  static uint8_t pos,prev_power[4];
  uint8_t power;

  if(systemSettings.setupMode==enable || (Iron.Error.safeMode && Iron.Error.active)){
    return;
  }
  prev_power[pos]=Iron.CurrentIronPower;                                                      // Circular buffer
  if(++pos>3){ pos=0; }
  power = ((uint16_t)prev_power[0]+prev_power[1]+prev_power[2]+prev_power[3])/4;              // Average of last 4 powers

  // If by any means the PWM output is higher than max calculated, generate error
  if((Iron.Pwm_Out > (Iron.Pwm_Period+1)) || (Iron.Pwm_Out != __HAL_TIM_GET_COMPARE(Iron.Pwm_Timer,Iron.Pwm_Channel))){
    Error_Handler();
  }
  if(systemSettings.settings.tempUnit==mode_Celsius){
    TempStep = 25;
    TempLimit = 500;
  }else{
    TempStep = 45;
    TempLimit = 930;
  }

  if(power>1 && (Iron.RunawayStatus==runaway_ok)  && (Iron.DebugMode==disable) &&(tipTemp > Iron.CurrentSetTemperature)){

    if(tipTemp>TempLimit){ Iron.RunawayLevel=runaway_500; }
    else{
      for(int8_t c=runaway_100; c>=runaway_ok; c--){                                        // Check temperature diff
        Iron.RunawayLevel=c;
        if(tipTemp > (Iron.CurrentSetTemperature + (TempStep*Iron.RunawayLevel)) ){         // 25ºC steps
          break;                                                                            // Stop at the highest overrun condition
        }
      }
    }
    if(Iron.RunawayLevel!=runaway_ok){                                                      // Runaway detected?
      if(Iron.prevRunawayLevel==runaway_ok){                                                // First overrun detection?
        Iron.prevRunawayLevel=Iron.RunawayLevel;                                            // Yes, store in prev level
        Iron.RunawayTimer = CurrentTime;                                                    // Store time
      }
      else{                                                                                 // Was already triggered
        switch(Iron.RunawayLevel){
          case runaway_ok:                                                                  // No problem (<25ºC difference)
            break;                                                                          // (Never used here)
          case runaway_25:                                                                  // Temp >25°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>20000){                                      // 20 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY25);
            }
            break;
          case runaway_50:                                                                  // Temp >50°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>10000){                                      // 10 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY50);
            }
            break;
          case runaway_75:                                                                  // Temp >75°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>5000){                                       // 5 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY75);
            }
            break;
          case runaway_100:                                                                 // Temp >100°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>2000){                                       // 2 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY100);
            }
            break;
          case runaway_500:                                                                 // Exceed 500ºC!
            if((CurrentTime-Iron.RunawayTimer)>2000){                                       // 2 second limit
              Iron.RunawayStatus=runaway_triggered;
              FatalError(error_RUNAWAY500);
            }
            break;
          default:                                                                          // Unknown overrun state
            Iron.RunawayStatus=runaway_triggered;
            FatalError(error_RUNAWAY_UNKNOWN);
            break;
        }
      }
    }
    return;                                                                                 // Runaway active, return
  }
  Iron.RunawayTimer = CurrentTime;                                                          // If no runaway detected, reset values
  Iron.prevRunawayLevel=runaway_ok;
}

// Update PWM max value based on current supply voltage, heater resistance and power limit setting
#ifdef USE_VIN
void updatePowerLimit(void){
  uint32_t volts = getSupplyVoltage_v_x10();                                                // Get last voltage reading x10
  volts = (volts*volts)/10;                                                                 // (Vx10 * Vx10)/10 = (V*V)*10 (x10 for fixed point precision)
  if(volts==0){
    volts=1;                                                                                // set minimum value to avoid division by 0
  }
  uint32_t PwmPeriod=Iron.Pwm_Period+1;                                                       // Read complete PWM period
  uint32_t maxPower = volts/systemSettings.Profile.impedance;                               // Compute max power with current voltage and impedance(Impedance stored in x10)
  if(systemSettings.Profile.power >= maxPower){                                             // If set power is already higher than the max possible power given the voltage and heater resistance
    Iron.Pwm_Max = PwmPeriod;                                                               // Set max PWM
  }
  else{                                                                                     // Else,
    Iron.Pwm_Max = (PwmPeriod*systemSettings.Profile.power)/maxPower;                       // Compute max PWM output for current power limit
    if(Iron.Pwm_Period > PwmPeriod){
      Iron.Pwm_Max = PwmPeriod;
    }
    else if(Iron.Pwm_Period==0){
      Iron.Pwm_Max = 1;
    }
  }
}
#endif

// Sets no Iron detection threshold
void setNoIronValue(uint16_t noiron){
  systemSettings.Profile.noIronValue=noiron;
}

// Change the iron operating mode in stand mode
void setModefromStand(uint8_t mode){
  if( GetIronError() ||
      ((Iron.changeMode==mode) && (Iron.CurrentMode==mode)) ||
      ((Iron.CurrentMode==mode_sleep) && (mode==mode_standby)) ||
      ((Iron.CurrentMode==mode_boost) && (mode==mode_run)) ){
    return;
  }
  if(Iron.changeMode!=mode){
    Iron.changeMode = mode;                                                                 // Update mode
    Iron.LastModeChangeTime = HAL_GetTick();                                                // Reset debounce timer
  }
  Iron.updateStandMode = needs_update;                                                           // Set flag
}

// Set the iron operating mode
void setCurrentMode(uint8_t mode){
  Iron.CurrentModeTimer = HAL_GetTick();                                                    // Refresh current mode timer
  if(mode==mode_standby){
    Iron.CurrentSetTemperature = systemSettings.Profile.standbyTemperature;                 // Set standby temp
  }
  else if(mode==mode_boost){
    Iron.CurrentSetTemperature = systemSettings.Profile.UserSetTemperature+systemSettings.Profile.boostTemperature;
    if(Iron.CurrentSetTemperature>systemSettings.Profile.MaxSetTemperature){
      Iron.CurrentSetTemperature=systemSettings.Profile.MaxSetTemperature;
    }
  }
  else{
    Iron.CurrentSetTemperature = systemSettings.Profile.UserSetTemperature;                 // Set user temp (sleep mode ignores this)
  }
  if(Iron.CurrentMode != mode){                                                             // If current mode is different
    resetPID();
    buzzer_long_beep();
    Iron.CurrentMode = mode;
    modeChanged(mode);
    if(Iron.CurrentMode == mode_run){
      Iron.temperatureReached = 0;
    }
  }
}

// Called from program timer if WAKE change is detected
void IronWake(bool source){                                                                 // source: handle shake, encoder push button
  if(Iron.Error.Flags || systemSettings.settings.WakeInputMode==mode_stand){
    return;
  }

  if(Iron.CurrentMode==mode_standby){
    if( (source==wakeButton && !(systemSettings.settings.buttonWakeMode & wake_standby)) ||
        (source==wakeInput && !(systemSettings.settings.shakeWakeMode & wake_standby))){

      return;
    }
  }
  else if(Iron.CurrentMode==mode_sleep){
    if( (source==wakeButton && !(systemSettings.settings.buttonWakeMode & wake_sleep)) ||
        (source==wakeInput && !(systemSettings.settings.shakeWakeMode & wake_sleep))){

      return;
    }
  }
  if(Iron.CurrentMode<mode_boost){
    setCurrentMode(mode_run);
  }
}

void readWake(void){
  static bool last_wake=0;
    bool now_wake = WAKE_input();

    if(last_wake!=now_wake){                                            // If wake sensor input changed
      last_wake=now_wake;
      if(systemSettings.settings.WakeInputMode==mode_stand){   // In stand mode
        if(now_wake){
          setModefromStand(mode_run);
        }
        else{
          setModefromStand(systemSettings.settings.StandMode);          // Set sleep or standby mode depending on system setting
        }
      }
      else{
        IronWake(wakeInput);
        Iron.shakeActive = 1;
        Iron.lastShakeTime = HAL_GetTick();
      }
    }
}


// Checks for non critical iron errors (Errors that can be cleared)
void checkIronError(void){
  uint32_t CurrentTime = HAL_GetTick();
  int16_t ambTemp = readColdJunctionSensorTemp_x10(old_reading, mode_Celsius);
  IronError_t Err = { 0 };
  Err.safeMode = Iron.Error.safeMode;
  Err.NTC_high = (ambTemp > 800);
  Err.NTC_low = (ambTemp < -200);
  #ifdef USE_VIN
  Err.V_low = (getSupplyVoltage_v_x10() < systemSettings.settings.lvp);
  #endif
  Err.noIron = (TIP.last_raw>systemSettings.Profile.noIronValue);

  if(current_screen==&Screen_boot || CurrentTime<1000){                               // Don't check sensor errors during first second or in boot screen, wait for readings need to get stable
    Err.Flags &= _SAFE_MODE;
  }

  if(Err.Flags){
    Iron.Error.Flags = Err.Flags | (Iron.Error.Flags & _ACTIVE);
    Iron.LastErrorTime = CurrentTime;
    if(!Iron.Error.active){
      if(Err.Flags!=_NO_IRON){                                                        // Avoid alarm if only the tip is removed
        buzzer_alarm_start();
      }
      Iron.Error.active = 1;
      setCurrentMode(mode_sleep);
      Iron.Pwm_Out = 0;
      __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, 0);
      configurePWMpin(output_Low);
    }
  }
  else if (Iron.Error.active && !Err.Flags){                                                // If global flag set, but no errors
    if((CurrentTime-Iron.LastErrorTime)>(systemSettings.settings.errorDelay*100)){                // Check enough time has passed
      Iron.Error.Flags = 0;
      buzzer_alarm_stop();
      setCurrentMode(mode_run);
    }
  }
  else{
    Iron.Error.Flags=_NOERROR;
  }
}


bool GetIronError(void){
  return Iron.Error.Flags;
}

void setSafeMode(bool mode){
  if(mode==disable && Iron.Error.Flags==(_ACTIVE |_SAFE_MODE)){                             // If only failsafe was active? (This should only happen because it was on first init screen)
    Iron.Error.Flags = _NOERROR;
    setCurrentMode(mode_run);
  }
  else{
    if(mode==enable){
      configurePWMpin(output_Low);
    }
    Iron.Error.safeMode=mode;
    checkIronError();
  }
}


bool GetSafeMode() {
  return(Iron.Error.safeMode && Iron.Error.active);
}

void setDebugTemp(uint16_t value) {
  Iron.Debug_SetTemperature = value;
}

uint16_t getDebugTemp(void){
  return Iron.Debug_SetTemperature;
}

void setDebugMode(uint8_t value) {
  Iron.DebugMode = value;
}
uint8_t getDebugMode(void){
  return Iron.DebugMode;
}

void setUserTemperature(uint16_t temperature) {
  Iron.temperatureReached = 0;
  if(systemSettings.Profile.UserSetTemperature != temperature){
    systemSettings.Profile.UserSetTemperature = temperature;
    if(Iron.CurrentMode==mode_run){
      Iron.CurrentSetTemperature=temperature;
      resetPID();
    }
  }
}

uint16_t getUserTemperature() {
  return systemSettings.Profile.UserSetTemperature;
}

uint8_t getCurrentMode() {
  return Iron.CurrentMode;
}

int8_t getCurrentPower() {
  return Iron.CurrentIronPower;
}

uint16_t getSetTemperature(){
  return Iron.CurrentSetTemperature;
}

void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback) {
  setTemperatureReachedCallbackStruct_t *s = _malloc(sizeof(setTemperatureReachedCallbackStruct_t));
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
  currentModeChangedCallbackStruct_t *s = _malloc(sizeof(currentModeChangedCallbackStruct_t));
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
