/*
 * screen.cpp
 *
 *  Created on: Jul 31, 2017
 *      Author: jose
 */

#include "screen.h"
#include <stdlib.h>
#include "string.h"
#include <math.h>

widget_t *screen_addWidget(screen_t *scr) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		last_widget = scr->widgets;
		while(last_widget->next_widget)
			last_widget = last_widget->next_widget;
	}

	widget_t *new_widget = malloc(sizeof(widget_t));
	if(new_widget == NULL)
		Error_Handler();
	new_widget->next_widget = NULL;
	new_widget->parent = scr;
	if(last_widget)
		last_widget->next_widget = new_widget;
	else
		scr->widgets = new_widget;
	return new_widget;
}

widget_t * screen_tabToWidget(screen_t * scr, uint8_t tab) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		last_widget = scr->widgets;
		while(last_widget) {
			if(last_widget->type == widget_editable) {
				if(last_widget->editable.selectable.tab == tab)
					return last_widget;
			}
			last_widget = last_widget->next_widget;
		}
	}
	return last_widget;
}

int default_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	int ret = -1;
	if(input == Rotate_Increment_while_click) {
		uint8_t i = scr->index;
		++i;
		if(i == screen_last_scrollable)
			i = 1;
		return i;
	}
	else if(input == Rotate_Decrement_while_click) {
		uint8_t i = scr->index;
		--i;
		if(i == 0)
			i = screen_last_scrollable - 1;
		return i;
	}
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
		last_widget = scr->widgets;
		while(last_widget) {
			if(last_widget->draw)
				last_widget->draw(last_widget);
			last_widget = last_widget->next_widget;
		}
	}
}

void default_screenUpdate(screen_t *scr) {
	widget_t *last_widget = NULL;
	if(scr->widgets) {
		last_widget = scr->widgets;
		displayOnly_wiget_t *dis;
		while(last_widget) {
			dis = extractDisplayPartFromWidget(last_widget);
			if(dis->update) {
					dis->update(last_widget);
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
