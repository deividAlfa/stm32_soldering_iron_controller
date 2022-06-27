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

#define TIP_DETECT_TIME         5                           // Pulse before reading adc, to detect tip presence. In uS
#define RUNAWAY_DEPTH           3                           // History count of power values stored to compute the average power for runaway monitor
#define RUNAWAY_RESET_CYCLES    3                           // Cycles to bypass runaway check when temperature was changed

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

#define ErrorMask    (uint8_t)0b11111                      // mask for used error bit fields (skipping global flag)

#define FLAG_NOERROR      0
#define FLAG_NO_IRON      1
#define FLAG_NTC_HIGH     2
#define FLAG_NTC_LOW      4
#define FLAG_V_LOW        8
#define FLAG_SAFE_MODE    16
#define FLAG_ACTIVE       128

void readWake(void);
bool IronWake(bool source);
void resetIronError(void);
void checkIronError(void);
bool isIronInError(void);
IronError_t getIronErrorFlags(void);
void updatePowerLimit(void);
void runAwayCheck(void);
void setSafeMode(bool mode);
bool getSafeMode(void);
void setCurrentMode(uint8_t mode);
void setModefromStand(uint8_t mode);
void setUserTemperature(uint16_t temperature);
uint16_t getUserTemperature(void);
uint8_t getCurrentMode(void);
int8_t getCurrentPower();
void initTimers(void);
void setPwmMul(uint16_t mult);
void setReadDelay(uint16_t delay);
void setReadPeriod(uint16_t period);
void setNoIronValue(uint16_t noiron);
void setSystemTempUnit(bool unit);
bool getSystemTempUnit(void);
void addSetTemperatureReachedCallback(setTemperatureReachedCallback callback);
void addModeChangedCallback(currentModeChanged callback);
void handleIron(void);
void ironInit(TIM_HandleTypeDef *delaytimer, TIM_HandleTypeDef *pwmtimer, uint32_t pwmchannel);
uint8_t getIronOn();
void setCalibrationMode(uint8_t mode);
bool isIronInCalibrationMode(void);
void configurePWMpin(uint8_t mode);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *_htim);
TIM_HandleTypeDef* getIronReadTimer(void);
TIM_HandleTypeDef* getIronPwmTimer(void);
void ironSchedulePwmUpdate(void);
bool getBootCompleteFlag(void);
void setBootCompleteFlag(void);
uint32_t getIronPwmOutValue();
uint16_t getIronTargetTemperature(void);
uint32_t getIronCurrentModeTimer(void);
bool isIronTargetTempReached(void);
bool getIronShakeFlag(void);
void clearIronShakeFlag(void);
uint32_t getIronLastShakeTime(void);
uint16_t getUserSetTemperature();

#endif /* IRON_H_ */
