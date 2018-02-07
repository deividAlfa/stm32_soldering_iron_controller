/*
 * debug_screen.h
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_DEBUG_SCREEN_H_
#define GRAPHICS_GUI_DEBUG_SCREEN_H_

#include "screen.h"

void debug_screen_setup(screen_t *scr);
void debug_screen2_setup(screen_t *scr);
void setPWM_tim(TIM_HandleTypeDef *);
#endif /* GRAPHICS_GUI_DEBUG_SCREEN_H_ */
