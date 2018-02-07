/*
 * buzzer.h
 *
 *  Created on: Jul 18, 2017
 *      Author: jose
 */

#ifndef GENERALIO_BUZZER_H_
#define GENERALIO_BUZZER_H_

#include "stm32f1xx_hal.h"

#define SHORT_BEEP 	100
#define LONG_BEEP	1500
#define ALARM		800

void buzzer_short_beep();
void buzzer_long_beep();
void buzzer_alarm_start();
void buzzer_alarm_stop();
void handle_buzzer();
void buzzer_init();
#endif /* GENERALIO_BUZZER_H_ */
