/*
 * oled.c
 *
 *  Created on: Aug 1, 2017
 *      Author: jose
 */


#include <stdlib.h>
#include "oled.h"

static screen_t *screens = NULL;
static screen_t *current_screen;

screen_t *oled_addScreen(uint8_t index) {
	screen_t *ret = malloc(sizeof(screen_t));
	if(!ret)
		Error_Handler();
	ret->index = index;
	ret->next_screen = NULL;
	ret->init = NULL;
	ret->draw = NULL;
	ret->onExit = NULL;
	ret->onEnter = NULL;
	ret->processInput = NULL;
	ret->widgets = NULL;
	ret->current_widget = NULL;
	if(screens == NULL) {
		screens = ret;
	}
	else {
		screen_t *temp = screens;
		while(temp->next_screen) {
			temp = temp->next_screen;
		}
		temp->next_screen = ret;
	}
	return ret;
}

void oled_draw() {
	current_screen->draw(current_screen);
	UG_Update();
	update_display();
}

void oled_update() {
	if(current_screen->update)
		current_screen->update(current_screen);
}

void oled_init() {
	screen_t *scr = screens;
	while(scr) {
		if(scr->index == 0) {
			scr->init(scr);
			current_screen = scr;
			return;
		}
	}
}

void oled_processInput(RE_Rotation_t input, RE_State_t *state) {
	int ret = current_screen->processInput(current_screen, input, state);
	if(ret != -1) {
		screen_t *scr = screens;
		while(scr) {
			if(scr->index == ret) {
				UG_FillScreen(C_BLACK);
				if(current_screen->onExit)
					current_screen->onExit(scr);
				if(scr->onEnter)
					scr->onEnter(current_screen);
				scr->init(scr);
				if(scr->update)
					scr->update(scr);
				current_screen = scr;
				return;
			}
			scr = scr->next_screen;
		}
	}
}
