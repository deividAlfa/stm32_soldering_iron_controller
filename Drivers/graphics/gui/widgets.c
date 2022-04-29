/*
 * widgets.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "widgets.h"
#include "screen.h"
#include "oled.h"
#include "gui.h"
static char displayString[32];
static bool callFromCombo;

#ifdef COMBO_SLIDE_TEXT
struct{
  uint8_t status;
  int16_t offset;
  int16_t len;
  int16_t limit;
  uint32_t time;
}slide;
#endif

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "widgets.c"
#endif

void newWidget(widget_t **new, widgetType type, struct screen_t *scr){
  widget_t *w=_malloc(sizeof(widget_t));
  if(!w || !scr) Error_Handler();
  switch(type){
    case widget_combo:
      w->content = _malloc(sizeof(comboBox_widget_t));
      break;
    case widget_display:
      w->content = _malloc(sizeof(displayOnly_widget_t));
      break;
    case widget_multi_option:
    case widget_editable:
      w->content = _malloc(sizeof(editable_widget_t));
      break;
    case widget_button:
    case widget_bmp_button:
      w->content = _malloc(sizeof(button_widget_t));
      break;
    default:
      Error_Handler();
  }
  if(!w->content) Error_Handler();
  screen_addWidget(w,scr);
  widgetDefaultsInit(w, type);
  if(new){
    *new = w;
  }
}

editable_widget_t *newEditable(widgetType type){
  editable_widget_t *edit=_malloc(sizeof(editable_widget_t));
  if(!edit) Error_Handler();
  editableDefaultsInit(edit,type);
  return edit;
}

comboBox_item_t *newComboItem(void){
  comboBox_item_t *item=_malloc(sizeof(comboBox_item_t));
  if(!item) Error_Handler();
  return item;
}

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w) {
  if(!w)
    return NULL;
  switch (w->type) {
    case widget_display:
      return (displayOnly_widget_t*)w->content;
    case widget_editable:
    case widget_multi_option:
      return &((editable_widget_t*)w->content)->inputData;
    case widget_combo:
    {
      comboBox_widget_t *combo = (comboBox_widget_t*)w->content;
      if((combo->currentItem) && ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption))){
        return &((comboBox_widget_t*)w->content)->currentItem->widget->inputData;
      }
    }
    default:
      return NULL;
      break;
  }
  return NULL;
}

editable_widget_t * extractEditablePartFromWidget(widget_t *w) {
  if(!w)
    return NULL;
  switch (w->type) {
    case widget_editable:
    case widget_multi_option:
      return (editable_widget_t*)w->content;
    case widget_combo:
    {
      comboBox_widget_t *combo = (comboBox_widget_t*)w->content;
      if((combo->currentItem) && ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption))){
        return ((comboBox_widget_t*)w->content)->currentItem->widget;
      }
    }
    default:
      return NULL;
      break;
  }
  return NULL;
}

selectable_widget_t * extractSelectablePartFromWidget(widget_t *w) {
  if(!w)
    return NULL;
  switch (w->type) {
    case widget_editable:
    case widget_multi_option:
      return &((editable_widget_t*)w->content)->selectable;
    case widget_button:
    case widget_bmp_button:
      return &((button_widget_t*)w->content)->selectable;
    case widget_combo:
    {
      if(callFromCombo){
        comboBox_widget_t *combo = (comboBox_widget_t*)w->content;
        if((combo->currentItem) && ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption))){
          return &((comboBox_widget_t*)w->content)->currentItem->widget->selectable;
        }
      }
      else{
        return &((comboBox_widget_t*)w->content)->selectable;
      }
    }
    default:
      return NULL;
      break;
  }
  return NULL;
}
void widgetDefaultsInit(widget_t *w, widgetType type){
  if(!w || !w->content){ return; }

  w->type = type;
  w->draw = &default_widgetDraw;
  w->update = &default_widgetUpdate;
  w->enabled = 1;
  w->frameType = frame_auto;
  w->refresh = refresh_idle;
  w->radius=-1;
  w->posX = 0;
  w->posY = 0;
  w->width = 0;
  w->next_widget = NULL;
  comboBox_widget_t *combo;
  if(type==widget_combo){
    combo = (comboBox_widget_t*)w->content;
    combo->first = NULL;
    combo->currentItem = NULL;                    // Needs to be cleared before the extract from widget functions.
  }
  selectable_widget_t *sel = extractSelectablePartFromWidget(w);
  displayOnly_widget_t *dis = extractDisplayPartFromWidget(w);
  editable_widget_t *edit = extractEditablePartFromWidget(w);
  if(sel) {
    sel->previous_state = widget_idle;
    sel->state = widget_idle;
    sel->tab = 0;
    sel->processInput = &default_widgetProcessInput;
    sel->longPressAction = NULL;
  }
  if(dis){
    dis->font = default_font;
    dis->getData = NULL;
    dis->reservedChars = 0;
    dis->number_of_dec = 0;
    dis->type = field_int32;
    dis->displayString=displayString;
    dis->endString=NULL;
    dis->stringStart=0;
    dis->last_value = 0;
    dis->textAlign = align_center;
    dis->dispAlign = align_disabled;
  }
  if(edit){
    edit->step=1;
    edit->big_step=1;
    edit->current_edit=0;
    edit->max_value=2147483647;
    edit->min_value=0;
    edit->setData=NULL;
  }
  switch (type) {
    case widget_multi_option:
      edit->max_value = 255;
      edit->numberOfOptions = 0;
      edit->options = NULL;
      break;
    case widget_combo:
      w->frameType = frame_combo;
      w->draw = &comboBoxDraw;
      w->parent->current_widget = w;
      combo->selectable.processInput = &comboBoxProcessInput;
      combo->font = default_font;
      combo->currentScroll = 0;
      break;
    case widget_button:
    {
      button_widget_t* button = (button_widget_t*)w->content;
      button->action = NULL;
      button->displayString = NULL;
      button->xbm = NULL;
      button->last_xbm = NULL;
      button->font = default_font;
      break;
    }
    default:
      break;
  }

}
void editableDefaultsInit(editable_widget_t* editable, widgetType type){
  widget_t w;
  w.content=editable;
  widgetDefaultsInit(&w, type);
}

void insertDot(char *str, uint8_t dec){
  for(int x = strlen(str); x > (int)strlen(str) - (int)dec - 2; --x) {
    str[x + 1] = str[x];
  }
  str[strlen(str) - dec - 1] = '.';
}

void widgetEnable(widget_t* w){
  if(w && !w->enabled){
    w->enabled = 1;
    w->refresh = refresh_triggered;
  }
}

void widgetDisable(widget_t* w){
  if(w && w->enabled){
    w->enabled = 0;
    w->parent->refresh=screen_Erase;
  }
}

void default_widgetUpdate(widget_t *w) {
  if(!w){ return ; }
  if(!w->enabled){
    w->refresh=refresh_idle;
    return;
  }
  button_widget_t* button;
  bmp_widget_t* bmp;
  int32_t val_ui=0;
  displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
  editable_widget_t *edit = extractEditablePartFromWidget(w);

  switch(w->type){
    case widget_bmp_button:
      button = ((button_widget_t*)w->content);
      if(!button->xbm){
        widgetDisable(w);
        return;
      }
      break;
    case widget_button:
      button = ((button_widget_t*)w->content);
      if(!button->displayString){
        widgetDisable(w);
        return;
      }
      val_ui=strsum(button->displayString);
      break;
    case widget_bmp:
      bmp = ((bmp_widget_t*)w->content);
      if(!bmp->xbm){
        widgetDisable(w);
        return;
      }
      break;

    case widget_multi_option:
      if(!dis){
        widgetDisable(w);
        return;
      };
      val_ui=strsum(edit->options[*(uint8_t*)dis->getData()]);  // Get string sum
      break;

    case widget_display:
    case widget_editable:
      if(!dis){
        widgetDisable(w);
        return;
      }
      if(dis->type == field_string){
        val_ui=strsum(dis->getData());                          // Get string sum
        break;
      }
      else{
        val_ui = *(int32_t*)dis->getData();
        uint8_t sum = strsum(dis->endString);
        widgetDetectChange(w,(val_ui + sum));                   // Check for changes in value and endString
        return;
      }
      break;
    default:
      return;
  }
  widgetDetectChange(w,val_ui);
}

void widgetDetectChange(widget_t* w, int32_t val){
  displayOnly_widget_t* dis;
  bmp_widget_t* bmp;
  button_widget_t* button;
  bool refresh=0;
  switch(w->type){
    case widget_display:
    case widget_editable:
    case widget_multi_option:
      dis=extractDisplayPartFromWidget(w);
      if(dis->last_value!=val){
        dis->last_value=val;
        refresh=1;
      }
      break;

    case widget_button:
      button = (button_widget_t*)w->content;
      if(button->last_value != val){
        button->last_value = val;
        refresh=1;
      }
      break;

    case widget_bmp_button:
      button = (button_widget_t*)w->content;
      if(button->last_xbm != button->xbm){
        button->last_xbm = button->xbm;
        refresh=1;
      }
      break;

    case widget_bmp:
      bmp = (bmp_widget_t*)w->content;
      if(bmp->last_xbm != bmp->xbm){
        bmp->last_xbm = bmp->xbm;
        refresh=1;
      }
      break;

    default:
      break;
  }
  if(refresh && w->refresh==refresh_idle){
    w->refresh=refresh_triggered;
  }
}

// Clear widget field before drawing widget with new data
void widgetClearField(widget_t* w){
  if(!w){ return; }
  selectable_widget_t* sel = extractSelectablePartFromWidget(w);
  displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
  button_widget_t* button;
  bmp_widget_t* bmp;
  uint8_t cHeight = u8g2_GetMaxCharHeight(&u8g2);
  uint8_t r;

  if(w->frameType!=frame_combo){
    u8g2_SetDrawColor(&u8g2, BLACK);
    if(w->parent->refresh < screen_Erase ){
      switch((uint8_t)w->type){
        case widget_bmp:
          bmp = (bmp_widget_t*)w->content;
          u8g2_DrawBox(&u8g2, w->posX, w->posY, bmp->xbm[0], bmp->xbm[1]);
          break;

        case widget_bmp_button:
          button = (button_widget_t*)w->content;
          u8g2_DrawBox(&u8g2, w->posX, w->posY, button->xbm[0]+4, button->xbm[1]+4);
          break;

        case widget_button:
          button = (button_widget_t*)w->content;
          u8g2_DrawBox(&u8g2, w->posX, w->posY, w->width, cHeight+1);
          break;

        case widget_display:
          u8g2_DrawBox(&u8g2, w->posX, w->posY, w->width, cHeight+1);       // Draw black square to erase previous data
          break;

        case widget_editable:
        case widget_multi_option:
          u8g2_DrawBox(&u8g2, w->posX ,w->posY, w->width, cHeight+1);
          break;
      }
    }
    if( (sel && (sel->state==widget_edit) && (dis->type!=field_string) && (w->frameType==frame_auto)) || (w->frameType==frame_solid) ){
      u8g2_SetDrawColor(&u8g2, WHITE);
      if(w->radius<0){
        r=(cHeight-1)/2;
      }
      else{
        r = w->radius;
      }
      if(sel){
        u8g2_DrawRBox(&u8g2, w->posX ,w->posY, w->width, cHeight+1,r );
      }
      else{
        u8g2_DrawRBox(&u8g2, w->posX ,w->posY, w->width+2, cHeight+1, r);
      }

      u8g2_SetDrawColor(&u8g2, XOR);
      return;
    }
    u8g2_SetDrawColor(&u8g2, WHITE);
  }
  else{                                             // In combo
    if(sel->state==widget_edit){                    // Being edited
      u8g2_SetDrawColor(&u8g2, XOR);                // Set XOR color
    }
    else{                                           // Not editing
      u8g2_SetDrawColor(&u8g2, WHITE);              // Set white color
    }
  }
}


void widgetAlign(widget_t* w){
  if(!w || w->frameType==frame_combo){ return; }

  uint8_t strWidth=0, stringStart=0, textAlign=align_disabled, dispAlign=align_disabled;
  displayOnly_widget_t *dis=extractDisplayPartFromWidget(w);
  selectable_widget_t *sel=extractSelectablePartFromWidget(w);
  editable_widget_t *edit = extractEditablePartFromWidget(w);
  button_widget_t* button = NULL;
  switch(w->type){
    case widget_button:
      button = (button_widget_t *)w->content;
      strWidth=u8g2_GetUTF8Width(&u8g2, button->displayString);
      break;

    case widget_display:
    case widget_editable:
      if(dis->type == field_string){
        strWidth=u8g2_GetUTF8Width(&u8g2, (char *)dis->getData());
      }
      else{
        strWidth=u8g2_GetUTF8Width(&u8g2, displayString);
      }
      break;

    case widget_multi_option:
      strWidth=u8g2_GetUTF8Width(&u8g2,  (char *)edit->options[*(uint8_t*)dis->getData()]);
      break;
    default:
      return;
  }
  if(!strWidth){                // Empty widget, wrong settings
    widgetDisable(w);           // Disable widget
    return;
  }
  if(dis){
    textAlign = dis->textAlign;
    dispAlign = dis->dispAlign;
  }
  else if(w->type == widget_button){
    textAlign = align_center;
    dispAlign = align_disabled;
  }
  else{
    return;
  }
  if(sel && (w->frameType!=frame_disabled)){      // If selectable, extra space for not overlapping the frame
    if(w->width < strWidth+7){                    // If width too small
      if(strWidth+7<displayWidth){                   // If fits oled size
        w->width=strWidth+7;                      // Use width from str width
      }
      else{
        w->width = strWidth;                      // Else, don't add extra space
      }
    }
  }
  else{
    if(w->width < strWidth){                      // If width too small
      w->width=strWidth;                          // Use width from str width
    }
  }

  if(w->width > displayWidth){
    if(textAlign != align_disabled)
      textAlign = align_left;
    if(dispAlign != align_disabled)
      dispAlign = align_left;
  }

  switch(dispAlign){
    case align_disabled:
      break;
    case align_center:
      w->posX =(displayWidth - w->width)/2;
      break;
    case align_right:
      w->posX = displayWidth - w->width;
      break;
    case align_left:
    default:
      w->posX = 0;
      break;
  }

  switch(textAlign){
    case align_disabled:
      break;
    case align_center:
      stringStart = (w->posX + ((w->width-strWidth)/2));
      break;
    case align_right:
      stringStart =(w->posX + (w->width-strWidth));
      if(sel){
        if(stringStart >= 3){
          stringStart -= 3;
        }
      }
      break;
    case align_left:
    default:
      if(sel && ((w->posX+3)<=displayWidth)){
        stringStart=w->posX+3;
      }
      else{
        stringStart = w->posX;
      }
      break;
  }

  if(w->type == widget_button){
    button->stringStart=stringStart;
  }
  else if(dis){
    dis->stringStart = stringStart;
  }
}

uint8_t default_widgetDraw(widget_t *w) {
  if(!w || !w->enabled){ return 0; }

  bool frameDraw = 0;
  bool frameColor = BLACK;
  displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
  editable_widget_t* edit = extractEditablePartFromWidget(w);
  selectable_widget_t* sel = extractSelectablePartFromWidget(w);
  button_widget_t* button = NULL;
  bmp_widget_t* bmp = NULL;
  uint8_t refresh = w->refresh | w->parent->refresh;
  uint8_t cHeight = 0;

  if( dis ){
    if(u8g2.font != dis->font){
      u8g2_SetFont(&u8g2, dis->font);
    }
    cHeight = u8g2_GetMaxCharHeight(&u8g2);
  }
  else if((w->type == widget_button)||(w->type == widget_bmp_button)){
    button = (button_widget_t*)w->content;
    if(w->type == widget_button){
      if(u8g2.font != button->font){
        u8g2_SetFont(&u8g2, button->font);
      }
      cHeight = u8g2_GetMaxCharHeight(&u8g2);
    }
  }
  else if(w->type == widget_bmp){
    bmp = (bmp_widget_t*)w->content;
  }
  if(w->frameType==frame_outline){
    frameDraw=1;
    frameColor = WHITE;
  }
  else if(w->frameType==frame_auto){
    if(sel) {
      if(refresh==refresh_idle){                      // If not forced refresh, check for changes
        switch (sel->state) {
          case widget_selected:
            if(sel->previous_state != widget_selected){
              frameDraw=1;
              frameColor = WHITE;
              if(sel->previous_state == widget_edit){
                refresh=refresh_triggered;
              }
            }
            break;
          case widget_idle:
            if(sel->previous_state != widget_idle){
              frameDraw=1;
            }
            break;
          case widget_edit:
            if(sel->previous_state != widget_edit){
              refresh=refresh_triggered;
            }
            break;
          default:
            return 0;
        }
      }
      else{                                           // Else, redraw frames
        switch (sel->state) {
          case widget_selected:
              frameDraw=1;
              frameColor = WHITE;
            break;
          case widget_idle:
          case widget_edit:
            break;
          default:
            return 0;
        }
      }
    }
  }

  if(refresh){
    if(dis && dis->type==field_int32){
      int32_t val_ui = *(int32_t*)dis->getData();
      uint8_t decimals = dis->number_of_dec+1;
      if(val_ui<0){
        decimals++;
      }
      if(decimals>10){ decimals=10; }
      snprintf(dis->displayString, dis->reservedChars+1, "%0*ld", decimals, (int32_t)val_ui);    // Convert value into string

      uint8_t dispLen=strlen(dis->displayString);
      uint8_t endLen=strlen(dis->endString);
      if(dis->number_of_dec){                                                 // If there're decimals
        if(dis->reservedChars >= (dispLen+1)){                                // Ensure there's enough space in the string for adding the decimal point
          insertDot(dis->displayString,  dis->number_of_dec);                 // Insert decimal dot
        }
      }
      if(dis->reservedChars >= (dispLen+endLen)){                             // Ensure there's enough space in the string for adding the end String
        strcat(dis->displayString, dis->endString);                           // Append endString
      }
      dis->displayString[dis->reservedChars]=0;                               // Ensure last string char is 0
    }

    widgetAlign(w);        // Align
    widgetClearField(w);        // Clear old field
    if(w->refresh==refresh_triggered){
      w->refresh=refresh_idle;
    }

    switch(w->type){

      case widget_bmp:
        u8g2_SetDrawColor(&u8g2, WHITE);
        u8g2_DrawXBMP(&u8g2, w->posX, w->posY, bmp->xbm[0], bmp->xbm[1], &bmp->xbm[2]);
        return 1;

      case widget_bmp_button:
        u8g2_SetDrawColor(&u8g2, WHITE);
        u8g2_DrawXBMP(&u8g2, w->posX+2, w->posY+2, button->xbm[0], button->xbm[1], &button->xbm[2]);
        break;

      case widget_button:
        u8g2_DrawUTF8(&u8g2, button->stringStart, w->posY+2,  button->displayString);
        break;

      case widget_display:
        if(dis->type == field_string){
          u8g2_DrawUTF8(&u8g2, dis->stringStart, w->posY,  dis->getData());
        }
        else{
          u8g2_DrawUTF8(&u8g2, dis->stringStart, w->posY,  dis->displayString);
        }
        break;

      case widget_editable:
        if(dis->type == field_string){
          u8g2_DrawUTF8(&u8g2,  dis->stringStart, w->posY+2,  dis->getData());
          if(sel->state == widget_edit){
            char t[20];
            strcpy(t,dis->getData());

            t[edit->current_edit+1]=0;
            uint8_t x2=u8g2_GetUTF8Width(&u8g2, t);

            t[edit->current_edit]=0;
            uint8_t x1=u8g2_GetUTF8Width(&u8g2, t);

            u8g2_SetDrawColor(&u8g2, BLACK);
            u8g2_DrawBox(&u8g2, dis->stringStart, w->posY+ cHeight, w->width, 2);

            u8g2_SetDrawColor(&u8g2, WHITE);
            u8g2_DrawBox(&u8g2,  dis->stringStart+x1, w->posY+ cHeight, x2-x1, 2);

          }
          else if(sel->previous_state == widget_edit){
            u8g2_SetDrawColor(&u8g2, BLACK);
            u8g2_DrawBox(&u8g2, dis->stringStart, w->posY+ cHeight, w->width, 2);
          }
        }
        else{
          u8g2_DrawUTF8(&u8g2,dis->stringStart, w->posY+2,  dis->displayString);
        }
        break;

      case widget_multi_option:
        u8g2_DrawUTF8(&u8g2, dis->stringStart, w->posY+2, edit->options[*(uint8_t*)dis->getData()]);// Draw string
        break;

      default:
        return 0;
    }
  }

  if(sel) {
    sel->previous_state = sel->state;
  }

  if(frameDraw ||(refresh && w->frameType==frame_outline)){
    switch(w->type){
      case widget_bmp_button:
        u8g2_SetDrawColor(&u8g2, frameColor);
        u8g2_DrawRFrame(&u8g2, w->posX, w->posY, button->xbm[0]+4,  button->xbm[1]+4, 4);
        break;
      case widget_editable:
      case widget_button:
      case widget_multi_option:
        u8g2_SetDrawColor(&u8g2, frameColor);
        uint8_t r;
        if(w->radius<0){
          r=(cHeight-1)/2;
        }
        else{
          r = w->radius;
        }
        u8g2_DrawRFrame(&u8g2, w->posX ,w->posY, w->width, cHeight+1, r);
      default:
        break;
    }
    refresh=1;                                    // For drawing detection
  }
  u8g2_SetDrawColor(&u8g2, WHITE);
  return refresh;
}

#ifdef COMBO_SLIDE_TEXT

uint8_t comboBoxDraw(widget_t *w) {
  if(!w || !w->enabled){ return 0; }                                                // Return if null or disabled

  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  if(!combo){ return 0; }                                                           // Return if null
  comboBox_item_t *item = combo->first;
  if(!item){ return 0; }                                                            // Return if null

  uint16_t yDim = displayHeight - w->posY;
  uint8_t height;
  int8_t frameY=0;
  int8_t posY;
  bool drawFrame=0;
  bool drawSeparator=0;
  uint8_t scroll = 0;
  uint8_t r;
  uint16_t len = 0;
  uint8_t refresh_slide=0;
  int16_t offset;

  if( w->parent->refresh==screen_Idle){                                                           // If screen idling

      switch(slide.status){

        case slide_reset:
          if(u8g2.font != combo->font){
            u8g2_SetFont(&u8g2, combo->font);
          }
          slide.offset = 0;                                                                   // Clear offset
          slide.len = u8g2_GetUTF8Width(&u8g2, combo->currentItem->text);                     // Compute string length and limit only once

          if(combo->currentItem->type==combo_Editable || combo->currentItem->type==combo_MultiOption){                      // For editable widgets, limit is ~half of the oled width
            slide.limit = (displayWidth/2)+8-4;
          }
          else{
            slide.limit = displayWidth-8;                                                          // Else, use all the space available for the label
          }
          if(slide.len<slide.limit){                                                            // Disable if label text shorter than limit (No need to slide text)
            slide.status = slide_disabled;
            break;
          }
          slide.status = slide_restart;
          slide.time = current_time;
          break;

        case slide_disabled:
          break;

        case slide_restart:
          if((current_time-slide.time)>500){                                                   // If resetted, freeze the text 500ms
            slide.time = current_time;
            slide.status = slide_running;
          }
          break;

        case slide_running:
          if((current_time-slide.time)>19){                                                    // If text sliding running, update every 20mS
            slide.time=current_time;
            if((slide.len - ++slide.offset)<=slide.limit){                                    // Increase x offset, check limits
              slide.status = slide_limit;                                                     // If reached end
            }
            refresh_slide=1;
          }
          break;
        case slide_limit:
          if((current_time-slide.time)>500){                                                 // If reached the end, freeze the text 500ms
            slide.time = current_time;
            slide.status = slide_restart;
            slide.offset = 0;
            refresh_slide=1;
          }

          break;
        default:
          break;
      }
      if(w->refresh==refresh_idle && !refresh_slide){                                       // If nothing to do, return
        return 0;
      }
  }
  else{
    slide.offset=0;                                                                         // If screen erased, reset offset
    slide.status = slide_reset;
  }


  if(w->refresh==refresh_triggered){
    w->refresh=refresh_idle;
  }
  if(w->parent->refresh < screen_Erase){                    // If screen not erased already
    if(!refresh_slide){                                     // If not updating sliding text
      w->parent->refresh = screen_Erased;                   //
      fillBuffer(BLACK, fill_dma);                          // Erase fast using dma
    }
  }
  if(u8g2.font != combo->font){
    u8g2_SetFont(&u8g2, combo->font);
  }
  height= u8g2_GetMaxCharHeight(&u8g2)+1;                 // +1 to allow separation between combobox items

  if(w->radius<0){
    r=(height-1)/2;
  }
  else{
    r = w->radius;
  }

  while(scroll < combo->currentScroll) {
    if(!item->next_item)
      break;
    item = item->next_item;
    if(item->enabled)
      ++scroll;
  }
  for(uint8_t y = 0; y < yDim / height; ++y) {
    uint8_t align = item->dispAlign;
    bool editable = (item->type==combo_Editable)||(item->type==combo_MultiOption);

                                                                      // Store Y position
    if(item == combo->currentItem) {                                  // If the current combo item is selected
      offset=slide.offset;
      frameY=y * height + w->posY;
      if(refresh_slide==1){                                           // If only refreshing sliding text and this is the matching item
        len = slide.len;
        refresh_slide++;                                              // = 2 means update current item label
        u8g2_SetDrawColor(&u8g2, BLACK);
        if (editable){                                                // Clear textfield
          u8g2_DrawBox(&u8g2, 4, frameY+1, (displayWidth/2)+6, height-2);
        }
        else{
          u8g2_DrawBox(&u8g2, 4, frameY+1, displayWidth-8, height-2);
        }
      }
      else{                                   // Normal drawing, draw frame
        drawFrame=1;
        drawSeparator = editable;             // Draw separator if editable
      }
    }
    else{
      if(refresh_slide==2){                   // I sliding text has been updated, nothing else to do
        break;
      }
      offset=0;                               // If not selected item, use 0 offset
    }
    if(!refresh_slide || refresh_slide==2){                             // Only continue if no sliding text (full redraw) or this is the item that needs updating
      if (!editable){                                                   // If screen or action item, just draw the label
        if(!refresh_slide){
          len = u8g2_GetUTF8Width(&u8g2, item->text);                   // When sliding text, length is already done.
        }
        if(len>displayWidth-9){                                            // If too long, override with left align to provide sliding text
          align=align_left;
        }
        u8g2_SetDrawColor(&u8g2, WHITE);
        u8g2_SetClipWindow(&u8g2, 4, 0, displayWidth-4, displayHeight-1);

        if(align==align_left){
          u8g2_DrawUTF8(&u8g2, (int16_t)4-offset, y * height + w->posY +2, item->text);
        }
        else if(align==align_right){
          u8g2_DrawUTF8(&u8g2, displayWidth-3-len, y * height + w->posY +2, item->text);
        }
        else{     // Align center
          u8g2_DrawUTF8(&u8g2, (displayWidth-1-len)/2, y * height + w->posY +2, item->text);
        }
      }
      else{                                                                                 // Editable or multioption
        if(!refresh_slide){                                                                 // Draw the editable data if not coming for text slide.
          editable_widget_t* edit = item->widget;
          selectable_widget_t *sel = &edit->selectable;
          displayOnly_widget_t* dis = &edit->inputData;
          default_widgetUpdate(w);
          u8g2_SetClipWindow(&u8g2, (displayWidth/2)+10, 0, displayWidth-5, displayHeight-1);
          posY = y * height + w->posY;                                                      // Set drawing Ypos same as the current combo option
          if(sel->state==widget_edit){                                                      // If edit mode
            u8g2_SetClipWindow(&u8g2, (displayWidth/2)+11, 0, displayWidth-1, displayHeight-1);      // Draw edit frame
            u8g2_SetDrawColor(&u8g2, WHITE);
            u8g2_DrawRBox(&u8g2, (displayWidth/2)+11-4, frameY, displayWidth-((displayWidth/2)+11-4), height, r);  // Only ~half of the width, the rest is used for the label, not highlighted
            u8g2_SetDrawColor(&u8g2, BLACK);
          }
          else{
            u8g2_SetDrawColor(&u8g2, WHITE);
          }
          if(item->type==combo_MultiOption){
            len = u8g2_GetUTF8Width(&u8g2,edit->options[*(uint8_t*)dis->getData()]);
          }
          else if(item->type==combo_Editable){
            if(dis->type==field_int32){
              int32_t val_ui = *(int32_t*)dis->getData();                         // Get data
              uint8_t decimals = dis->number_of_dec+1;                            // Load decimal count
              if(val_ui<0){                                                       // If negative, add a decimal (decimals are just used as min char output in sprintf)
                decimals++;
              }
              if(decimals>10){ decimals=10; }                                     // Limit max decimals
              snprintf(dis->displayString, dis->reservedChars+1, "%0*ld", decimals, (int32_t)val_ui);    // Convert value into string
              uint8_t dispLen=strlen(dis->displayString);                         // Get string len
              uint8_t endLen=strlen(dis->endString);                              // Get endStr len
              if(dis->number_of_dec){                                             // If there're decimals
                if(dis->reservedChars >= (dispLen+1)){                            // Ensure there's enough space in the string for adding the decimal point
                  insertDot(dis->displayString,  dis->number_of_dec);             // Insert decimal dot
                }
              }
              if(dis->reservedChars >= (dispLen+endLen)){                         // Ensure there's enough space in the string for adding the end String
                strcat(dis->displayString, dis->endString);                       // Append endString
              }
              dis->displayString[dis->reservedChars]=0;                           // Ensure last string char is 0
              len=u8g2_GetUTF8Width(&u8g2,dis->displayString);
            }
            else if(dis->type==field_string){
              strncpy(displayString,dis->getData(),dis->reservedChars+1);
              len=u8g2_GetUTF8Width(&u8g2,displayString);
            }
          }

          dis->stringStart = displayWidth-len-5;                                   // Align to the left measuring actual string width
          if(item->type==combo_Editable){
            if((dis->type==field_string && (sel->state==widget_edit))){
              char str[sizeof(displayString)+1];
              uint8_t start,width;
              strcpy(str,displayString);
              str[edit->current_edit+1]=0;
              width=u8g2_GetUTF8Width(&u8g2, str);
              str[edit->current_edit]=0;
              start=u8g2_GetUTF8Width(&u8g2, str);
              width-=start;

              u8g2_SetDrawColor(&u8g2, BLACK);
              u8g2_DrawBox(&u8g2, dis->stringStart+start, posY+1, width+1, height-2);
              u8g2_SetDrawColor(&u8g2, XOR);
              u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2, displayString);
            }
            else{
              u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2, dis->displayString);
            }
          }
          else if(item->type==combo_MultiOption){
            u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2,  edit->options[*(uint8_t*)dis->getData()]);
          }
        }

        u8g2_SetDrawColor(&u8g2, WHITE);
        u8g2_SetClipWindow(&u8g2, 4, 0, (displayWidth/2)+8, displayHeight-1);
        u8g2_DrawUTF8(&u8g2, (int16_t)4-offset, y * height + w->posY +2, item->text);                 // Draw the combo item label
      }
    }
    do {
      item = item->next_item;
    }while(item && !item->enabled);                                                                     // Find next enabled item

    if(!item){
      break;
    }
  }
  u8g2_SetMaxClipWindow(&u8g2);
  if(drawFrame){
    u8g2_SetDrawColor(&u8g2, WHITE);
    if(drawSeparator){
      u8g2_DrawVLine(&u8g2, (displayWidth/2)+10, frameY, height);
    }
    u8g2_DrawRFrame(&u8g2, 0, frameY, displayWidth, height,  r);
  }
  return 1;
}
#else
uint8_t comboBoxDraw(widget_t *w) {
  if(!w || !w->enabled){ return 0; }                                                // Return if null or disabled

  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  if(!combo){ return 0; }                                                           // Return if null
  comboBox_item_t *item = combo->first;
  if(!item){ return 0; }                                                            // Return if null

  uint16_t yDim = displayHeight - w->posY;
  uint8_t height;
  int8_t frameY=0;
  int8_t posY;
  bool drawFrame=1;
  uint8_t scroll = 0;
  uint8_t r;
  if(w->refresh==refresh_idle && w->parent->refresh==screen_Idle){
    return 0;
  }


  if(w->refresh==refresh_triggered){
    w->refresh=refresh_idle;
  }
  if(w->parent->refresh < screen_Erase){        // If screen not erased already
    w->parent->refresh = screen_Erased;
    fillBuffer(BLACK, fill_dma);                          // Erase fast using dma
  }
  if(u8g2.font != combo->font){
    u8g2_SetFont(&u8g2, combo->font);
  }
  height= u8g2_GetMaxCharHeight(&u8g2)+1;                 // +1 to allow separation between combobox items

  if(w->radius<0){
    r=(height-1)/2;
  }
  else{
    r = w->radius;
  }


  while(scroll < combo->currentScroll) {
    if(!item->next_item)
      break;
    item = item->next_item;
    if(item->enabled)
      ++scroll;
  }
  for(uint8_t y = 0; y < yDim / height; ++y) {
                                                                      // Store Y position
    if(item == combo->currentItem) {                                  // If the current combo item is selected
      frameY=y * height + w->posY;
    }

    if ((item->type==combo_Screen)||(item->type==combo_Action)){      // If screen or action item, just draw the label
      u8g2_SetDrawColor(&u8g2, WHITE);
      if(item->dispAlign==align_left){
        u8g2_DrawUTF8(&u8g2, 4, y * height + w->posY +2, item->text);
      }
      else{
        uint8_t len = u8g2_GetUTF8Width(&u8g2, item->text);
        if(item->dispAlign==align_right){
          u8g2_DrawUTF8(&u8g2, displayWidth-3-len, y * height + w->posY +2, item->text);
        }
        else{
          u8g2_DrawUTF8(&u8g2, (displayWidth-1-len)/2, y * height + w->posY +2, item->text);
        }
      }
    }
    else if ((item->type==combo_Editable)||(item->type==combo_MultiOption)){
      editable_widget_t* edit = item->widget;
      selectable_widget_t *sel = &edit->selectable;
      displayOnly_widget_t* dis = &edit->inputData;
      default_widgetUpdate(w);
      uint8_t len=0;
      if(sel->state==widget_edit){                                      // If edit mode
        drawFrame=0;
        u8g2_SetDrawColor(&u8g2, WHITE);
        u8g2_DrawRBox(&u8g2, 0, frameY, displayWidth, height, r);
        u8g2_SetDrawColor(&u8g2, BLACK);
      }
      else{
        u8g2_SetDrawColor(&u8g2, WHITE);
      }

      if(item->type==combo_MultiOption){
        len = u8g2_GetUTF8Width(&u8g2,edit->options[*(uint8_t*)dis->getData()]);
      }
      else if(item->type==combo_Editable){
        if(dis->type==field_int32){
          int32_t val_ui = *(int32_t*)dis->getData();                         // Get data
          uint8_t decimals = dis->number_of_dec+1;                            // Load decimal count
          if(val_ui<0){                                                       // If negative, add a decimal (decimals are just used as min char output in sprintf)
            decimals++;
          }
          if(decimals>10){ decimals=10; }                                     // Limit max decimals
          snprintf(dis->displayString, dis->reservedChars+1, "%0*ld", decimals, (int32_t)val_ui);    // Convert value into string
          uint8_t dispLen=strlen(dis->displayString);                         // Get string len
          uint8_t endLen=strlen(dis->endString);                              // Get endStr len
          if(dis->number_of_dec){                                             // If there're decimals
            if(dis->reservedChars >= (dispLen+1)){                            // Ensure there's enough space in the string for adding the decimal point
              insertDot(dis->displayString,  dis->number_of_dec);             // Insert decimal dot
            }
          }
          if(dis->reservedChars >= (dispLen+endLen)){                         // Ensure there's enough space in the string for adding the end String
            strcat(dis->displayString, dis->endString);                       // Append endString
          }
          dis->displayString[dis->reservedChars]=0;                           // Ensure last string char is 0
          len=u8g2_GetUTF8Width(&u8g2,dis->displayString);
        }
        else if(dis->type==field_string){
          strncpy(displayString,(char*)dis->getData(),dis->reservedChars+1);
          len=u8g2_GetUTF8Width(&u8g2,displayString);
        }
      }

      posY = y * height + w->posY;                                          // Set widget Ypos same as the current combo option
      dis->stringStart = displayWidth-len-5;                                   // Align to the left measuring actual string width
      if(item->type==combo_Editable){
        if((dis->type==field_string && (sel->state==widget_edit))){
          char str[sizeof(displayString)+1];
          uint8_t start,width;
          strcpy(str,displayString);
          str[edit->current_edit+1]=0;
          width=u8g2_GetUTF8Width(&u8g2, str);
          str[edit->current_edit]=0;
          start=u8g2_GetUTF8Width(&u8g2, str);
          width-=start;

          u8g2_SetDrawColor(&u8g2, BLACK);
          u8g2_DrawBox(&u8g2, dis->stringStart+start, posY+1, width+1, height-2);
          u8g2_SetDrawColor(&u8g2, XOR);
          u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2, displayString);
        }
        else{
          u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2, dis->displayString);
        }
      }
      else if(item->type==combo_MultiOption){
        u8g2_DrawUTF8(&u8g2,dis->stringStart, posY+2,  edit->options[*(uint32_t*)dis->getData()]);
      }
      u8g2_DrawUTF8(&u8g2, 4, y * height + w->posY +2, item->text);          // Draw the combo item label
    }
    do {
      item = item->next_item;
    }while(item && !item->enabled);

    if(!item){
      break;
    }
  }
  if(drawFrame){
    u8g2_SetDrawColor(&u8g2, WHITE);
    u8g2_DrawRFrame(&u8g2, 0, frameY, displayWidth, height,  r);
  }
  return 1;
}
#endif

int comboBoxProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state) {

  if(!w || !w->enabled || input == Rotate_Nothing ){
    return -1;
  }
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;


  if(u8g2.font != combo->font){
    u8g2_SetFont(&u8g2, combo->font);
  }

  uint8_t firstIndex = combo->currentScroll;
  uint16_t yDim = displayHeight - w->posY;
  uint16_t height = u8g2_GetMaxCharHeight(&u8g2)+1;
  uint8_t maxIndex = yDim / height;
  uint8_t lastIndex = combo->currentScroll + maxIndex -1;
  selectable_widget_t *sel;
  if(w->refresh==refresh_idle){
    w->refresh=refresh_triggered;                                                                             // Update in combo erases whole screen (to avoid possible leftovers)
  }
  if((input == Click) || (input == LongClick)){                                                               // If clicked
    if (combo->currentItem->type==combo_Action){                                                              // If combo Action type
      return combo->currentItem->action(w, input);                                                                   // Process action
    }
    else if (combo->currentItem->type==combo_Screen){                                                         // If combo screen type
      return combo->currentItem->action_screen;                                                               // Return screen index
    }
  }
  if(input!=Rotate_Nothing){
    if ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption)){           // If combo option type
      sel = &combo->currentItem->widget->selectable;                                                            // Get selectable data
      combo->selectable.state = sel->state;
      combo->selectable.previous_state = sel->previous_state;
      if(((input == Click) && (sel->state!=widget_edit)) || sel->state==widget_edit){                         // If widget in edit mode
        callFromCombo=1;
        sel->processInput(w, input, state);                                                                   // Process widget
        callFromCombo=0;
        return -1;                                                                                            // Return
      }
    }
    if(input == Rotate_Increment) {
      //Set comboBox item to next comboBox
      comboBox_item_t *current = combo->currentItem->next_item;
      // if comboBox is disabled, skip. While comboBox still valid
      while(current && !current->enabled) {
        current = current->next_item;
      }
      // If comboBox valid(didn't reach end)
      if(current) {
        combo->currentItem = current;
        uint8_t index = comboItemToIndex(w, current);
        if(index > lastIndex)
          ++combo->currentScroll;
      }
      #ifdef COMBO_SLIDE_TEXT
      slide.status=slide_reset;
      #endif
    }
    else if(input == Rotate_Decrement) {
      comboBox_item_t *current = NULL;
      // If comboBox is not the first element
      if(combo->currentItem != combo->first){
        do {
          current = combo->first;
          while(current->next_item != combo->currentItem) {
            current = current->next_item;
          }
          combo->currentItem = current;
        }while(!current->enabled);
        uint8_t index = comboItemToIndex(w, current);
        if(index < firstIndex)
          --combo->currentScroll;
      }
      #ifdef COMBO_SLIDE_TEXT
      slide.status=slide_reset;
      #endif
    }
  }
  return -1;
}

int32_t strsum(char* str){
  int32_t sum=0;
  while(*str){
    sum+=*str;
    str++;
  }
  return sum;
}

//returns -1 if processed, -2 if not processed, or next screen
int default_widgetProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state){
  if(!w || !w->enabled || (input == Rotate_Nothing)){ return -1;  }

  selectable_widget_t* sel = extractSelectablePartFromWidget(w);
  if(!sel) {  return -2;  }

  displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
  editable_widget_t* edit = extractEditablePartFromWidget(w);
  comboBox_widget_t* combo = NULL;

  if(w->type==widget_combo){
    combo = (comboBox_widget_t*)w->content;
  }
  if(input == LongClick) {
    if(sel->longPressAction){
      return sel->longPressAction(w);
    }
    else{
      input = Click;
    }
  }

  if(input == Click) {
    switch (sel->state) {
      case widget_selected:
        if(w->refresh==refresh_idle){
          w->refresh = refresh_triggered;
        }
        if((w->type == widget_button)||(w->type == widget_bmp_button)){
          return ((button_widget_t*)w->content)->action(w);
        }
        if(dis->type == field_string){
          strcpy(dis->displayString, (char*)edit->inputData.getData());
          edit->current_edit = 0;
        }
        sel->state = widget_edit;
        sel->previous_state = widget_selected;
        break;

      case widget_edit:
        if(w->refresh==refresh_idle){
          w->refresh = refresh_triggered;
        }
        if(dis->type == field_string) {
          ++edit->current_edit;
          if(edit->current_edit== dis->reservedChars){
            sel->state = widget_selected;
            sel->previous_state = widget_edit;
            edit->current_edit = 0;
          }
        }
        else {
          sel->state = widget_selected;
          sel->previous_state = widget_edit;
        }
        break;

      default:
        break;
    }
    return -1;
  }
  if( edit && sel->state==widget_edit) {
    int32_t val_ui;
    int32_t inc;
    if(w->refresh==refresh_idle){
      w->refresh=refresh_triggered;
    }
    if(abs(state->Diff) > 1){
      inc = edit->big_step;
    }
    else{
      inc = edit->step;
    }
    if(state->Diff < 0){
        inc = -inc;
    }
    if( (w->type == widget_multi_option || (combo && combo->currentItem->type==combo_MultiOption)) ) {
        int32_t option = *(int32_t*)dis->getData();
        if(input == Rotate_Increment){
          if(option < edit->numberOfOptions -1)
            option++;
          else
            option = edit->numberOfOptions -1;
        }
        else if(input == Rotate_Decrement){
          if(option > 0)
            option--;
          else
            option=0;
        }
        edit->setData(&option);
    }
    else if(dis->type==field_string){
      if(input == Rotate_Decrement_while_click ||input == Rotate_Increment_while_click){
        if(input == Rotate_Decrement_while_click){
          if(edit->current_edit==0){
            edit->current_edit=dis->reservedChars-1;
          }
          else{
            edit->current_edit--;
          }
        }
        else{
          if(edit->current_edit<dis->reservedChars-1){
            edit->current_edit++;
          }
          else{
            edit->current_edit=0;
          }
        }
        return -1;
      }
      char current_edit = (char)dis->displayString[edit->current_edit];
      current_edit += inc;

      switch(current_edit){//     \0, ' ', 0-9, A-Z
        case '0'-1:
          current_edit =' ';
          break;

        case ' '-1:
          current_edit='Z';
          break;

        case 'Z'+1:
          current_edit=' ';
          break;

        case ' '+1:
          current_edit = '0';
          break;

        case '9'+1:
          current_edit ='A';
          break;
        case 'A'-1:
          current_edit ='9';
          break;
        default:
          break;
      }
      dis->displayString[edit->current_edit]=(char)current_edit;
      edit->setData(dis->displayString);
    }
    else if(dis->type==field_int32){
      if(!dis->displayString){                            // If empty
        widgetDisable(w);                                 // This shouldn't happen. Disable widget to avoid possible errors.
        return -1;
      }
      val_ui = *(int32_t*)dis->getData();

      if( (inc>0) && ((val_ui+inc) < val_ui) ){           // Check for overflow
        val_ui = edit->max_value;
      }
      else if( (inc<0) && ((val_ui+inc) > val_ui) ){      // Check for underflow
        val_ui = edit->min_value;
      }
      else{
        val_ui += inc;
        if(val_ui < edit->min_value) {                    // Check limits
          val_ui = edit->min_value;
        }
        if(val_ui > edit->max_value) {
          val_ui= edit->max_value;
        }
      }
      edit->setData(&val_ui);
    }
    return -1;
  }
  else if (sel->state == widget_selected) {
    uint8_t next = 0xFF;
    int previous = -1;
    widget_t *next_w = NULL;
    widget_t *previous_w = NULL;
    widget_t *first_w = w;
    widget_t *last_w = w;
    widget_t *scan = w->parent->widgets;
    while(scan) {
      selectable_widget_t *e = extractSelectablePartFromWidget(scan);
      if(e) {
        if((e->tab > sel->tab) && (e->tab < next) && scan->enabled) {
          next = e->tab;
          next_w =scan;
        }
        if((e->tab < sel->tab) && (e->tab > previous) && scan->enabled) {
          previous = e->tab;
          previous_w = scan;
        }
        if((e->tab < extractSelectablePartFromWidget(first_w)->tab) && scan->enabled)
          first_w = scan;
        if((e->tab > extractSelectablePartFromWidget(last_w)->tab) && scan->enabled)
          last_w = scan;
      }
      scan = scan->next_widget;
    }
    if(next_w == NULL)
      next_w = first_w;
    if(previous_w == NULL)
      previous_w = last_w;
    if((input == Rotate_Increment) && next_w && (next_w != w)) {
      sel->state = widget_idle;
      sel->previous_state = widget_selected;
      w->parent->current_widget = next_w;
      extractSelectablePartFromWidget(next_w)->previous_state = extractSelectablePartFromWidget(next_w)->state;
      extractSelectablePartFromWidget(next_w)->state = widget_selected;
      return -1;
    }
    else if((input == Rotate_Decrement) && previous_w && (previous_w != w)) {
      sel->state = widget_idle;
      sel->previous_state = widget_selected;
      w->parent->current_widget = previous_w;
      extractSelectablePartFromWidget(previous_w)->previous_state = extractSelectablePartFromWidget(previous_w)->state;
      extractSelectablePartFromWidget(previous_w)->state = widget_selected;
      return -1;
    }
  }
  return -2;
}

void newComboScreen(widget_t *w, char *label, uint8_t actionScreen, comboBox_item_t **newItem){
  comboBox_item_t *item = _malloc(sizeof(comboBox_item_t));
  if(!item || !w || !label){
    Error_Handler();
  }
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  item->text = label;
  item->next_item = NULL;
  item->action_screen = actionScreen;
  item->type = combo_Screen;
  item->enabled = 1;
  item->dispAlign= align_center;

  comboBox_item_t *next = combo->first;

  if(!next) {
    combo->first = item;
    combo->currentItem = item;
  }
  else{
    while(next->next_item){
      next = next->next_item;
    }
    next->next_item = item;
  }
  if(newItem){
    *newItem = item;
  }
}

// Only allows Editable or multioption widgets
void newComboEditable(widget_t *w, char *label, editable_widget_t **newEdit, comboBox_item_t **newItem){
  comboBox_item_t *item = _malloc(sizeof(comboBox_item_t));
  editable_widget_t *edit = _malloc(sizeof(editable_widget_t));
  if(!item || !w || !label || !edit ){
    Error_Handler();
  }
  editableDefaultsInit(edit, widget_editable);
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  item->text = label;
  item->next_item = NULL;
  item->widget = edit;
  item->type = combo_Editable;
  item->enabled = 1;
  item->dispAlign = 0; // Unused, automatically adjusted

  comboBox_item_t *next = combo->first;
  edit->selectable.state=widget_selected;
  edit->selectable.previous_state=widget_selected;

  if(!next) {
    combo->first = item;
    combo->currentItem = item;
  }
  else{
    while(next->next_item){
      next = next->next_item;
    }
    next->next_item = item;
  }
  if(newItem){
    *newItem = item;
  }
  if(newEdit){
    *newEdit = edit;
  }
}

// Only allows Editable or multioption widgets
void newComboMultiOption(widget_t *w, char *label, editable_widget_t **newEdit, comboBox_item_t **newItem){
  comboBox_item_t *item = _malloc(sizeof(comboBox_item_t));
  editable_widget_t *edit = _malloc(sizeof(editable_widget_t));
  if(!item || !w || !label || !edit ){
    Error_Handler();
  }
  editableDefaultsInit(edit, widget_multi_option);
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;

  item->text = label;
  item->next_item = NULL;
  item->widget = edit;
  item->type = combo_MultiOption;
  item->enabled = 1;
  item->dispAlign = 0; // Unused, automatically adjusted
  edit->selectable.state=widget_selected;
  edit->selectable.previous_state=widget_selected;
  comboBox_item_t *next = combo->first;

  if(!next) {
    combo->first = item;
    combo->currentItem = item;
  }
  else{
    while(next->next_item){
      next = next->next_item;
    }
    next->next_item = item;
  }
  if(newItem){
    *newItem = item;
  }
  if(newEdit){
    *newEdit = edit;
  }
}

void newComboAction(widget_t *w, char *label, int (*action)(widget_t *w, RE_Rotation_t input), comboBox_item_t **newItem){
  comboBox_item_t *item = _malloc(sizeof(comboBox_item_t));
  if(!item || !w || !label || !action){
    Error_Handler();
  }
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  item->text = label;
  item->next_item = NULL;
  item->action = action;
  item->type = combo_Action;
  item->enabled = 1;
  item->dispAlign= align_center;
  comboBox_item_t *next = combo->first;

  if(!next) {
    combo->first = item;
    combo->currentItem = item;
  }
  else{
    while(next->next_item){
      next = next->next_item;
    }
    next->next_item = item;
  }
  if(newItem){
    *newItem = item;
  }
}

void comboResetIndex(widget_t *w){
  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
  if((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption)){
      selectable_widget_t* sel = &combo->currentItem->widget->selectable;
    if(sel){
      sel->state = widget_selected;
      sel->previous_state = widget_selected;
    }
  }
  combo->currentItem = combo->first;
  combo->currentScroll=0;
}

uint8_t comboItemToIndex(widget_t *w, comboBox_item_t *item) {
  if(!w || !item){ return 0; }
  uint8_t index = 0;
  comboBox_item_t *i = ((comboBox_widget_t*)w->content)->first;
  while(i->next_item && i != item) {
    i = i->next_item;
    if(i->enabled)
      index++;
  }
  return index;
}

comboBox_item_t *comboIndexToItem(widget_t *w, uint8_t index) {
  if(!w){ return NULL; }
  comboBox_widget_t *combo = (comboBox_widget_t*)w->content;
  comboBox_item_t *i=combo->first;
  if(!i){ return NULL; }
  while(i->next_item && index) {
      i = i->next_item;
      if(i->enabled){
        index--;
      }
   }
  return i;
}
