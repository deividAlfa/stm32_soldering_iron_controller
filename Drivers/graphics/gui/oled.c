/*
 * oled.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */


#include <stdlib.h>
#include "oled.h"

static screen_t *screens = NULL;
screen_t *current_screen;
uint32_t current_time;
uint32_t screen_timer;
uint8_t last_scr;
static RE_State_t* RE_State;

RE_Rotation_t (*RE_GetData)(RE_State_t*);
RE_Rotation_t RE_Rotation;


void oled_addScreen(screen_t *screen, uint8_t index){
  screen->processInput = &default_screenProcessInput;
  screen->init = &default_init;
  screen->draw = &default_screenDraw;
  screen->update = &default_screenUpdate;
  screen->onEnter = NULL;
  screen->onExit = NULL;
  screen->index = index;
  screen->next_screen = NULL;
  screen->widgets = NULL;
  screen->current_widget = NULL;
  if(screens == NULL) {
    screens = screen;
  }
  else {
    screen_t *temp = screens;
    while(temp->next_screen) {
      temp = temp->next_screen;
    }
    temp->next_screen = screen;
  }
}

void oled_draw() {

  if(oled.status!=oled_idle) { return; }                // If Oled busy, skip update

  current_time = HAL_GetTick();
  if(current_screen->draw(current_screen)){
    update_display();                                   // Only update if something was drawn
  }
}

void oled_update() {
  if(current_screen->update){
    current_time = HAL_GetTick();
    current_screen->update(current_screen);
  }
  oled_draw();
}

void oled_init(RE_Rotation_t (*GetData)(RE_State_t*), RE_State_t *State) {
  screen_timer = current_time = HAL_GetTick();
  RE_State = State;
  RE_GetData = GetData;
  screen_t *scr = screens;
  while(scr) {
    if(scr->index == 0) {
      if(scr->create){                                // Create entering screen
        scr->create(scr);
      }
      scr->init(scr);
      current_screen = scr;
      return;
    }
  }
}
static RE_State_t* RE_State;

// Free all screen resources after exiting it.
void oled_destroy_screen(screen_t *scr){
  widget_t *w = scr->widgets;
  widget_t *next;
  if(!w){ return; }
  do{
    next=w->next_widget;
    if(w->content){
      if(w->type==widget_combo){
        comboBox_item_t *Item = ((comboBox_widget_t*)w->content)->first;
        comboBox_item_t *Next;
        if(Item){
          do{
            Next=Item->next_item;
            if( (Item->type==combo_Editable) || (Item->type==combo_MultiOption)){
              _free(Item->widget);
            }
            _free(Item);
            Item=Next;
          }while(Next);
        }
      }
      _free(w->content);
    }
    _free(w);
    w = next;
  }while(next);
  scr->current_widget=NULL;
  scr->widgets=NULL;
}

void oled_backup_comboStatus(screen_t *scr){
  if((scr->current_widget)&&(scr->current_widget->type==widget_combo)){
    comboBox_widget_t *combo = (comboBox_widget_t*)scr->current_widget->content;
    if(combo->currentItem){
      scr->backup_combo_index = comboItemToIndex(scr->current_widget,combo->currentItem);
      scr->backup_combo_scroll=combo->currentScroll;
    }
    else{
      scr->backup_combo_index=0;
      scr->backup_combo_scroll=0;
    }
  }
}
void oled_restore_comboStatus(screen_t *scr){
  if((scr->current_widget)&&(scr->current_widget->type==widget_combo)){
    comboBox_widget_t *combo = (comboBox_widget_t*)scr->current_widget->content;
    combo->currentScroll=scr->backup_combo_scroll;
    combo->currentItem = comboIndexToItem(scr->current_widget,scr->backup_combo_index);
  }
}
void oled_processInput(void) {
  RE_Rotation = (*RE_GetData)(RE_State);
  current_time = HAL_GetTick();
  int ret = current_screen->processInput(current_screen, RE_Rotation, RE_State);
  if(ret > 0) {   // -1 do nothing, -2 nothing processed
    screen_t *scr = screens;
    FillBuffer(BLACK, fill_dma);
    current_time = HAL_GetTick();
    while(scr) {
      if(scr->index == ret) {
        last_scr = current_screen->index;

        if(current_screen->onExit){
          current_screen->onExit(scr);
        }

        if(current_screen->create){                     // If "create" exists,it means it's a dynamic screen
          oled_backup_comboStatus(current_screen);      // Save combo position
          oled_destroy_screen(current_screen);          // Destroy exiting screen
        }

        if(scr->create){                                // Create entering screen
          scr->create(scr);
          oled_restore_comboStatus(scr);                // Restore combo position
        }

        screen_timer = current_time = HAL_GetTick();

        scr->init(scr);
        if(scr->onEnter){
          scr->onEnter(current_screen);
        }

        current_screen = scr;

        ret = scr->processInput(scr, Rotate_Nothing, RE_State);     // Force first pass without activity to update screen

        if( ret >0){
          scr = screens;                                            // If new screen returned a different screen, start over
        }
        else{
          if(scr->update){
            scr->update(scr);
          }
          scr->refresh=screen_Erased;
          break;
        }
      }
      scr = scr->next_screen;
    }
  }
}
void oled_handle(void){
  oled_processInput();
  oled_update();
}
