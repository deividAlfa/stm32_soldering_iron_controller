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
char displayString[32];
displayOnly_widget_t * extractDisplayPartFromWidget(widget_t *w) {
  comboBox_widget_t* combo;
	if(!w)
		return NULL;
	switch (w->type) {
		case widget_display:
		  return (displayOnly_widget_t*)w->content;
		case widget_editable:
		case widget_multi_option:
		  return &((editable_widget_t*)w->content)->inputData;
		case widget_combo:
		  combo = (comboBox_widget_t*)w->content;
		  if(((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption))&&(combo->currentItem)){
        return &((comboBox_widget_t*)w->content)->currentItem->widget->inputData;
      }
		default:
			return NULL;
			break;
	}
}

editable_widget_t * extractEditablePartFromWidget(widget_t *w) {
  comboBox_widget_t* combo;
	if(!w)
		return NULL;
	switch (w->type) {
		case widget_editable:
		case widget_multi_option:
      return (editable_widget_t*)w->content;
    case widget_combo:
      combo = (comboBox_widget_t*)w->content;
      if(((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption))&&(combo->currentItem)){
        return ((comboBox_widget_t*)w->content)->currentItem->widget;
      }
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
		case widget_multi_option:
      return &((editable_widget_t*)w->content)->selectable;
		case widget_button:
		case widget_bmp_button:
      return &((button_widget_t*)w->content)->selectable;
    case widget_combo:
      return &((comboBox_widget_t*)w->content)->selectable;
		default:
			return NULL;
			break;
	}
}
void widgetDefaultsInit(widget_t *w, widgetType t, void* content){
	if(!w || !content){ return; }
	w->type = t;
	w->draw = &default_widgetDraw;
	w->update = &default_widgetUpdate;
	w->enabled = 1;
	w->frameType = frame_auto;
	w->refresh = refresh_idle;
	w->radius=-1;
	w->posX = 0;
	w->posY = 0;
	w->content = content;
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
	switch (t) {
		case widget_multi_option:
			edit->max_value = 255;
			edit->numberOfOptions = 0;
			edit->options = NULL;
			break;
		case widget_combo:
		{
		  comboBox_widget_t* combo = content;
			combo->currentItem = NULL;
			w->frameType = frame_combo;
			combo->first = NULL;
			w->draw = &comboBoxDraw;
			combo->currentScroll = 0;
			combo->selectable.processInput = &comboBoxProcessInput;
			w->parent->current_widget = w;
			combo->font = default_font;
			break;
		}
		case widget_button:
		{
		  button_widget_t* button = (button_widget_t*)content;
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
	sel = extractSelectablePartFromWidget(w);
	dis = extractDisplayPartFromWidget(w);
	edit = extractEditablePartFromWidget(w);

}
void editableDefaultsInit(editable_widget_t* editable, widgetType type){
  widget_t w;
  widgetDefaultsInit(&w, type, editable);
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

		case widget_bmp:
      bmp = ((bmp_widget_t*)w->content);
      if(!bmp->xbm){
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
			val_ui=strsum(button->displayString);				// Get string sum
			break;

		case widget_multi_option:
			if(!dis){
				widgetDisable(w);
				return;
			};
			val_ui=strsum(edit->options[*(uint8_t*)dis->getData()]);				// Get string sum
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
				val_ui = *(int32_t*)dis->getData();
				uint8_t sum = strsum(dis->endString);
				widgetDetectChange(w,(val_ui + sum));						// Check for changes in value and endString
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
	if(dis && dis->font==u8g2_font_iron){ cHeight+=3; }// TODO this font reports lower height, needs to be manually adjusted
	uint8_t r;

	if(w->frameType!=frame_combo){
		u8g2_SetDrawColor(&u8g2, BLACK);
		if(w->parent->refresh < screenRefresh_eraseNow ){
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
				  if(button->font==u8g2_font_iron){ cHeight+=3; }               // TODO this font reports lower height, needs to be manually adjusted
					u8g2_DrawBox(&u8g2, w->posX, w->posY, w->width, cHeight+1);
					break;

				case widget_display:
					u8g2_DrawBox(&u8g2, w->posX, w->posY, w->width, cHeight+1);			// Draw black square to erase previous data
					break;

				case widget_editable:
				case widget_multi_option:
					if(dis->font==u8g2_font_iron){ cHeight+=3; }                  // TODO this font reports lower height, needs to be manually adjusted
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
		if(sel->state==widget_edit){										// Being edited
			u8g2_SetDrawColor(&u8g2, XOR);								// Set XOR color
		}
		else{																						// Not editing
			u8g2_SetDrawColor(&u8g2, WHITE);							// Set white color
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
			strWidth=u8g2_GetStrWidth(&u8g2, button->displayString);
			break;

		case widget_display:
		case widget_editable:
			if(dis->type == field_string){
				strWidth=u8g2_GetStrWidth(&u8g2, (char *)dis->getData());
			}
			else{
				strWidth=u8g2_GetStrWidth(&u8g2, displayString);
			}
			break;

		case widget_multi_option:
			strWidth=u8g2_GetStrWidth(&u8g2,  (char *)edit->options[*(uint8_t*)dis->getData()]);
			break;
		default:
			return;
	}
	if(!strWidth){							// Empty widget, wrong settings
		widgetDisable(w);					// Disable widget
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
	if(sel && (w->frameType!=frame_disabled)){			// If selectable, extra space for not overlapping the frame
		if(w->width < strWidth+7){				            // If width too small
		  if(strWidth+7<OledWidth){                   // If fits oled size
		    w->width=strWidth+7;                      // Use width from str width
		  }
		  else{
		    w->width = strWidth;                      // Else, don't add extra space
		  }
		}
	}
	else{
		if(w->width < strWidth){		// If width too small
			w->width=strWidth;				// Use width from str width
		}
	}

	if(w->width > OledWidth){
	  if(textAlign != align_disabled)
	    textAlign = align_left;
	  if(dispAlign != align_disabled)
	    dispAlign = align_left;
	}

	switch(dispAlign){
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
			if(sel && ((w->posX+3)<=OledWidth)){
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

void default_widgetDraw(widget_t *w) {
	if(!w || !w->enabled){ return; }

	bool frameDraw = 0;
	bool frameColor = BLACK;
	displayOnly_widget_t* dis = extractDisplayPartFromWidget(w);
	editable_widget_t* edit = extractEditablePartFromWidget(w);
	selectable_widget_t* sel = extractSelectablePartFromWidget(w);
  button_widget_t* button = NULL;
  bmp_widget_t* bmp = NULL;
	const uint8_t* font = NULL;
	uint8_t refresh = w->refresh | w->parent->refresh;
	uint8_t cHeight = 0;

	if( dis ){
		u8g2_SetFont(&u8g2, dis->font);
		cHeight = u8g2_GetMaxCharHeight(&u8g2);
		font = dis->font;
	}
	else if((w->type == widget_button)||(w->type == widget_bmp_button)){
    button = (button_widget_t*)w->content;
	  if(w->type == widget_button){
	    u8g2_SetFont(&u8g2, button->font);
      cHeight = u8g2_GetMaxCharHeight(&u8g2);
      font = button->font;
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
			else{																	                  // Else, redraw frames
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
		if(dis && dis->type==field_int32){
			int32_t val_ui = *(int32_t*)dis->getData();
      uint8_t decimals = dis->number_of_dec+1;
      if(val_ui<0){
        decimals++;
      }
      if(decimals>10){ decimals=10; }
			snprintf(dis->displayString, dis->reservedChars+1, "%0*ld", decimals, (int32_t)val_ui);		// Convert value into string

			uint8_t dispLen=strlen(dis->displayString);
			uint8_t endLen=strlen(dis->endString);
			if(dis->number_of_dec){																				        // If there're decimals
				if(dis->reservedChars >= (dispLen+1)){															// Ensure there's enough space in the string for adding the decimal point
					insertDot(dis->displayString,  dis->number_of_dec);								// Insert decimal dot
				}
			}
			if(dis->reservedChars >= (dispLen+endLen)){														// Ensure there's enough space in the string for adding the end String
				strcat(dis->displayString, dis->endString);													// Append endString
			}
			dis->displayString[dis->reservedChars]=0;															// Ensure last string char is 0
		}

		widgetAlign(w);				// Align
		widgetClearField(w);				// Clear old field
		if(w->refresh==refresh_triggered){
			w->refresh=refresh_idle;
		}

		switch(w->type){

			case widget_bmp:
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawXBMP(&u8g2, w->posX, w->posY, bmp->xbm[0], bmp->xbm[1], &bmp->xbm[2]);
				return;

			case widget_bmp_button:
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawXBMP(&u8g2, w->posX+2, w->posY+2, button->xbm[0], button->xbm[1], &button->xbm[2]);
				break;

      case widget_button:
        u8g2_DrawStr(&u8g2, button->stringStart, w->posY+2,  button->displayString);// Draw string
        break;

			case widget_display:
				if(dis->type == field_string){
					u8g2_DrawStr(&u8g2, dis->stringStart, w->posY,  dis->getData());
				}
				else{
					u8g2_DrawStr(&u8g2, dis->stringStart, w->posY,  dis->displayString);
				}
				break;

			case widget_editable:
				if(dis->type == field_string){
					u8g2_DrawStr(&u8g2,  dis->stringStart, w->posY+2,  dis->getData());
					if(sel->state == widget_edit){
						char t[20];
						strcpy(t,dis->getData());

						t[edit->current_edit+1]=0;
						uint8_t x2=u8g2_GetStrWidth(&u8g2, t);

						t[edit->current_edit]=0;
						uint8_t x1=u8g2_GetStrWidth(&u8g2, t);

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
					u8g2_DrawStr(&u8g2,dis->stringStart, w->posY+2,  dis->displayString);
				}
				break;

			case widget_multi_option:
				u8g2_DrawStr(&u8g2, dis->stringStart, w->posY+2, edit->options[*(uint8_t*)dis->getData()]);// Draw string
				break;

			default:
				return;
		}
	}

	if(sel) {
		sel->previous_state = sel->state;
	}

	if(frameDraw || w->frameType==frame_outline){
		if(font==u8g2_font_iron){ cHeight+=3; }
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
	}
	u8g2_SetDrawColor(&u8g2, WHITE);
}


void comboBoxDraw(widget_t *w) {
	uint16_t yDim = OledHeight - w->posY;
	uint8_t height;
  int8_t frameY=0;
  int8_t posY;;
	uint8_t drawFrame=1;
	comboBox_widget_t* combo = (comboBox_widget_t*)w->content;
	comboBox_item_t *item = combo->first;
	uint8_t scroll = 0;
  uint8_t r;
	if(!w || ((w->refresh==refresh_idle) && (w->parent->refresh==screenRefresh_idle))){
		return;
	}


	if(w->refresh==refresh_triggered){
		w->refresh=refresh_idle;
	}
	if(w->parent->refresh < screenRefresh_eraseNow){				// If screen not erased already
		w->parent->refresh = screenRefresh_alreadyErased;
		FillBuffer(BLACK, fill_dma);							            // Erase fast using dma
	}

	if(!item){ return; }										                // Return if null item

	u8g2_SetFont(&u8g2, combo->font);
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
		if(item == combo->currentItem) {																	// If the current combo item is selected
			frameY=y * height + w->posY;
		}

		if ((item->type==combo_Screen)||(item->type==combo_Action)){	    // If screen or action item, just draw the label
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
		else if ((item->type==combo_Editable)||(item->type==combo_MultiOption)){
      editable_widget_t* edit = item->widget;
			selectable_widget_t *sel = &edit->selectable;
      displayOnly_widget_t* dis = &edit->inputData;
			default_widgetUpdate(w);
			uint8_t len=0;
			if(sel->state==widget_edit){																	  // If edit mode
				drawFrame=0;
				u8g2_SetDrawColor(&u8g2, WHITE);
				u8g2_DrawRBox(&u8g2, 0, frameY, OledWidth, height, r);
				u8g2_SetDrawColor(&u8g2, XOR);
			}
			else{
				u8g2_SetDrawColor(&u8g2, WHITE);
			}

			if(item->type==combo_MultiOption){
				len = u8g2_GetStrWidth(&u8g2,edit->options[*(uint8_t*)dis->getData()]);
			}
			else if(item->type==combo_Editable){
				int32_t val_ui = *(int32_t*)dis->getData();                       // Get data
	      uint8_t decimals = dis->number_of_dec+1;                          // Load decimal count
	      if(val_ui<0){                                                     // If negatiove, add a decimal (decimals are just used as min char output in sprintf)
	        decimals++;
	      }
	      if(decimals>10){ decimals=10; }                                   // Limit max decimals
				snprintf(dis->displayString, dis->reservedChars+1, "%0*ld", decimals, (int32_t)val_ui);		// Convert value into string
				uint8_t dispLen=strlen(dis->displayString);                       // Get string len
				uint8_t endLen=strlen(dis->endString);                            // Get endStr len
				if(dis->number_of_dec){																				    // If there're decimals
					if(dis->reservedChars >= (dispLen+1)){													// Ensure there's enough space in the string for adding the decimal point
						insertDot(dis->displayString,  dis->number_of_dec);						// Insert decimal dot
					}
				}
				if(dis->reservedChars >= (dispLen+endLen)){												// Ensure there's enough space in the string for adding the end String
					strcat(dis->displayString, dis->endString);											// Append endString
				}
				dis->displayString[dis->reservedChars]=0;													// Ensure last string char is 0
				len=u8g2_GetStrWidth(&u8g2,dis->displayString);
			}
			posY = y * height + w->posY;																				// Set widget Ypos same as the current combo option
			dis->stringStart = OledWidth-len-5;																	// Align to the left measuring actual string width
			if(item->type==combo_Editable){
				u8g2_DrawStr(&u8g2,dis->stringStart, posY+2,  dis->displayString);  // Draw  widget data
			}
			else{
				u8g2_DrawStr(&u8g2,dis->stringStart, posY+2,  edit->options[*(uint8_t*)dis->getData()]);
			}
			u8g2_DrawStr(&u8g2, 4, y * height + w->posY +2, item->text);				// Draw the combo item label
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
	comboBox_item_t *i = ((comboBox_widget_t*)combo->content)->first;
	while(i && i != item) {
		i = i->next_item;
		if(i->enabled)
			++ index;
	}
	return index;
}

int comboBoxProcessInput(widget_t *w, RE_Rotation_t input, RE_State_t *state) {

  comboBox_widget_t* combo = (comboBox_widget_t*)w->content;

	if(!w || (input == Rotate_Nothing)){
		return -1;
	}
	u8g2_SetFont(&u8g2, combo->font);

	uint8_t firstIndex = combo->currentScroll;
	uint16_t yDim = OledHeight - w->posY;
	uint16_t height = u8g2_GetMaxCharHeight(&u8g2)+1;
	uint8_t maxIndex = yDim / height;
	uint8_t lastIndex = combo->currentScroll + maxIndex -1;
	selectable_widget_t *sel;
	if(w->refresh==refresh_idle){
		w->refresh=refresh_triggered;                                                                                         // Update in combo erases whole screen (to avoid possible leftovers)
	}
	if ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption)){													// If combo editable type
		sel = &combo->currentItem->widget->selectable;									                                                      // Get selectable data
	}
	if((input == Click)||(input == LongClick)){																								                              // If clicked
	  if ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption)){												// If combo editable type
			if(sel->state==widget_idle){																					                                              // If widget idle
				sel->state=widget_edit;																				                                                    // Change to edit
			}
			else if(sel->state==widget_edit){																			                                              // If widget in edit mode
				sel->state=widget_idle;																				                                                    // Change to idle
			}
			return -1;																								                                                          // Do nothing else
		}
		if (combo->currentItem->type==combo_Action){												                                                  // If combo Action type
			return combo->currentItem->action();                                                                                // Process action
		}
		else if (combo->currentItem->type==combo_Screen){											                                                // If combo screen type
			return combo->currentItem->action_screen;												                                                    // Return screen index
		}
	}
	if((input == Rotate_Increment) || (input == Rotate_Decrement) ||													                              // If rotation data
	  (input == Rotate_Increment_while_click) || (input == Rotate_Decrement_while_click)){
	  if ((combo->currentItem->type==combo_Editable)||(combo->currentItem->type==combo_MultiOption)){												// If combo option type
			if(sel->state==widget_edit){																			                                                  // If widget in edit mode
				default_widgetProcessInput(w, input, state);				                                                              // Process widget
				return -1;																							                                                          // Return
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
	comboBox_widget_t* combo = NULL;
	if(!w || (input == Rotate_Nothing)){ return -1;	}
	if(!sel) {	return -2;	}

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
	if(((w->type == widget_editable) && (sel->state == widget_edit)) || ((w->type == widget_combo)&&combo->currentItem->type==combo_Editable)) {  // Combo only calls this when incrementing or decrementing
		int32_t val_ui;
		int32_t inc;
		if(w->refresh==refresh_idle){
			w->refresh=refresh_triggered;
		}
		if(input == Rotate_Increment_while_click){
		  input = Rotate_Increment;
		  inc = edit->big_step;
		}
		else if(input == Rotate_Decrement_while_click){
		  input = Rotate_Decrement;
		  inc = edit->big_step;
		}
		else{
			inc = edit->step;
		}
		if(state->Diff < 0){
				inc = -inc;
		}
		if(edit->inputData.type==field_string){
			int16_t current_edit = (char)dis->displayString[edit->current_edit];
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

		else if(edit->inputData.type==field_int32){
			if(!dis->displayString){	                // If empty
				widgetDisable(w);                       // This shouldn't happen. Disable widget to avoid possible errors.
				return -1;
			}
			val_ui = *(int32_t*)dis->getData();

			if( (inc>0) && ((val_ui+inc) < val_ui) ){					  // Check for overflow
				val_ui = edit->max_value;
			}
			else if( (inc<0) && ((val_ui+inc) > val_ui) ){		  // Check for underflow
				val_ui = edit->min_value;
			}
			else{
				val_ui += inc;
				if(val_ui < edit->min_value) {							      // Check limits
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
	else if( ((w->type == widget_multi_option) && (sel->state == widget_edit)) || ((w->type == widget_combo)&&combo->currentItem->type==combo_MultiOption)) {
		if(w->refresh==refresh_idle){
			w->refresh=refresh_triggered;
		}
		int temp = *(uint8_t*)dis->getData();
		if(input == Rotate_Increment)
			++temp;
		else if(input == Rotate_Decrement)
			--temp;
		if(temp < 0)
			temp = edit->numberOfOptions - 1;
		else if(temp > edit->numberOfOptions -1)
			temp = 0;
		edit->setData(&temp);
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
  comboBox_widget_t* comboW = (comboBox_widget_t*)combo->content;
	if(combo->type==widget_combo){
		item->text = label;
		item->next_item = NULL;
		item->action_screen = actionScreen;
		item->type = combo_Screen;
		item->enabled = 1;
		item->dispAlign= align_center;

		comboBox_item_t *next = comboW->first;

		if(!next) {
		  comboW->first = item;
		  comboW->currentItem = item;
			return;
		}
		while(next->next_item){
			next = next->next_item;
		}
		next->next_item = item;
	}
}

// Only allows Editable or multioption widgets
void comboAddEditable(comboBox_item_t* item, widget_t *combo, char *label, editable_widget_t *editable){
	if(!item || !combo || !label || !editable ){
	  return;
	}
  comboBox_widget_t* comboW = (comboBox_widget_t*)combo->content;

  item->text = label;
  item->next_item = NULL;
  item->widget = editable;
  item->type = combo_Editable;
  item->enabled = 1;
  comboBox_item_t *next = comboW->first;

  if(!next) {
    comboW->first = item;
    comboW->currentItem = item;
    return;
  }
  while(next->next_item){
    next = next->next_item;
  }
  next->next_item = item;
}

// Only allows Editable or multioption widgets
void comboAddMultiOption(comboBox_item_t* item, widget_t *combo, char *label, editable_widget_t *editable){
  if(!item || !combo || !label || !editable ){
    return;
  }
  comboBox_widget_t* comboW = (comboBox_widget_t*)combo->content;

  item->text = label;
  item->next_item = NULL;
  item->widget = editable;
  item->type = combo_MultiOption;
  item->enabled = 1;
  comboBox_item_t *next = comboW->first;

  if(!next) {
    comboW->first = item;
    comboW->currentItem = item;
    return;
  }
  while(next->next_item){
    next = next->next_item;
  }
  next->next_item = item;
}

void comboAddAction(comboBox_item_t* item, widget_t *combo, char *label, int (*action)()){
	if(!item || !combo || !label || !action){ return; }
  comboBox_widget_t* comboW = (comboBox_widget_t*)combo->content;
	if(action){												// If not null
		item->text = label;
		item->next_item = NULL;
		item->action = action;
		item->type = combo_Action;
		item->enabled = 1;
		item->dispAlign= align_center;
		comboBox_item_t *next = comboW->first;

		if(!next) {
			comboW->first = item;
			comboW->currentItem = item;
			return;
		}
		while(next->next_item){
			next = next->next_item;
		}
		next->next_item = item;
	}
}

void comboResetIndex(widget_t *combo){
  comboBox_widget_t* comboW = (comboBox_widget_t*)combo->content;
	if((comboW->currentItem->type==combo_Editable)||(comboW->currentItem->type==combo_MultiOption)){
			selectable_widget_t* sel = &comboW->currentItem->widget->selectable;
		if(sel){
			sel->state = widget_idle;
			sel->previous_state = widget_idle;
		}
	}
	comboW->currentItem = comboW->first;
	comboW->currentScroll=0;
}
