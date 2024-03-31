/*
 * debug_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef _GUI_DEBUG_SCREEN_H_
#define _GUI_DEBUG_SCREEN_H_


#define ENABLE_DEBUG_SCREEN


#include "screen.h"

#ifdef ENABLE_DEBUG_SCREEN
extern screen_t Screen_debug;

void debug_screen_setup(screen_t *scr);
void debug2_screen_setup(screen_t *scr);

#endif

#endif /* GRAPHICS_GUI_DEBUG_SCREEN_H_ */
