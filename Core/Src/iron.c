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
#define __BASE_FILE__ "iron.c"
#endif

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
  uint8_t             updateStandMode;                      // Flag to indicate the stand mode must be changed
  uint8_t             shakeActive;                          // Flag to indicate handle movement
  uint8_t             temperatureReached;                   // Flag for temperature calibration
  uint8_t             updatePwm;                            // Flag to indicate PWM need to be updated
  IronError_t         Error;                                // Error flags
  uint8_t             lastMode;                             // Last mode before error condition.
  uint8_t             boot_complete;                        // Flag set to 1 when boot screen exits (Used for error handlding)

  uint16_t            Pwm_Period;                           // PWM period
  uint16_t            Pwm_Max;                              // Max PWM output for power limit
  int16_t             UserSetTemperature;                   // Run mode user setpoint
  int16_t             TargetTemperature;                    // Actual set (target) temperature (Setpoint)

  uint32_t           Pwm_Out;                              // Last calculated PWM value
  uint32_t           LastModeChangeTime;                   // Last time the mode was changed (To provide debouncing)
  uint32_t           LastErrorTime;                        // last time iron error was detected
  uint32_t           lastShakeTime;                        // last time iron handle was moved (In shake mode)
  uint32_t           CurrentModeTimer;                     // Time since actual mode was set
  uint32_t           RunawayTimer;                         // Runaway timer

  TIM_HandleTypeDef* Read_Timer;                          // Pointer to the Read timer
  TIM_HandleTypeDef* Pwm_Timer;                           // Pointer to the PWM timer
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

  if(systemSettings.Profile.WakeInputMode == mode_shake){
    setCurrentMode(systemSettings.settings.initMode);
  }
  else{
    if(WAKE_input()){
      setCurrentMode(mode_run);
    }
    else{
      setCurrentMode(systemSettings.Profile.StandMode);
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
    if( (systemSettings.setupMode==enable) || systemSettings.settings.state!=initialized || systemSettings.Profile.state!=initialized ||
        (systemSettings.Profile.ID != systemSettings.currentProfile) || (systemSettings.currentProfile>profile_C210)){

      setSafeMode(enable);
    }
  }

  checkIronError();                                                           // Check iron error presence, must be called before coldJuctionSensor
  readColdJunctionSensorTemp_x10(new_reading, mode_Celsius);                  // to reset the NTC detection status
  readTipTemperatureCompensated(new_reading, read_average, mode_Celsius);     // Update readings
  checkIronError();                                                           // Check iron error again (Special case when iron has been detected, show other errors)

  // Controls external mode changes (from stand mode changes), this acts as a debouncing timer
  if(Iron.updateStandMode==needs_update){
    if(Iron.Error.active || Iron.calibrating){                                      // Ignore changes when error active or calibrating
      Iron.updateStandMode=no_update;
    }
    else if((CurrentTime-Iron.LastModeChangeTime)>100){                             // Wait 100mS with no changes (de-bouncing)
      Iron.updateStandMode=no_update;
      setCurrentMode(Iron.changeMode);
    }
  }

  // If sleeping or error, stop here
  if(!Iron.boot_complete || Iron.CurrentMode==mode_sleep || Iron.Error.active) {                           // For safety, force PWM low everytime
    Iron.Pwm_Out=0;
    __HAL_TIM_SET_COMPARE(Iron.Pwm_Timer, Iron.Pwm_Channel, 0);
    Iron.CurrentIronPower=0;
    return;
  }

  // Controls inactivity timer and enters low power modes
  if(!Iron.calibrating){                                                                      // Don't check timeout when calibrating
    uint32_t mode_time = CurrentTime - Iron.CurrentModeTimer;

    if((Iron.CurrentMode==mode_boost) && (mode_time>systemSettings.Profile.boostTimeout)){    // If boost mode and time expired
      setCurrentMode(mode_run);
    }
    else if(Iron.CurrentMode==mode_run){                                                      // If running
      if(systemSettings.Profile.standbyTimeout){                                              // If standby timer enabled
        if(mode_time>systemSettings.Profile.standbyTimeout){                                  // Check timeout
          setCurrentMode(mode_standby);
        }
      }
      else{                                                                                   // Otherwise, check sleep timeout
        if(mode_time>systemSettings.Profile.sleepTimeout){                                    //
          setCurrentMode(mode_sleep);
        }
      }
    }
    else if(Iron.CurrentMode==mode_standby){                                                  // If in standby
      if(mode_time>systemSettings.Profile.sleepTimeout){                                      // Check sleep timeout
        setCurrentMode(mode_sleep);
      }
    }
  }


  if(Iron.updatePwm){                                                                       // If pending PWM period update, refresh Iron Pwm_period
    Iron.Pwm_Period = ((systemSettings.Profile.readPeriod+1)/systemSettings.Profile.pwmMul)-1;
  }

  #ifdef USE_VIN
  updatePowerLimit();                                                                         // Update power limit values
  #endif

  // Update PID
  Iron.Pwm_Out = calculatePID(human2adc(Iron.TargetTemperature), TIP.last_avg, Iron.Pwm_Max);

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
      temperatureReached( Iron.TargetTemperature);
      Iron.temperatureReached = 1;
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

  __disable_irq();
  if(systemSettings.Profile.tempUnit != unit){
    systemSettings.Profile.tempUnit = unit;
    Iron.UserSetTemperature = round_10(TempConversion(Iron.UserSetTemperature,unit,0));                                         // User temp is loaded from the profile, thus it uses the same unit
    systemSettings.Profile.defaultTemperature = round_10(TempConversion(systemSettings.Profile.defaultTemperature,unit,0));
    systemSettings.Profile.standbyTemperature = round_10(TempConversion(systemSettings.Profile.standbyTemperature,unit,0));
    systemSettings.Profile.MaxSetTemperature = round_10(TempConversion(systemSettings.Profile.MaxSetTemperature,unit,0));
    systemSettings.Profile.MinSetTemperature = round_10(TempConversion(systemSettings.Profile.MinSetTemperature,unit,0));
    systemSettings.Profile.boostTemperature = round_10(TempIncrementConversion(systemSettings.Profile.boostTemperature,unit));
  }
  if(systemSettings.settings.tempUnit != unit){
    systemSettings.settings.tempUnit = unit;
  }
  __enable_irq();
  setCurrentMode(Iron.CurrentMode);     // Reload temps
}
bool getSystemTempUnit(void){
  return systemSettings.settings.tempUnit;
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
  __disable_irq();
 systemSettings.Profile.readDelay=delay;
  __enable_irq();
}


void setReadPeriod(uint16_t period){
  __disable_irq();
 systemSettings.Profile.readPeriod=period;
 Iron.updatePwm=1;
  __enable_irq();
}

void setPwmMul(uint16_t mult){
  __disable_irq();
  systemSettings.Profile.pwmMul=mult;
  Iron.updatePwm=1;
  __enable_irq();
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

  if(systemSettings.setupMode==enable || (Iron.Error.safeMode && Iron.Error.active)){
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
  __disable_irq();
  systemSettings.Profile.noIronValue=noiron;
  __enable_irq();
}

// Change the iron operating mode in stand mode
void setModefromStand(uint8_t mode){
  if( isIronInError() ||
      ((Iron.changeMode==mode) && (Iron.CurrentMode==mode)) ||
      ((Iron.CurrentMode==mode_sleep) && (mode==mode_standby)) ||
      ((Iron.CurrentMode==mode_boost) && (mode==mode_run)) ){
    return;
  }
  __disable_irq();
  if(Iron.changeMode!=mode){
    Iron.changeMode = mode;                                                                 // Update mode
    Iron.LastModeChangeTime = HAL_GetTick();                                                // Reset debounce timer
  }
  Iron.updateStandMode = needs_update;                                                      // Set flag
  __enable_irq();
}

// Set the iron operating mode
void setCurrentMode(uint8_t mode){
  if(Iron.Error.active){
    mode=mode_sleep;                                                                      // If error active, override with sleep mode
  }

  __disable_irq();
  CurrentTime=HAL_GetTick();                                                              // Update local time value just in case it's called by handleIron, to avoid drift
  Iron.CurrentModeTimer = CurrentTime;                                                    // Refresh current mode timer
  if(mode==mode_standby){
    if(Iron.UserSetTemperature < systemSettings.Profile.standbyTemperature)
    {
      // if the user set temp is smaller than the standby temp, don't heat up the iron in standby
      Iron.TargetTemperature = Iron.UserSetTemperature;               // Set standby temp
    }
    else
    {
      Iron.TargetTemperature = systemSettings.Profile.standbyTemperature;               // Set standby temp
    }
  }
  else if(mode==mode_boost){
    Iron.TargetTemperature = Iron.UserSetTemperature+systemSettings.Profile.boostTemperature;
    if(Iron.TargetTemperature>systemSettings.Profile.MaxSetTemperature){
      Iron.TargetTemperature=systemSettings.Profile.MaxSetTemperature;
    }
  }
  else{
    Iron.TargetTemperature = Iron.UserSetTemperature;                                 // Set user temp (sleep mode ignores this)
  }
  if(Iron.CurrentMode != mode){                                                           // If current mode is different
    Iron.CurrentMode = mode;
    resetPID();
    resetRunAway();
    if(!Iron.calibrating){
      buzzer_long_beep();
      modeChanged(mode);
    }
    if(mode == mode_run){
      Iron.temperatureReached = 0;
    }
  }
  __enable_irq();
}

// Called from program timer if WAKE change is detected
bool IronWake(bool source){                                                                 // source: handle shake, encoder push button
  static uint32_t last_time;
  if(Iron.Error.Flags || systemSettings.Profile.WakeInputMode==mode_stand){
    return 0;
  }

  if(Iron.CurrentMode==mode_standby){
    if( (source==wakeButton && !(systemSettings.settings.buttonWakeMode & wake_standby)) ||
        (source==wakeInput && !(systemSettings.settings.shakeWakeMode & wake_standby))){

      return 0;
    }
  }
  else if(Iron.CurrentMode==mode_sleep){
    if( (source==wakeButton && !(systemSettings.settings.buttonWakeMode & wake_sleep)) ||
        (source==wakeInput && !(systemSettings.settings.shakeWakeMode & wake_sleep))){

      return 0;
    }
  }

  if(systemSettings.Profile.shakeFiltering && source==wakeInput){      // Sensitivity workaround enabled
    uint32_t time=(HAL_GetTick()-last_time);
    last_time = HAL_GetTick();
    if(time<100 || time>500){                                           // Ignore changes happening faster than 100mS or slower than 500mS.
      return 0;
    }
  }
  if(Iron.CurrentMode<mode_boost){
      __disable_irq();
      setCurrentMode(mode_run);
      __enable_irq();
  }
  return 1;
}

void readWake(void){
  static bool last_wake=0;
    bool now_wake = WAKE_input();

    if(last_wake!=now_wake){                                            // If wake sensor input changed
      last_wake=now_wake;
      if(systemSettings.Profile.WakeInputMode==mode_stand){   // In stand mode
        if(now_wake){
          setModefromStand(mode_run);
        }
        else{
          setModefromStand(systemSettings.Profile.StandMode);          // Set sleep or standby mode depending on system setting
        }
      }
      else{
        if(IronWake(wakeInput)){
          Iron.shakeActive = 1;
          Iron.lastShakeTime = HAL_GetTick();
        }
      }
    }
}

void resetIronError(void){
  __disable_irq();
  Iron.Error.Flags &= (FLAG_ACTIVE | FLAG_SAFE_MODE | FLAG_NO_IRON);           // Clear all errors except active, safe mode and no iron
  Iron.LastErrorTime += (systemSettings.Profile.errorTimeout+2);               // Bypass timeout
  checkIronError();                                                            // Refresh Errors
  __enable_irq();
}

// Checks for non critical iron errors (Errors that can be cleared)

void checkIronError(void){
  CurrentTime = HAL_GetTick();
  IronError_t Err;
  Err.Flags=0;
  Err.noIron = (TIP.last_raw>systemSettings.Profile.noIronValue);
  Err.safeMode = Iron.Error.safeMode;

  if(!Iron.Error.noIron){                                                               // Bypass other errors when no iron detected
      Err.NTC_high =  (last_NTC_C > 800);
      Err.NTC_low =  (last_NTC_C < -200);
      #ifdef USE_VIN
      Err.V_low = (getSupplyVoltage_v_x10() < systemSettings.settings.lvp);
      #endif
  }

  if(Err.Flags){

    if(Err.noIron){																																			// If no iron flag
      Iron.Error.Flags &= (FLAG_ACTIVE | FLAG_SAFE_MODE | FLAG_NO_IRON );		    				// Clear other existing errors except safe mode
    }
    Iron.Error.Flags |= Err.Flags;                                                    	// Update Iron errors

    Iron.LastErrorTime = CurrentTime;
    if(!Iron.Error.active){
      if(Err.Flags!=FLAG_NO_IRON){                                                      // Avoid alarm if only the tip is removed
        buzzer_alarm_start();
      }
      Iron.lastMode = Iron.CurrentMode;
      Iron.Error.active = 1;
      setCurrentMode(mode_sleep);
      configurePWMpin(output_Low);
    }
  }
  else if (Iron.Error.active && !Err.Flags){                                            // If global flag set, but no errors
    if((CurrentTime-Iron.LastErrorTime)>systemSettings.Profile.errorTimeout){           // Check if enough time has passed
      Iron.Error.Flags = 0;
      buzzer_alarm_stop();
      if(Iron.boot_complete){                                                           // If error happened after booting, set resume mode
        if(systemSettings.Profile.errorResumeMode==error_sleep){
          setCurrentMode(mode_sleep);
        }
        else if(systemSettings.Profile.errorResumeMode==error_run){
          setCurrentMode(mode_run);
        }
        else{
          setCurrentMode(Iron.lastMode);
        }
      }
      else{                                                                             // If error before booting, set init mode
        setCurrentMode(systemSettings.settings.initMode);
      }
    }
  }
  else{
    Iron.Error.Flags=FLAG_NOERROR;
  }
}


bool isIronInError(void){
  return Iron.Error.Flags;
}

void setSafeMode(bool mode){
  __disable_irq();
  if(mode==disable && Iron.Error.Flags==(FLAG_ACTIVE | FLAG_SAFE_MODE)){                 // If only failsafe was active? (This should only happen because it was on first init screen)
    Iron.Error.Flags = FLAG_NOERROR;
    setCurrentMode(mode_run);
  }
  else{
    if(mode==enable){
      configurePWMpin(output_Low);
    }
    Iron.Error.safeMode=mode;
    checkIronError();
  }
  __enable_irq();
}


bool GetSafeMode() {
  return(Iron.Error.safeMode && Iron.Error.active);
}

void setCalibrationMode(uint8_t mode){
  __disable_irq();
  Iron.calibrating = mode;

  if(mode==enable){
    setCurrentMode(mode_run);
  }

  __enable_irq();
}

bool isIronInCalibrationMode(void){
  return Iron.calibrating;
}

void setUserTemperature(uint16_t temperature) {
  __disable_irq();
  Iron.UserSetTemperature = temperature;
  if(Iron.CurrentMode==mode_run){
    resetPID();
    resetRunAway();
    Iron.temperatureReached = 0;
    Iron.TargetTemperature = temperature;
  }
  __enable_irq();
}

uint16_t getUserTemperature() {
 return Iron.UserSetTemperature;
}

uint8_t getCurrentMode() {
  return Iron.CurrentMode;
}

int8_t getCurrentPower() {
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

TIM_HandleTypeDef* getIronReadTimer(void)
{
  return Iron.Read_Timer;
}

TIM_HandleTypeDef* getIronPwmTimer(void)
{
  return Iron.Pwm_Timer;
}

IronError_t getIronErrorFlags(void)
{
  return Iron.Error;
}

void ironSchedulePwmUpdate(void)
{
  Iron.updatePwm = true;
}

bool getBootCompleteFlag(void)
{
  return Iron.boot_complete;
}

void setBootCompleteFlag(void)
{
  Iron.boot_complete = true;
}

uint32_t getIronPwmOutValue()
{
  return Iron.Pwm_Out;
}

uint16_t getIronTargetTemperature(void)
{
  return Iron.TargetTemperature;
}

uint16_t getUserSetTemperature()
{
  return Iron.UserSetTemperature;
}

uint32_t getIronCurrentModeTimer(void)
{
  return Iron.CurrentModeTimer;
}

bool isIronTargetTempReached(void)
{
  return Iron.temperatureReached;
}

bool getIronShakeFlag(void)
{
  return Iron.shakeActive;
}

void clearIronShakeFlag(void)
{
  Iron.shakeActive = false;
}

uint32_t getIronLastShakeTime(void)
{
  return Iron.lastShakeTime;
}
