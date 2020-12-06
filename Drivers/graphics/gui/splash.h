/*
 * splash.h
 *
 *  Created on: Aug 7, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_SPLASH_H_
#define GRAPHICS_GUI_SPLASH_H_

#include "screen.h"

void splash_setup(screen_t * scr);
int splash_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *);
void splash_init(screen_t * scr);
void splash_draw(screen_t * scr);
#endif /* GRAPHICS_GUI_SPLASH_H_ */
