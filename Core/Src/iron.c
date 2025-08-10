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
#include "display.h"
#include "screens.h"
#include "oled.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#endif
#define __BASE_FILE__ "iron.c"

typedef struct {
  uint8_t             Pwm_Channel;                          // PWM channel
  uint8_t             CurrentIronPower;                     // Last output power
  uint8_t             CurrentMode;                          // Actual working mode (Standby, Sleep, Normal, Boost)
  uint8_t             changeMode;                           // change working mode to (Standby, Sleep, Normal, Boost)
  uint8_t             resetRunawayHistory;                  // Flag to indicate it must reset the history (When temp is changed)
  uint8_t             RunawayLevel;                         // Runaway actual level
  uint8_t             prevRunawayLevel;                     // Runaway previous level
  uint8_t             RunawayStatus;                        // Runaway triggered flag
  uint8_t             calibrating;                          // Flag to indicate calibration state (don't save temperature settings)
  uint8_t             standMode_update;                     // Flag to indicate the stand mode must be changed
  uint8_t             standMode_beepDone;                   // Flag to indicate the stand change already beeped
  uint8_t             tipchange;                            // Flag to indicate we're changing the tip
  uint8_t             shakeActive;                          // Flag to indicate handle movement
  uint8_t             temperatureReached;                   // Flag for temperature calibration
  uint8_t             updatePwm;                            // Flag to indicate PWM need to be updated
  uint8_t             lastWakeSrc;                          // Flag to indicate last wake source
  IronError_t         Error;                                // Error flags
  uint8_t             lastMode;                             // Last mode before error condition.
  uint8_t             boot_complete;                        // Flag set to 1 when boot screen exits (Used for error handling)
  uint8_t             boot_loaded;                          // Flag set to 1 when the initial boot profile was loaded
  uint8_t             err_resumed;                          // Flag set to 1 when the iron was resumed after a system error
  uint8_t             Load_det_pos;                         // For load detection
  uint8_t             waiting_conversion;                   // USeful if you want to wait somwhere until the the ADC cycle is completed (Set by user, clear after conversion)

  uint16_t            Pwm_Period;                           // PWM period
  uint16_t            Pwm_Max;                              // Max PWM output for power limit
  int16_t             UserSetTemperature;                   // Run mode user setpoint
  int16_t             TargetTemperature;                    // Actual set (target) temperature (Setpoint)

  uint32_t            Load_det_value;                       // For load detection
  uint32_t            Load_det_bf[SMARTACTIVE_BFSZ];        // For load detection
  uint32_t            Load_det_Time;                        // For load detection
  uint32_t            Pwm_Out;                              // Last calculated PWM value
  uint32_t            LastModeChangeTime;                   // Last time the mode was changed (To provide debouncing)
  uint32_t            LastErrorTime;                        // last time iron error was detected
  uint32_t            lastShakeTime;                        // last time iron handle was moved (In shake mode)
  uint32_t            CurrentModeTimer;                     // Time since actual mode was set
  uint32_t            RunawayTimer;                         // Runaway timer

  TIM_HandleTypeDef*  Read_Timer;                          // Pointer to the Read timer
  TIM_HandleTypeDef*  Pwm_Timer;                           // Pointer to the PWM timer
}iron_t;

static volatile uint32_t CurrentTime;
static volatile iron_t Iron;

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
  Iron.Read_Timer     = delaytimer;
  Iron.Pwm_Channel    = pwmchannel;
  Iron.Error.Flags    = FLAG_NOERROR;

  if(getProfileSettings()->WakeInputMode == mode_shake){
    setCurrentMode(getSystemSettings()->initMode, MLONG_BEEP);
  }
  else{
    if(WAKE_input()){
      setCurrentMode(mode_run, MLONG_BEEP);
    }
    else{
      setCurrentMode(getProfileSettings()->StandMode, MLONG_BEEP);
    }
  }
  initTimers();
  #ifdef USE_NTC
  detectNTC();
  #endif
}

void handleIron(void) {
  static uint32_t reachedTime = 0;
  CurrentTime = HAL_GetTick();
  if(!Iron.Error.safeMode){
    if( (getSettings()->setupMode==enable) || getSystemSettings()->version!=SYSTEM_SETTINGS_VERSION || getProfileSettings()->version!=PROFILE_SETTINGS_VERSION ||
        (getProfileSettings()->ID != getCurrentProfile()) || (getCurrentProfile()>=NUM_PROFILES)){

      setIronSafeMode(enable);
    }
  }

  checkIronError();                                                           // Check iron error presence, must be called before coldJuctionSensor
  readColdJunctionSensorTemp_x10(new_reading, mode_Celsius);                  // to reset the NTC detection status
  readNewTipTemperatureCompensated(read_average, mode_Celsius);               // Update readings
  checkIronError();                                                           // Check iron error again (Special case when iron has been detected, show other errors)

  // Controls external mode changes (from stand mode changes), this acts as a debouncing timer
  if(Iron.standMode_update==needs_update){
    if(Iron.Error.active || Iron.calibrating){                                      // Ignore changes when error active or calibrating
      Iron.standMode_update=no_update;
    }
    else{
      if( (Iron.changeMode < mode_run) && (Iron.changeMode>=Iron.CurrentMode) ){    // Received a sleep/standby request from the handle while already in sleep or standby, probably noise, ignore
        Iron.standMode_update = no_update;
        Iron.lastWakeSrc = wakeSrc_Stand;
      }
      else{
                      // Delay of 500ms when removing the handle, "standDelay" when placing the handle.
        uint32_t delay = (Iron.changeMode < mode_run) ? (getProfileSettings()->standDelay ? 1000UL*getProfileSettings()->standDelay : 500 ) : 500;
        uint32_t elapsed = CurrentTime-Iron.LastModeChangeTime;

        if (!Iron.standMode_beepDone && (elapsed > 500) ){                   // Apply a small delay of 500ms for the beep
          Iron.standMode_beepDone = 1;
          buzzer_beep(MLONG_BEEP);
        }
        if(elapsed > delay){
          Iron.standMode_beepDone = 0;
          Iron.standMode_update = no_update;
          Iron.lastWakeSrc = wakeSrc_Stand;
        }
      }
    }
  }

  Iron.waiting_conversion = 0;

  // If sleeping or error, stop here
  if(!Iron.boot_complete || Iron.CurrentMode==mode_sleep || Iron.Error.active) {                // For safety, force PWM low everytime
    Iron.Pwm_Out=0;
    __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, 0);
    Iron.CurrentIronPower=0;
    return;
  }

  // Controls inactivity timer and enters low power modes
  if(!Iron.calibrating){                                                                        // Don't check timeout when calibrating
    uint32_t mode_time = CurrentTime - Iron.CurrentModeTimer;

    // Smart wake detection only in active modes, after 10 seconds of entering the current mode
    if(getProfileSettings()->smartActiveEnabled==enable && (Iron.CurrentMode>mode_standby) && (mode_time>9999)){
      if(Iron.Load_det_value > getProfileSettings()->smartActiveLoad){                       // Check threshold
        Iron.CurrentModeTimer = CurrentTime-10000;                                            // Preload the timeout with 10 seconds so smartActiv keeps working
        Iron.lastWakeSrc = wakeSrc_Smart;
        Iron.lastShakeTime = CurrentTime;
        Iron.shakeActive = 1;                                                                 // Show wake icon
        Iron.Load_det_value = 0;
        for(uint8_t i=0; i<SMARTACTIVE_BFSZ; i++)
          Iron.Load_det_bf[i] = 0;                                                            // Clear buffer
      }
      else if(Iron.lastWakeSrc==wakeSrc_Smart && !Iron.shakeActive && mode_time<10600){      // Keep blinking the shake icon for some time after smartActive was triggered
        Iron.lastShakeTime = CurrentTime;
        Iron.shakeActive = 1;
      }

    }
    if((Iron.CurrentMode==mode_coldboost) && (mode_time>getProfileSettings()->coldBoostTimeout)){                              // If cold boost mode and time expired
      setCurrentMode(mode_run, MLONG_BEEP);
    }
    else if((Iron.CurrentMode==mode_boost) && (mode_time>getProfileSettings()->boostTimeout)){    // If boost mode and time expired
      setCurrentMode(mode_run, MLONG_BEEP);
    }
    else if(Iron.CurrentMode==mode_run){                                                      // If running
      if(getProfileSettings()->standbyTimeout){                                              // If standby timer enabled
        if(mode_time>getProfileSettings()->standbyTimeout){                                  // Check timeout
          setCurrentMode(mode_standby, MLONG_BEEP);
        }
      }
      else{                                                                                   // Otherwise, check sleep timeout
        if(mode_time>getProfileSettings()->sleepTimeout){                                    //
          setCurrentMode(mode_sleep, MLONG_BEEP);
        }
      }
    }
    else if(Iron.CurrentMode==mode_standby){                                                  // If in standby
      if(mode_time>getProfileSettings()->sleepTimeout){                                      // Check sleep timeout
        setCurrentMode(mode_sleep, MLONG_BEEP);
      }
    }
  }


  if(Iron.updatePwm){                                                                         // If pending PWM period update, refresh Iron Pwm_period
    Iron.Pwm_Period = ((getProfileSettings()->readPeriod+1)/getProfileSettings()->pwmMul)-1;
  }
#ifdef USE_VIN
  updatePowerLimit();                                                                         // Update power limit values
#endif
  // Update PID
  int32_t target = human2adc(Iron.TargetTemperature);
  Iron.Pwm_Out = calculatePID(target, TIP.last_avg, Iron.Pwm_Max);

  if((getProfileSettings()->smartActiveEnabled==enable) && ((CurrentTime - Iron.Load_det_Time)>199)){   // Sample load every 200ms
    Iron.Load_det_Time = CurrentTime;
    int32_t err = target - TIP.last_avg;
    Iron.Load_det_bf[Iron.Load_det_pos] = (err<0 ? 0 : err);    // Taking only the positive errors remove PID overshooting negative errors
    if(++Iron.Load_det_pos>(SMARTACTIVE_BFSZ-1))                // Makes detection more sensible
      Iron.Load_det_pos=0;
    Iron.Load_det_value=0;
    for(uint8_t i=0; i<SMARTACTIVE_BFSZ; i++)
      Iron.Load_det_value += Iron.Load_det_bf[i];
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
  else{                                                                                       // Iron.Pwm_Out should never be greater than Iron.Pwm_Max
    Error_Handler();
  }

  if(Iron.updatePwm){                                                                         // If pending PWM period update, refresh timer
    Iron.updatePwm=0;
    __HAL_TIM_SET_AUTORELOAD(Iron.Pwm_Timer, Iron.Pwm_Period);
  }
  __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, Iron.Pwm_Out);                      // Load new calculated PWM Duty

  // For calibration process. Add +-5ºC detection margin
  int16_t setTemp = Iron.TargetTemperature;
  if(getSystemTempUnit()==mode_Farenheit){
    setTemp = TempConversion(setTemp, mode_Celsius, 0);
  }
  if( !Iron.temperatureReached && abs(setTemp-last_TIP_C)<5){                                 // Allow +-5° margin for noisier stations
    if(reachedTime==0){
      reachedTime=CurrentTime;
    }
    else if(CurrentTime-reachedTime>1000){                                                    // Wait 1s for stable readings
      Iron.temperatureReached = 1;
      if(Iron.CurrentMode > mode_standby){                                                    // Beep when reaching target temperature, but only in run & boost modes
        temperatureReached( Iron.TargetTemperature);
      }
    }
  }
  else{
    reachedTime = 0;
  }
}

// Round to closest 10
uint16_t round_10(uint16_t input){
  if((input%10)>5){
    input+=(10-input%10);                                                                 // ex. 640°F=337°C->340°C)
  }
  else{
    input-=input%10;                                                                      // ex. 300°C=572°F->570°F
  }
  return input;
}

// Changes the system temperature unit
void setSystemTempUnit(bool unit){

  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  if(getProfileSettings()->tempUnit != unit){
    getProfileSettings()->tempUnit = unit;
    Iron.UserSetTemperature = round_10(TempConversion(Iron.UserSetTemperature,unit,0));                                         // User temp is loaded from the profile, thus it uses the same unit
    getProfileSettings()->standbyTemperature = round_10(TempConversion(getProfileSettings()->standbyTemperature,unit,0));
    getProfileSettings()->MaxSetTemperature = round_10(TempConversion(getProfileSettings()->MaxSetTemperature,unit,0));
    getProfileSettings()->MinSetTemperature = round_10(TempConversion(getProfileSettings()->MinSetTemperature,unit,0));
    getProfileSettings()->boostTemperature = round_10(TempIncrementConversion(getProfileSettings()->boostTemperature,unit));
    getProfileSettings()->coldBoostTemperature = round_10(TempIncrementConversion(getProfileSettings()->coldBoostTemperature,unit));
  }
  if(getSystemSettings()->tempUnit != unit){
    getSystemSettings()->tempUnit = unit;
  }
  __set_PRIMASK(_irq);
  setCurrentMode(Iron.CurrentMode, 0);     // Reload temps, no beeping
}
bool getSystemTempUnit(void){
  return getSystemSettings()->tempUnit;
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

  Iron.Read_Timer->Init.Period = getProfileSettings()->readPeriod-(getProfileSettings()->readDelay+1);
  if (HAL_TIM_Base_Init(Iron.Read_Timer) != HAL_OK){
    Error_Handler();
  }

  #ifdef PWM_TIMER_HALFCLOCK
  Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/100000)-1;                               // 5uS input clock
  #else
  Iron.Pwm_Timer->Init.Prescaler = (SystemCoreClock/200000)-1;
  #endif
  Iron.Pwm_Period = ((getProfileSettings()->readPeriod+1)/ getProfileSettings()->pwmMul)-1;
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
 getProfileSettings()->readDelay=delay;
}


void setReadPeriod(uint16_t period){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
 getProfileSettings()->readPeriod=period;
 Iron.updatePwm=1;
  __set_PRIMASK(_irq);
}

void setPwmMul(uint16_t mult){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  getProfileSettings()->pwmMul=mult;
  Iron.updatePwm=1;
  __set_PRIMASK(_irq);
}

void configurePWMpin(uint8_t mode){
  GPIO_InitTypeDef GPIO_InitStruct = {0};
#ifndef DISABLE_OUTPUT
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
#else                                                                 // Output disabled with #define switch, set always low.
  PWM_GPIO_Port->BSRR = PWM_Pin<<16;
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
#endif
  GPIO_InitStruct.Pin =   PWM_Pin;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWM_GPIO_Port, &GPIO_InitStruct);
}


void resetRunAway(void){
#if defined RUNAWAY_RESET_CYCLES && RUNAWAY_RESET_CYCLES>0
  Iron.resetRunawayHistory=1;
#endif
}

void runAwayCheck(void){
  uint32_t CurrentTime = HAL_GetTick();
  uint16_t setTemp = Iron.TargetTemperature;
  static uint8_t pos, prev_power[RUNAWAY_DEPTH];
  uint8_t power = 0;

  if(getSettings()->setupMode==enable || (Iron.Error.safeMode && Iron.Error.active)){
    return;
  }
#if defined RUNAWAY_RESET_CYCLES && RUNAWAY_RESET_CYCLES>0
  if(Iron.resetRunawayHistory){
    if(Iron.resetRunawayHistory==1)                                                             // Clear power history only once
      for(uint8_t t=0; t<RUNAWAY_DEPTH; t++)
         prev_power[t]=0;
    if(++Iron.resetRunawayHistory>(RUNAWAY_RESET_CYCLES-1))                                         // Clear flag
      Iron.resetRunawayHistory=0;
  }
#endif
  if(getSystemTempUnit()==mode_Farenheit){
    setTemp = TempConversion(setTemp, mode_Celsius, 0);
  }
  if(!Iron.resetRunawayHistory){                                                                // Ignore power if flag is set
    prev_power[pos]=Iron.CurrentIronPower;                                                      // Circular buffer
    if(++pos>RUNAWAY_DEPTH-1)
      pos=0;
    for(uint8_t t=0; t<RUNAWAY_DEPTH; t++)
      power += prev_power[t];
    power /= RUNAWAY_DEPTH;
  }


  // If by any means the PWM output is higher than max calculated, generate error
  if((Iron.Pwm_Out > (Iron.Pwm_Period+1)) || (Iron.Pwm_Out != __HAL_TIM_GET_COMPARE(Iron.Pwm_Timer,Iron.Pwm_Channel))){
    Error_Handler();
  }

  if(power && (Iron.RunawayStatus==runaway_ok) && (last_TIP_C > setTemp)){

    if(last_TIP_C>500){ Iron.RunawayLevel=runaway_500; }                                    // 500ºC limit
    else{
      for(int8_t c=runaway_100; c>=runaway_ok; c--){                                        // Check temperature diff
        Iron.RunawayLevel=c;
        if(last_TIP_C > (setTemp + (25*Iron.RunawayLevel)) ){                               // 25ºC steps
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
              fatalError(error_RUNAWAY25);
            }
            break;
          case runaway_50:                                                                  // Temp >50°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>10000){                                      // 10 second limit
              Iron.RunawayStatus=runaway_triggered;
              fatalError(error_RUNAWAY50);
            }
            break;
          case runaway_75:                                                                  // Temp >75°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>5000){                                       // 5 second limit
              Iron.RunawayStatus=runaway_triggered;
              fatalError(error_RUNAWAY75);
            }
            break;
          case runaway_100:                                                                 // Temp >100°C over setpoint
            if((CurrentTime-Iron.RunawayTimer)>2000){                                       // 2 second limit
              Iron.RunawayStatus=runaway_triggered;
              fatalError(error_RUNAWAY100);
            }
            break;
          case runaway_500:                                                                 // Exceed 500ºC!
            if((CurrentTime-Iron.RunawayTimer)>2000){                                       // 2 second limit
              Iron.RunawayStatus=runaway_triggered;
              fatalError(error_RUNAWAY500);
            }
            break;
          default:                                                                          // Unknown overrun state
            Error_Handler();
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

  uint32_t t = getProfileSettings()->readPeriod + 1;                                        // Complete PWM period
  uint32_t te = t - getProfileSettings()->readDelay - 1;                                    // Effective period after subtracting read delay
  uint32_t z = getProfileSettings()->impedance;                                             // Heater resistance
  uint32_t pwr = getProfileSettings()->power;                                               // Power limit value
  uint32_t maxPower = volts * te / z / t;                                                   // Compute max power with current voltage, impedance (Stored in x10) and effective pwm cycle
  uint32_t max;
  if(pwr >= maxPower){                                                                      // If set power is already higher than the max possible power given the voltage and heater resistance
     max = te;                                                                              // Set max PWM
  }
  else{                                                                                     // Else,
    max = te * pwr / maxPower;                                                              // Compute max PWM output for current power limit
  }
  Iron.Pwm_Max = max / getProfileSettings()->pwmMul;                                  // Adjust for PWM multiplier value
}
#endif

// Sets no Iron detection threshold
void setNoIronValue(uint16_t noiron){
  getProfileSettings()->noIronValue=noiron;
}

// Change the iron operating mode in stand mode
void setModefromStand(uint8_t mode){
  if( getIronError() ||                                                         // Skip change if:
      ((Iron.CurrentMode==mode_sleep) && (mode==mode_standby)) ||               // Setting sleep mode while in standby
      ((Iron.CurrentMode==mode_boost) && (mode==mode_run)) ){                   // Setting run mode while in boost
    return;
  }

  Iron.changeMode = mode;                                                       // Update mode
  Iron.LastModeChangeTime = HAL_GetTick();                                      // Reset debounce timer
  Iron.standMode_update = needs_update;                                          // Set flag
}

// Set the iron operating mode
void setCurrentMode(uint8_t mode, uint16_t beep_time){
  if(Iron.Error.active){
    mode=mode_sleep;                                                                      // If error active, override with sleep mode
  }

  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  CurrentTime=HAL_GetTick();                                                              // Update local time value just in case it's called by handleIron, to avoid drift
  Iron.CurrentModeTimer = CurrentTime;                                                    // Refresh current mode timer

  if(!getIronCalibrationMode() && getProfileSettings()->coldBoostEnabled){
    int16_t tipTemp = (getProfileSettings()->tempUnit == mode_Farenheit) ? last_TIP_F : last_TIP_C;
    if(mode==mode_run && getCurrentMode() < mode_run && (Iron.UserSetTemperature - tipTemp) > getProfileSettings()->coldBoostTemperature)
      mode=mode_coldboost;
  }
  if(mode==mode_standby){
    if(Iron.UserSetTemperature < getProfileSettings()->standbyTemperature)
    {
      // if the user set temp is smaller than the standby temp, don't heat up the iron in standby
      Iron.TargetTemperature = Iron.UserSetTemperature;               // Set standby temp
    }
    else
    {
      Iron.TargetTemperature = getProfileSettings()->standbyTemperature;               // Set standby temp
    }
  }
  else if((mode==mode_boost) || (mode==mode_coldboost)){
    Iron.TargetTemperature = Iron.UserSetTemperature+getProfileSettings()->boostTemperature;
    if(Iron.TargetTemperature>getProfileSettings()->MaxSetTemperature){
      Iron.TargetTemperature=getProfileSettings()->MaxSetTemperature;
    }
  }
  else{
    Iron.TargetTemperature = Iron.UserSetTemperature;                                 // Set user temp (sleep mode ignores this)
  }
  if(Iron.CurrentMode != mode){                                                           // If current mode is different
    Iron.CurrentMode = mode;
    //resetPID();
    resetRunAway();
    if(!Iron.calibrating){
      buzzer_beep(beep_time);
      modeChanged(mode);
    }
    Iron.temperatureReached = 0;                                                    // Reset temperature reached flag
  }
  __set_PRIMASK(_irq);
}

// Called from program timer if WAKE change is detected
bool IronWake(wakeSrc_t src){                                                                 // source: handle shake, encoder push button
  static uint32_t last_time;

  if(Iron.Error.Flags || getProfileSettings()->WakeInputMode==mode_stand){                   // Ignore wake calls when error is present or in stand mode
    return 0;
  }
  if(Iron.CurrentMode==mode_standby){                                                         // Check whether current mode is allowed to be waken from the specified source
    if( (src==wakeSrc_Button && !(getSystemSettings()->buttonWakeMode & wake_standby)) ||
        (src==wakeSrc_Shake && !(getSystemSettings()->shakeWakeMode & wake_standby))   ){
      return 0;
    }
  }
  else if(Iron.CurrentMode==mode_sleep){
    if( (src==wakeSrc_Button && !(getSystemSettings()->buttonWakeMode & wake_sleep)) ||
        (src==wakeSrc_Shake && !(getSystemSettings()->shakeWakeMode & wake_sleep))   ){
      return 0;
    }
  }
  if(getProfileSettings()->shakeFiltering && src==wakeSrc_Shake){                            // Shake filtering enabled ?
    uint32_t time=(HAL_GetTick()-last_time);
    last_time = HAL_GetTick();
    if(time<100 || time>500){                                                                 // Ignore changes happening faster than 100mS or slower than 500mS.
      return 0;
    }
  }
  if(Iron.CurrentMode<mode_boost)
      setCurrentMode(mode_run, MLONG_BEEP);
  Iron.lastWakeSrc = src;
  return 1;
}

void readWake(void){
  static bool last_wake=0;
  bool now_wake;

#ifdef STAND_Pin                                                        // Dedicated Stand pin
  if(getProfileSettings()->WakeInputMode==mode_stand){
    now_wake = STAND_input();
  }
  else{
    now_wake = WAKE_input();
  }
#else                                                                   // Shared pin (Same signal works either as Stand or Shake input)
  now_wake = WAKE_input();
#endif

  if(last_wake!=now_wake){                                            // If wake sensor input changed
    if(Iron.tipchange)                                                // Suppress any actions while changing the tip
      return;

    last_wake=now_wake;
    if(getProfileSettings()->WakeInputMode==mode_shake){
      if(IronWake(wakeSrc_Shake)){
        Iron.shakeActive = 1;
        Iron.lastShakeTime = HAL_GetTick();
      }
    }
    else if(getProfileSettings()->WakeInputMode==mode_stand){
      setModefromStand(now_wake ? mode_run: getProfileSettings()->StandMode);
    }
  }
}

void resetIronError(void){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  Iron.Error.Flags &= (FLAG_ACTIVE | FLAG_SAFE_MODE | FLAG_NO_IRON);           // Clear all errors except active, safe mode and no iron
  Iron.LastErrorTime += (getProfileSettings()->errorTimeout+2);               // Bypass timeout
  checkIronError();                                                            // Refresh Errors
  __set_PRIMASK(_irq);
}

// Checks for non critical iron errors (Errors that can be cleared)

void checkIronError(void){
  CurrentTime = HAL_GetTick();
  IronError_t Err;
  Err.Flags=0;
  Err.noIron = (TIP.last_raw>getProfileSettings()->noIronValue);
  Err.safeMode = Iron.Error.safeMode;

  if(!Iron.Error.noIron){                                                               // Bypass other errors when no iron detected
      Err.NTC_high =  (last_NTC_C > 800);                                               // As the NTC is often connected in the handle
      Err.NTC_low =  (last_NTC_C < -200);                                               // This way only the "No iron" image is shown.
      #ifdef USE_VIN
      Err.V_low = (getSupplyVoltage_v_x10() < getSystemSettings()->lvp);
      #endif
  }

  if(Err.Flags){                                                                        // Errors detected
    if(Err.noIron)                                                                      // If no iron Detected
      Iron.Error.Flags &= (FLAG_ACTIVE | FLAG_SAFE_MODE | FLAG_NO_IRON );               // Clear other existing errors, but keep safe mode, no iron and active flags.
    Iron.Error.Flags |= Err.Flags;                                                      // Update stored Iron errors
    Iron.LastErrorTime = CurrentTime;                                                   // Update error time
    if(!Iron.Error.active){                                                             // Active flag wasnt set, this is a first occurring error
      Iron.lastMode = Iron.CurrentMode;                                                 // Save current mode
      if(Err.Flags!=FLAG_NO_IRON){                                                      // Avoid alarm if only the tip is removed
        setCurrentMode(mode_sleep, 0);                                                  // Set sleep mode, no beeping as alarm is active
        buzzer_alarm_start();                                                           // Start alarm
      }
      else{
        setCurrentMode(mode_sleep, MEDIUM_BEEP);                                        // Set sleep mode, short beep, tip removed
      }
      Iron.Error.active = 1;                                                            // Set active flag
      configurePWMpin(output_Low);                                                      // Force pin low to completely remove the power
    }
  }
  else if (Iron.Error.active && !Err.Flags){                                            // If global flag set, but no errors
    if((CurrentTime-Iron.LastErrorTime)>getProfileSettings()->errorTimeout){           // Check if enough time has passed
      buzzer_alarm_stop();                                                              // Stop alarm
      Iron.Error.Flags = FLAG_NOERROR;                                                  // Clear error flags
      Iron.err_resumed=0;                                                               // Clear resume flag so the mode is is restored
    }
  }
  if(!Iron.err_resumed && !Iron.Error.active){                                          // Resume after error

    if(!Iron.boot_loaded && (current_time > getProfileSettings()->errorTimeout+3000))  // If the system stays in error state for more than 3 seconds after boot, assume it's a problem
      Iron.boot_loaded=1;

    Iron.err_resumed = 1;                                                               // Set resume flag
    if(!Iron.boot_loaded){                                                              // Boot profile not loaded, use boot mode
      Iron.boot_loaded=1;
      setCurrentMode(getSystemSettings()->initMode, 0);
    }
    else{                                                                               // Already initialized boot mode, this error happened later
      if(getProfileSettings()->errorResumeMode==error_sleep)                           // Load mode defined in errorResumeMode
        setCurrentMode(mode_sleep, MEDIUM_BEEP);
      else if(getProfileSettings()->errorResumeMode==error_run)
        setCurrentMode(mode_run, MEDIUM_BEEP);
      else
        setCurrentMode(Iron.lastMode, MEDIUM_BEEP);
    }
  }
}

bool getIronError(void){
  return Iron.Error.Flags;
}

uint32_t getIronLastErrorTime(void){
  return Iron.LastErrorTime;
}

void setIronSafeMode(uint8_t mode){
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  if(mode==disable && Iron.Error.Flags==(FLAG_ACTIVE | FLAG_SAFE_MODE)){                 // If only failsafe was active? (This should only happen because it was on first init screen)
    Iron.Error.Flags = FLAG_NOERROR;
    setCurrentMode(mode_run, 0);
  }
  else{
    if(mode==enable){
      configurePWMpin(output_Low);
    }
    Iron.Error.safeMode=mode;
    checkIronError();
  }
  __set_PRIMASK(_irq);
}

void setIronTipChange(uint8_t mode){
  if(mode)
    setCurrentMode(mode_sleep, MLONG_BEEP);
  Iron.tipchange = mode;
}

bool GetSafeMode(void){
  return(Iron.Error.safeMode && Iron.Error.active);
}

void setIronCalibrationMode(uint8_t mode){
  Iron.calibrating = mode;
  if(mode==enable){
    setCurrentMode(mode_run, 0);
  }
}

bool getIronCalibrationMode(void){
  return Iron.calibrating;
}

void setUserTemperature(int16_t temperature) {
  uint32_t _irq = __get_PRIMASK();
  __disable_irq();
  Iron.UserSetTemperature = temperature;
  if(Iron.CurrentMode==mode_run){
    //resetPID();
    resetRunAway();
    Iron.temperatureReached = 0;
    Iron.TargetTemperature = temperature;
  }
  __set_PRIMASK(_irq);
}

uint16_t getUserTemperature(void){
 return Iron.UserSetTemperature;
}

uint8_t getCurrentMode(void){
  return Iron.CurrentMode;
}

int8_t getCurrentPower(void){
  return Iron.CurrentIronPower;
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

TIM_HandleTypeDef* getIronReadTimer(void){
  return Iron.Read_Timer;
}

TIM_HandleTypeDef* getIronPwmTimer(void){
  return Iron.Pwm_Timer;
}

IronError_t getIronErrorFlags(void){
  return Iron.Error;
}

void ironSchedulePwmUpdate(void){
  Iron.updatePwm = true;
}

bool getBootCompleteFlag(void){
  return Iron.boot_complete;
}

void setBootCompleteFlag(void){
  Iron.boot_complete = true;
}

uint32_t getIronPwmOutValue(){
  return Iron.Pwm_Out;
}

uint16_t getIronTargetTemperature(void){
  return Iron.TargetTemperature;
}

uint32_t getIronCurrentModeTimer(void){
  return Iron.CurrentModeTimer;
}

bool isIronTargetTempReached(void){
  return Iron.temperatureReached;
}

bool getIronShakeFlag(void){
  return Iron.shakeActive;
}
void clearIronShakeFlag(void){
  Iron.shakeActive = false;
}

uint32_t getIronLastShakeTime(void){
  return Iron.lastShakeTime;
}

wakeSrc_t getIronWakeSource(void){
  return Iron.lastWakeSrc;
}

void waitForNextConversion(void){
  Iron.waiting_conversion  = 1;
  while(Iron.waiting_conversion);
}
