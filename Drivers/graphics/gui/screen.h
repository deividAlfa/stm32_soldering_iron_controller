/*
 * screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_SCREEN_H_
#define GRAPHICS_GUI_SCREEN_H_

//#define ENABLE_DEBUG_SCREEN

#include "main.h"
#include "iron.h"
#include "pid.h"
#include "settings.h"
#include "ssd1306.h"

#include "widgets.h"
#include "adc_global.h"
#include "buzzer.h"
#include "rotary_encoder.h"
#include "tempsensors.h"
#include "voltagesensors.h"

enum {	screen_boot,
		screen_main,
		screen_settingsmenu,
		screen_pid,
		screen_iron,
		screen_system,
			screen_reset,
				screen_reset_confirmation,
		screen_edit_iron_tips,
			screen_edit_tip_name,
		screen_edit_calibration_wait,
			screen_edit_calibration_input,
#ifdef ENABLE_DEBUG_SCREEN
		screen_debug,
			screen_debug2,
#endif
		};

typedef struct screen_t screen_t;
enum{ screen_idle=0, screen_refresh, screen_eraseAndRefresh, screen_blankRefresh};
struct screen_t
{
	struct screen_t *next_screen;
	widget_t *widgets;
	widget_t *current_widget;
	bool enabled;
	uint8_t refresh;
	int (*processInput)(struct screen_t *scr, RE_Rotation_t input, RE_State_t *);
	void (*update)(screen_t *scr);
	void (*draw)(screen_t *scr);
	void (*onExit)(screen_t *scr);
	void (*onEnter)(screen_t *scr);
	uint8_t index;
	void (*init)(screen_t *scr);
};

screen_t Screen_boot;
screen_t Screen_main;
screen_t Screen_settingsmenu;
screen_t Screen_pid;
screen_t Screen_iron;
screen_t Screen_system;
	screen_t Screen_reset;
		screen_t Screen_reset_confirmation;
screen_t Screen_edit_iron_tips;
	screen_t Screen_edit_tip_name;
screen_t Screen_edit_calibration_wait;
	screen_t Screen_edit_calibration_input;
#ifdef ENABLE_DEBUG_SCREEN
screen_t Screen_debug;
	screen_t Screen_debug2;
#endif

widget_t *screen_tabToWidget(screen_t * scr, uint8_t tab);
void screen_addWidget(widget_t *widget, screen_t *scr);
void default_screenDraw(screen_t *scr);
int default_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *);
void default_screenUpdate(screen_t *scr);
void screen_setDefaults(screen_t *scr);
void default_init(screen_t *scr);

#endif /* GRAPHICS_GUI_SCREEN_H_ */
