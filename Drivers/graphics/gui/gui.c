/*
 * gui.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "screen.h"
#include "screens.h"
#include "oled.h"
#include "gui.h"

u8g2_t u8g2;

void guiInit(void) {

  u8g2_SetupDisplay(&u8g2, u8x8_d_ssd1306_128x64_noname, u8x8_cad_001, u8x8_dummy_cb, u8x8_dummy_cb);  // Use 128x64 ssd1306 settings, dummy functions (u8g2 won't send data to screen)
  u8g2_SetupBuffer(&u8g2, oled.buffer, 8, u8g2_ll_hvline_vertical_top_lsb, U8G2_R0);          //

  u8g2_SetFontMode(&u8g2,1);                                  // Set font transparent
  u8g2_SetFontDirection(&u8g2, 0);                            // No rotation
  u8g2_SetFontPosTop(&u8g2);                                  // Take upper font ref. as start drawing position
  oled_addScreen(&Screen_boot, screen_boot);
  boot_screen_setup(&Screen_boot);

  oled_addScreen(&Screen_main,screen_main);
  main_screen_setup(&Screen_main);

  oled_addScreen(&Screen_settings,screen_settings);
  settings_screen_setup(&Screen_settings);

  oled_addScreen(&Screen_iron,screen_iron);
  iron_screen_setup(&Screen_iron);

  oled_addScreen(&Screen_system,screen_system);
  system_screen_setup(&Screen_system);

  oled_addScreen(&Screen_reset,screen_reset);
  reset_screen_setup(&Screen_reset);

  oled_addScreen(&Screen_tip_list,screen_tip_list);
  tip_list_screen_setup(&Screen_tip_list);

  oled_addScreen(&Screen_tip_settings, screen_tip_settings);
  tip_settings_screen_setup(&Screen_tip_settings);

  oled_addScreen(&Screen_calibration,screen_calibration);
  calibration_screen_setup(&Screen_calibration);

#ifdef ENABLE_DEBUG_SCREEN

  oled_addScreen(&Screen_debug,screen_debug);
  debug_screen_setup(&Screen_debug);

#endif
}
