/*
 * oled.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */


#include <stdlib.h>
#include "oled.h"

static screen_t *screens = NULL;
static screen_t *current_screen;
static RE_State_t* RE_State;

RE_Rotation_t (*RE_GetData)(RE_State_t*);
RE_Rotation_t RE_Rotation;


void oled_addScreen(screen_t *screen, uint8_t index) {
	screen->index = index;
	screen->next_screen = NULL;
	screen->init = NULL;
	screen->draw = NULL;
	screen->onExit = NULL;
	screen->onEnter = NULL;
	screen->processInput = NULL;
	screen->widgets = NULL;
	screen->current_widget = NULL;
	if(screens == NULL) {
		screens = screen;
	}
	else {
		screen_t *temp = screens;
		while(temp->next_screen) {
			temp = temp->next_screen;
		}
		temp->next_screen = screen;
	}
}

void oled_draw() {

#ifndef Soft_SPI
	if(oled.status!=oled_idle) { return; }		// If Oled busy, skip update
#endif
	current_screen->draw(current_screen);
	update_display();
}

void oled_update() {
	if(current_screen->update)
		current_screen->update(current_screen);
	oled_draw();
}

void oled_init(RE_Rotation_t (*GetData)(RE_State_t*), RE_State_t *State) {
	RE_State = State;
	RE_GetData = GetData;
	screen_t *scr = screens;
	while(scr) {
		if(scr->index == 0) {
			scr->init(scr);
			current_screen = scr;
			return;
		}
	}
}
static RE_State_t* RE_State;


void oled_processInput(void) {
	RE_Rotation = (*RE_GetData)(RE_State);
	int ret = current_screen->processInput(current_screen, RE_Rotation, RE_State);
	if(ret != -1) {
		screen_t *scr = screens;
		while(scr) {
			if(scr->index == ret) {
				FillBuffer(BLACK, fill_dma);
				scr->refresh=screenRefresh_alreadyErased;			// Changed screen, force full display erase using dma
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
void oled_handle(void){
	oled_processInput();
	oled_update();
}
