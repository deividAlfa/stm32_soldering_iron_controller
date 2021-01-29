/*
 * widgets.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_WIDGETS_H_
#define GRAPHICS_GUI_WIDGETS_H_

#include "rotary_encoder.h"
#define default_font u8g2_font_t0_16_tr
//u8g2_font_helvR10_tr //ok
//u8g2_font_t0_16_tr //bien
//u8g2_font_DigitalDisco_tr
//u8g2_font_timB10_tr
//u8g2_font_timB12_tr

typedef enum widgetStateType {widget_idle, widget_selected, widget_edit, widget_error}widgetStateType;
typedef enum widgetFieldType {field_int32, field_bmp, field_string}widgetFieldType;
typedef enum AlignType { align_disabled, align_left, align_center, align_right }AlignType;
typedef enum widgetFrameType {frame_auto, frame_solid, frame_outline, frame_disabled, frame_combo}widgetFrameType;
typedef enum widgetType {widget_combo, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button}widgetType;
typedef enum comboType {combo_Screen, combo_Option, combo_Action}comboType;
typedef enum widgetRefreshType {refresh_idle, refresh_triggered, refresh_always}widgetRefreshType;
typedef struct widget_t widget_t;
typedef struct comboBox_item_t comboBox_item_t;

typedef struct selectable_widget_t {
	widgetStateType state;
	widgetStateType previous_state;
	int (*processInput)(widget_t*, RE_Rotation_t, RE_State_t *);
	int (*longPressAction)(widget_t*);
	uint8_t tab;
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
	bool enabled;
	comboType type;
	AlignType dispAlign;
	union{
		widget_t *widget;
		uint8_t action_screen;
		int (*action)();
	};
} comboBox_item_t;

typedef struct displayOnly_widget_t {
	void * (*getData)();
	uint8_t number_of_dec;
	uint8_t reservedChars;
	widgetFieldType type;
} displayOnly_widget_t;

typedef struct editable_t {
	displayOnly_widget_t inputData;
	selectable_widget_t selectable;
	void (*setData)(void *);
	uint16_t step;
	uint16_t big_step;
	int32_t min_value;
	int32_t max_value;
	uint8_t current_edit;
} editable_widget_t;

typedef struct multi_option_widget_t {
	editable_widget_t editable;
	uint8_t numberOfOptions;
	char **options;
} multi_option_widget_t;


typedef struct button_widget_t {
	int (*action)(widget_t*);
	selectable_widget_t selectable;
} button_widget_t;


struct widget_t
{
	widget_t *next_widget;
	uint8_t posX;
	uint8_t posY;
	uint8_t width;
	bool enabled;
	widgetRefreshType refresh;
	widgetFrameType frameType;
	widgetType type;
	int8_t radius;
	union{
		struct{
			AlignType dispAlign;
			AlignType textAlign;
			uint8_t stringPos;
			char* displayString;
			char* endString;
			const uint8_t *font;
			int32_t last_value;
		};
		struct{
			const uint8_t* xbm;
			const uint8_t* last_xbm;
		};
	};
	struct screen_t *parent;
	void (*draw)(widget_t*);
	void (*update)(widget_t*);
	union {
		editable_widget_t editableWidget;
		displayOnly_widget_t displayWidget;
		multi_option_widget_t multiOptionWidget;
		comboBox_widget_t comboBoxWidget;
		button_widget_t buttonWidget;
	};
};

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w);
editable_widget_t * extractEditablePartFromWidget(widget_t *);
selectable_widget_t * extractSelectablePartFromWidget(widget_t *w);
void widgetAlign(widget_t* w);
void widgetDefaultsInit(widget_t* w, widgetType t);
void default_widgetDraw(widget_t* w);
void default_widgetUpdate(widget_t *w);
void widgetDetectChange(widget_t* w, int32_t val);
void widgetClearField(widget_t* w);
void widgetPrintVal(widget_t* w, int32_t val_ui);
int32_t widgetGetVal(widget_t* w);
void widgetEnable(widget_t* w);
void widgetDisable(widget_t* w);
int default_widgetProcessInput(widget_t* w, RE_Rotation_t, RE_State_t *);
int comboBoxProcessInput(widget_t* w, RE_Rotation_t, RE_State_t *);
void comboBoxDraw(widget_t *w);
void comboAddScreen(comboBox_item_t* item, widget_t *combo, char *label, uint8_t actionScreen);
void comboAddOption(comboBox_item_t* item, widget_t *combo, char *label, widget_t* w);
void comboAddAction(comboBox_item_t* item, widget_t *combo, char *label, int (*action)());
void comboResetIndex(widget_t *combo);
int32_t strsum(char* str);
#endif /* GRAPHICS_GUI_WIDGETS_H_ */
