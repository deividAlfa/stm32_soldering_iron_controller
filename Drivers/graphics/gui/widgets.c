/*
 * widgets.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "widgets.h"
#include "screen.h"
#include "oled.h"
#include "gui.h"

displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w) {
	if(!w)
		return NULL;
	switch (w->type) {
		case widget_display:
			return &w->displayWidget;
			break;
		case widget_editable:
			return &w->editableWidget.inputData;
			break;
		case widget_multi_option:
			return &w->multiOptionWidget.editable.inputData;
		default:
			return NULL;
			break;
	}
}

editable_widget_t * extractEditablePartFromWidget(widget_t *w) {
	if(!w)
		return NULL;
	switch (w->type) {
		case widget_editable:
			return &w->editableWidget;
			break;
		case widget_multi_option:
			return &w->multiOptionWidget.editable;
		default:
			return NULL;
			break;
	}
}

selectable_widget_t * extractSelectablePartFromWidget(widget_t *w) {
	if(!w)
		return NULL;
	switch (w->type) {
		case widget_editable:
			return &w->editableWidget.selectable;
			break;
		case widget_multi_option:
			return &w->multiOptionWidget.editable.selectable;
		case widget_button:
		case widget_bmp_button:
			return &w->buttonWidget.selectable;
		case widget_combo:
			return &w->comboBoxWidget.selectable;
		default:
			return NULL;
			break;
	}
}
void widgetDefaultsInit(widget_t *w, widgetType t){
	if(!w){ return; }
	w->type = t;
	w->draw = &default_widgetDraw;
	w->update = &default_widgetUpdate;
	w->enabled = 1;
	w->frameType = frame_auto;
	w->refresh = refresh_idle;
	w->radius=-1;
	w->posX = 0;
	w->posY = 0;
	w->displayString = NULL;
	w->endString = NULL;
	w->stringPos = 0;
	w->last_value = 0;
	w->xbm = NULL;
	w->last_xbm = NULL;
	w->textAlign = align_center;
	w->dispAlign = align_disabled;
	w->font = default_font ;
	w->next_widget = NULL;
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
		dis->getData = NULL;
		dis->reservedChars = 0;
		dis->number_of_dec = 0;
		dis->type = field_int32;
	}
	if(edit){
		edit->step=1;
		edit->big_step=1;
		edit->current_edit=0;
		edit->max_value=2147483647;
		edit->min_value=0;
		edit->setData=NULL;
	}
	switch (t) {
		case widget_multi_option:
			edit->max_value = 255;
			//w->multiOptionWidget.currentOption = 0;
			//w->multiOptionWidget.defaultOption = 0;
			w->multiOptionWidget.numberOfOptions = 0;
			w->multiOptionWidget.options = NULL;
			break;
		case widget_combo:
			w->comboBoxWidget.currentItem = NULL;
			w->frameType = frame_combo;
			w->comboBoxWidget.first = NULL;
			w->draw = &comboBoxDraw;
			w->comboBoxWidget.currentScroll = 0;
			w->comboBoxWidget.selectable.processInput = &comboBoxProcessInput;
			w->parent->current_widget = w;

			break;
		case widget_button:
			w->buttonWidget.action = NULL;
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

void widgetEnable(widget_t* w){
	if(w && !w->enabled){
		w->enabled = 1;
		w->refresh = refresh_triggered;
		//default_widgetUpdate(w);	// Update widget before drawing
	}
}

void widgetDisable(widget_t* w){
	if(w && w->enabled){
		w->enabled = 0;
		widgetClearField(w);
	}
}

void default_widgetUpdate(widget_t *w) {
	if(!w){ return ; }
	if(!w->enabled){
		w->refresh=refresh_idle;
		return;
	}
	int32_t val_ui=0;
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
	switch(w->type){
		case widget_bmp_button:
		case widget_bmp:
			if(!w->xbm){
				widgetDisable(w);
				return;
			}
			break;

		case widget_button:
		case widget_label:
			if(!w->displayString){
				widgetDisable(w);
				return;
			}
			val_ui=strsum(w->displayString);				// Get string sum
			break;

		case widget_multi_option:
			if(!dis){
				widgetDisable(w);
				return;
			};
			val_ui=strsum(w->multiOptionWidget.options[*(uint8_t*)dis->getData()]);				// Get string sum
			break;

		case widget_display:
		case widget_editable:
			if(!dis){
				widgetDisable(w);
				return;
			}
			if(dis->type == field_string){
				val_ui=strsum(dis->getData());				// Get string sum
				break;
			}
			else{
				if(!w->displayString){	// If empty
					widgetDisable(w);
					return;
				}
				val_ui = *(int32_t*)dis->getData();
				uint8_t sum = strsum(w->endString);
				widgetDetectChange(w,(val_ui + sum));						// Check for changes in value and endString

				if((w->refresh) || (!*w->displayString)){										// Ensure displaystring is initialized
					snprintf(w->displayString, dis->reservedChars+1, "%0*ld", dis->number_of_dec+1, (int32_t)val_ui);	// Convert value into string
					uint8_t dispLen=strlen(w->displayString);
					uint8_t endLen=strlen(w->endString);

					if(dis->number_of_dec){																// If there're decimals
						if(dis->reservedChars >= (dispLen+1)){						// Ensure there's enough space in the string for adding the decimal point
							insertDot(w->displayString,  dis->number_of_dec);							// Insert decimal dot
						}
					}
					if(dis->reservedChars >= (dispLen+endLen)){	// Ensure there's enough space in the string for adding the end String
						strcat(w->displayString, w->endString);									// Append endString
					}
					w->displayString[dis->reservedChars]=0;											// Ensure last string char is 0
				}
				return;
			}
			break;
		default:
			return;
	}
	widgetDetectChange(w,val_ui);
}

void widgetDetectChange(widget_t* w, int32_t val){
	switch(w->type){
	case widget_display:
	case widget_editable:
	case widget_multi_option:
	case widget_label:
	case widget_button:
		if(w->last_value!=val){
			w->last_value=val;
			if(w->refresh==refresh_idle){
				w->refresh=refresh_triggered;
			}
		}
		break;

	case widget_bmp_button:
	case widget_bmp:
		if(w->last_xbm != w->xbm){
			w->last_xbm = w->xbm;
			if(w->refresh==refresh_idle){
				w->refresh=refresh_triggered;
			}
		}
		break;

	default:
		break;
	}
}

// Clear widget field before drawing widget with new data
void widgetClearField(widget_t* w){
	if(!w){ return; }
	selectable_widget_t* sel = extractSelectablePartFromWidget(w);
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
	uint8_t cHeight = u8g2_GetMaxCharHeight(&u8g2);
	if(w->font==u8g2_font_iron){ cHeight+=3; }
	uint8_t r;

	if(w->frameType!=frame_combo){
		u8g2_SetDrawColor(&u8g2, BLACK);
		if(w->parent->refresh < screen_eraseAndRefresh ){
			switch((uint8_t)w->type){
				case widget_bmp:
					u8g2_DrawBox(&u8g2, w->posX, w->posY, w->xbm[0], w->xbm[1]);
					break;

				case widget_bmp_button:
					u8g2_DrawBox(&u8g2, w->posX, w->posY, w->xbm[0]+4, w->xbm[1]+4);
					break;

				case widget_label:
				case widget_display:
					u8g2_DrawBox(&u8g2, w->posX, w->posY, w->width, cHeight+1);			// Draw black square to erase previous data
					break;

				case widget_editable:
				case widget_button:
				case widget_multi_option:
					if(w->font==u8g2_font_iron){ cHeight+=3; }
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
	else{																							// In combo
		if(sel->state==widget_edit){																// Being edited
			u8g2_SetDrawColor(&u8g2, XOR);															// Set XOR color
		}
		else{																						// Not editing
			u8g2_SetDrawColor(&u8g2, WHITE);														// Set white color
		}
	}
}

// -2: Not applicable, -1: Error, else: xPos for drawing string
void widgetAlign(widget_t* w){
	if(!w || w->frameType==frame_combo){ return; }

	uint8_t strWidth;
	displayOnly_widget_t *dis=extractDisplayPartFromWidget(w);
	selectable_widget_t *sel=extractSelectablePartFromWidget(w);
	switch(w->type){
		case widget_button:
		case widget_label:
			strWidth=u8g2_GetStrWidth(&u8g2, w->displayString);
			break;

		case widget_display:
		case widget_editable:
			if(dis->type == field_string){
				strWidth=u8g2_GetStrWidth(&u8g2, (char *)dis->getData());
			}
			else{
				strWidth=u8g2_GetStrWidth(&u8g2, w->displayString);
			}
			break;

		case widget_multi_option:
			strWidth=u8g2_GetStrWidth(&u8g2,  (char *)w->multiOptionWidget.options[*(uint8_t*)dis->getData()]);
                                        			break;
		default:
			return;
	}
	if(!strWidth){							// Empty widget, wrong settings
		widgetDisable(w);					// Disable widget
		return;
	}
	if(sel){									// If selectable, extra space for not overlapping the frame
		if(w->width < strWidth+7){				// If width too small
			w->width=strWidth+7;				// Use width from str width
		}
	}
	else{
		if(w->width < strWidth){			// If width too small
			w->width=strWidth;				// Use width from str width
		}
	}
	if(w->width > OledWidth){
		w->posX = 0;
		w->stringPos = 0;
		return;
	}

	switch(w->dispAlign){
		case align_disabled:
			break;
		case align_center:
			w->posX =(OledWidth - w->width)/2;
			break;
		case align_right:
			w->posX = OledWidth - w->width;
			break;
		case align_left:
		default:
			w->posX = 0;
			break;
	}

	switch(w->textAlign){
		case align_center:
			w->stringPos = (w->posX + ((w->width-strWidth)/2));
			break;
		case align_right:
			w->stringPos =(w->posX + (w->width-strWidth));
			if(sel){
				if(w->stringPos >= 3){
					w->stringPos -= 3;
				}
			}
			break;
		case align_disabled:
		case align_left:
		default:
			if(sel && ((w->posX+3)<=OledWidth)){
				w->stringPos=w->posX+3;
			}
			else{
				w->stringPos = w->posX;
			}
			break;
	}
}

void default_widgetDraw(widget_t *w) {
	if(!w || !w->enabled){ return; }

	bool frameDraw = 0;
	bool frameColor = BLACK;
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
	editable_widget_t* edit = extractEditablePartFromWidget(w);
	selectable_widget_t* sel = extractSelectablePartFromWidget(w);
	uint8_t refresh = w->refresh | w->parent->refresh;
	uint8_t cHeight = 0;

	if((sel  || dis || (w->type ==widget_label)) && (w->type != widget_bmp_button)){
		u8g2_SetFont(&u8g2, w->font);
		cHeight = u8g2_GetMaxCharHeight(&u8g2);
	}
	if(w->frameType==frame_outline){
		frameDraw=1;
		frameColor = WHITE;
	}
	else if(w->frameType==frame_auto){
		if(sel) {
			if(refresh==refresh_idle){															// If not forced refresh, check for changes
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
						return;
				}
			}
			else{																	// Else, redraw frames
				switch (sel->state) {
					case widget_selected:
							frameDraw=1;
							frameColor = WHITE;
						break;
					case widget_idle:
					case widget_edit:
						break;
					default:
						return;
				}
			}
		}
	}

	if(refresh){
		widgetAlign(w);				// Align
		widgetClearField(w);				// Clear old field


		if(w->refresh==refresh_triggered){
			w->refresh=refresh_idle;
		}

		switch(w->type){

			case widget_bmp:
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawXBMP(&u8g2, w->posX, w->posY, w->xbm[0], w->xbm[1], &w->xbm[2]);
				return;

			case widget_bmp_button:
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawXBMP(&u8g2, w->posX+2, w->posY+2, w->xbm[0], w->xbm[1], &w->xbm[2]);
				break;

			case widget_display:
				if(dis->type == field_string){
					u8g2_DrawStr(&u8g2, w->stringPos, w->posY,  dis->getData());
				}
				else{
					u8g2_DrawStr(&u8g2, w->stringPos, w->posY,  w->displayString);
				}
				break;

			case widget_editable:
				if(dis->type == field_string){
					u8g2_DrawStr(&u8g2,  w->stringPos, w->posY+2,  dis->getData());
					if(sel->state == widget_edit){
						char t[20];
						strcpy(t,dis->getData());

						t[edit->current_edit+1]=0;
						uint8_t x2=u8g2_GetStrWidth(&u8g2, t);

						t[edit->current_edit]=0;
						uint8_t x1=u8g2_GetStrWidth(&u8g2, t);

						u8g2_SetDrawColor(&u8g2, BLACK);
						u8g2_DrawBox(&u8g2, w->stringPos, w->posY+ cHeight, w->width, 2);

						u8g2_SetDrawColor(&u8g2, WHITE);
						u8g2_DrawBox(&u8g2,  w->stringPos+x1, w->posY+ cHeight, x2-x1, 2);

					}
					else if(sel->previous_state == widget_edit){
						u8g2_SetDrawColor(&u8g2, BLACK);
						u8g2_DrawBox(&u8g2, w->stringPos, w->posY+ cHeight, w->width, 2);
					}
				}
				else{
					u8g2_DrawStr(&u8g2,w->stringPos, w->posY+2,  w->displayString);
				}
				break;

			case widget_button:
				u8g2_DrawStr(&u8g2, w->stringPos, w->posY+2,  w->displayString);// Draw string
				break;

			case widget_label:
				u8g2_DrawStr(&u8g2, w->stringPos, w->posY,  w->displayString);// Draw string
				break;

			case widget_multi_option:
				u8g2_DrawStr(&u8g2, w->stringPos, w->posY+2, w->multiOptionWidget.options[*(uint8_t*)dis->getData()]);// Draw string
				break;

			default:
				return;
		}
	}

	if(sel) {
		sel->previous_state = sel->state;
	}

	if(frameDraw || w->frameType==frame_outline){
		if(w->font==u8g2_font_iron){ cHeight+=3; }
		switch(w->type){
			case widget_bmp_button:
				u8g2_SetDrawColor(&u8g2, frameColor);
				u8g2_DrawRFrame(&u8g2, w->posX, w->posY, w->xbm[0]+4,  w->xbm[1]+4, 4);
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
	}
	u8g2_SetDrawColor(&u8g2, WHITE);
}


void comboBoxDraw(widget_t *w) {
	uint16_t yDim = OledHeight - w->posY;
	uint8_t height;
	int8_t frameY=0;
	uint8_t drawFrame=1;
	comboBox_item_t *item = w->comboBoxWidget.first;
	uint8_t scroll = 0;
	uint8_t r;

	if(!w || ((w->refresh==refresh_idle) && (w->parent->refresh==screen_idle))){
		return;
	}
	if(w->refresh==refresh_triggered){
		w->refresh=refresh_idle;
	}
	if(w->parent->refresh < screen_eraseAndRefresh){				// If screen not erased already
		w->parent->refresh = screen_blankRefresh;
		FillBuffer(BLACK, fill_dma);							// Erase fast using dma
	}

	if(!item){ return; }										// Return if null item

	u8g2_SetFont(&u8g2, w->font);
	height= u8g2_GetMaxCharHeight(&u8g2)+1;// +1 to allow separation between combobox items

	if(w->radius<0){
		r=(height-1)/2;
	}
	else{
		r = w->radius;
	}


	while(scroll < w->comboBoxWidget.currentScroll) {
		if(!item->next_item)
			break;
		item = item->next_item;
		if(item->enabled)
			++scroll;
	}
	for(uint8_t y = 0; y < yDim / height; ++y) {
																												// Store Y position
		if(item == w->comboBoxWidget.currentItem) {																					// If the current combo item is selected
			frameY=y * height + w->posY;
		}

		if ((item->type==combo_Screen)||(item->type==combo_Action)){	// If screen or action item, just draw the label
			u8g2_SetDrawColor(&u8g2, WHITE);
			if(item->dispAlign==align_left){
				u8g2_DrawStr(&u8g2, 4, y * height + w->posY +2, item->text);
			}
			else{
				uint8_t len = u8g2_GetStrWidth(&u8g2, item->text);
				if(item->dispAlign==align_right){
					u8g2_DrawStr(&u8g2, OledWidth-3-len, y * height + w->posY +2, item->text);
				}
				else{
					u8g2_DrawStr(&u8g2, (OledWidth-1-len)/2, y * height + w->posY +2, item->text);
				}
			}
		}
		else if (item->type==combo_Option){
			selectable_widget_t *sel = extractSelectablePartFromWidget(item->widget);
			default_widgetUpdate(item->widget);
			uint8_t len=0;
			if(item->widget->type==widget_multi_option){
				displayOnly_widget_t* dis = extractDisplayPartFromWidget(item->widget);
				len = u8g2_GetStrWidth(&u8g2,item->widget->multiOptionWidget.options[*(uint8_t*)dis->getData()]);
			}
			else if(item->widget->type==widget_editable){
				len=u8g2_GetStrWidth(&u8g2,item->widget->displayString);
			}
			item->widget->posY = y * height + w->posY;																						// Set widget Ypos same as the current combo option
			item->widget->stringPos = OledWidth-len-5;																							// Align to the left measuring actual string width

			if(sel->state==widget_edit){																									// If edit mode
				drawFrame=0;
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawRBox(&u8g2, 0, frameY, OledWidth, height, r);
				u8g2_SetDrawColor(&u8g2, XOR);
			}
			else{
				u8g2_SetDrawColor(&u8g2, WHITE);
			}
			u8g2_DrawStr(&u8g2, 4, y * height + w->posY +2, item->text);																// Draw the label
			default_widgetDraw(item->widget);																								// Draw widget
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
		u8g2_DrawRFrame(&u8g2, 0, frameY, OledWidth, height,  r);
	}
	return;
}

uint8_t comboItemToIndex(widget_t *combo, comboBox_item_t *item) {
	if(!combo || !item){ return 0; }
	uint8_t index = 0;
	comboBox_item_t *i = combo->comboBoxWidget.first;
	while(i && i != item) {
		i = i->next_item;
		if(i->enabled)
			++ index;
	}
	return index;
}

int comboBoxProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state) {
	if(!w || (input == Rotate_Nothing)){
		return -1;
	}
	u8g2_SetFont(&u8g2, w->font);

	uint8_t firstIndex = w->comboBoxWidget.currentScroll;
	uint16_t yDim = OledHeight - w->posY;
	uint16_t height = u8g2_GetMaxCharHeight(&u8g2)+1;
	uint8_t maxIndex = yDim / height;
	uint8_t lastIndex = w->comboBoxWidget.currentScroll + maxIndex -1;
	selectable_widget_t *sel;
	if(w->refresh==refresh_idle){
		w->refresh=refresh_triggered;
	}
	if (w->comboBoxWidget.currentItem->type==combo_Option){													// If combo option type
		sel = extractSelectablePartFromWidget(w->comboBoxWidget.currentItem->widget);									// Get selectable data
	}
	if(input == Click){																								// If clicked
		if (w->comboBoxWidget.currentItem->type==combo_Option){												// If combo option type
			if(sel->state==widget_idle){																					// If widget idle
				sel->state=widget_edit;																				// Change to edit
			}
			else if(sel->state==widget_edit){																			// If widget in edit mode
				sel->state=widget_idle;																				// Change to idle
			}
			return -1;																								// Do nothing else
		}
		if (w->comboBoxWidget.currentItem->type==combo_Action){												// If combo option type
			return w->comboBoxWidget.currentItem->action();
		}
		else if (w->comboBoxWidget.currentItem->type==combo_Screen){											// If combo screen type
			return w->comboBoxWidget.currentItem->action_screen;												// Return screen index
		}
	}
	if((input == Rotate_Increment)||(input == Rotate_Decrement)){													// If rotation data
		if(w->comboBoxWidget.currentItem->type==combo_Option){													// If combo option type
			if(sel->state==widget_edit){																			// If widget in edit mode
				default_widgetProcessInput(w->comboBoxWidget.currentItem->widget, input, state);				// Process widget
				return -1;																							// Return
			}
		}
		if(input == Rotate_Increment) {
			//Set comboBox item to next comboBox
			comboBox_item_t *current = w->comboBoxWidget.currentItem->next_item;
			// if comboBox is disabled, skip. While comboBox still valid
			while(current && !current->enabled) {
				current = current->next_item;
			}
			// If comboBox valid(didn't reach end)
			if(current) {
				w->comboBoxWidget.currentItem = current;
				uint8_t index = comboItemToIndex(w, current);
				if(index > lastIndex)
					++w->comboBoxWidget.currentScroll;
			}
		}
		else if(input == Rotate_Decrement) {
			comboBox_item_t *current = NULL;
			// If comboBox is not the first element
			if(w->comboBoxWidget.currentItem != w->comboBoxWidget.first){
				do {
					current = w->comboBoxWidget.first;
					while(current->next_item != w->comboBoxWidget.currentItem) {
						current = current->next_item;
					}
					w->comboBoxWidget.currentItem = current;
				}while(!current->enabled);
				uint8_t index = comboItemToIndex(w, current);
				if(index < firstIndex)
					--w->comboBoxWidget.currentScroll;
			}
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
int default_widgetProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state) {
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
	selectable_widget_t* sel = extractSelectablePartFromWidget(w);
	editable_widget_t* edit = extractEditablePartFromWidget(w);

	if(!w || (input == Rotate_Nothing)){ return -1;	}
	if(!sel) {	return -2;	}

	if(input == LongClick) {
		if(sel->longPressAction)
			return sel->longPressAction(w);
		else
			return -1;
			//input = Click;
	}

	if(input == Click) {
		switch (sel->state) {
			case widget_selected:
				if(w->refresh==refresh_idle){
					w->refresh = refresh_triggered;
				}
				if((w->type == widget_button)||(w->type == widget_bmp_button)){
					return w->buttonWidget.action(w);
				}
				if(dis->type == field_string){
					strcpy(w->displayString, (char*)w->editableWidget.inputData.getData());
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
	if((w->type == widget_editable) && (sel->state == widget_edit)) {
		int32_t val_ui;
		int32_t inc;
		if(w->refresh==refresh_idle){
			w->refresh=refresh_triggered;
		}

		if(abs(state->Diff) > 1) {//TODO test
			inc = w->editableWidget.big_step;
			if(state->Diff < 0)
				inc = -inc;
		}
		else
			inc = w->editableWidget.step * state->Diff;

		if(edit->inputData.type==field_string){
			int16_t current_edit = (char)w->displayString[edit->current_edit];
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
			w->displayString[edit->current_edit]=(char)current_edit;
			w->editableWidget.setData(w->displayString);
		}

		else if(edit->inputData.type==field_int32){
			if(!w->displayString){	// If empty
				widgetDisable(w);
				return -1;
			}
			val_ui = *(int32_t*)dis->getData();

			if( (inc>0) && ((val_ui+inc) < val_ui) ){					// Check of overflow
				val_ui = edit->max_value;
			}
			else if( (inc<0) && ((val_ui+inc) > val_ui) ){				// Check for underflow
				val_ui = edit->min_value;
			}
			else{
				val_ui += inc;
				if(val_ui < edit->min_value) {							// Check limits
					val_ui = edit->min_value;
				}
				if(val_ui > edit->max_value) {
					val_ui= edit->max_value;
				}
			}
			w->editableWidget.setData(&val_ui);
		}
		return -1;
	}
	else if((w->type == widget_multi_option) && (sel->state == widget_edit)) {
		if(w->refresh==refresh_idle){
			w->refresh=refresh_triggered;
		}
		int temp = *(uint8_t*)dis->getData();
		if(input == Rotate_Increment)
			++temp;
		else if(input == Rotate_Decrement)
			--temp;
		if(temp < 0)
			temp = w->multiOptionWidget.numberOfOptions - 1;
		else if(temp > w->multiOptionWidget.numberOfOptions -1)
			temp = 0;
		w->multiOptionWidget.editable.setData(&temp);
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

void comboAddScreen(comboBox_item_t* item,widget_t *combo, char *label, uint8_t actionScreen){
	if(!item || !combo || !label){ return; }
	if(combo->type==widget_combo){
		item->text = label;
		item->next_item = NULL;
		item->action_screen = actionScreen;
		item->type = combo_Screen;
		item->enabled = 1;
		item->dispAlign= align_center;

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

void comboAddOption(comboBox_item_t* item, widget_t *combo, char *label, widget_t *w){
	if(!item || !combo || !label || !w){ return; }
	if( (w->type==widget_editable) || (w->type==widget_multi_option) ){				// Only allow Editable  or multioption widgets
		item->text = label;
		item->next_item = NULL;
		item->widget = w;
		item->type = combo_Option;
		item->enabled = 1;
		w->frameType = frame_combo;
		w->font = combo->font;
		w->textAlign = align_disabled;
		w->dispAlign = align_disabled;
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
	if(!item || !combo || !label || !action){ return; }
	if(action){												// If not null
		item->text = label;
		item->next_item = NULL;
		item->action = action;
		item->type = combo_Action;
		item->enabled = 1;
		item->dispAlign= align_center;
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
