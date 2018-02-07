/*
 * gui.c
 *
 *  Created on: Aug 1, 2017
 *      Author: jose
 */

#include "gui.h"
#include "main_screen.h"
#include "debug_screen.h"
#include "splash.h"
#include "settings_screen.h"
#include "calibration_screen.h"

void guiInit() {
	screen_t *scr = oled_addScreen(screen_splash);
	splash_setup(scr);
	scr = oled_addScreen(screen_main);
	main_screen_setup(scr);
	scr = oled_addScreen(screen_debug);
	debug_screen_setup(scr);
	scr = oled_addScreen(screen_debug2);
	debug_screen2_setup(scr);
	scr = oled_addScreen(screen_settings);
	settings_screen_setup(scr);
	scr = oled_addScreen(screen_edit_calibration_wait);
	calibration_screen_setup(scr);
}
