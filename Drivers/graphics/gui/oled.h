/*
 * oled.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_DISPLAY_H_
#define GRAPHICS_GUI_DISPLAY_H_

#include "screen.h"
extern screen_t *current_screen;
extern uint32_t current_time;
extern uint32_t screen_timer;
extern uint8_t last_scr;

void oled_destroy_screen(screen_t *scr);
void oled_backup_comboStatus(screen_t *scr);
void oled_restore_comboStatus(screen_t *scr);

void oled_addScreen(screen_t *screen, screens_t index);
void oled_draw(void);
void oled_init(RE_Rotation_t (*Rotation)(RE_State_t*), RE_State_t *State);
void oled_processInput(void);
void oled_update(void);
void oled_handle(void);
#endif /* GRAPHICS_GUI_DISPLAY_H_ */
