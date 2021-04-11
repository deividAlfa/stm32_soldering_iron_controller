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

static uint16_t plotData[100];
static uint8_t plot_Index;
static uint32_t plotTime;
static bool plotUpdate;

static uint32_t barTime;

static uint8_t sleepWidth;
static uint8_t sleepHeigh;
static uint32_t sleepTim=0;
static uint8_t Slp_xpos=30, Slp_ypos=14;
static int8_t Slp_xadd=1, Slp_yadd=1;


static int32_t temp;
static char *tipName[TipSize];
enum mode{  main_irontemp=0, main_disabled, main_ironstatus, main_setpoint, main_tipselect, main_menu,  main_setMode};
enum{ status_running=0x20, status_sleep, status_error };
enum { temp_numeric, temp_graph };
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
	uint32_t idleTick;
	uint32_t enteredSleep;
	bool idle;
	bool ActivityOn;

	uint8_t ironStatus;
	uint8_t prevIronStatus;
	uint8_t setMode;
	uint8_t currentMode;
	bool displayMode;
	uint8_t menuPos;
	widget_t* Selected;
}mainScr;

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets functions
//-------------------------------------------------------------------------------------------------------------------------------


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
		Screen_main.refresh=screen_eraseAndRefresh;	// Redraw screen erasing using dma (faster than erasing each widget)
	}
}

static void * getTip() {
	temp = systemSettings.Profile.currentTip;
	return &temp;
}

static void * main_screen_getIronTemp() {
	if(mainScr.update){
		mainScr.lastTip=readTipTemperatureCompensated(stored_reading,read_Avg);
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

static void updateIronPower() {
	static uint32_t stored=0;
	static uint32_t updateTim;
	if((HAL_GetTick()-updateTim)>9){
		updateTim = HAL_GetTick();
		int32_t tmpPwr = getCurrentPower();
		if(tmpPwr < 0){
			tmpPwr = 0 ;
		}
		tmpPwr = tmpPwr<<12;
		stored = ( ((stored<<6)-stored)+tmpPwr+(1<<11))>>6 ;
		tmpPwr = stored>>12;
		mainScr.lastPwr=tmpPwr;
	}
}

static void setMainWidget(widget_t* w){
	selectable_widget_t* sel =extractSelectablePartFromWidget(w);
	mainScr.drawTick=HAL_GetTick();
	Screen_main.refresh=screen_eraseAndRefresh;
	widgetDisable(mainScr.Selected);
	mainScr.Selected=w;
	widgetEnable(w);
	Screen_main.current_widget=w;
	if(sel){
		sel->state=widget_edit;
		sel->previous_state=widget_selected;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen functions
//-------------------------------------------------------------------------------------------------------------------------------
static void setMainScrTempUnit(void) {
	if(systemSettings.settings.tempUnit==mode_Farenheit){
		Widget_IronTemp.endString="\260F";
		#ifdef USE_NTC
		Widget_AmbTemp.endString="F";
		#endif
		Widget_SetPoint.endString="\260F";
		Widget_SetPoint.editableWidget.max_value=900;
		Widget_SetPoint.editableWidget.min_value=350;
	}
	else{
		Widget_IronTemp.endString="\260C";		// \260 = ASCII dec. 176(°) in octal representation
		#ifdef USE_NTC
		Widget_AmbTemp.endString="C";
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
	setMainScrTempUnit();
	mainScr.idleTick=HAL_GetTick();

}

int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	updateIronPower();
	uint32_t currentTime = HAL_GetTick();

	if(mainScr.update){							// This was set on a previous pass. We reset the flag now.
		mainScr.update = 0;
	}
	if((HAL_GetTick()-mainScr.updateTick)>systemSettings.settings.guiUpdateDelay){
		mainScr.update=1;						// Update realtime readings slower than the rest of the GUI
		mainScr.updateTick=currentTime;
	}

	if(GetIronError()){
		mainScr.ironStatus = status_error;
	}
	else if(getCurrentMode()==mode_sleep){
		mainScr.ironStatus = status_sleep;
	}
	else{
		mainScr.ironStatus = status_running;
	}

	if(Iron.newActivity && !mainScr.ActivityOn && mainScr.currentMode==main_irontemp){
		mainScr.ActivityOn=1;
	}

	else if(Iron.newActivity && mainScr.ActivityOn){
		if((currentTime-Iron.lastActivityTime)>200){
			u8g2_SetDrawColor(&u8g2, BLACK);
			u8g2_DrawBox(&u8g2, 0,OledHeight-pulseXBM[1], pulseXBM[0], pulseXBM[1]);
			mainScr.ActivityOn=0;
			Iron.newActivity=0;
		}
	}

	if(input!=Rotate_Nothing){
		mainScr.idleTick=currentTime;
	}

	if(input==Click && (currentTime - mainScr.enteredSleep) >500 ){		// Disable button wake for 500mS after manually entering sleep mode
		IronWake(source_wakeButton);
	}

	switch(mainScr.currentMode){
		case main_irontemp:
			if(mainScr.ironStatus!=status_running){							// When the screen goes to disable state
				memset(plotData,0,sizeof(plotData));						// Clear plotdata
				plot_Index=0;													// Reset X
				mainScr.setMode=main_disabled;
				mainScr.currentMode=main_setMode;
				break;
			}
			if((input==LongClick)){
				return screen_settingsmenu;
			}
			else if((input==Rotate_Increment_while_click)){
				mainScr.setMode=main_tipselect;
				mainScr.currentMode=main_setMode;
			}
			else if((input==Rotate_Decrement_while_click)){
				mainScr.enteredSleep = currentTime;
				setCurrentMode(mode_sleep);
			}
			else if((input==Rotate_Increment)||(input==Rotate_Decrement)){
				mainScr.setMode=main_setpoint;
				mainScr.currentMode=main_setMode;
			}
			else if(input==Click){
				mainScr.update=1;
				scr->refresh=screen_eraseAndRefresh;
				if(mainScr.displayMode==temp_numeric){
					mainScr.displayMode=temp_graph;
					widgetDisable(&Widget_IronTemp);
				}
				else if(mainScr.displayMode==temp_graph){
					mainScr.displayMode=temp_numeric;
					widgetEnable(&Widget_IronTemp);
				}
			}
			break;

		case main_disabled:
		{
			enum{ dim_idle, dim_down, dim_up };
			static uint8_t dimDisplay=dim_idle;
			static uint32_t dimTimer=0;
			if(((currentTime-mainScr.idleTick)>5000) && (getContrast()>5)){
				dimDisplay=dim_down;
			}
			if((input==Rotate_Decrement) || (input==Rotate_Increment)){
				dimDisplay=dim_up;
			}
			else if(input!=Rotate_Nothing){
				dimDisplay = dim_idle;
				setContrast(systemSettings.settings.contrast);
			}

			if(dimDisplay!=dim_idle){
				if(currentTime-dimTimer>9){
					uint8_t contrast = getContrast();
					dimTimer = currentTime;
					mainScr.idleTick = currentTime;
					if(dimDisplay==dim_down){
						if(contrast>5){
							dimTimer=currentTime;
							setContrast(contrast-5);
						}
						else{
							dimDisplay=dim_idle;
						}
					}
					else{
						if(systemSettings.settings.contrast>(contrast+5)){
							setContrast(contrast+5);
						}
						else{
							setContrast(systemSettings.settings.contrast);
							dimDisplay=dim_idle;
						}
					}
				}
			}

			if((input==LongClick)){
				return screen_settingsmenu;
			}
			else if((input==Rotate_Increment_while_click)){
				mainScr.setMode=main_tipselect;
				mainScr.currentMode=main_setMode;
			}
			if(mainScr.ironStatus==status_running){
				setContrast(systemSettings.settings.contrast);
				mainScr.setMode=main_irontemp;
				mainScr.currentMode=main_setMode;
			}
			break;
		}
		case main_menu:
			if(currentTime-mainScr.idleTick > 5000){
				mainScr.currentMode=main_setMode;
				mainScr.setMode=main_irontemp;
			}
			break;
		case main_setpoint:
		case main_tipselect:

			switch((uint8_t)input){
				case LongClick:
					return -1;
				case Rotate_Nothing:
					if( (mainScr.currentMode==main_setpoint && currentTime-mainScr.idleTick > 1000) || (mainScr.currentMode!=main_setpoint && currentTime-mainScr.idleTick > 5000)){
				case Click:
						mainScr.currentMode=main_setMode;
						mainScr.setMode=main_irontemp;
						return -1;
					}
					break;
				default:
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

	if(mainScr.currentMode==main_setMode){
		mainScr.update=1;
		mainScr.idleTick=currentTime;
		scr->refresh=screen_eraseAndRefresh;
		mainScr.currentMode=mainScr.setMode;
		switch(mainScr.currentMode){
		case main_disabled:
			widgetDisable(&Widget_IronTemp);
			break;
		case main_irontemp:
			setMainWidget(&Widget_IronTemp);
			if(mainScr.displayMode==temp_graph){
				widgetDisable(&Widget_IronTemp);
			}
			break;
		case main_menu:
			break;
		case main_setpoint:
			setMainWidget(&Widget_SetPoint);
			break;
		case main_tipselect:
			setMainWidget(&Widget_TipSelect);
			break;
		default:
			break;
		}
		return -1;
	}
	return default_screenProcessInput(scr, input, state);
}


void main_screen_draw(screen_t *scr){
	uint8_t scr_refresh;
	static uint16_t lastState = 0;
	uint16_t currentState = Iron.Error.Flags + mainScr.ironStatus + mainScr.currentMode;		// Simple checksum to detect changes
	if(currentState!=lastState){
		currentState=lastState;
		scr->refresh=screen_eraseAndRefresh;
	}
	if(Widget_SetPoint.refresh || Widget_IronTemp.refresh){
		scr->refresh=screen_eraseAndRefresh;
	}
	if(mainScr.currentMode!=main_disabled && (HAL_GetTick()-plotTime)>99){			// Only store values if running
		plotUpdate=1;
		scr->refresh=screen_eraseAndRefresh;
		plotTime=HAL_GetTick();
		int16_t t = readTipTemperatureCompensated(stored_reading,read_Avg);
		if(systemSettings.settings.tempUnit==mode_Farenheit){
			t = TempConversion(t, mode_Celsius, 0);
		}

		if (t<160) t = 160;
		if (t>500) t = 500;

		plotData[plot_Index] = t;
		if(++plot_Index>99){
			plot_Index=0;
		}
	}
	if(mainScr.ironStatus==status_sleep && mainScr.currentMode==main_disabled){
		if((HAL_GetTick()-sleepTim)>50){
			sleepTim=HAL_GetTick();
			scr->refresh=screen_eraseAndRefresh;
			Slp_xpos += Slp_xadd;
			Slp_ypos += Slp_yadd;
			if((Slp_xpos+sleepWidth)>OledWidth){
				Slp_xadd = -1;
			}
			else if(Slp_xpos==0){
				Slp_xadd = 1;
			}
			if(Slp_ypos+sleepHeigh>OledHeight){
				Slp_yadd = -1;
			}
			else if(Slp_ypos<14){
				Slp_yadd = 1;
			}
		}
	}

	if(scr->refresh==screen_eraseAndRefresh){
		FillBuffer(BLACK,fill_dma);
		scr->refresh=screen_blankRefresh;
		// Screen is erased now, draw anything here before main screen draws
	}
	scr_refresh=scr->refresh;
	default_screenDraw(scr);

	if(scr_refresh){
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
		if(mainScr.currentMode==main_disabled){
			u8g2_SetFont(&u8g2, u8g2_font_mainBig);
			if(mainScr.ironStatus==status_error){
				uint8_t Err_ypos = 14;


				if(Iron.Error.Flags==0x81){						// 0x81 = Only "No iron detected". Don't show error just for it
					u8g2_SetFont(&u8g2, u8g2_font_mainBig);
					putStrAligned("NO IRON", 26, align_center);
				}
				else{
					char errStr[16];
					sprintf(errStr, "ERROR %X",Iron.Error.Flags);
					u8g2_SetFont(&u8g2, u8g2_font_t0_16_tr);
					putStrAligned(errStr, Err_ypos, align_center);
					Err_ypos+=13;
					if(Iron.Error.failState){
						putStrAligned("Internal failure", Err_ypos, align_center);
						Err_ypos+=13;
					}
					if(Iron.Error.V_low){
						putStrAligned("Voltage low!", Err_ypos, align_center);
						Err_ypos+=13;
					}
					if(Iron.Error.noIron){
						putStrAligned("No iron detected", Err_ypos, align_center);
						Err_ypos+=13;
					}
					if(Iron.Error.NTC_high){
						putStrAligned("NTC read high!", Err_ypos, align_center);
						Err_ypos+=13;
					}
					else if(Iron.Error.NTC_low){
						putStrAligned("NTC read low!", Err_ypos, align_center);
						Err_ypos+=13;
					}
				}
			}
			else if(mainScr.ironStatus==status_sleep){
				u8g2_DrawStr(&u8g2, Slp_xpos, Slp_ypos, "SLEEP");
			}
		}
		else if(mainScr.currentMode==main_tipselect){
			u8g2_SetFont(&u8g2, u8g2_font_t0_16_tr);
			putStrAligned("TIP SELECTION", 16, align_center);
		}
	}

	if(mainScr.ironStatus==status_running){
		if( scr_refresh || (HAL_GetTick()-barTime)>9){	// Update every 10mS or if screen was erased
			if(scr_refresh<screen_eraseAndRefresh){
 				u8g2_SetDrawColor(&u8g2,BLACK);
				u8g2_DrawBox(&u8g2, 13 , OledHeight-6, 100, 5);
			}
			u8g2_SetDrawColor(&u8g2,WHITE);
			u8g2_DrawBox(&u8g2, 13, OledHeight-5, mainScr.lastPwr, 3);
			u8g2_DrawRFrame(&u8g2, 13, OledHeight-6, 100, 5, 2);
		}

		if((scr_refresh || plotUpdate) && mainScr.currentMode==main_irontemp && mainScr.displayMode==temp_graph){	//Update every 100mS or if screen is erased
			plotUpdate=0;
			int16_t t = Iron.CurrentSetTemperature;

			bool magnify = true;		// for future, to support both graph types

			if(systemSettings.settings.tempUnit==mode_Farenheit){
				t = TempConversion(t, mode_Celsius, 0);
			}

			// plot is 16-56 V, 14-113 H ?
			u8g2_DrawVLine(&u8g2, 11, 16, 41);						// left scale

			if (magnify) {											// graphing magnified
				for(uint8_t y=16; y<57; y+=10){
					u8g2_DrawHLine(&u8g2, 7, y, 4); 				// left ticks
				}

				for(uint8_t x=0; x<100; x++){
					uint8_t pos=plot_Index+x;
					if(pos>99){ pos-=100; }							// Reset index if > 99

					uint16_t plotV = plotData[pos];

					if (plotV < t-20) plotV = 0;
					else if (plotV >= t+20) plotV = 40;
					else plotV = (plotV-t+20) ; 					// relative to t, +-20C
					u8g2_DrawVLine(&u8g2, x+13, 56-plotV, plotV);	// data points
				}

				uint8_t set= 36;
				u8g2_DrawTriangle(&u8g2, 124, set-5, 124, set+5, 115, set);		// set temp marker

			} else {												// graphing full range
				for(uint8_t y=16; y<57; y+=13){
					u8g2_DrawHLine(&u8g2, 7, y, 4); 				// left ticks
				}

				for(uint8_t x=0; x<100; x++){
					uint8_t pos=plot_Index+x;
					if(pos>99) pos-=100;

					uint16_t plotV = plotData[pos];

					if (plotV<180) plotV = 0;
					else plotV = (plotV-180) >> 3; 					// divide by 8, (500-180)/8=40
					u8g2_DrawVLine(&u8g2, x+13, 56-plotV, plotV);	// data points
				}

				uint8_t set;
				if(t<188){ set = 1; }
				else {
					set=(t-180)>>3;
				}
				set= 56-set;
				u8g2_DrawTriangle(&u8g2, 124, set-5, 124, set+5, 115, set);		// set temp marker
			}
		}
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
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=5;
	w->textAlign=align_center;
	w->dispAlign=align_center;
	w->font=u8g2_font_iron;
	w->posY = 17;
	w->displayWidget.getData = &main_screen_getIronTemp;

	// Tip temperature setpoint
	w=&Widget_SetPoint;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_editable);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=5;
	w->posY = Widget_IronTemp.posY-2;
	dis->getData = &getTemp;
	w->dispAlign=align_center;
	w->textAlign=align_center;
	w->font=Widget_IronTemp.font;
	w->editableWidget.selectable.tab = 1;
	w->editableWidget.setData = (void (*)(void *))&setTemp;
	w->frameType=frame_solid;
	w->radius=0;
	w->enabled=0;

	#ifdef USE_VIN
	//V input display
	w = &Widget_Vsupply;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
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
	widgetDefaultsInit(w, widget_display);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=7;
	w->dispAlign=align_right;
	w->textAlign=align_center;
	w->posY= 2;
	dis->number_of_dec=1;
	w->font=u8g2_font_labels;
	dis->getData = &main_screen_getAmbTemp;
	#endif

	// Tips
	w=&Widget_TipSelect;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option);
	dis=extractDisplayPartFromWidget(w);
	dis->reservedChars=TipCharSize-1;
	w->posY = 32;
	w->dispAlign=align_center;
	w->textAlign=align_center;
	w->multiOptionWidget.editable.inputData.getData = &getTip;
	w->multiOptionWidget.editable.inputData.number_of_dec = 0;
	w->multiOptionWidget.editable.big_step = 0;
	w->multiOptionWidget.editable.step = 0;
	w->multiOptionWidget.editable.selectable.tab = 2;
	w->multiOptionWidget.editable.setData = (void (*)(void *))&setTip;
	w->multiOptionWidget.options = tipName;
	w->enabled=0;
	w->frameType=frame_disabled;

	setMainWidget(&Widget_IronTemp);
	u8g2_SetFont(&u8g2,u8g2_font_mainBig);
	sleepWidth=u8g2_GetStrWidth(&u8g2, "SLEEP")+2;
	sleepHeigh= u8g2_GetMaxCharHeight(&u8g2)+3;
}

