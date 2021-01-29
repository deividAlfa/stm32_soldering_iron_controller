/*
 * gui.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_GUI_H_
#define GRAPHICS_GUI_GUI_H_

#include "oled.h"
#include "screen.h"
#include "gui_strings.h"

extern u8g2_t u8g2;

void guiInit(void);
void guiDraw();
#endif /* GRAPHICS_GUI_GUI_H_ */
