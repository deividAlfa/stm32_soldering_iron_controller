/*
 * calibration_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_CALIBRATION_SCREEN_H_
#define GRAPHICS_GUI_CALIBRATION_SCREEN_H_
#include "screen.h"


extern screen_t Screen_edit_calibration;
extern screen_t Screen_edit_calibration_start;
extern screen_t Screen_edit_calibration_adjust;
extern screen_t Screen_edit_calibration_input;

void calibration_screen_setup(screen_t *scr);
void cal_screenUpdate(screen_t *scr);
void editcalibration_screenDraw(screen_t *scr);
#endif /* GRAPHICS_GUI_CALIBRATION_SCREEN_H_ */
