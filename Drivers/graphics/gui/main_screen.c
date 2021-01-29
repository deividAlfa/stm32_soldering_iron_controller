                                                                                                                                                                                                                                                                                                                                                                                                                                                   /*
 * main_screen.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#include "main_screen.h"
#include "oled.h"
#include "gui.h"

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen variables
//-------------------------------------------------------------------------------------------------------------------------------
static int32_t temp;
static char *tipName[TipSize];
enum mode{ main_irontemp, main_tempgraph, main_setmsg, main_msg, main_noiron, main_setpoint, main_tipselect, main_menu,  main_resume};
enum{ msg_running, msg_sleep, msg_noiron };
const uint8_t pulseXBM[] ={
	8,9,
	0x04, 0x0A, 0x0A, 0x0A, 0x89, 0x50, 0x50, 0x50, 0x20, };

const uint8_t tempXBM[] ={
	5,10,
	0x04, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, };

const uint8_t voltXBM[] ={
	5,10,
	0x10, 0x08, 0x0C, 0x06, 0x0F, 0x1E, 0x0C, 0x06, 0x02, 0x01, };

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
#ifdef USE_NTC
static widget_t Widget_AmbTemp;
#endif
#ifdef USE_VIN
static widget_t Widget_Vsupply;
#endif
static widget_t Widget_IronTemp;
static widget_t Widget_TipSelect;
static widget_t Widget_SetPoint;
static widget_t Widget_Shortcut;

static struct{
	uint32_t updateTick;
	bool update;

	uint16_t lastTip;
	uint8_t lastPwr;

	#ifdef USE_NTC
	int16_t lastAmb;
	#endif

	#ifdef USE_VIN
	uint16_t lastVin;
	#endif
	uint32_t drawTick;
	bool ActivityOn;
	uint8_t msgStatus;
	uint8_t mode;
	uint8_t lastMode;
	uint8_t lastTempMode;
	uint8_t shortcut;
	widget_t* Selected;
}mainScr;

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------

static void setShortcut(uint16_t *val) {
	if(mainScr.shortcut!=*val){
		mainScr.shortcut=*val;
		Screen_main.refresh=2;	// Redraw screen erasing using dma (faster than erasing each widget)
	}
}

static void * getShortcut() {
	temp = mainScr.shortcut;
	return &temp;
}

static void setTemp(uint16_t *val) {
	setSetTemperature(*val);
}

static void * getTemp() {
	temp = systemSettings.Profile.UserSetTemperature;
	return &temp;
}


static void setTip(uint8_t *val) {
	if(systemSettings.Profile.currentTip != *val){
		systemSettings.Profile.currentTip = *val;
		setCurrentTip(*val);
		Screen_main.refresh=2;	// Redraw screen erasing using dma (faster than erasing each widget)
	}
}

static void * getTip() {
	temp = systemSettings.Profile.currentTip;
	return &temp;
}

static void * main_screen_getIronTemp() {
	if(mainScr.update){
		uint16_t t= readTipTemperatureCompensated(stored_reading,read_Avg);
		if (mainScr.lastTip!=t){
			mainScr.lastTip=t;
			if(mainScr.mode==main_irontemp){
				Screen_main.refresh=2;	// Redraw screen erasing using dma (faster than erasing each widget)
			}
		}
	}
	temp=mainScr.lastTip;
	return &temp;
}

#ifdef USE_VIN
static void * main_screen_getVin() {
	if(mainScr.update){
		mainScr.lastVin = getSupplyVoltage_v_x10();
	}
	temp=mainScr.lastVin;
	return &temp;
}
#endif

#ifdef USE_NTC
static void * main_screen_getAmbTemp() {
	if(mainScr.update){
		mainScr.lastAmb = readColdJunctionSensorTemp_x10(systemSettings.settings.tempUnit);
	}
	temp=mainScr.lastAmb;
	return &temp;
}
#endif

static bool updateIronPower() {
	static uint32_t stored=0;

	int32_t tmpPwr = getCurrentPower();
	if(tmpPwr < 0){
		tmpPwr = 0 ;
	}

	//tmpPwr = ((tmpPwr*323)>>8)<<12;
	tmpPwr = tmpPwr<<12;
	stored = ( ((stored<<9 )-stored)+tmpPwr+(1<<11))>>9 ;
	tmpPwr = stored>>12;
	if(tmpPwr!=mainScr.lastPwr){
		mainScr.lastPwr=tmpPwr;
		return 1;
	}
	return 0;
}

static void setMainWidget(widget_t* w){
	mainScr.drawTick=HAL_GetTick();
	Screen_main.refresh=screen_eraseAndRefresh;
	widgetDisable(mainScr.Selected);
	mainScr.Selected=w;
	widgetEnable(w);
	Screen_main.current_widget=w;
	w->editableWidget.selectable.state=widget_edit;
	w->editableWidget.selectable.previous_state=widget_selected;
}

static int ShortcutProcessInput(widget_t* w, RE_Rotation_t r, RE_State_t * s){
	if(r==Click){
		mainScr.mode=main_resume;
		switch(mainScr.shortcut){
			case 0:
				break;
			case 1:				// TIPS
				mainScr.mode=main_tipselect;
				setMainWidget(&Widget_TipSelect);
				return -1;
			case 2:				// PID
				return screen_pid;
			case 3:				// IRON
				return screen_iron;
			case 4:				// SYSTEM
				return screen_system;
			case 5:				// EDIT TIPS
				return screen_edit_iron_tips;
			case 6:				// CALIBRATIONS
				return screen_edit_calibration_wait;
			default:
				break;
		}
	}
	else{
		default_widgetProcessInput(w, r, s);
	}
	return -1;
}


//-------------------------------------------------------------------------------------------------------------------------------
// Main screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void setMainScrTempUnit(void) {
	if(systemSettings.settings.tempUnit==mode_Farenheit){
		Widget_IronTemp.endString="\260F";
		#ifdef USE_NTC
		Widget_AmbTemp.endString="\260F";
		#endif
		Widget_SetPoint.endString="\260F";
		Widget_SetPoint.editableWidget.max_value=900;
		Widget_SetPoint.editableWidget.min_value=350;
	}
	else{
		Widget_IronTemp.endString="\260C";
		#ifdef USE_NTC
		Widget_AmbTemp.endString="\260C";
		#endif
		Widget_SetPoint.endString="\260C";
		Widget_SetPoint.editableWidget.max_value=480;
		Widget_SetPoint.editableWidget.min_value=180;
	}
}


static void main_screen_init(screen_t *scr) {
	default_init(scr);
	Widget_TipSelect.multiOptionWidget.numberOfOptions = systemSettings.Profile.currentNumberOfTips;
	Widget_SetPoint.editableWidget.step = systemSettings.settings.tempStep;
	Widget_SetPoint.editableWidget.big_step = systemSettings.settings.tempStep;
	Widget_SetPoint.editableWidget.step = systemSettings.settings.tempStep;
	Widget_SetPoint.editableWidget.big_step = systemSettings.settings.tempStep;
	setMainScrTempUnit();
	mainScr.mode=main_irontemp;
	mainScr.lastMode=main_irontemp;
	mainScr.lastTempMode=main_irontemp;
	setMainWidget(&Widget_IronTemp);
}
int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	if(mainScr.update){
		mainScr.update = 0;
	}
	if((HAL_GetTick()-mainScr.updateTick)>systemSettings.settings.guiUpdateDelay){
		mainScr.update=1;						//Update realtime readings slower than the rest of the GUI
		mainScr.updateTick=HAL_GetTick();
	}

	if(!GetIronPresence()){
		mainScr.msgStatus = msg_noiron;
	}
	else if(getCurrentMode()==mode_sleep){
		mainScr.msgStatus = msg_sleep;
	}
	else{
		mainScr.msgStatus = msg_running;
	}

	if(Iron.newActivity && !mainScr.ActivityOn && mainScr.mode==main_irontemp){
		mainScr.ActivityOn=1;
	}

	else if(Iron.newActivity && mainScr.ActivityOn){
		if((HAL_GetTick()-Iron.lastActivityTime)>50){
			u8g2_SetDrawColor(&u8g2, BLACK);
			u8g2_DrawBox(&u8g2, 0,OledHeight-pulseXBM[1], pulseXBM[0], pulseXBM[1]);
			mainScr.ActivityOn=0;
			Iron.newActivity=0;
		}
	}

	if((input!=Rotate_Nothing)&&(input!=LongClick)){
		IronWake(source_wakeButton);
	}
	if((mainScr.msgStatus!=msg_running)&&(mainScr.lastMode!=main_msg)){
		mainScr.lastMode=main_msg;
		if((mainScr.mode!=main_menu)&&(mainScr.mode!=main_tipselect)){
			mainScr.mode=main_resume;
		}
	}

	if((input==LongClick)){
		mainScr.lastMode=mainScr.mode;
		mainScr.mode=main_menu;
		setMainWidget(&Widget_Shortcut);
	}
	switch(mainScr.mode){

		case main_msg:
			if(mainScr.msgStatus == msg_running){
				mainScr.lastMode=mainScr.lastTempMode;
				mainScr.mode=main_resume;
			}
			break;
		case main_irontemp:
		case main_tempgraph:
			if((input==Rotate_Increment)||(input==Rotate_Decrement)){
				scr->refresh=screen_eraseAndRefresh;
				mainScr.lastMode=mainScr.mode;
				mainScr.lastTempMode=mainScr.mode;
				mainScr.mode=main_setpoint;
				setMainWidget(&Widget_SetPoint);
			}
			else if(input==Click){
				scr->refresh=screen_eraseAndRefresh;
				mainScr.lastMode=mainScr.mode;
				mainScr.lastTempMode=mainScr.mode;
				if(mainScr.mode==main_tempgraph){
					mainScr.mode=main_irontemp;
					widgetEnable(&Widget_IronTemp);
				}
				else if(mainScr.mode==main_irontemp){
					mainScr.mode=main_tempgraph;
					widgetDisable(&Widget_IronTemp);
				}
			}

			break;

		case main_menu:
			if(input==Click){
				break;
			}

		case main_setpoint:
		case main_tipselect:
			switch((uint8_t)input){
				case LongClick:
				case Rotate_Nothing:
					if((HAL_GetTick()-mainScr.drawTick > 500) && (mainScr.mode==main_setpoint)){
				case Click:
						mainScr.mode=main_resume;
						return -1;
					}
					break;
				default:
					mainScr.drawTick=HAL_GetTick();
					if(input==Rotate_Increment_while_click){
						input=Rotate_Increment;
					}
					if(input==Rotate_Decrement_while_click){
						input=Rotate_Decrement;
					}
					break;
			}

		default:
			break;
	}

	if(mainScr.mode==main_resume){
		scr->refresh=screen_eraseAndRefresh;
		widgetDisable(mainScr.Selected);
		mainScr.mode=mainScr.lastMode;
		if(mainScr.lastMode==main_msg){
			return -1;
		}
		setMainWidget(&Widget_IronTemp);
		if(mainScr.lastMode==main_tempgraph){
			widgetDisable(&Widget_IronTemp);
			return -1;
		}
	}
	return default_screenProcessInput(scr, input, state);
	//return -1;
}
uint8_t plotData[100];
uint8_t plotX;
uint32_t plotTime;
void main_screen_draw(screen_t *scr){
	uint8_t scr_refresh=scr->refresh;

	if(scr->refresh==screen_eraseAndRefresh){
		FillBuffer(BLACK,fill_dma);
		scr->refresh=screen_blankRefresh;
		// Clean screen here, draw here before main screen draw
	}

	default_screenDraw(scr);
/*
	if(temp<=100){
		x=0;
	}
	else{
		x=temp>>8;
	}
	*/
	if(scr_refresh>=screen_eraseAndRefresh){
		u8g2_SetDrawColor(&u8g2, WHITE);
		#ifdef USE_NTC
		u8g2_DrawXBMP(&u8g2, Widget_AmbTemp.posX-tempXBM[0]-2, Widget_AmbTemp.posY, tempXBM[0], tempXBM[1], &tempXBM[2]);
		#endif

		#ifdef USE_VIN
		u8g2_DrawXBMP(&u8g2, Widget_Vsupply.posX-voltXBM[0]-2, Widget_Vsupply.posY, voltXBM[0], voltXBM[1], &voltXBM[2]);
		#endif
		if(mainScr.ActivityOn){
			u8g2_DrawXBMP(&u8g2, 0,OledHeight-pulseXBM[1], pulseXBM[0], pulseXBM[1], &pulseXBM[2]);
		}
		if((mainScr.msgStatus==msg_noiron) && (mainScr.mode==main_msg)){
			u8g2_SetFont(&u8g2, u8g2_font_main_msg);
			putStrAligned("NO IRON", Widget_TipSelect.posY, align_center);
		}
		if((mainScr.msgStatus==msg_sleep) && (mainScr.mode==main_msg)){
			u8g2_SetFont(&u8g2, u8g2_font_main_msg);
			putStrAligned("SLEEP", Widget_TipSelect.posY, align_center);
		}
	}

	if((scr_refresh || updateIronPower())){				// Value changed or screen redrawed?

		if(scr_refresh<screen_eraseAndRefresh){
			u8g2_SetDrawColor(&u8g2,BLACK);
			u8g2_DrawBox(&u8g2, 13 , OledHeight-6, 100, 5);
		}
		u8g2_SetDrawColor(&u8g2,WHITE);
		u8g2_DrawBox(&u8g2, 13, OledHeight-5, mainScr.lastPwr, 3);
		u8g2_DrawRFrame(&u8g2, 13, OledHeight-6, 100, 5, 2);
	}
	if((HAL_GetTick()-plotTime)>99){
		scr->refresh=screen_eraseAndRefresh;
		plotTime=HAL_GetTick();
		uint16_t t = readTipTemperatureCompensated(stored_reading,read_Avg)>>4;
		if(t>31){ t=31; }
		plotData[plotX] = t;
		if(++plotX>99){
			plotX=0;
		}
	}
	if(scr_refresh && mainScr.mode==main_tempgraph){
		u8g2_DrawVLine(&u8g2, 10, 22, 31);
		for(uint8_t x=22; x<=54; x+=6){
			u8g2_DrawHLine(&u8g2, 7, x, 3);
		}
		for(uint8_t x=0; x<100; x++){
			uint8_t pos=plotX+x;
			if(pos>99){
				pos -=100;
			}
			u8g2_DrawVLine(&u8g2, x+13, 53-plotData[pos], plotData[pos]);
		}
		uint8_t set=53-(Iron.CurrentSetTemperature>>4);
		u8g2_DrawTriangle(&u8g2, 120, set-5, 120, set+5, 115, set);
		u8g2_SetDrawColor(&u8g2, XOR);
		u8g2_DrawHLine(&u8g2, 13, set, 100);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen setup
//-------------------------------------------------------------------------------------------------------------------------------
void main_screen_setup(screen_t *scr) {

	//
	for(int x = 0; x < TipSize; x++) {
		tipName[x] = systemSettings.Profile.tip[x].name;
	}

	screen_setDefaults(scr);
	scr->draw = &main_screen_draw;
	scr->init = &main_screen_init;
	scr->processInput = &main_screenProcessInput;
	widget_t *w;
	displayOnly_widget_t* dis;

	//iron tip temperature display
	w=&Widget_IronTemp;
	screen_addWidget(w,scr);
	static char irontmp[6];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=irontmp;
	dis->reservedChars=5;
	w->textAlign=align_center;
	w->dispAlign=align_center;
	w->font=u8g2_font_iron;
	w->posY = 18;
	w->displayWidget.getData = &main_screen_getIronTemp;

	// Tip temperature setpoint
	w=&Widget_SetPoint;
	screen_addWidget(w,scr);
	static char SetP[6];
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=SetP;
	dis->reservedChars=5;
	w->posY = Widget_IronTemp.posY-2;
	dis->getData = &getTemp;
	w->dispAlign=align_center;
	w->textAlign=align_center;
	w->font=Widget_IronTemp.font;
	w->editableWidget.selectable.tab = 1;
	w->editableWidget.setData = (void (*)(void *))&setTemp;
	w->frameType=frame_disabled;
	w->enabled=0;

	#ifdef USE_VIN
	//V input display
	w = &Widget_Vsupply;
	screen_addWidget(w, scr);
	static char vin[6];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=vin;
	w->endString="V";
	dis->reservedChars=5;
	w->textAlign=align_center;
	dis->number_of_dec=1;
	//w->font=u8g2_font_wizzard12labels;
	w->font=u8g2_font_labels;
	w->posX = voltXBM[0]+2;
	w->posY= 2;
	dis->getData = &main_screen_getVin;
	//w->width = 40;
	#endif

	#ifdef USE_NTC
	//Ambient temperature display
	w=&Widget_AmbTemp;
	screen_addWidget(w,scr);
	static char ntc[7];
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	w->displayString=ntc;
	dis->reservedChars=6;
	w->dispAlign=align_right;
	w->textAlign=align_center;
	w->posY= 2;
	dis->number_of_dec=1;
	w->font=u8g2_font_labels;
	dis->getData = &main_screen_getAmbTemp;
	//w->width = 24;
	#endif

	// Tips
	w=&Widget_TipSelect;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=TipCharSize-1;
	w->posY = 22;
	w->dispAlign=align_center;
	w->textAlign=align_center;
	w->font=u8g2_font_main_msg;
	w->multiOptionWidget.editable.inputData.getData = &getTip;
	w->multiOptionWidget.editable.inputData.number_of_dec = 0;
	w->multiOptionWidget.editable.big_step = 0;
	w->multiOptionWidget.editable.step = 0;
	w->multiOptionWidget.editable.selectable.tab = 2;
	w->multiOptionWidget.editable.setData = (void (*)(void *))&setTip;
	w->multiOptionWidget.options = tipName;
	w->enabled=0;
	w->frameType=frame_disabled;

	//********[ Shortcut Widget ]***********************************************************
	//
	w = &Widget_Shortcut;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->getData = &getShortcut;
	w->posY = Widget_TipSelect.posY;
	w->dispAlign=align_center;
	w->textAlign=align_center;
	w->editableWidget.big_step = 1;
	w->editableWidget.step = 1;
	w->font=u8g2_font_main_msg;
	w->frameType=frame_disabled;
	w->enabled=0;

	w->editableWidget.setData = (void (*)(void *))&setShortcut;
	w->multiOptionWidget.editable.selectable.processInput=&ShortcutProcessInput;
	w->editableWidget.max_value = 6;
	w->editableWidget.min_value = 0;
	w->multiOptionWidget.options =shortcuts;
	w->multiOptionWidget.numberOfOptions = 7;

}

