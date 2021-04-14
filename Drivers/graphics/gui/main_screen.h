/*
 * main_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_MAIN_SCREEN_H_
#define GRAPHICS_GUI_MAIN_SCREEN_H_

#include "screen.h"



extern screen_t Screen_main;

void main_screen_setup(screen_t *scr);
void main_screen_draw(screen_t *scr);
extern volatile uint16_t seconds2;
extern volatile uint16_t count;
#endif /* GRAPHICS_GUI_MAIN_SCREEN_H_ */
