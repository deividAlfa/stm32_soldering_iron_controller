/*
 * debug_screen.c
 *
 *  Created on: Aug 2, 2017
 *      Author: jose
 */

#include "debug_screen.h"
#include "../../generalIO/adc_global.h"
#include "../../generalIO/tempsensors.h"

uint16_t temp;


static void * debug_screen_getADC1_1() {
	temp = Iron.Temp.Temp_Adc_Avg;
	return &temp;
}
static void * debug_screen_getP() {
	temp = getPID_P()* 10000000;
	return &temp;
}
static void * debug_screen_getI() {
	temp = getPID_I()* 10000000;
	return &temp;
}
static void * debug_screen_getD() {
	temp = getPID_D()* 10000000;
	return &temp;
}
static void * debug_screen_getError() {
	temp = getError();
	return &temp;
}
static void * debug_screen_getOutput() {
	temp = getOutput();
	return &temp;
}
static void debug_screen_init(screen_t *scr) {
	UG_FontSetHSpace(0);
	UG_FontSetVSpace(0);
	default_init(scr);
}

//temp_adc
//kp ki kd
//error
//present output
void debug_screen_setup(screen_t *scr) {
	scr->draw = &default_screenDraw;
	scr->processInput = &default_screenProcessInput;
	scr->init = &debug_screen_init;
	scr->update = &default_screenUpdate;

	//ADC iron display

	widget_t *widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	char *s = "ADC 1:";
	strcpy(widget->displayString, s);
	widget->posX =32;
	widget->posY = 0;
	widget->font_size = &FONT_6X8;
	widget->reservedChars = 6;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 69;
	widget->posY = 0;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getADC1_1;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;


	//P TERM


	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_label);
	s = "P:      I:      D:";
	strcpy(widget->displayString, s);
	widget->posX = 12;
	widget->posY = 20;
	widget->font_size = &FONT_6X8;
	widget->reservedChars = 18;

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 3;
	widget->posY = 33;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getP;
	widget->displayWidget.number_of_dec = 2;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 6;

	//I TERM

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 50;
	widget->posY = 33;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getI;
	widget->displayWidget.number_of_dec = 2;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 6;

	//D TERM

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 91;
	widget->posY = 33;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getD;
	widget->displayWidget.number_of_dec = 2;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 6;

	//ERROR

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 0;
	widget->posY = 50;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getError;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;

	//OUTPUT

	widget = screen_addWidget(scr);
	widgetDefaultsInit(widget, widget_display);
	widget->posX = 40;
	widget->posY = 50;
	widget->font_size = &FONT_6X8;
	widget->displayWidget.getData = &debug_screen_getOutput;
	widget->displayWidget.number_of_dec = 0;
	widget->displayWidget.type = field_uinteger16;
	widget->reservedChars = 4;


}
