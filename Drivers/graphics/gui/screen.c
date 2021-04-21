/*
 * screen.cpp
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "screen.h"
#include "oled.h"

void screen_addWidget(widget_t *widget, screen_t *scr) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		last_widget = scr->widgets;
		while(last_widget->next_widget)
			last_widget = last_widget->next_widget;
	}

	widget->next_widget = NULL;
	widget->parent = scr;
	if(last_widget)
		last_widget->next_widget = widget;
	else
		scr->widgets = widget;
}

widget_t * screen_tabToWidget(screen_t * scr, uint8_t tab) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		last_widget = scr->widgets;
		while(last_widget) {
			if(last_widget->type == widget_editable) {
				if(((editable_widget_t*)last_widget->content)->selectable.tab == tab)
					return last_widget;
			}
			last_widget = last_widget->next_widget;
		}
	}
	return last_widget;
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

void default_screenDraw(screen_t *scr) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		if(scr->refresh==screenRefresh_eraseNow){
			FillBuffer(BLACK,fill_dma);
		}
		last_widget = scr->widgets;
		while(last_widget) {
			if(last_widget->draw){
				last_widget->draw(last_widget);
			}
			last_widget = last_widget->next_widget;
		}
		scr->refresh=screenRefresh_idle;
	}

}

void default_screenUpdate(screen_t *scr) {
	if(scr->widgets) {
		widget_t *last_widget = scr->widgets;
		while(last_widget) {
			if(last_widget->update){
				last_widget->update(last_widget);
			}
			last_widget = last_widget->next_widget;
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
	scr->processInput = &default_screenProcessInput;
	scr->init = &default_init;
	scr->draw = &default_screenDraw;
	scr->update = &default_screenUpdate;
	scr->onEnter = NULL;
	scr->onExit = NULL;
}
