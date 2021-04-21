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
// Just some fonts I tried and wanted to remember
//u8g2_font_helvR10_tr  // ok
//u8g2_font_t0_16_tr    // meh
//u8g2_font_DigitalDisco_tr
//u8g2_font_timB10_tr
//u8g2_font_timB12_tr

typedef enum widgetStateType {widget_idle, widget_selected, widget_edit, widget_error}widgetStateType;
typedef enum widgetFieldType {field_int32, field_bmp, field_string}widgetFieldType;
typedef enum AlignType { align_disabled, align_left, align_center, align_right }AlignType;
typedef enum widgetFrameType {frame_auto, frame_solid, frame_outline, frame_disabled, frame_combo}widgetFrameType;
typedef enum widgetType {widget_combo, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button}widgetType;
typedef enum comboType {combo_Screen, combo_Editable, combo_MultiOption, combo_Action}comboType;
typedef enum widgetRefreshType {refresh_idle, refresh_triggered, refresh_always}widgetRefreshType;
typedef struct widget_t widget_t;
typedef struct comboWidget_t comboWidget_t;
typedef struct comboBox_item_t comboBox_item_t;
typedef struct selectable_widget_t selectable_widget_t;
typedef struct editable_widget_t editable_widget_t;
typedef struct button_widget_t button_widget_t;
typedef struct displayOnly_widget_t displayOnly_widget_t;
typedef struct comboBox_widget_t comboBox_widget_t;
typedef struct bmp_widget_t bmp_widget_t;

struct selectable_widget_t {
	widgetStateType state;
	widgetStateType previous_state;
	uint8_t tab;
	int (*processInput)(widget_t*, RE_Rotation_t, RE_State_t *);
	int (*longPressAction)(widget_t*);
};

struct displayOnly_widget_t {
  widgetFieldType type;
  AlignType dispAlign;
  AlignType textAlign;
  int32_t last_value;
	uint8_t number_of_dec;
	uint8_t reservedChars;
  uint8_t stringStart;
  const uint8_t *font;
  char* displayString;
  char* endString;
	void * (*getData)();
};

struct editable_widget_t {
	int8_t current_edit;
	uint8_t numberOfOptions;
	int16_t step;
	int16_t big_step;
	int32_t min_value;
	int32_t max_value;
	char **options;
	void (*setData)(void *);
	selectable_widget_t selectable;
	displayOnly_widget_t inputData;
};

struct button_widget_t {
  union{
    struct{
      char* displayString;
      const uint8_t* font;
      int32_t last_value;
      uint8_t stringStart;
    };
    struct{
      const uint8_t* xbm;
      const uint8_t* last_xbm;
    };
  };
	int (*action)(widget_t*);
	selectable_widget_t selectable;
};

struct bmp_widget_t {
  const uint8_t* xbm;
  const uint8_t* last_xbm;
};

struct comboBox_widget_t {
  const uint8_t* font;
  selectable_widget_t selectable;
  comboBox_item_t *first;
  uint8_t currentScroll;
  comboBox_item_t *currentItem;
} ;

struct comboBox_item_t {
  comboBox_item_t *next_item;
  comboType type;
  uint8_t enabled;
  AlignType dispAlign;
  union{
    uint8_t action_screen;
    int (*action)();
    editable_widget_t *widget;
  };
  char *text;
};

struct widget_t
{
	widgetType type;
	widget_t *next_widget;
	widgetRefreshType refresh;
	widgetFrameType frameType;
	uint8_t posX;
	uint8_t posY;
	uint8_t width;
	uint8_t enabled;
	int8_t radius;

	struct screen_t *parent;
	void (*draw)(widget_t*);
	void (*update)(widget_t*);
	void* content;
};

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w);
editable_widget_t * extractEditablePartFromWidget(widget_t *);
selectable_widget_t * extractSelectablePartFromWidget(widget_t *w);
void widgetAlign(widget_t* w);
void widgetDefaultsInit(widget_t *w, widgetType t, void* content);
void editableDefaultsInit(editable_widget_t* editable, widgetType type);
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
//void comboAddEditable(comboBox_item_t* item, widget_t *combo, char *label, widget_t* w);
void comboAddEditable(comboBox_item_t* item, widget_t *combo, char *label, editable_widget_t *editable);
void comboAddMultiOption(comboBox_item_t* item, widget_t *combo, char *label, editable_widget_t *editable);
void comboAddAction(comboBox_item_t* item, widget_t *combo, char *label, int (*action)());
void comboResetIndex(widget_t *combo);
int32_t strsum(char* str);
#endif /* GRAPHICS_GUI_WIDGETS_H_ */
