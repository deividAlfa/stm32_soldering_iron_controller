/*
 * screens.h
 *
 *  Created on: Jul 30, 2021
 *      Author: David
 */

#ifndef _GUI_SCREENS_H_
#define _GUI_SCREENS_H_


#define ENABLE_DEBUG_SCREEN

#include "board.h"

#include "tip_list_screen.h"
#include "tip_settings_screen.h"
#include "boot_screen.h"
#include "main_screen.h"
#include "settings_screen.h"
#include "display_screen.h"
#include "iron_screen.h"
#include "system_screen.h"
#include "reset_screen.h"
#include "calibration_screen.h"
#include "debug_screen.h"
#ifdef ENABLE_ADDONS
#include "addons_screen.h"
#endif
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
#include "addon_fume_extractor_settings_screen.h"
#endif
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
#include <addon_switch_off_reminder_settings_screen.h>
#endif

#include "oled.h"
#include "gui.h"

#endif /* GRAPHICS_GUI_SCREENS_SCREENS_H_ */
