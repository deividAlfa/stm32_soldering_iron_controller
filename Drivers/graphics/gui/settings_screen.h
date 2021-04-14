/*
 * debug_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef SETTINGS_GUI_DEBUG_SCREEN_H_
#define SETTINGS_GUI_DEBUG_SCREEN_H_

#include "screen.h"


extern screen_t Screen_settingsmenu;
extern screen_t Screen_pid;
extern screen_t Screen_iron;
extern screen_t Screen_system;
extern screen_t Screen_reset;
extern screen_t Screen_reset_confirmation;
extern screen_t Screen_edit_iron_tips;
extern screen_t Screen_edit_tip_name;

void settings_screen_setup(screen_t *scr);

#endif /* SETTINGS_GUI_DEBUG_SCREEN_H_ */
