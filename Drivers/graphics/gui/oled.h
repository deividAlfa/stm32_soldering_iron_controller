/*
 * oled.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_OLED_H_
#define GRAPHICS_GUI_OLED_H_

#include "screen.h"

void oled_addScreen(screen_t *screen, uint8_t index);
void oled_draw(void);
void oled_init(RE_Rotation_t (*Rotation)(RE_State_t*), RE_State_t *State);
void oled_processInput(void);
void oled_update(void);
void oled_handle(void);
#endif /* GRAPHICS_GUI_OLED_H_ */
