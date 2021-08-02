/*
 * screen_common.h
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#ifndef GRAPHICS_GUI_SCREENS_SCREEN_COMMON_H_
#define GRAPHICS_GUI_SCREENS_SCREEN_COMMON_H_


#include "settings.h"
#include "board.h"
#include "screens.h"
#include "oled.h"
#include "gui.h"

extern uint32_t settingsTimer;
extern int32_t temp;
extern uint8_t profile, Selected_Tip;
extern bool disableTipCopy;
extern char *tipName;
extern bool newTip;
extern bool troll_enabled;
int longClickReturn(widget_t *w);
int autoReturn_ProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state);


#endif /* GRAPHICS_GUI_SCREENS_SCREEN_COMMON_H_ */
