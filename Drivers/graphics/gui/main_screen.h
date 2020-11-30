/*
 * main_screen.h
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_MAIN_SCREEN_H_
#define GRAPHICS_GUI_MAIN_SCREEN_H_

#include "screen.h"

void main_screen_setup(screen_t *scr);
void main_screen_draw(screen_t *scr);
extern volatile uint16_t seconds2;
extern volatile uint16_t count;
#endif /* GRAPHICS_GUI_MAIN_SCREEN_H_ */
