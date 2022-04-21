/*
 * addon_switch_off_reminder_settings_screen.h
 *
 *  Created on: 2022. ápr. 21.
 *      Author: KocsisV
 */

#ifndef GRAPHICS_GUI_SCREENS_ADDON_SWITCH_OFF_REMINDER_SETTINGS_SCREEN_H_
#define GRAPHICS_GUI_SCREENS_ADDON_SWITCH_OFF_REMINDER_SETTINGS_SCREEN_H_

#include "screen.h"

#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER

extern screen_t Screen_switch_off_reminder_settings;

void addons_screen_switch_off_reminder_setup(screen_t *scr);

#endif /* ENABLE_ADDON_SWITCH_OFF_REMINDER */

#endif /* GRAPHICS_GUI_SCREENS_ADDON_SWITCH_OFF_REMINDER_SETTINGS_SCREEN_H_ */
