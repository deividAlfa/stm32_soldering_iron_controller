/*
 * widgets.h
 *
 *  Created on: Aug 11, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_GUI_WIDGETS_H_
#define GRAPHICS_GUI_WIDGETS_H_

#include <stdlib.h>
#include "string.h"
#include <math.h>

#include "../generalIO/rotary_encoder.h"
#include "ssd1306.h"
#include "ugui.h"

typedef enum widgetStateType {widget_idle, widget_selected, widget_edit, widget_error}widgetStateType;
typedef enum widgetFieldType {field_float, field_integer, field_uinteger16, field_bmp, field_string}widgetFieldType;
typedef enum widgetType {widget_combo, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button}widgetType;
enum {screen_splash, screen_main, screen_settings, screen_debug, screen_debug2, screen_last_scrollable, screen_edit_pid, screen_edit_sleep, screen_edit_contrast, screen_edit_boost, screen_edit_iron_tips, screen_edit_iron_tip, screen_edit_tip_name, screen_edit_calibration_wait, screen_edit_calibration_input, screen_LAST};

typedef struct widget_t widget_t;
typedef struct comboBox_item_t comboBox_item_t;

typedef struct selectable_widget_t {
	widgetStateType state;
	widgetStateType previous_state;
	int (*processInput)(widget_t*, RE_Rotation_t, RE_State_t *);
	int (*longPressAction)(widget_t*);
	int tab;
} selectable_widget_t;
typedef struct comboBox_widget_t {
	selectable_widget_t selectable;
	comboBox_item_t *items;
	uint8_t currentScroll;
	comboBox_item_t *currentItem;
} comboBox_widget_t;

typedef struct comboBox_item_t {
	char *text;
	comboBox_item_t *next_item;
	uint8_t action_screen;
	uint8_t enabled;
} comboBox_item_t;

typedef struct displayOnly_widget_t {
	void * (*getData)();
	widgetFieldType type;
	uint8_t	number_of_dec;
	void (*update)(widget_t*);
} displayOnly_wiget_t;

typedef struct editable_t {
	displayOnly_wiget_t inputData;
	selectable_widget_t selectable;
	void (*setData)(void *);
	uint8_t step;
	uint8_t big_step;
	uint16_t max_value;
	uint16_t min_value;
	uint8_t current_edit;
} editable_wiget_t;

typedef struct multi_option_widget_t {
	editable_wiget_t editable;
	uint8_t numberOfOptions;
	uint8_t defaultOption;
	uint8_t currentOption;
	char **options;
} multi_option_widget_t;

typedef struct bmp_wiget_t {
	UG_BMP bmp;
} bmp_wiget_t;

typedef struct label_wiget_t {

} label_wiget_t;

typedef struct button_widget_t {
	int (*action)(widget_t*);
	selectable_widget_t selectable;
} button_widget_t;

struct widget_t
{
	uint8_t inverted;
	char displayString[20];
	uint8_t reservedChars;
	uint8_t posX;
	uint8_t posY;
	const UG_FONT *font_size;
	widget_t *next_widget;
	uint8_t enabled;
	widgetType type;
	struct screen_t *parent;
	void (*draw)(widget_t*);
	union {
		label_wiget_t label;
		editable_wiget_t editable;
		displayOnly_wiget_t displayWidget;
		bmp_wiget_t displayBmp;
		multi_option_widget_t multiOptionWidget;
		comboBox_widget_t comboBoxWidget;
		button_widget_t buttonWidget;
	};
};

displayOnly_wiget_t * extractDisplayPartFromWidget(widget_t *widget);
editable_wiget_t * extractEditablePartFromWidget(widget_t *widget);
selectable_widget_t * extractSelectablePartFromWidget(widget_t *widget);
void widgetDefaultsInit(widget_t *w, widgetType t);
void default_widgetDraw(widget_t *widget);
void default_widgetUpdate(widget_t *widget);
int default_widgetProcessInput(widget_t *, RE_Rotation_t, RE_State_t *);
int comboBoxProcessInput(widget_t *, RE_Rotation_t, RE_State_t *);
void comboBoxDraw(widget_t *widget);
comboBox_item_t *comboAddItem(widget_t *combo, char *label, uint8_t actionScreen);

#endif /* GRAPHICS_GUI_WIDGETS_H_ */
