/*
 * calibration_screen.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_CALIBRATION_SCREEN_H_
#define GRAPHICS_GUI_CALIBRATION_SCREEN_H_
#include "screen.h"

typedef enum { cal_250=0, cal_400=1, cal_0=2, cal_input_250=10, cal_input_400=11, cal_finished=20, cal_failed=21, cal_needsAdjust=22 }state_t;
extern const int16_t state_temps[2];

extern screen_t Screen_calibration;
extern screen_t Screen_calibration_start;
extern screen_t Screen_calibration_settings;

void calibration_screen_setup(screen_t *scr);
void cal_screenUpdate(screen_t *scr);
void editcalibration_screenDraw(screen_t *scr);
#endif /* GRAPHICS_GUI_CALIBRATION_SCREEN_H_ */
