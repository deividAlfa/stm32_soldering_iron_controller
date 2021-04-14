/*
 * boot_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_BOOT_H_
#define GRAPHICS_GUI_BOOT_H_
#include "screen.h"


extern screen_t Screen_boot;


void boot_screen_setup(screen_t * scr);
int boot_screen_processInput(screen_t * scr, RE_Rotation_t input, RE_State_t *);
void boot_screen_init(screen_t * scr);
void boot_screen_draw(screen_t * scr);
#endif /* GRAPHICS_GUI_BOOT_H_ */
