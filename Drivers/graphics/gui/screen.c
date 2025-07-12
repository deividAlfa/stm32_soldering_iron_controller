/*
 * screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "screen.h"
#include "oled.h"

void screen_addWidget(widget_t *widget, screen_t *scr) {
  widget_t *current_widget = NULL;
  if(scr->widgets) {
    current_widget = scr->widgets;
    while(current_widget->next_widget)
      current_widget = current_widget->next_widget;
  }

  widget->next_widget = NULL;
  widget->parent = scr;
  if(current_widget)
    current_widget->next_widget = widget;
  else
    scr->widgets = widget;
}

widget_t * screen_tabToWidget(screen_t * scr, uint8_t tab) {
  widget_t *current_widget = NULL;
  if(scr->widgets) {
    current_widget = scr->widgets;
    while(current_widget) {
      if(current_widget->type == widget_editable) {
        if(((editable_widget_t*)current_widget->content)->selectable.tab == tab)
          return current_widget;
      }
      current_widget = current_widget->next_widget;
    }
  }
  return current_widget;
}

int default_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
  int ret = -1;
  selectable_widget_t *sel = extractSelectablePartFromWidget(scr->current_widget);
  if(sel) {
    if(sel->processInput) {
      ret = sel->processInput(scr->current_widget, input, state);
    }
  }
  return ret;
}

uint8_t default_screenDraw(screen_t *scr) {
  widget_t *current_widget = NULL;
  uint8_t ret = 0;
  if(scr->widgets) {
    if(scr->state==screen_Erase){
      scr->state=screen_Erased;
      fillBuffer(BLACK,fill_dma);
    }
    ret = (scr->state==screen_Erased);       // Set return value if screen was erased to force update.

    current_widget = scr->widgets;
    while(current_widget) {
      if(current_widget->draw){
        ret |= current_widget->draw(current_widget);
      }
      current_widget = current_widget->next_widget;
    }
    scr->state=screen_Idle;
  }
  return ret;
}

void default_screenUpdate(screen_t *scr) {
  if(scr->widgets) {
    widget_t *current_widget = scr->widgets;
    while(current_widget) {
      if(current_widget->update){
        current_widget->update(current_widget);
      }
      current_widget = current_widget->next_widget;
    }

  }
}

void default_init(screen_t *scr) {
  if(scr->current_widget)
    return;
  if(!scr->widgets)
    return;
  int c = 1000;
  widget_t *w = scr->widgets;
  selectable_widget_t *sel;
  while(w) {
    sel = extractSelectablePartFromWidget(w);
    if(sel) {
      if(sel->tab < c) {
        c = sel->tab;
      }
    }
    w = w->next_widget;
  }
  w = scr->widgets;
  scr->current_widget = NULL;
  while(w) {
    sel = extractSelectablePartFromWidget(w);
    if(sel) {
      if(sel->tab == c) {
        scr->current_widget = w;
        if(sel->state == widget_idle)
          sel->state = widget_selected;
        return;
      }
    }
    w = w->next_widget;
  }
}

void screen_setDefaults(screen_t *scr) {
    scr->next_screen = NULL;
    scr->widgets = NULL;
    scr->current_widget = NULL;
    scr->init = &default_init;
    scr->create = NULL;
    scr->processInput = &default_screenProcessInput;
    scr->update = &default_screenUpdate;
    scr->draw = &default_screenDraw;
    scr->onEnter = NULL;
    scr->onExit = NULL;
    scr->state = screen_Erase;
    scr->enabled = true;
    scr->index = 0;
}
