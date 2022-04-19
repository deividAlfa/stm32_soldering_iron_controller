/*
 * addons_screen.h
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#ifndef GRAPHICS_GUI_SCREENS_ADDONS_SCREEN_H_
#define GRAPHICS_GUI_SCREENS_ADDONS_SCREEN_H_

#include "screen.h"

#ifdef ENABLE_ADDONS

extern screen_t Screen_addons;

void addons_screen_setup(screen_t *scr);

#endif /* ENABLE_ADDONS */
#endif /* GRAPHICS_GUI_SCREENS_ADDONS_SCREEN_H_ */
