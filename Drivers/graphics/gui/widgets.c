/*
 * widgets.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "widgets.h"
#include "screen.h"
#include "oled.h"
#include <stdio.h>

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *widget) {
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

editable_widget_t * extractEditablePartFromWidget(widget_t *widget) {
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
		case widget_bmp_button:
			return &widget->buttonWidget.selectable;
		case widget_combo:
			return &widget->comboBoxWidget.selectable;
		default:
			return NULL;
			break;
	}
}
void widgetDefaultsInit(widget_t *w, widgetType t, char* c, uint8_t len){
	w->type = t;
	w->draw = &default_widgetDraw;
	w->enabled = 1;
	w->font_size = &FONT_8X14_reduced;
	w->inverted = 0;
	w->posX = 0;
	w->posY = 0;
	w->next_widget = NULL;
	w->displayString = c;
	w->dispStrlen = sz;
	selectable_widget_t *sel;
	sel = extractSelectablePartFromWidget(w);
	if(sel) {
		sel->previous_state = widget_idle;
		sel->state = widget_idle;
		sel->tab = 0;
		sel->processInput = &default_widgetProcessInput;
		sel->longPressAction = NULL;
		sel->onEditAction = NULL;
		sel->onSelectAction = NULL;
	}
	switch (t) {
		case widget_bmp:
			w->displayBmp.bmp = NULL;
			break;
		case widget_display:
			w->displayWidget.getData = NULL;
			w->displayWidget.number_of_dec = 0;
			w->displayWidget.type = field_uinteger16;
			w->displayWidget.update = &default_widgetUpdate;
			w->displayWidget.justify = justify_left;
			w->displayWidget.hasEndStr = 0;
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
			w->comboBoxWidget.first = NULL;
			w->draw = &comboBoxDraw;
			w->comboBoxWidget.currentScroll = 0;
			w->comboBoxWidget.selectable.processInput = &comboBoxProcessInput;
			break;

		case widget_bmp_button:
			w->buttonWidget.bmp = NULL;
		case widget_button:
			w->buttonWidget.action = NULL;
			break;
		case widget_label:
			break;
		default:
			break;
	}
}


static void insertDot(char *str, uint8_t dec) {//TODO Fix PID screen values >"99.9" appear as "1000.x"
	char pos=0;
	if(!dec || str==NULL){
		return;
	}
	for(int x = strlen(str); x > (int)strlen(str) - (int)dec - 2; --x) {
		str[x + 1] = str[x];
	}
	str[strlen(str) - dec - 1] = '.';

	if(str[strlen(str) - dec - 2] == ' '){			// If no number before '.'
		str[strlen(str) - dec - 2] = '0';			// Put a zero
	}
	for(int x = 0; x < (int)strlen(str); x++) {		// Find '.' and fill zeros if spaces are found until the first number
		if(!pos){
			if(str[x] == '.'){ pos =1; }
		}
		else{
			if(str[x] ==' '){ str[x] ='0'; }
			else{
				break;						// Found a number
			}
		}
	}
}
void default_widgetUpdate(widget_t *widget) {
	void *data;
	int32_t val_ui;
	int8_t val_ui_size, decimals=0, endStrLen=0;
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(widget);

	if(!dis){ return; }

	data = dis->getData();
	UG_FontSelect(widget->font_size);

	switch (widget->type) {
	case widget_multi_option:
		widget->multiOptionWidget.currentOption = *(uint8_t*)data;
		break;

	default:
		switch (dis->type) {
		case field_integer16:
		case field_uinteger16:
		case field_int32:
			// To keep the compiler happy ensuring that displayString is not overflowed
			if(dis->hasEndStr==1){
				endStrLen = strlen(widget->EndStr);
			}
			// Get the data
			if(dis->type==field_integer16){
				val_ui = *(int16_t*)data;
			}
			else if(dis->type==field_uinteger16){
				val_ui = *(uint16_t*)data;
			}
			else if(dis->type==field_int32){
				val_ui = *(int32_t*)data;
			}
			if(dis->number_of_dec){														// If decimals used
				if((dis->number_of_dec+endStrLen)>widget->dispStrlen){				// If decimals larger than display string length (-2 for '.' + string termination)
					decimals = (widget->dispStrlen) - endStrLen;						// Reduce decimals
				}
				else{
					decimals = dis->number_of_dec;										// Else, leave all the decimals
				}
				if(val_ui){																// If val_ui has value
					val_ui_size = snprintf(NULL, 0, "%ld", val_ui); 					// Get val_ui length
					if( (val_ui_size + decimals+endStrLen) > widget->dispStrlen ){	// If val_ui length + decimals larger than displayString size
						if((val_ui_size + endStrLen)>widget->dispStrlen){	// If val_ui + endStrLen length already larger than displayString size
							decimals = 0;												// Don't use decimals
						}
						else{
							decimals = widget->dispStrlen - (val_ui_size + endStrLen);// else, reduce number of decimals to fit in displayString
						}
					}
				}
			}
			if(widget->displayWidget.justify == justify_right){
				snprintf(widget->displayString,widget->dispStrlen+1,"%*ld", widget->dispStrlen - decimals-endStrLen, val_ui);
			}
			else{
				snprintf(widget->displayString,widget->dispStrlen+1,"%ld", val_ui);
				}
			insertDot(widget->displayString, decimals);
			if(endStrLen){
				strcat(widget->displayString, widget->EndStr);
			}
			break;

		case field_string:
			break;

		default:
			break;
		}
	}
}
enum{ select_frame=0, edit_frame};

void default_widgetDraw(widget_t *widget) {

	UG_COLOR selFrameColor = C_BLACK;
	bool selFrameDraw = 0;
	UG_COLOR editFrameColor = C_BLACK;
	bool editFrameDraw = 0;

	uint8_t c;
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(widget);
	editable_widget_t* edit = extractEditablePartFromWidget(widget);
	selectable_widget_t* sel = extractSelectablePartFromWidget(widget);
	uint8_t cHeight=widget->font_size->char_height;
	uint8_t cWidth=widget->font_size->char_width;
	uint8_t wiY=widget->posY;
	uint8_t wiX=widget->posX;
	char space[widget->dispStrlen+1];

	if(widget->dispStrlen){
		for(c=0;c<widget->dispStrlen;c++){
			space[c]=' ';										// Fill with spaces
		}
		space[c]=0;												// Null termination
	}

	if(!widget->enabled){ return; }
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);

	if(!widget->inverted){
		UG_SetBackcolor ( C_BLACK ) ;
		UG_SetForecolor ( C_WHITE ) ;
	}
	else{
		UG_SetBackcolor ( C_WHITE ) ;
		UG_SetForecolor ( C_BLACK ) ;
	}

	if(sel) {
		switch (sel->state) {
			case widget_edit:
				if(sel->previous_state != widget_edit){

					selFrameDraw=1;
					selFrameColor = C_BLACK;
					editFrameDraw=1;
					editFrameColor = C_WHITE;

					//UG_SetBackcolor ( C_WHITE ) ;
					//UG_SetForecolor ( C_BLACK ) ;
				}
				break;

			case widget_selected:
				if(sel->previous_state == widget_edit){
					selFrameDraw=1;
					selFrameColor = C_WHITE;
					editFrameDraw=1;
					editFrameColor = C_BLACK;
				}
				else if(sel->previous_state == widget_idle){
					selFrameDraw=1;
					selFrameColor = C_WHITE;
				}
				break;

			case widget_idle:
				if(sel->previous_state != widget_idle){
					selFrameDraw=1;
					selFrameColor = C_BLACK;
				}
				break;
			default:
				if(sel->state != sel->previous_state){
					selFrameDraw=1;
					selFrameColor = C_BLACK;
					editFrameDraw=1;
					editFrameColor = C_BLACK;
				}
				break;
		}
	}

	UG_FontSelect(widget->font_size);

	switch(widget->type){

		case widget_bmp:
			UG_DrawBMP_MONO(wiX ,wiY , widget->displayBmp.bmp);
			break;

		case widget_bmp_button:
			UG_DrawBMP_MONO(wiX ,wiY , widget->buttonWidget.bmp);
			break;

		case widget_display:
		case widget_editable:
			space[widget->dispStrlen] = (char)'\0';
			UG_PutString(wiX ,wiY , space);
			if(dis->type == field_string){

				UG_SetBackcolor ( C_BLACK ) ;
				UG_SetForecolor ( C_WHITE ) ;
				UG_PutString(wiX ,wiY ,extractDisplayPartFromWidget(widget)->getData());
			}
			else{
				widget->displayString[widget->dispStrlen] = (char)'\0';
				UG_PutString(wiX ,wiY , widget->displayString);

			}
			if((dis->type == field_string) && (widget->type == widget_editable) && (sel->state == widget_edit) ){
				UG_PutChar(widget->displayString[edit->current_edit], wiX + cWidth * edit->current_edit, wiY,  C_WHITE, C_BLACK);
				UG_DrawLine(wiX + cWidth * edit->current_edit+1,wiY+ cHeight,wiX + cWidth * edit->current_edit+cWidth-3,wiY+ cHeight, C_WHITE);
				UG_DrawLine(wiX + cWidth * edit->current_edit+1,wiY+ cHeight+1,wiX + cWidth * edit->current_edit+cWidth-3,wiY+ cHeight+1, C_WHITE);
				if(edit->current_edit){
					UG_DrawLine(wiX + cWidth * (edit->current_edit-1)+1,wiY+ cHeight,wiX + cWidth * (edit->current_edit-1)+cWidth-3,wiY+ cHeight, C_BLACK);
					UG_DrawLine(wiX + cWidth * (edit->current_edit-1)+1,wiY+ cHeight+1,wiX + cWidth * (edit->current_edit-1)+cWidth-3,wiY+ cHeight+1, C_BLACK);
				}
			}
			break;

		case widget_button:
		case widget_label:
			UG_PutString(wiX ,wiY , widget->displayString);
			break;

		case widget_multi_option:
			UG_PutString(wiX ,wiY, widget->multiOptionWidget.options[widget->multiOptionWidget.currentOption]);
			break;
	}
/*
	if(selFrameDraw) {
		switch(widget->type){
			case widget_bmp_button:
				UG_DrawRoundFrame(wiX - 2, wiY - 2,	wiX + widget->buttonWidget.bmp->width + 1, wiY + widget->buttonWidget.bmp->height + 1, 2, selFrameColor);
				break;

			case widget_editable:
			case widget_button:
			case widget_label:
			case widget_multi_option:
				UG_DrawRoundFrame(wiX - 2, wiY - 1,	wiX + widget->dispStrlen * cWidth + 1,  wiY + cHeight -1, 2, selFrameColor);
				break;
		}
	}

*/

	if(selFrameColor){
		if(editFrameDraw) {
			switch(widget->type){
				case widget_editable:
				case widget_multi_option:
				//	UG_DrawLine(wiX - 4, wiY - 1, wiX - 4, wiY + cHeight -1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY - 1, wiX - 3, wiY + cHeight -1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY - 1, wiX-1, wiY - 1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY + cHeight -1, wiX-1, wiY + cHeight -1, editFrameColor);

					//UG_DrawLine(wiX+1 + widget->dispStrlen * cWidth + 2, wiY - 1, wiX+1 + widget->dispStrlen * cWidth + 2, wiY + cHeight - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth + 2, wiY - 1, wiX + widget->dispStrlen * cWidth + 2, wiY + cHeight - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth, wiY - 1, wiX + widget->dispStrlen * cWidth + 2, wiY - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth, wiY + cHeight -1, wiX + widget->dispStrlen * cWidth + 2, wiY + cHeight -1, editFrameColor);

					break;
			}
		}
		if(selFrameDraw) {
			switch(widget->type){
				case widget_bmp_button:
					UG_DrawRoundFrame(wiX - 2, wiY - 2,	wiX + widget->buttonWidget.bmp->width + 1, wiY + widget->buttonWidget.bmp->height + 1, 2, selFrameColor);
					break;

				case widget_editable:
				case widget_button:
				case widget_label:
				case widget_multi_option:
					UG_DrawRoundFrame(wiX - 2, wiY - 1,	wiX + widget->dispStrlen * cWidth + 1,  wiY + cHeight -1, 2, selFrameColor);
					break;
			}
		}
	}
	else{
		if(selFrameDraw) {
			switch(widget->type){
				case widget_bmp_button:
					UG_DrawRoundFrame(wiX - 2, wiY - 2,	wiX + widget->buttonWidget.bmp->width + 1, wiY + widget->buttonWidget.bmp->height + 1, 2, selFrameColor);
					break;

				case widget_editable:
				case widget_button:
				case widget_label:
				case widget_multi_option:
					UG_DrawRoundFrame(wiX - 2, wiY - 1,	wiX + widget->dispStrlen * cWidth + 1,  wiY + cHeight -1, 2, selFrameColor);
					break;
			}
		}
		if(editFrameDraw) {
			switch(widget->type){
				case widget_editable:
				case widget_multi_option:
					//UG_DrawLine(wiX - 4, wiY - 1, wiX - 4, wiY + cHeight -1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY - 1, wiX - 3, wiY + cHeight -1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY - 1, wiX-1, wiY - 1, editFrameColor);
					UG_DrawLine(wiX - 3, wiY + cHeight -1, wiX-1, wiY + cHeight -1, editFrameColor);

					//UG_DrawLine(wiX+1 + widget->dispStrlen * cWidth + 2, wiY - 1, wiX+1 + widget->dispStrlen * cWidth + 2, wiY + cHeight - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth + 2, wiY - 1, wiX + widget->dispStrlen * cWidth + 2, wiY + cHeight - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth, wiY - 1, wiX + widget->dispStrlen * cWidth + 2, wiY - 1, editFrameColor);
					UG_DrawLine(wiX + widget->dispStrlen * cWidth, wiY + cHeight -1, wiX + widget->dispStrlen * cWidth + 2, wiY + cHeight -1, editFrameColor);

					break;
			}
		}
	}

}

void comboBoxDraw(widget_t *widget) {
	uint8_t wiY=widget->posY;
	uint16_t yDim = UG_GetYDim() - wiY;
	uint8_t cHeight=widget->font_size->char_height;
	uint8_t cWidth=widget->font_size->char_width;
	uint16_t height = cHeight+2;		//+2 to allow separation between items
	comboBox_item_t *item = widget->comboBoxWidget.first;
	uint8_t scroll = 0;

	if(!item){ return; }										// If null item return (would cause hard fault if widget not properly initialized)
	while(scroll < widget->comboBoxWidget.currentScroll) {
		if(!item->next_item)
			break;
		item = item->next_item;
		if(item->enabled)
			++scroll;
	}
	UG_SetBackcolor ( C_BLACK ) ;
	UG_SetForecolor ( C_WHITE ) ;
	UG_FontSelect(widget->font_size);
	for(uint8_t y = 0; y < yDim / height; ++y) {
		UG_FillFrame(0, y * height + wiY, UG_GetXDim(), y * height + wiY + cHeight, C_BLACK);
		if ((item->type==combo_Screen)||(item->type==combo_Action)){																										// If screen combo item, just draw the label
			UG_PutString( (UG_GetXDim()-(strlen(item->text)* cWidth))/2 ,y * height + wiY +1, item->text);
		}
		else if (item->type==combo_Option){																									// If option combo item
			UG_PutString( 2 ,y * height + wiY+1, item->text);																		// Draw the label
			item->widget->font_size=widget->font_size;																						// Set widget font size the same font size as the combo widget
			item->widget->posY=y * height + wiY+1;																					// Set widget Ypos same as the current combo option
			default_widgetUpdate(item->widget);																								// Update widget
			default_widgetDraw(item->widget);																								// Draw widget
			if(extractSelectablePartFromWidget(item->widget)->state==widget_edit){															// Restore color (Edit mode widget inverts colors)
				UG_SetBackcolor ( C_BLACK ) ;
				UG_SetForecolor ( C_WHITE ) ;
			}
		}
		if(item == widget->comboBoxWidget.currentItem) {																					// Draw the frame if the current combo item is selected
			UG_DrawRoundFrame(0, y * height + wiY, UG_GetXDim() -1, y * height + wiY + cHeight, 2, C_WHITE );
			//UG_DrawFrame(0, y * height + wiY, UG_GetXDim() -1, y * height + wiY + cHeight, C_WHITE);
		}

		do {
			item = item->next_item;
		}while(item && !item->enabled);

		if(!item){
			break;
		}
	}
	return;
}

uint8_t comboItemToIndex(widget_t *combo, comboBox_item_t *item) {
	uint8_t index = 0;
	comboBox_item_t *i = combo->comboBoxWidget.first;
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
	uint16_t height = widget->font_size->char_height+2;
	uint8_t maxIndex = yDim / height;
	uint8_t lastIndex = widget->comboBoxWidget.currentScroll + maxIndex -1;
	selectable_widget_t *sel;
	if (widget->comboBoxWidget.currentItem->type==combo_Option){													// If combo option type
		sel = extractSelectablePartFromWidget(widget->comboBoxWidget.currentItem->widget);									// Get selectable data
	}
	if(input == Click){																								// If clicked
		if (widget->comboBoxWidget.currentItem->type==combo_Option){												// If combo option type
			if(sel->state==widget_idle){																					// If widget idle
				sel->state=widget_edit;																				// Change to edit
			}
			else if(sel->state==widget_edit){																			// If widget in edit mode
				sel->state=widget_idle;																				// Change to idle
			}
			return -1;																								// Do nothing else
		}
		if (widget->comboBoxWidget.currentItem->type==combo_Action){												// If combo option type
			return widget->comboBoxWidget.currentItem->action();
		}
		else if (widget->comboBoxWidget.currentItem->type==combo_Screen){											// If combo screen type
			return widget->comboBoxWidget.currentItem->action_screen;												// Return screen index
		}
	}
	if((input == Rotate_Increment)||(input == Rotate_Decrement)){													// If rotation data
		if(widget->comboBoxWidget.currentItem->type==combo_Option){													// If combo option type
			if(sel->state==widget_edit){																			// If widget in edit mode
				default_widgetProcessInput(widget->comboBoxWidget.currentItem->widget, input, state);				// Process widget
				return -1;																							// Return
			}
		}
		if(input == Rotate_Increment) {
			//Set comboBox item to next comboBox
			comboBox_item_t *current = widget->comboBoxWidget.currentItem->next_item;
			// if comboBox is disabled, skip. While comboBox still valid
			while(current && !current->enabled) {
				current = current->next_item;
			}
			// If comboBox valid(didn't reach end)
			if(current) {
				widget->comboBoxWidget.currentItem = current;
				uint8_t index = comboItemToIndex(widget, current);
				if(index > lastIndex)
					++widget->comboBoxWidget.currentScroll;
			}
		}
		else if(input == Rotate_Decrement) {
			comboBox_item_t *current = NULL;
			// If comboBox is not the first element
			if(widget->comboBoxWidget.currentItem != widget->comboBoxWidget.first){
				do {
					current = widget->comboBoxWidget.first;
					while(current->next_item != widget->comboBoxWidget.currentItem) {
						current = current->next_item;
					}
					widget->comboBoxWidget.currentItem = current;
				}while(!current->enabled);
				uint8_t index = comboItemToIndex(widget, current);
				if(index < firstIndex)
					--widget->comboBoxWidget.currentScroll;
			}
		}
	}
	return -1;
}
//returns -1 if processed, -2 if not processed, or next screen
int default_widgetProcessInput(widget_t *widget, RE_Rotation_t input, RE_State_t *state) {
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(widget);
	selectable_widget_t* sel = extractSelectablePartFromWidget(widget);
	editable_widget_t* edit = extractEditablePartFromWidget(widget);
	uint8_t cHeight=widget->font_size->char_height;
	uint8_t cWidth=widget->font_size->char_width;
	uint8_t wiY=widget->posY;
	uint8_t wiX=widget->posX;

	if(input == Rotate_Nothing){
		return -1;
	}
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
					if((widget->type == widget_button)||(widget->type == widget_bmp_button)){
						return widget->buttonWidget.action(widget);
					}
					if(dis->type == field_string){
						edit->current_edit = 0;
					}
					sel->state = widget_edit;
					sel->previous_state = widget_selected;
					if(sel->onEditAction){
						return sel->onEditAction(widget);
					}
					break;

				case widget_edit:
					if(dis->type == field_string) {
						++edit->current_edit;
						if(edit->current_edit == widget->dispStrlen)
						{
							sel->state = widget_selected;
							sel->previous_state = widget_edit;
							UG_DrawLine(wiX + cWidth * (edit->current_edit-1)+1,wiY+ cHeight,wiX + cWidth * (edit->current_edit-1)+cWidth-3,wiY+ cHeight, C_BLACK);
							UG_DrawLine(wiX + cWidth * (edit->current_edit-1)+1,wiY+ cHeight+1,wiX + cWidth * (edit->current_edit-1)+cWidth-3,wiY+ cHeight+1, C_BLACK);
						}
					}
					else {
						sel->state = widget_selected;
						sel->previous_state = widget_edit;
						if(sel->onSelectAction){
							return sel->onSelectAction(widget);
						}
					}
					break;
				default:
					break;
			}
			return -1;
		}
		if((widget->type == widget_editable) && (sel->state == widget_edit)) {
			uint16_t ui16;
			char *str;
			int16_t inc;
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
				if(inc>0){
					if((uint16_t)(ui16 + inc)<ui16){
						ui16 = widget->editable.max_value;		// Check we don't overflow the int
					}
					else{
						ui16 = ui16 + inc;
					}
					if(ui16 > widget->editable.max_value){		// Check we don't pass the max value
						ui16 = widget->editable.max_value;
					}
				}
				else{
					if((uint16_t)(ui16 + inc)>ui16){
						ui16 = widget->editable.min_value;		// Check we don't underflow the int
					}
					else{
						ui16 = ui16 + inc;
					}
					if(ui16 < widget->editable.min_value){		// Check we don't pass the min value
						ui16 = widget->editable.min_value;
					}
				}
				widget->editable.setData(&ui16);
				break;
			case field_string:
				str = (char*)widget->editable.inputData.getData();
				strcpy(widget->displayString, str);
				widget->displayString[edit->current_edit] += inc;
				if(widget->displayString[edit->current_edit] < 0x20) {
					if(inc > 0) {
						widget->displayString[edit->current_edit] = 0x20;
					}
					else
						widget->displayString[edit->current_edit] = 0x7e;
				}
				if(widget->displayString[edit->current_edit] > 0x7e) {
					if(inc > 0) {
						widget->displayString[edit->current_edit] = 0x20;
					}
					else
						widget->displayString[edit->current_edit] = 0x7e;
				}

				widget->editable.setData(widget->displayString);
				break;
			default:
				break;
			}
			return -1;
		}
		else if((widget->type == widget_multi_option) && (sel->state == widget_edit)) {
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

void comboAddScreen(comboBox_item_t* item,widget_t *combo, char *label, uint8_t actionScreen){
	if(combo->type==widget_combo){
		item->text = label;
		item->next_item = NULL;
		item->action_screen = actionScreen;
		item->type = combo_Screen;
		item->enabled = 1;

		comboBox_item_t *next = combo->comboBoxWidget.first;

		if(!next) {
			combo->comboBoxWidget.first = item;
			combo->comboBoxWidget.currentItem = item;
			return;
		}
		while(next->next_item){
			next = next->next_item;
		}
		next->next_item = item;
	}
}

void comboAddOption(comboBox_item_t* item, widget_t *combo, char *label, widget_t *widget){

	if( (widget->type==widget_editable) || (widget->type==widget_multi_option) ){				// Only allow Editable  or multioption widgets
		item->text = label;
		item->next_item = NULL;
		item->widget = widget;
		item->type = combo_Option;
		item->enabled = 1;
		comboBox_item_t *next = combo->comboBoxWidget.first;

		if(!next) {
			combo->comboBoxWidget.first = item;
			combo->comboBoxWidget.currentItem = item;
			return;
		}
		while(next->next_item){
			next = next->next_item;
		}
		next->next_item = item;
	}
}
void comboAddAction(comboBox_item_t* item, widget_t *combo, char *label, int (*action)()){

	if(action){												// If not null
		item->text = label;
		item->next_item = NULL;
		item->action = action;
		item->type = combo_Action;
		item->enabled = 1;
		comboBox_item_t *next = combo->comboBoxWidget.first;

		if(!next) {
			combo->comboBoxWidget.first = item;
			combo->comboBoxWidget.currentItem = item;
			return;
		}
		while(next->next_item){
			next = next->next_item;
		}
		next->next_item = item;
	}
}

void comboResetIndex(widget_t *combo){
	combo->comboBoxWidget.currentItem = combo->comboBoxWidget.first;
	combo->comboBoxWidget.currentScroll=0;
}
