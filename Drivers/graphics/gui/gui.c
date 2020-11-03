/*
 * gui.c
 *
 *  Created on: Aug 1, 2017
 *      Author: jose
 */

#include "gui.h"
#include "../ugui.h"
#include "main_screen.h"
#include "debug_screen.h"
#include "debug_screen2.h"
#include "splash.h"
#include "settings_screen.h"
#include "calibration_screen.h"
#include "oled.h"
#include "../ssd1306.h"


/* To debug free ram
 *
 *  oled_addscreen calls several malloc(), each screen can take from ~400 bytes up to a few KB!
 *  So if you get Hard faults after touching the screens, check this!
*/

/*
uint16_t freeram(void){
	 uint16_t ram=1;
	 uint8_t* ptr;
	 while(ram<20000){
		 ptr=malloc(ram);
		 if(ptr==NULL){
			 return ram;
		 }
		 else{
			 free(ptr);
			 ram+=100;
		 }
	  }
}
*/

void guiInit(TIM_HandleTypeDef *tim) {
	screen_t *scr;
	//volatile uint16_t  free;

	//free=freeram();

	scr = oled_addScreen(screen_splash);
	splash_setup(scr);
	//free=freeram();

	scr  = oled_addScreen(screen_main);
	main_screen_setup(scr);
	//free=freeram();

	scr = oled_addScreen(screen_debug);
	debug_screen_setup(scr);
	//free=freeram();

	scr = oled_addScreen(screen_debug2);
	debug_screen2_setup(scr);
	//free=freeram();

	scr = oled_addScreen(screen_settings);
	settings_screen_setup(scr);
	//free=freeram();

	scr = oled_addScreen(screen_edit_calibration_wait);
	calibration_screen_setup(scr);
	//free=freeram();

	UG_Init(&user_gui, pset, 128, 64);
	oled_init();
	oled_draw();
	UG_Update();
	update_display();
    setPWM_tim(tim);
}
