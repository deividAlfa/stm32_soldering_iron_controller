/*
 * debug_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_DEBUG_SCREEN_H_
#define GRAPHICS_GUI_DEBUG_SCREEN_H_

#include "screen.h"

// Uncomment to enable debug menu

//#define ENABLE_DEBUG_SCREEN


extern screen_t Screen_debug;
extern screen_t Screen_debug2;

void debug_screen_setup(screen_t *scr);
void debug2_screen_setup(screen_t *scr);

#endif /* GRAPHICS_GUI_DEBUG_SCREEN_H_ */
