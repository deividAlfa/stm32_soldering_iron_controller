/*
 * widgets.c
 *
 *  Created on: Aug 11, 2017
 *      Author: jose
 */

#include "widgets.h"
#include "screen.h"
#include "oled.h"
#include <stdio.h>

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
void widgetDefaultsInit(widget_t *w, widgetType t) {
	w->type = t;
	w->draw = &default_widgetDraw;
	w->enabled = 1;
	w->font_size = &FONT_8X14_reduced;
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
		sel->force_state = 0;
		sel->NoHighlight = 0;
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
			w->comboBoxWidget.items = NULL;
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
	if(!dec){
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
	volatile int32_t val_ui;
	volatile int8_t val_ui_size, decimals,endStrLen;
	char *str;

	displayOnly_wiget_t *dis = extractDisplayPartFromWidget(widget);
	if(!dis)
		return;
	data = dis->getData();
	UG_FontSelect(widget->font_size);

	switch (widget->type) {
	case widget_multi_option:
		strcpy(widget->displayString, widget->multiOptionWidget.options[*(uint8_t*)data]);
		widget->multiOptionWidget.currentOption = *(uint8_t*)data;
		break;

	default:
		switch (dis->type) {
		case field_integer16:
		case field_uinteger16:
		case field_int32:
			// To keep the compiler happy ensuring that displayString is not overflowed
			if(dis->hasEndStr==1){
				endStrLen = strlen(widget->endString);
			}
			else{
				endStrLen=0;
			}
			if( widget->reservedChars > (sizeof(widget->displayString) -1) ){
				widget->reservedChars = (sizeof(widget->displayString) -1);
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
				if((dis->number_of_dec+endStrLen)>widget->reservedChars){				// If decimals larger than display string length (-2 for '.' + string termination)
					decimals = widget->reservedChars - endStrLen;						// Reduce decimals
				}
				else{
					decimals = dis->number_of_dec;										// Else, leave all the decimals
				}
				if(val_ui){																// If val_ui has value
					val_ui_size = snprintf(NULL, 0, "%ld", val_ui); 					// Get val_ui length
					if( (val_ui_size + decimals+endStrLen) > widget->reservedChars ){	// If val_ui length + decimals larger than displayString size
						if((val_ui_size + endStrLen)>widget->reservedChars){	// If val_ui + endStrLen length already larger than displayString size
							decimals = 0;												// Don't use decimals
						}
						else{
							decimals = widget->reservedChars - (val_ui_size + endStrLen);// else, reduce number of decimals to fit in displayString
						}
					}
				}
			}
			else{
				decimals = 0;															// Don't use decimals
			}
			if(widget->displayWidget.justify == justify_right){
				snprintf(widget->displayString,sizeof(widget->displayString),"%*ld", (widget->reservedChars - decimals-endStrLen), val_ui);
			}
			else{
				snprintf(widget->displayString,sizeof(widget->displayString),"%ld", val_ui);
				}
			insertDot(widget->displayString, decimals);
			if(endStrLen){
				strcat(widget->displayString, widget->endString);
			}
			break;

		case field_string:
			str = (char*)(data);
			strncpy(widget->displayString, str, sizeof(widget->displayString));
			break;

		default:
			break;
		}
	}
}

void default_widgetDraw(widget_t *widget) {
	UG_COLOR color = C_BLACK;
	uint8_t draw_frame = 0;
	uint8_t c;
	char space[sizeof(widget->displayString)];
	for(c=0;c<sizeof(widget->displayString)-1;c++){
		space[c]=' ';										// Fill with spaces
	}
	space[c]=0;												// Null termination

	if(!widget->enabled){ return; }
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	if(!widget->inverted) {
		UG_SetBackcolor ( C_BLACK ) ;
		UG_SetForecolor ( C_WHITE ) ;
	} else {
		UG_SetBackcolor ( C_WHITE ) ;
		UG_SetForecolor ( C_BLACK ) ;
	}
	if(widget->type == widget_bmp) {
		UG_DrawBMP_MONO(widget->posX ,widget->posY , widget->displayBmp.bmp);
		return;
	}

	if((widget->type == widget_editable || widget->type == widget_multi_option || widget->type == widget_button || widget->type == widget_bmp_button)) {
		selectable_widget_t *sel = extractSelectablePartFromWidget(widget);
		if((sel->state != widget_selected) && (sel->previous_state == widget_selected)) {
			draw_frame = 1;
			color = C_BLACK;
		}
		if(!sel->NoHighlight){	// If not forced in no highlight mode
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
		else{								// Else, show always not highlighted
			draw_frame = 0;
			UG_SetBackcolor ( C_BLACK ) ;
			UG_SetForecolor ( C_WHITE ) ;
		}
	}
	UG_FontSelect(widget->font_size);
	//widget_combo, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button

	if((widget->type == widget_display)||(widget->type == widget_editable)||(widget->type == widget_multi_option)){
		if(extractDisplayPartFromWidget(widget)->type != field_string){
			space[widget->reservedChars] = (char)'\0';
			UG_PutString(widget->posX ,widget->posY , space);
			widget->displayString[widget->reservedChars] = (char)'\0';
			UG_PutString(widget->posX ,widget->posY , widget->displayString);
		}
	}
	// BMP button
	if(widget->type == widget_bmp_button){
		UG_DrawBMP_MONO(widget->posX ,widget->posY , widget->buttonWidget.bmp);
		if(draw_frame) {
			UG_DrawRoundFrame(widget->posX - 2, widget->posY - 2,
			widget->posX + widget->buttonWidget.bmp->width + 1,
			widget->posY + widget->buttonWidget.bmp->height + 1, 2, color);
		}
		return;
	}
	if( (widget->type == widget_display)||(widget->type == widget_editable)||(widget->type == widget_multi_option) ) {
		if((extractDisplayPartFromWidget(widget)->type == field_string)) {
			UG_SetBackcolor ( C_BLACK ) ;
			UG_SetForecolor ( C_WHITE ) ;
			space[widget->reservedChars] = (char)'\0';
			UG_PutString(widget->posX ,widget->posY , space);
			widget->displayString[widget->reservedChars] = (char)'\0';
			UG_PutString(widget->posX ,widget->posY , widget->displayString);
			if((widget->type == widget_editable) && (extractSelectablePartFromWidget(widget)->state == widget_edit)){
				UG_PutChar(widget->displayString[extractEditablePartFromWidget(widget)->current_edit], widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit, widget->posY,  C_WHITE, C_BLACK);
				UG_DrawLine(widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit+1,widget->posY+ widget->font_size->char_height,widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height, C_WHITE);
				UG_DrawLine(widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit+1,widget->posY+ widget->font_size->char_height+1,widget->posX + widget->font_size->char_width * extractEditablePartFromWidget(widget)->current_edit+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height+1, C_WHITE);
				if(extractEditablePartFromWidget(widget)->current_edit){
					UG_DrawLine(widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+1,widget->posY+ widget->font_size->char_height,widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height, C_BLACK);
					UG_DrawLine(widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+1,widget->posY+ widget->font_size->char_height+1,widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height+1, C_BLACK);
				}
			}
		}
	}
	// Text button, label or editable widget
	//widget_combo, widget_label, widget_display, widget_editable, widget_bmp, widget_multi_option, widget_button, widget_bmp_button}widgetType;
	if( (widget->type == widget_button) ||(widget->type == widget_label)){
		UG_PutString(widget->posX ,widget->posY , widget->displayString);
	}
	if(draw_frame) {
		UG_DrawRoundFrame(widget->posX - 2, widget->posY - 1,
				widget->posX + widget->reservedChars * widget->font_size->char_width + 1,
				widget->posY + widget->font_size->char_height -1, 2, color);
		/*
		UG_DrawFrame(widget->posX - 1, widget->posY - 1,
		widget->posX + widget->reservedChars * widget->font_size->char_width + 1,
		widget->posY + widget->font_size->char_height -1, color);
		*/
	}

}
void comboBoxDraw(widget_t *widget) {
	uint16_t yDim = UG_GetYDim() - widget->posY;
	uint16_t height = widget->font_size->char_height+2;		//+2 to allow separation between items
	comboBox_item_t *item = widget->comboBoxWidget.items;
	uint8_t scroll = 0; ;
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
		UG_FillFrame(0, y * height + widget->posY, UG_GetXDim(), y * height + widget->posY + widget->font_size->char_height, C_BLACK);
		if (item->type==combo_Screen){																										// If screen combo item, just draw the label
			UG_PutString( (UG_GetXDim()-(strlen(item->text)* widget->font_size->char_width))/2 ,y * height + widget->posY +1, item->text);
		}
		else if (item->type==combo_Option){																									// If option combo item
			UG_PutString( 2 ,y * height + widget->posY+1, item->text);																		// Draw the label
			item->widget->font_size=widget->font_size;																						// Set widget font size the same font size as the combo widget
			item->widget->posY=y * height + widget->posY+1;																					// Set widget Ypos same as the current combo option
			default_widgetUpdate(item->widget);																								// Update widget
			default_widgetDraw(item->widget);																								// Draw widget
			if(extractSelectablePartFromWidget(item->widget)->state==widget_edit){															// Restore color (Edit mode widget inverts colors)
				UG_SetBackcolor ( C_BLACK ) ;
				UG_SetForecolor ( C_WHITE ) ;
			}
		}
		if(item == widget->comboBoxWidget.currentItem) {																					// Draw the frame if the current combo item is selected
			UG_DrawRoundFrame(0, y * height + widget->posY, UG_GetXDim() -1, y * height + widget->posY + widget->font_size->char_height, 2, C_WHITE );
			//UG_DrawFrame(0, y * height + widget->posY, UG_GetXDim() -1, y * height + widget->posY + widget->font_size->char_height, C_WHITE);
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
			/*
			 * Enable to allow circular mode(go to start after reaching the end )
			 */
			/*
			//Else, we reached the end of comboBox list																// This was added, then removed as I found it more annoying than useful
			else{
				// Search and select the first comboBox
				while(widget->comboBoxWidget.currentItem != widget->comboBoxWidget.items){
					do {
						current = widget->comboBoxWidget.items;
						while(current->next_item != widget->comboBoxWidget.currentItem) {
							current = current->next_item;
						}
						widget->comboBoxWidget.currentItem = current;
					}while(!current->enabled);
					uint8_t index = comboItemToIndex(widget, current);
					if(index < firstIndex){
						--widget->comboBoxWidget.currentScroll;
					}
				}
			}
			*/
		}
		else if(input == Rotate_Decrement) {
			comboBox_item_t *current = NULL;
			// If comboBox is the first element
			if(widget->comboBoxWidget.currentItem == widget->comboBoxWidget.items){
				/*
				 * Enable to allow circular mode(go to end after reaching the start )
				 */
			/*
				// Pointer to next comboBox element
				current = widget->comboBoxWidget.currentItem->next_item;
				// Search and set the last comboBox
				while(current->next_item){
					do{
						current = current->next_item;
					}while(!current->enabled&&current);

					if(current) {
						uint8_t index = comboItemToIndex(widget, current);
						if(index > lastIndex){
							++widget->comboBoxWidget.currentScroll;
						}
						widget->comboBoxWidget.currentItem = current;
					}
				}
			*/
			}
			else{
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
		}
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
					if((widget->type == widget_button)||(widget->type == widget_bmp_button))
						return widget->buttonWidget.action(widget);
					if(extractDisplayPartFromWidget(widget)->type == field_string)
						extractEditablePartFromWidget(widget)->current_edit = 0;
					if(!sel->force_state){
						sel->state = widget_edit;
						sel->previous_state = widget_selected;
						break;
					}
					else if(sel->onEditAction){
						return sel->onEditAction(widget);
					}

					break;
				case widget_edit:
					if(extractDisplayPartFromWidget(widget)->type == field_string) {
						++extractEditablePartFromWidget(widget)->current_edit;
						if(extractEditablePartFromWidget(widget)->current_edit == widget->reservedChars)
						{
							sel->state = widget_selected;
							sel->previous_state = widget_edit;
							UG_DrawLine(widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+1,widget->posY+ widget->font_size->char_height,widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height, C_BLACK);
							UG_DrawLine(widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+1,widget->posY+ widget->font_size->char_height+1,widget->posX + widget->font_size->char_width * (extractEditablePartFromWidget(widget)->current_edit-1)+widget->font_size->char_width-3,widget->posY+ widget->font_size->char_height+1, C_BLACK);
						}
					}
					else {
						if(!sel->force_state){
							sel->state = widget_selected;
							sel->previous_state = widget_edit;
							break;
						}
						else if(sel->onSelectAction){
							return sel->onSelectAction(widget);
						}
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
				widget->displayString[extractEditablePartFromWidget(widget)->current_edit] += inc;
				if(widget->displayString[extractEditablePartFromWidget(widget)->current_edit] < 0x20) {
					if(inc > 0) {
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 0x20;
					}
					else
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 0x7e;
				}
				if(widget->displayString[extractEditablePartFromWidget(widget)->current_edit] > 0x7e) {
					if(inc > 0) {
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 0x20;
					}
					else
						widget->displayString[extractEditablePartFromWidget(widget)->current_edit] = 0x7e;
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

void comboAddScreen(comboBox_item_t* item,widget_t *combo, char *label, uint8_t actionScreen){
	if(combo->type==widget_combo){
		item->text = label;
		item->next_item = NULL;
		item->action_screen = actionScreen;
		item->type = combo_Screen;
		item->enabled = 1;

		comboBox_item_t *last = combo->comboBoxWidget.items;

		if(!last) {
			combo->comboBoxWidget.items = item;
			combo->comboBoxWidget.currentItem = item;
			return;
		}
		while(last->next_item){
			last = last->next_item;
		}
		last->next_item = item;
	}
}

void comboAddOption(comboBox_item_t* item, widget_t *combo, char *label, widget_t *widget){

	if( (widget->type==widget_editable) || (widget->type==widget_multi_option) ){				// Only allow Editable  or multioption widgets
		item->text = label;
		item->next_item = NULL;
		item->widget = widget;
		item->type = combo_Option;
		item->enabled = 1;
		comboBox_item_t *last = combo->comboBoxWidget.items;

		if(!last) {
			combo->comboBoxWidget.items = item;
			combo->comboBoxWidget.currentItem = item;
			return;
		}
		while(last->next_item){
			last = last->next_item;
		}
		last->next_item = item;
	}
}

void comboResetIndex(widget_t *combo){
	uint8_t firstIndex = combo->comboBoxWidget.currentScroll;
	comboBox_item_t *current = combo->comboBoxWidget.currentItem->next_item;
	while(combo->comboBoxWidget.currentItem != combo->comboBoxWidget.items){
		do {
			current = combo->comboBoxWidget.items;
			while(current->next_item != combo->comboBoxWidget.currentItem) {
				current = current->next_item;
			}
			combo->comboBoxWidget.currentItem = current;
		}while(!current->enabled);

		uint8_t index = comboItemToIndex(combo, current);
		if(index < firstIndex){
			--combo->comboBoxWidget.currentScroll;
		}
	}
}
