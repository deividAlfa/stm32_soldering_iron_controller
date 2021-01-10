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
#include "tiptype_screen.h"
#include "oled.h"
#include "screen.h"


void guiInit(void) {
	UG_Init(&user_gui, pset, 128, 64);

	oled_addScreen(&Screen_splash, screen_splash);
	splash_setup(&Screen_splash);

	oled_addScreen(&Screen_main,screen_main);
	main_screen_setup(&Screen_main);

	oled_addScreen(&Screen_settingsmenu,screen_settingsmenu);
	settings_screen_setup(&Screen_settingsmenu);

	oled_addScreen(&Screen_debug,screen_debug);
	debug_screen_setup(&Screen_debug);

	oled_addScreen(&Screen_debug2,screen_debug2);
	debug2_screen_setup(&Screen_debug2);

	oled_addScreen(&Screen_edit_calibration_wait,screen_edit_calibration_wait);
	calibration_screen_setup(&Screen_edit_calibration_wait);

	oled_addScreen(&Screen_tiptype,screen_tiptype);
	tiptype_screen_setup(&Screen_tiptype);
}
