/*
 * addon_fume_extractor_settings_screen.h
 *
 *  Created on: 2022. ápr. 19.
 *      Author: KocsisV
 */

#ifndef GRAPHICS_GUI_SCREENS_ADDON_FUME_EXTRACTOR_SETTINGS_SCREEN_H_
#define GRAPHICS_GUI_SCREENS_ADDON_FUME_EXTRACTOR_SETTINGS_SCREEN_H_

#include "screen.h"

#ifdef ENABLE_ADDON_FUME_EXTRACTOR

extern screen_t Screen_fume_extractor_settings;

void addons_screen_fume_extractor_setup(screen_t *scr);

#endif /* ENABLE_ADDON_FUME_EXTRACTOR */
#endif /* GRAPHICS_GUI_SCREENS_ADDON_FUME_EXTRACTOR_SETTINGS_SCREEN_H_ */
