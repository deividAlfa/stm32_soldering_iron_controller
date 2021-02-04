/*
 * gui.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "boot_screen.h"
#include "gui.h"
#include "main_screen.h"
#include "settings_screen.h"
#include "calibration_screen.h"
#include "oled.h"
#include "screen.h"
#ifdef ENABLE_DEBUG_SCREEN
#include "debug_screen.h"
#endif

u8g2_t u8g2;

void guiInit(void) {

	u8g2_SetupDisplay(&u8g2, u8x8_d_ssd1306_128x64_noname, u8x8_cad_001, u8x8_dummy_cb, u8x8_dummy_cb);	// Use 128x64 ssd1306 settings, dummy functions (u8g2 won't send data to screen)
	u8g2_SetupBuffer(&u8g2, oled.buffer, 8, u8g2_ll_hvline_vertical_top_lsb, U8G2_R0);					//

	u8g2_SetFontMode(&u8g2,1);																// Set font transparent
	u8g2_SetFontDirection(&u8g2, 0);														// No rotation
	u8g2_SetFontPosTop(&u8g2);																// Take upper font ref. as start drawing position

	oled_addScreen(&Screen_boot, screen_boot);
	boot_screen_setup(&Screen_boot);

	oled_addScreen(&Screen_main,screen_main);
	main_screen_setup(&Screen_main);

	oled_addScreen(&Screen_settingsmenu,screen_settingsmenu);
	settings_screen_setup(&Screen_settingsmenu);
	//settings_screen_setup();
#ifdef ENABLE_DEBUG_SCREEN

	oled_addScreen(&Screen_debug,screen_debug);
	debug_screen_setup(&Screen_debug);

	oled_addScreen(&Screen_debug2,screen_debug2);
	debug2_screen_setup(&Screen_debug2);
#endif
	oled_addScreen(&Screen_edit_calibration_wait,screen_edit_calibration_wait);
	calibration_screen_setup(&Screen_edit_calibration_wait);
}
