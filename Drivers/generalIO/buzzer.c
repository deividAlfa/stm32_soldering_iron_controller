/*
 * buzzer.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "buzzer.h"
#include "iron.h"
#include "settings.h"
#include "main.h"

typedef enum {STATE_SB, STATE_LB, STATE_FB, STATE_AL_H,STATE_AL_L, STATE_IDLE} buzzer_state_type;

static buzzer_state_type buzzer_state = STATE_IDLE;
static uint32_t last_time;

static void tempReachedCall(uint16_t temp) {
	buzzer_short_beep();
}
static setTemperatureReachedCallback ironTempReachedCallback = &tempReachedCall;

void buzzer_init() {
	addSetTemperatureReachedCallback(ironTempReachedCallback);
}

void buzzer_short_beep() {
	if(!systemSettings.settings.buzzerMode){ return; }
	buzzer_state = STATE_SB;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_long_beep() {
	if(!systemSettings.settings.buzzerMode){ return; }
	buzzer_state = STATE_LB;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_fatal_beep() {
	if(!systemSettings.settings.buzzerMode){ return; }
	buzzer_state = STATE_FB;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_alarm_start(){
	if(!systemSettings.settings.buzzerMode){ return; }
	buzzer_state = STATE_AL_H;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_alarm_stop() {
	buzzer_state = STATE_IDLE;
	BUZZER_OFF;
}
void handle_buzzer() {
	uint32_t delta = HAL_GetTick() - last_time;
	if(!systemSettings.settings.buzzerMode){
		if(buzzer_state != STATE_IDLE){
			buzzer_state = STATE_IDLE;
			BUZZER_OFF;
		}
		return;
	}
	switch (buzzer_state) {
	case STATE_SB:
		if (delta > SHORT_BEEP) {
			BUZZER_OFF;
			buzzer_state = STATE_IDLE;
		}
		break;
	case STATE_LB:
		if (delta > LONG_BEEP) {
			BUZZER_OFF;
			buzzer_state = STATE_IDLE;
		}
		break;
	case STATE_FB:
		if (delta > FATAL_BEEP) {
			BUZZER_OFF;
			buzzer_state = STATE_IDLE;
		}
		break;
	case STATE_AL_H:
		if(delta > ALARM_HIGH){
				buzzer_state=STATE_AL_L;
				BUZZER_OFF;
				last_time = HAL_GetTick();
		}
		break;
	case STATE_AL_L:
		if(delta > ALARM_LOW){
			buzzer_state=STATE_AL_H;
			BUZZER_ON;
			last_time = HAL_GetTick();
		}
		break;
	case STATE_IDLE:
		break;
	default:
		BUZZER_OFF;
		buzzer_state = STATE_IDLE;
		break;
	}
}
