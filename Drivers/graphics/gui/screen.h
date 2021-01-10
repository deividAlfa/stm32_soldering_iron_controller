/*
 * screen.h
 *
 *  Created on: Jul 31, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_SCREEN_H_
#define GRAPHICS_GUI_SCREEN_H_

#include "main.h"
#include "iron.h"
#include "pid.h"
#include "settings.h"
#include "ssd1306.h"
#include "ugui.h"
#include "widgets.h"
#include "adc_global.h"
#include "buzzer.h"
#include "rotary_encoder.h"
#include "tempsensors.h"
#include "voltagesensors.h"

enum {	screen_splash, screen_main, screen_settingsmenu, screen_last_scrollable,
		screen_debug, screen_debug2, screen_pid, screen_system, screen_iron,
		screen_advanced, screen_edit_pwm, screen_edit_detection,screen_edit_misc,screen_irontype, screen_edit_iron_tips,
		screen_edit_iron_tip, screen_edit_tip_name, screen_edit_calibration_wait,screen_edit_test_opts, screen_edit_calibration_input, screen_reset};

typedef struct screen_t screen_t;

struct screen_t
{
	struct screen_t *next_screen;
	widget_t *widgets;
	widget_t *current_widget;
	uint8_t enabled;
	int (*processInput)(struct screen_t *scr, RE_Rotation_t input, RE_State_t *);
	void (*update)(screen_t *scr);
	void (*draw)(screen_t *scr);
	void (*onExit)(screen_t *scr);
	void (*onEnter)(screen_t *scr);
	uint8_t index;
	void (*init)(screen_t *scr);
};
screen_t Screen_splash;
screen_t Screen_main;
screen_t Screen_settingsmenu;
screen_t Screen_last_scrollable;
screen_t Screen_pid;
screen_t Screen_system;
screen_t Screen_iron;
screen_t Screen_advanced;
screen_t Screen_irontype;
screen_t Screen_edit_iron_tips;
screen_t Screen_edit_iron_tip;
screen_t Screen_edit_tip_name;
screen_t Screen_edit_calibration_wait;
screen_t Screen_edit_calibration_input;
screen_t Screen_debug;
screen_t Screen_debug2;
screen_t Screen_reset;

widget_t *screen_tabToWidget(screen_t * scr, uint8_t tab);
void screen_addWidget(widget_t *widget, screen_t *scr);
void default_screenDraw(screen_t *scr);
int default_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *);
void default_screenUpdate(screen_t *scr);
void default_init(screen_t *scr);

#endif /* GRAPHICS_GUI_SCREEN_H_ */
