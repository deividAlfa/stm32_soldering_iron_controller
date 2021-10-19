/*
 * widgets.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GRAPHICS_GUI_WIDGETS_H_
#define GRAPHICS_GUI_WIDGETS_H_

#include "rotary_encoder.h"

#define default_font u8g2_font_menu

//#define COMBO_SLIDE_TEXT                                                    // Testing feature, not enabled by default

typedef enum widgetStateType {widget_idle, widget_selected, widget_edit, widget_error}widgetStateType;
typedef enum widgetFieldType {field_int32, field_bmp, field_string}widgetFieldType;
typedef enum AlignType { align_disabled, align_left, align_center, align_right }AlignType;
typedef enum widgetFrameType {frame_auto, frame_solid, frame_outline, frame_disabled, frame_combo}widgetFrameType;
typedef enum widgetType {widget_combo, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button}widgetType;
typedef enum comboType {combo_Screen, combo_Editable, combo_MultiOption, combo_Action}comboType;
typedef enum widgetRefreshType {refresh_idle, refresh_triggered, refresh_always}widgetRefreshType;
typedef enum slideStatus {slide_reset, slide_disabled, slide_restart, slide_running, slide_limit}slideStatus;

typedef struct widget_t widget_t;
typedef struct selectable_widget_t selectable_widget_t;
typedef struct editable_widget_t editable_widget_t;
typedef struct button_widget_t button_widget_t;
typedef struct displayOnly_widget_t displayOnly_widget_t;
typedef struct bmp_widget_t bmp_widget_t;
typedef struct comboBox_widget_t comboBox_widget_t;
typedef struct comboBox_item_t comboBox_item_t;

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
  uint8_t number_of_dec;
  uint8_t reservedChars;
  uint8_t stringStart;
  const uint8_t *font;
  char* displayString;
  char* endString;
  void * (*getData)();                      // For multioption & field_int32, use uint32_t*. For field_string use char*
  int32_t last_value;
};

struct editable_widget_t {
  int8_t current_edit;
  uint8_t numberOfOptions;
  int16_t step;
  int16_t big_step;
  int32_t min_value;
  int32_t max_value;
  char * const * options;
  void (*setData)(void *);                  // For multioption & field_int32, use uint32_t*. For field_string use char*
  selectable_widget_t selectable;
  displayOnly_widget_t inputData;
};

struct button_widget_t {
  union{
    struct{
      char* displayString;
      uint8_t stringStart;
      const uint8_t* font;
      int32_t last_value;
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
  uint8_t currentScroll;
  const uint8_t* font;
  comboBox_item_t *first;
  comboBox_item_t *currentItem;
  selectable_widget_t selectable;
};

struct comboBox_item_t {
  uint8_t enabled;
  comboType type;
  comboBox_item_t *next_item;
  AlignType dispAlign;
  union{
    uint8_t action_screen;
    int (*action)(widget_t* w, RE_Rotation_t input);
    editable_widget_t *widget;
  };
  char *text;
};

struct widget_t
{
  widgetType type;
  widgetRefreshType refresh;
  widgetFrameType frameType;
  uint8_t posX;
  uint8_t posY;
  uint8_t width;
  uint8_t enabled;
  int8_t radius;											// Frame radius: -1=auto(r=Height/2). 0=Square, else use defined radius
  widget_t *next_widget;

  struct screen_t *parent;
  uint8_t (*draw)(widget_t*);
  void (*update)(widget_t*);
  void* content;
};

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w);
editable_widget_t * extractEditablePartFromWidget(widget_t *);
selectable_widget_t * extractSelectablePartFromWidget(widget_t *w);

void newWidget(widget_t **new, widgetType type, struct screen_t *scr);
editable_widget_t *newEditable(widgetType type);
comboBox_item_t *newComboItem(void);

void widgetAlign(widget_t* w);
void widgetDefaultsInit(widget_t *w, widgetType type);
void editableDefaultsInit(editable_widget_t* editable, widgetType type);
uint8_t default_widgetDraw(widget_t* w);
void default_widgetUpdate(widget_t *w);
void widgetDetectChange(widget_t* w, int32_t val);
void widgetClearField(widget_t* w);
void widgetEnable(widget_t* w);
void widgetDisable(widget_t* w);
int default_widgetProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state);
int comboBoxProcessInput(widget_t* w, RE_Rotation_t, RE_State_t *);
uint8_t comboBoxDraw(widget_t *w);
void newComboScreen(widget_t *w, char *label, uint8_t actionScreen, comboBox_item_t **newItem);
void newComboEditable( widget_t *combo, char *label, editable_widget_t **newEdit, comboBox_item_t **newItem);
void newComboMultiOption(widget_t *w, char *label, editable_widget_t **newEdit, comboBox_item_t **newItem);
void newComboAction(widget_t *w, char *label, int (*action)(widget_t *w, RE_Rotation_t input), comboBox_item_t **newItem);
void comboResetIndex(widget_t *w);
uint8_t comboItemToIndex(widget_t *w, comboBox_item_t *item);
comboBox_item_t *comboIndexToItem(widget_t *w, uint8_t index);
int32_t strsum(char* str);
void insertDot(char *str, uint8_t dec);
#endif /* GRAPHICS_GUI_WIDGETS_H_ */
