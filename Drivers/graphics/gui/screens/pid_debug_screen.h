/*
 * pid_debug_screen.h
 *
 *  Created on: 30 jul. 2021
 *      Author: David
 */

#ifndef GRAPHICS_GUI_SCREENS_PID_DEBUG_SCREEN_H_
#define GRAPHICS_GUI_SCREENS_PID_DEBUG_SCREEN_H_

#define ENABLE_PID_DEBUG_SCREEN

#include "screen.h"


extern screen_t Screen_pid_debug;


void pid_debug_screen_setup(screen_t *scr);



#endif /* GRAPHICS_GUI_SCREENS_PID_DEBUG_SCREEN_H_ */
