/*
 * buzzer.c
 *
 *  Created on: Jul 18, 2017
 *      Author: jose
 */

#include "buzzer.h"
#include "../../../Src/iron.h"

typedef enum {STATE_SB, STATE_LB, STATE_AL, STATE_IDLE} buzzer_state_type;

static buzzer_state_type buzzer_state = STATE_IDLE;
static uint32_t last_time;

#define BUZZER_ON HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET)
#define BUZZER_OFF HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET)
#define BUZZER_TOGGLE HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin)

static void tempReachedCall(uint16_t temp) {
	buzzer_short_beep();
}
static setTemperatureReachedCallback ironTempReachedCallback = &tempReachedCall;

void buzzer_init() {
	addSetTemperatureReachedCallback(ironTempReachedCallback);
}

void buzzer_short_beep() {
	buzzer_state = STATE_SB;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_long_beep() {
	buzzer_state = STATE_LB;
	BUZZER_ON;
	last_time = HAL_GetTick();
}
void buzzer_alarm_start(){
	buzzer_state = STATE_AL;
}
void buzzer_alarm_stop() {
	buzzer_state = STATE_IDLE;
	BUZZER_OFF;
}
void handle_buzzer() {
	uint32_t delta = HAL_GetTick() - last_time;
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
	case STATE_AL:
		if (delta > ALARM) {
			BUZZER_TOGGLE;
			last_time = HAL_GetTick();
		}
		break;
	default:
			break;
	}
}
