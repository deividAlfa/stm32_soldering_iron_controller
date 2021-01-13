/*
 * widgets.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_WIDGETS_H_
#define GRAPHICS_GUI_WIDGETS_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rotary_encoder.h"
#include "ssd1306.h"
#include "ugui.h"
typedef enum widgetStateType {widget_idle, widget_selected, widget_edit, widget_error}widgetStateType;
typedef enum widgetFieldType {field_float, field_integer, field_integer16,field_uinteger16, field_int32, field_bmp, field_string}widgetFieldType;
typedef enum widgetTextJustifyType { justify_left=0, justify_right=1 }widgetTextJustifyType;
typedef enum widgetType {widget_combo, combo_Screen, combo_Option, combo_Action, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button}widgetType;
typedef struct widget_t widget_t;
typedef struct comboBox_item_t comboBox_item_t;

typedef struct selectable_widget_t {
	widgetStateType state;
	widgetStateType previous_state;
	int (*processInput)(widget_t*, RE_Rotation_t, RE_State_t *);
	int (*longPressAction)(widget_t*);
	int (*onEditAction)(widget_t*);
	int (*onSelectAction)(widget_t*);
	int tab;
} selectable_widget_t;

typedef struct comboBox_widget_t {
	selectable_widget_t selectable;
	comboBox_item_t *first;
	uint8_t currentScroll;
	comboBox_item_t *currentItem;
} comboBox_widget_t;

typedef struct comboBox_item_t {
	char *text;
	comboBox_item_t *next_item;
	struct{
		unsigned enabled:1;
		unsigned type:4;
	};
	union{
		widget_t *widget;
		uint8_t action_screen;
		int (*action)();
	};
} comboBox_item_t;

typedef struct displayOnly_widget_t {
	void * (*getData)();
	uint8_t type;
	uint8_t	number_of_dec;
	void (*update)(widget_t*);
	bool justify;
} displayOnly_widget_t;

typedef struct editable_t {
	displayOnly_widget_t inputData;
	selectable_widget_t selectable;
	void (*setData)(void *);
	uint16_t step;
	uint16_t big_step;
	uint32_t max_value;
	uint32_t min_value;
	uint8_t current_edit;
} editable_widget_t;

typedef struct multi_option_widget_t {
	editable_widget_t editable;
	uint8_t numberOfOptions;
	uint8_t defaultOption;
	uint8_t currentOption;
	char **options;
} multi_option_widget_t;

typedef struct bmp_wiget_t {
	UG_BMP_MONO* bmp;
} bmp_wiget_t;

typedef struct label_wiget_t {

} label_wiget_t;

typedef struct button_widget_t {
	int (*action)(widget_t*);
	selectable_widget_t selectable;
	UG_BMP_MONO* bmp;
} button_widget_t;


struct widget_t
{
	bool inverted;
	char* displayString;
	uint8_t dispStrlen;
	char* EndStr;
	uint8_t posX;
	uint8_t posY;
	const UG_FONT *font_size;
	widget_t *next_widget;
	bool enabled;
	uint8_t type;
	struct screen_t *parent;
	void (*draw)(widget_t*);
	union {
		label_wiget_t label;
		editable_widget_t editable;
		displayOnly_widget_t displayWidget;
		bmp_wiget_t displayBmp;
		multi_option_widget_t multiOptionWidget;
		comboBox_widget_t comboBoxWidget;
		button_widget_t buttonWidget;
	};
};

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *widget);
editable_widget_t * extractEditablePartFromWidget(widget_t *widget);
selectable_widget_t * extractSelectablePartFromWidget(widget_t *widget);
void widgetDefaultsInit(widget_t *w, widgetType t, char* displaystring, char* endStr, uint8_t len);
void default_widgetDraw(widget_t *widget);
void default_widgetUpdate(widget_t *widget);
int default_widgetProcessInput(widget_t *, RE_Rotation_t, RE_State_t *);
int comboBoxProcessInput(widget_t *, RE_Rotation_t, RE_State_t *);
void comboBoxDraw(widget_t *widget);
void comboAddScreen(comboBox_item_t* item,widget_t *combo, char *label, uint8_t actionScreen);
void comboAddOption(comboBox_item_t* item, widget_t *combo, char *label, widget_t *widget);
void comboAddAction(comboBox_item_t* item, widget_t *combo, char *label, int (*action)());
void comboResetIndex(widget_t *combo);
#endif /* GRAPHICS_GUI_WIDGETS_H_ */
