/*
 * iron.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef IRON_H_
#define IRON_H_
#include "pid.h"
#include "settings.h"

#define PWM_DETECT_TIME   5                                    // Pulse before reading adc, to detect tip presence. In uS

typedef void (*setTemperatureReachedCallback)(uint16_t);


typedef void (*currentModeChanged)(uint8_t);
typedef union{
  uint8_t Flags;                                            // Flag for errors (wrong iron connection, NTC, internal failure...)
  struct{
    unsigned  noIron:1;                                     // No iron detected
    unsigned  NTC_high:1;                                   // NTC too high
    unsigned  NTC_low:1;                                    // NTC too low
    unsigned  V_low:1;                                      // Voltage too low
    unsigned  safeMode:1;                                   // Shut down pwm by some reason. Error condition, first boot...
    unsigned  unused_b5:1;
    unsigned  unused_b6:1;
    unsigned  active:1;                                     // Errors active flag
  };
}IronError_t;
# define ErrorMask    (uint8_t)0b11111                      // mask for used error bit fields (skipping global flag)

#define FLAG_NOERROR      0
#define FLAG_NO_IRON      1
#define FLAG_NTC_HIGH     2
#define FLAG_NTC_LOW      4
#define FLAG_V_LOW        8
#define FLAG_SAFE_MODE    16
#define FLAG_ACTIVE       128

typedef struct {

  uint8_t             Pwm_Channel;                          // PWM channel
  uint8_t             CurrentIronPower;                     // Last output power
  uint8_t             CurrentMode;                          // Actual working mode (Standby, Sleep, Normal, Boost)
  uint8_t             changeMode;                           // change working mode to (Standby, Sleep, Normal, Boost)
  uint8_t             RunawayLevel;                         // Runaway actual level
  uint8_t             prevRunawayLevel;                     // Runaway previous level
  uint8_t             RunawayStatus;                        // Runaway triggered flag
  uint8_t             calibrating;                          // Flag to indicate calibration state (don't save temperature settings)
  uint8_t             updateStandMode;                      // Flag to indicate the stand mode must be changed
  uint8_t             shakeActive;                          // Flag to indicate handle movement
  uint8_t             temperatureReached;                   // Flag for temperature calibration
  uint8_t             DebugMode;                            // Flag to indicate Debug is enabled
  uint8_t             updatePwm;                            // Flag to indicate PWM need to be updated
  IronError_t         Error;                                // Error flags
  uint8_t             lastMode;                             // Last mode before error condition.
  uint8_t             boot_complete;                        // Flag set to 1 when boot screen exits (Used for error handlding)

  uint16_t            Pwm_Period;                           // PWM period
  uint16_t            Pwm_Max;                              // Max PWM output for power limit
  int16_t             UserSetTemperature;                   // Run mode user setpoint
  int16_t             CurrentSetTemperature;                // Actual set temperature (Setpoint)
  int16_t             Debug_SetTemperature;                 // Debug mode temperature

  uint32_t            Pwm_Out;                              // Last calculated PWM value
  uint32_t            LastModeChangeTime;                   // Last time the mode was changed (To provide debouncing)
  uint32_t            LastErrorTime;                        // last time iron error was detected
  uint32_t            lastShakeTime;                        // last time iron handle was moved (In shake mode)
  uint32_t            CurrentModeTimer;                     // Time since actual mode was set
  uint32_t            RunawayTimer;                         // Runaway timer

  TIM_HandleTypeDef   *Read_Timer;                          // Pointer to the Read timer
  TIM_HandleTypeDef   *Pwm_Timer;                           // Pointer to the PWM timer
}iron_t;


extern volatile iron_t Iron;
void readWake(void);
bool IronWake(bool source);
void resetIronError(void);
void checkIronError(void);
bool GetIronError(void);
void updatePowerLimit(void);
void runAwayCheck(void);
void setSafeMode(bool mode);
bool getSafeMode(void);
void setCurrentMode(uint8_t mode);
void setModefromStand(uint8_t mode);
void setUserTemperature(uint16_t temperature);
uint16_t getUserTemperature(void);
uint8_t getCurrentMode(void);
uint16_t getCurrentTemperature();
int8_t getCurrentPower();
void initTimers(void);
void setPwmMul(uint16_t mult);
void setReadDelay(uint16_t delay);
void setReadPeriod(uint16_t period);
void setNoIronValue(uint16_t noiron);
void setSystemTempUnit(bool unit);
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback);
void addModeChangedCallback(currentModeChanged callback);
void handleIron(void);
void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel);
uint8_t getIronOn();
void setDebugTemp(uint16_t value);
uint16_t getDebugTemp(void);
void setDebugMode(uint8_t value);
uint8_t getDebugMode(void);
void setCalibrationMode(uint8_t mode);
uint8_t getCalibrationMode(void);
void configurePWMpin(uint8_t mode);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim);
#endif /* IRON_H_ */
