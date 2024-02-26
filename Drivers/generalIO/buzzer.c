/*
 * buzzer.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "buzzer.h"
#include "iron.h"
#include "settings.h"
#include "main.h"

typedef enum {STATE_IDLE, STATE_BEEP, STATE_AL_H, STATE_AL_L } buzzer_state_type;

static buzzer_state_type buzzer_state = STATE_IDLE;
static uint32_t beep_end;

static void tempReachedCall(uint16_t temp) {
  buzzer_beep(SHORT_BEEP);
}
static setTemperatureReachedCallback ironTempReachedCallback = &tempReachedCall;

void buzzer_init() {
  addSetTemperatureReachedCallback(ironTempReachedCallback);
}

void buzzer_beep(uint32_t duration) {
  buzzer_state = STATE_BEEP;
  if(getSystemSettings()->buzzerMode){
    BUZZER_ON;
  }
  beep_end = HAL_GetTick() + duration;
}

void buzzer_force_beep(uint32_t duration) {
  buzzer_state = STATE_BEEP;
  BUZZER_ON;
  beep_end = HAL_GetTick() + duration;
}

void buzzer_alarm_start(){
  if(!getSystemSettings()->buzzerMode){ return; }
  buzzer_state = STATE_AL_H;
  if(getSystemSettings()->buzzerMode){
    BUZZER_ON;
  }
  beep_end = HAL_GetTick() + ALARM_HIGH;
}

void buzzer_alarm_stop() {
  buzzer_state = STATE_IDLE;
  BUZZER_OFF;
}

void handle_buzzer() {
  uint32_t now = HAL_GetTick();
  switch (buzzer_state) {

    case STATE_IDLE:
      break;

    case STATE_BEEP:
      if (beep_end < now) {
        BUZZER_OFF;
        buzzer_state = STATE_IDLE;
      }
      break;

    case STATE_AL_H:
      if (beep_end < now) {
          buzzer_state=STATE_AL_L;
          BUZZER_OFF;
          beep_end = now + ALARM_LOW;
      }
      break;

    case STATE_AL_L:
      if (beep_end < now) {
        buzzer_state=STATE_AL_H;
        if(getSystemSettings()->buzzerMode){
          BUZZER_ON;
        }
        beep_end = now + ALARM_HIGH;
      }
      break;

    default:
      BUZZER_OFF;
      buzzer_state = STATE_IDLE;
      break;
  }
}
