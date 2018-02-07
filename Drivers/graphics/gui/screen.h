/*
 * screen.h
 *
 *  Created on: Jul 31, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_SCREEN_H_
#define GRAPHICS_GUI_SCREEN_H_

#include "../generalIO/rotary_encoder.h"
#include "ssd1306.h"
#include "ugui.h"
#include "widgets.h"

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

	widget_t *screen_tabToWidget(screen_t * scr, uint8_t tab);
	widget_t *screen_addWidget(screen_t * scr);
	void default_screenDraw(screen_t *scr);
	int default_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *);
	void default_screenUpdate(screen_t *scr);
	void default_init(screen_t *scr);

#endif /* GRAPHICS_GUI_SCREEN_H_ */
