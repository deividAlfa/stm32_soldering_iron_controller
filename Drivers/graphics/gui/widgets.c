/*
 * widgets.c
 *
 *  Created on: Aug 11, 2017
 *      Author: jose
 */

#include "widgets.h"
#include "screen.h"

displayOnly_wiget_t * extractDisplayPartFromWidget(widget_t *widget) {
	switch (widget->type) {
		case widget_display:
			return &widget->displayWidget;
			break;
		case widget_editable:
			return &widget->editable.inputData;
			break;
		case widget_multi_option:
			return &widget->multiOptionWidget.editable.inputData;
		default:
			return NULL;
			break;
	}
}

editable_wiget_t * extractEditablePartFromWidget(widget_t *widget) {
	switch (widget->type) {
		case widget_editable:
			return &widget->editable;
			break;
		case widget_multi_option:
			return &widget->multiOptionWidget.editable;
		default:
			return NULL;
			break;
	}
}
selectable_widget_t * extractSelectablePartFromWidget(widget_t *widget) {
	if(!widget)
		return NULL;
	switch (widget->type) {
		case widget_editable:
			return &widget->editable.selectable;
			break;
		case widget_multi_option:
			return &widget->multiOptionWidget.editable.selectable;
		case widget_button:
			return &widget->buttonWidget.selectable;
		case widget_combo:
			return &widget->comboBoxWidget.selectable;
		default:
			return NULL;
			break;
	}
}
void widgetDefaultsInit(widget_t *w, widgetType t) {
	w->type = t;
	w->draw = &default_widgetDraw;
	w->enabled = 1;
	w->font_size = &FONT_8X14;
	w->inverted = 0;
	w->posX = 0;
	w->posY = 0;
	w->reservedChars = 5;
	w->next_widget = NULL;
	selectable_widget_t *sel;
	sel = extractSelectablePartFromWidget(w);
	if(sel) {
		sel->previous_state = widget_idle;
		sel->state = widget_idle;
		sel->tab = 0;
		sel->processInput = &default_widgetProcessInput;
		sel->longPressAction = NULL;
	}
	switch (t) {
		case widget_bmp:
			w->displayBmp.bmp.p = NULL;
			break;
		case widget_display:
			w->displayWidget.getData = NULL;
			w->displayWidget.number_of_dec = 0;
			w->displayWidget.type = field_uinteger16;
			w->displayWidget.update = &default_widgetUpdate;
			break;
		case widget_editable:
			w->editable.big_step = 10;
			w->editable.inputData.getData = NULL;
			w->editable.inputData.number_of_dec = 0;
			w->editable.inputData.type = field_uinteger16;
			w->editable.inputData.update = &default_widgetUpdate;
			w->editable.setData = NULL;
			w->editable.step = 1;
			w->editable.selectable.tab = 0;
			w->editable.max_value = 0xFFFF;
			w->editable.min_value = 0;
			break;
		case widget_multi_option:
			w->multiOptionWidget.editable.big_step = 10;
			w->multiOptionWidget.editable.inputData.getData = NULL;
			w->multiOptionWidget.editable.inputData.number_of_dec = 0;
			w->multiOptionWidget.editable.inputData.type = field_uinteger16;
			w->multiOptionWidget.editable.inputData.update = &default_widgetUpdate;
			w->multiOptionWidget.editable.setData = NULL;
			w->multiOptionWidget.editable.step = 1;
			w->multiOptionWidget.editable.selectable.tab = 0;
			w->multiOptionWidget.editable.max_value = 0xFF;
			w->multiOptionWidget.editable.min_value = 0;
			w->multiOptionWidget.currentOption = 0;
			w->multiOptionWidget.defaultOption = 0;
			w->multiOptionWidget.numberOfOptions = 0;
			w->multiOptionWidget.options = NULL;
			break;
		case widget_combo:
			w->comboBoxWidget.currentItem = NULL;
			w->comboBoxWidget.items = NULL;
			w->draw = &comboBoxDraw;
			w->comboBoxWidget.currentScroll = 0;
			w->comboBoxWidget.selectable.processInput = &comboBoxProcessInput;
			break;
		case widget_button:
			w->buttonWidget.action = NULL;
			break;
		case widget_label:
			break;
		default:
			break;
	}
}

static void insertDot(char *str, uint8_t dec) {
	for(int x = strlen(str); x > (int)strlen(str) - (int)dec - 2; --x) {
		str[x + 1] = str[x];
	}
	str[strlen(str) - dec - 1] = '.';
}

void default_widgetUpdate(widget_t *widget) {
	void * data;
	displayOnly_wiget_t *dis = extractDisplayPartFromWidget(widget);
	if(!dis)
		return;
	data = dis->getData();
	UG_FontSelect(widget->font_size);
	uint16_t val_ui16;
	char *str;
	switch (widget->type) {
	case widget_multi_option:
		strcpy(widget->displayString, widget->multiOptionWidget.options[*(uint8_t*)data]);
		widget->multiOptionWidget.currentOption = *(uint8_t*)data;
		break;
	default:
		switch (dis->type) {
			case field_uinteger16:
				val_ui16 = *((uint16_t*)(data));
				sprintf(widget->displayString,"%d", val_ui16);
				if(dis->number_of_dec) {
					insertDot(widget->displayString, dis->number_of_dec);
				}
				break;
			case field_string:
				str = (char*)(data);
				strcpy(widget->displayString, str);
				break;
			default:
				break;
		}
		break;
	}
}

void default_widgetDraw(widget_t *widget) {
	if(!widget->enabled)
		return;
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	if(!widget->inverted) {
		UG_SetBackcolor ( C_BLACK ) ;
		UG_SetForecolor ( C_WHITE ) ;
	} else {
		UG_SetBackcolor ( C_WHITE ) ;
		UG_SetForecolor ( C_BLACK ) ;
	}
	UG_COLOR color;
	uint8_t draw_frame = 0;
	if(widget->type == widget_bmp) {
		UG_DrawBMP(widget->posX ,widget->posY , &widget->displayBmp.bmp);
		return;
	}

	if((widget->type == widget_editable || widget->type == widget_multi_option || widget->type == widget_button)) {
		selectable_widget_t *sel = extractSelectablePartFromWidget(widget);
		if((sel->state != widget_selected) && (sel->previous_state == widget_selected)) {
			draw_frame = 1;
			color = C_BLACK;
		}
			switch (sel->state) {
				case widget_edit:
					UG_SetBackcolor ( C_WHITE ) ;
					UG_SetForecolor ( C_BLACK ) ;
					break;
				case widget_selected:
					draw_frame = 1;
					color = C_WHITE;
					break;
				default:
					UG_SetBackcolor ( C_BLACK ) ;
					UG_SetForecolor ( C_WHITE ) ;
					break;
			}
	}
	UG_FontSelect(widget->font_size);
	char space[sizeof(widget->displayString)] = "           ";
	if((widget->type != widget_label) && (extractDisplayPartFromWidget(widget)->type != field_string)) {
		space[widget->reservedChars] = (char)'\0';
		UG_PutString(widget->posX ,widget->posY , space);
		widget->displayString[widget->reservedChars] = (char)'\0';
		UG_PutString(widget->posX ,widget->posY , widget->displayString);
	}
	else if(extractDisplayPartFromWidget(widget)->type == field_string) {
		UG_SetBackcolor ( C_BLACK ) ;
		UG_SetForecolor ( C_WHITE ) ;
		space[widget->reservedChars] = (char)'\0';
		UG_PutString(widget->posX ,widget->posY , space);
		widget->displayString[widget->reservedChars] = (char)'\0';
		UG_PutString(widget->posX ,widget->posY , widget->displayString);
		if(extractSelectablePartFromWidget(widget)->state == widget_edit)
			UG_PutChar(widget->displayString[extractEditablePartFromWidget(widget)->current_edit], widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit, widget->posY, C_BLACK, C_WHITE);
	}
	else
		UG_PutString(widget->posX ,widget->posY , widget->displayString);
	if(draw_frame) {
		UG_DrawFrame(widget->posX - 1, widget->posY - 1,
											widget->posX + widget->reservedChars * widget->font_size->char_width + 1,
											widget->posY + widget->font_size->char_height -1, color);
	}
}

void comboBoxDraw(widget_t *widget) {
	uint16_t yDim = UG_GetYDim() - widget->posY;
	uint16_t height = widget->font_size->char_height;
	height += 2;
	comboBox_item_t *item = widget->comboBoxWidget.items;
	uint8_t scroll = 0;
	while(scroll < widget->comboBoxWidget.currentScroll) {
		if(!item->next_item)
			break;
		item = item->next_item;
		if(item->enabled)
			++scroll;
	}
	UG_FontSelect(widget->font_size);
	for(uint8_t x = 0; x < yDim / height; ++x) {
		UG_FillFrame(0, x * height + widget->posY -1, UG_GetXDim(), x * height + widget->posY + widget->font_size->char_height, C_BLACK);
		if(item == widget->comboBoxWidget.currentItem) {
			UG_DrawFrame(0, x * height + widget->posY -1, UG_GetXDim() -1, x * height + widget->posY + widget->font_size->char_height, C_WHITE);
		}
		UG_PutString(UG_GetXDim() / 2 - (strlen(item->text) / 2 * widget->font_size->char_width) ,x * height + widget->posY, item->text);
		do {
			item = item->next_item;
		}while(item && !item->enabled);
		if(!item)
			break;
	}
	return;
}

uint8_t comboItemToIndex(widget_t *combo, comboBox_item_t *item) {
	uint8_t index = 0;
	comboBox_item_t *i = combo->comboBoxWidget.items;
	while(i && i != item) {
		i = i->next_item;
		if(i->enabled)
			++ index;
	}
	return index;
}
int comboBoxProcessInput(widget_t *widget, RE_Rotation_t input, RE_State_t *state) {
	uint8_t firstIndex = widget->comboBoxWidget.currentScroll;
	uint16_t yDim = UG_GetYDim() - widget->posY;
	uint16_t height = widget->font_size->char_height;
	height += 2;
	uint8_t maxIndex = yDim / height;
	uint8_t lastIndex = widget->comboBoxWidget.currentScroll + maxIndex -1;
	if(input == Click)
		return widget->comboBoxWidget.currentItem->action_screen;
	else if(input == Rotate_Increment) {
		comboBox_item_t *current = widget->comboBoxWidget.currentItem->next_item;
		while(current && !current->enabled) {
			current = current->next_item;
		}
		if(current) {
			widget->comboBoxWidget.currentItem = current;
			uint8_t index = comboItemToIndex(widget, current);
			if(index > lastIndex)
				++widget->comboBoxWidget.currentScroll;
		}
	}
	else if(input == Rotate_Decrement) {
		if(widget->comboBoxWidget.currentItem == widget->comboBoxWidget.items)
			return -1;
		comboBox_item_t *current = NULL;
		do {
			current = widget->comboBoxWidget.items;
			while(current->next_item != widget->comboBoxWidget.currentItem) {
				current = current->next_item;
			}
			widget->comboBoxWidget.currentItem = current;
		}while(!current->enabled);
		uint8_t index = comboItemToIndex(widget, current);
		if(index < firstIndex)
			--widget->comboBoxWidget.currentScroll;
	}
	return -1;
}
//returns -1 if processed, -2 if not processed, or next screen
int default_widgetProcessInput(widget_t *widget, RE_Rotation_t input, RE_State_t *state) {
	if(input == Rotate_Nothing)
		return -1;
	selectable_widget_t *sel = extractSelectablePartFromWidget(widget);
	if(sel) {
		if(input == LongClick) {
			if(sel->longPressAction)
				return sel->longPressAction(widget);
			else
				input = Click;
		}
		if(input == Click) {
			switch (sel->state) {
				case widget_selected:
					if(widget->type == widget_button)
						return widget->buttonWidget.action(widget);
					if(extractDisplayPartFromWidget(widget)->type == field_string)
						extractEditablePartFromWidget(widget)->current_edit = 0;
					sel->state = widget_edit;
					sel->previous_state = widget_selected;
					break;
				case widget_edit:
					if(extractDisplayPartFromWidget(widget)->type == field_string) {
						++extractEditablePartFromWidget(widget)->current_edit;
						if(extractEditablePartFromWidget(widget)->current_edit == widget->reservedChars)
						{
							sel->state = widget_selected;
							sel->previous_state = widget_edit;
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
		if((widget->type == widget_editable) && (extractSelectablePartFromWidget(widget)->state == widget_edit)) {
			uint16_t ui16;
			char *str;
			int8_t inc;
			if(fabs(state->Diff) > 2) {
				inc = widget->editable.big_step;
				if(state->Diff < 0)
					inc = -1 * inc;
			}
			else
				inc = widget->editable.step * state->Diff;
			switch (widget->editable.inputData.type) {
			case field_uinteger16:
				ui16 = *(uint16_t*)widget->editable.inputData.getData();
				ui16 = ui16 + inc;
				if(ui16 > widget->editable.max_value)
					ui16 = widget->editable.max_value;
				else if(ui16 < widget->editable.min_value)
					ui16 = widget->editable.min_value;
				widget->editable.setData(&ui16);
				break;
			case field_string:
				str = (char*)widget->editable.inputData.getData();
				strcpy(widget->displayString, str);
				widget->displayString[extractEditablePartFromWidget(widget)->current_edit] += inc;
				if(widget->displayString[extractEditablePartFromWidget(widget)->current_edit] < 48) {
					if(inc > 0) {
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 48;
					}
					else
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 122;
				}
				if(widget->displayString[extractEditablePartFromWidget(widget)->current_edit] > 122) {
					if(inc > 0) {
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 48;
					}
					else
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 122;
				}

				widget->editable.setData(widget->displayString);
				break;
			default:
				break;
			}
			return -1;
		}
		else if((widget->type == widget_multi_option) && (extractSelectablePartFromWidget(widget)->state == widget_edit)) {
			int temp = widget->multiOptionWidget.currentOption;
			if(input == Rotate_Increment)
				++temp;
			else if(input == Rotate_Decrement)
				--temp;
			if(temp < 0)
				temp = widget->multiOptionWidget.numberOfOptions - 1;
			else if(temp > widget->multiOptionWidget.numberOfOptions -1)
				temp = 0;
			widget->multiOptionWidget.editable.setData(&temp);
		}
		else if (sel->state == widget_selected) {
			uint8_t next = 0xFF;
			int previous = -1;
			widget_t *next_w = NULL;
			widget_t *previous_w = NULL;
			widget_t *first_w = widget;
			widget_t *last_w = widget;
			widget_t *w = widget->parent->widgets;
			while(w) {
				selectable_widget_t *e = extractSelectablePartFromWidget(w);
				if(e) {
					if((e->tab > sel->tab) && (e->tab < next) && w->enabled) {
						next = e->tab;
						next_w = w;
					}
					if((e->tab < sel->tab) && (e->tab > previous) && w->enabled) {
						previous = e->tab;
						previous_w = w;
					}
					if((e->tab < extractSelectablePartFromWidget(first_w)->tab) && w->enabled)
						first_w = w;
					if((e->tab > extractSelectablePartFromWidget(last_w)->tab) && w->enabled)
						last_w = w;
				}
				w = w->next_widget;
			}
			if(next_w == NULL)
				next_w = first_w;
			if(previous_w == NULL)
				previous_w = last_w;
			if((input == Rotate_Increment) && next_w && (next_w != widget)) {
				sel->state = widget_idle;
				sel->previous_state = widget_selected;
				widget->parent->current_widget = next_w;
				extractSelectablePartFromWidget(next_w)->previous_state = extractSelectablePartFromWidget(next_w)->state;
				extractSelectablePartFromWidget(next_w)->state = widget_selected;
				return -1;
			}
			else if((input == Rotate_Decrement) && previous_w && (previous_w != widget)) {
				sel->state = widget_idle;
				sel->previous_state = widget_selected;
				widget->parent->current_widget = previous_w;
				extractSelectablePartFromWidget(previous_w)->previous_state = extractSelectablePartFromWidget(previous_w)->state;
				extractSelectablePartFromWidget(previous_w)->state = widget_selected;
				return -1;
			}
		}
	}
	return -2;
}

comboBox_item_t *comboAddItem(widget_t *combo, char *label, uint8_t actionScreen) {
	comboBox_item_t *item = malloc(sizeof(comboBox_item_t));
	if(!item)
		return NULL;
	item->text = label;
	item->next_item = NULL;
	item->action_screen = actionScreen;
	item->enabled = 1;

	comboBox_item_t *last = combo->comboBoxWidget.items;
	if(!last) {
		combo->comboBoxWidget.items = item;
		combo->comboBoxWidget.currentItem = item;
		return item;
	}
	while(last->next_item){
		last = last->next_item;
	}
	last->next_item = item;
	return item;
}
