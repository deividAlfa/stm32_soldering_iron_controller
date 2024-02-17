/*
 * buzzer.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GENERALIO_BUZZER_H_
#define GENERALIO_BUZZER_H_

#include "main.h"

#define SHORT_BEEP    (uint16_t)  50
#define MEDIUM_BEEP   (uint16_t)  300
#define LONG_BEEP     (uint16_t)  2000
#define ALARM_PERIOD  (uint16_t)  2000
#define ALARM_HIGH    (uint16_t)  10
#define ALARM_LOW     (uint16_t)  (ALARM_PERIOD-ALARM_HIGH)

void buzzer_beep(uint32_t duration);
void buzzer_force_beep(uint32_t duration);
void buzzer_alarm_start();
void buzzer_alarm_stop();
void handle_buzzer();
void buzzer_init();
#endif /* GENERALIO_BUZZER_H_ */
