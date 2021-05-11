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
enum mode{  main_irontemp=0, main_disabled, main_ironstatus, main_setpoint, main_tipselect, main_setMode};
enum{ status_running=0x20, status_sleep, status_error };
enum { temp_numeric, temp_graph };
const uint8_t shakeXBM[] ={
	9, 9,
	0x70, 0x00, 0x80, 0x00, 0x30, 0x01, 0x40, 0x01, 0x45, 0x01, 0x05, 0x00,
  0x19, 0x00, 0x02, 0x00, 0x1C, 0x00, };


const uint8_t tempXBM[] ={
	10, 13,
  0x70, 0x00, 0x8B, 0x00, 0x88, 0x00, 0xAB, 0x00, 0xA8, 0x00, 0xAB, 0x00,
  0xA8, 0x00, 0x24, 0x01, 0x72, 0x02, 0x72, 0x02, 0x72, 0x02, 0x04, 0x01,
  0xF8, 0x00, };

#ifdef USE_VIN
const uint8_t voltXBM[] ={
	6, 9,
	0x30, 0x18, 0x0C, 0x06, 0x1F, 0x18, 0x0C, 0x06, 0x01, };
#endif

const uint8_t warningXBM[] ={
  13, 13,
  0x40, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0x10, 0x01, 0x10, 0x01, 0x48, 0x02,
  0x48, 0x02, 0x44, 0x04, 0x44, 0x04, 0x02, 0x08, 0x42, 0x08, 0x01, 0x10,
  0xFF, 0x1F, };

//-------------------------------------------------------------------------------------------------------------------------------
// Main screen widgets
//-------------------------------------------------------------------------------------------------------------------------------
screen_t Screen_main;

#ifdef USE_NTC
static widget_t Widget_AmbTemp;
static displayOnly_widget_t display_AmbTemp;
#endif
#ifdef USE_VIN
static widget_t Widget_Vsupply;
static displayOnly_widget_t display_Vsupply;
#endif
static widget_t Widget_IronTemp;
static displayOnly_widget_t display_IronTemp;

static widget_t Widget_TipSelect;
static editable_widget_t editable_TipSelect;

static widget_t Widget_SetPoint;
static editable_widget_t editable_SetPoint;

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
	if(systemSettings.Profile.currentTip != *val){      // Tip temp uses huge font that partially overlaps other widgets
		systemSettings.Profile.currentTip = *val;
		setCurrentTip(*val);
		Screen_main.refresh=screenRefresh_eraseNow;	      // So, we must redraw the screen. Tip temp is drawed first, then the rest go on top.
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
	Screen_main.refresh=screenRefresh_eraseNow;
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
		display_IronTemp.endString="\260F";
		#ifdef USE_NTC
		display_AmbTemp.endString="F";
		#endif
		editable_SetPoint.inputData.endString="\260F";
	}
	else{
	  display_IronTemp.endString="\260C";		// \260 = ASCII dec. 176(°) in octal representation
		#ifdef USE_NTC
	  display_AmbTemp.endString="C";
		#endif
	  editable_SetPoint.inputData.endString="\260C";
	}
}


static void main_screen_init(screen_t *scr) {
	default_init(scr);

	mainScr.currentMode = main_irontemp;
  setMainWidget(&Widget_IronTemp);
  if(mainScr.displayMode==temp_graph){
    widgetDisable(&Widget_IronTemp);
  }

	editable_TipSelect.numberOfOptions = systemSettings.Profile.currentNumberOfTips;
  editable_SetPoint.step = systemSettings.settings.tempStep;
  editable_SetPoint.big_step = systemSettings.settings.tempStep;
  editable_SetPoint.max_value = systemSettings.Profile.MaxSetTemperature;
  editable_SetPoint.min_value = systemSettings.Profile.MinSetTemperature;
	setMainScrTempUnit();
	mainScr.idleTick=HAL_GetTick();

}

int main_screenProcessInput(screen_t * scr, RE_Rotation_t input, RE_State_t *state) {
	updateIronPower();
	uint32_t currentTime = HAL_GetTick();
	uint8_t error = GetIronError();

	if(mainScr.update){							            // This was set on a previous pass. We reset the flag now.
		mainScr.update = 0;
	}
	if((HAL_GetTick()-mainScr.updateTick)>systemSettings.settings.guiUpdateDelay){
		mainScr.update=1;						              // Update realtime readings slower than the rest of the GUI
		mainScr.updateTick=currentTime;
	}

	if(error){
	  if(!(mainScr.ironStatus == status_sleep && error==1)){
	    mainScr.ironStatus = status_error;
	  }
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
			u8g2_DrawBox(&u8g2, 0,OledHeight-shakeXBM[1], shakeXBM[0], shakeXBM[1]);
			mainScr.ActivityOn=0;
			Iron.newActivity=0;
		}
	}

	if(input!=Rotate_Nothing){
		mainScr.idleTick=currentTime;
	}

	// Check for click input and wake iron. Disable button wake for 500mS after manually entering sleep mode
	if(input==Click && ((mainScr.currentMode==main_irontemp) || (mainScr.currentMode==main_disabled)) && (currentTime - mainScr.enteredSleep) >500 ){
		IronWake(source_wakeButton);
	}

	switch(mainScr.currentMode){
		case main_irontemp:
			if(mainScr.ironStatus!=status_running){							// When the screen goes to disable state
				memset(plotData,0,sizeof(plotData));						  // Clear plotdata
				plot_Index=0;													            // Reset X
				mainScr.setMode=main_disabled;
				mainScr.ActivityOn = 0;
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
				scr->refresh=screenRefresh_eraseNow;
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
			uint8_t contrast = getContrast();

			if((currentTime-mainScr.idleTick)>15000 && contrast>5){
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
				if(systemSettings.settings.screenDimming && currentTime-dimTimer>9){
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
		case main_tipselect:
      if(input==LongClick){
        return screen_edit_tip_settings;
      }
		case main_setpoint:
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
		scr->refresh=screenRefresh_eraseNow;
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
	uint16_t currentState = Iron.Error.Flags + mainScr.ironStatus + mainScr.currentMode;		// Simple checksum method to detect changes
	if(lastState!=currentState){
		lastState=currentState;
		scr->refresh=screenRefresh_eraseNow;
	}
	if(Widget_SetPoint.refresh || Widget_IronTemp.refresh){
		scr->refresh=screenRefresh_eraseNow;
	}
	if(mainScr.currentMode!=main_disabled && (HAL_GetTick()-plotTime)>99){			            // Only store values if running
		plotUpdate=1;
		scr->refresh=screenRefresh_eraseNow;
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
			scr->refresh=screenRefresh_eraseNow;
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
			else if(Slp_ypos<16){
				Slp_yadd = 1;
			}
		}
	}

	if(scr->refresh==screenRefresh_eraseNow){
		FillBuffer(BLACK,fill_dma);
		scr->refresh=screenRefresh_alreadyErased;
		// Screen is erased now, draw anything here before main screen draws
	}
	scr_refresh=scr->refresh;
	default_screenDraw(scr);

	if(scr_refresh){
		u8g2_SetDrawColor(&u8g2, WHITE);

		#ifdef USE_NTC
		u8g2_DrawXBMP(&u8g2, Widget_AmbTemp.posX-tempXBM[0]-2, 0, tempXBM[0], tempXBM[1], &tempXBM[2]);
		#endif

		#ifdef USE_VIN
		u8g2_DrawXBMP(&u8g2, 0, 2, voltXBM[0], voltXBM[1], &voltXBM[2]);
		#endif
		if(mainScr.ActivityOn){
			u8g2_DrawXBMP(&u8g2, 57, 2, shakeXBM[0], shakeXBM[1], &shakeXBM[2]);
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
				u8g2_SetFont(&u8g2, u8g2_font_labels);
				if(!Iron.Error.Flags && readTipTemperatureCompensated(0,0)>120){
				  u8g2_DrawXBMP(&u8g2, 42,0, warningXBM[0], warningXBM[1], &warningXBM[2]);
				  u8g2_DrawStr(&u8g2, 55, 2, "HOT!");
				}
			}
		}
		else if(mainScr.currentMode==main_tipselect){
			u8g2_SetFont(&u8g2, u8g2_font_t0_16_tr);
			putStrAligned("TIP SELECTION", 16, align_center);
		}
	}

	if(mainScr.ironStatus==status_running){
		if( scr_refresh || (HAL_GetTick()-barTime)>9){	// Update every 10mS or if screen was erased
			if(scr_refresh<screenRefresh_eraseNow){       // If screen not erased
 				u8g2_SetDrawColor(&u8g2,BLACK);             // Draw a black square to wipe old widget data
				u8g2_DrawBox(&u8g2, 13 , OledHeight-6, 100, 5);
			}
			u8g2_SetDrawColor(&u8g2,WHITE);
			u8g2_DrawBox(&u8g2, 13, OledHeight-5, mainScr.lastPwr, 3);
			u8g2_DrawRFrame(&u8g2, 13, OledHeight-6, 100, 5, 2);
		}

		if((scr_refresh || plotUpdate) && mainScr.currentMode==main_irontemp && mainScr.displayMode==temp_graph){	//Update every 100mS or if screen is erased
			plotUpdate=0;
			uint8_t set;
			int16_t t = Iron.CurrentSetTemperature;

			bool magnify = true;		                        // for future, to support both graph types

			if(systemSettings.settings.tempUnit==mode_Farenheit){
				t = TempConversion(t, mode_Celsius, 0);
			}

			// plot is 16-56 V, 14-113 H ?
			u8g2_DrawVLine(&u8g2, 11, 16, 41);						  // left scale

			if (magnify) {											            // graphing magnified
				for(uint8_t y=16; y<57; y+=10){
					u8g2_DrawHLine(&u8g2, 7, y, 4); 				    // left ticks
				}

				for(uint8_t x=0; x<100; x++){
					uint8_t pos=plot_Index+x;
					if(pos>99){ pos-=100; }							        // Reset index if > 99

					uint16_t plotV = plotData[pos];

					if (plotV < t-20) plotV = 0;
					else if (plotV >= t+20) plotV = 40;
					else plotV = (plotV-t+20) ; 					      // relative to t, +-20C
					u8g2_DrawVLine(&u8g2, x+13, 56-plotV, plotV);	// data points
				}
				set= 36;

			} else {												                // graphing full range
				for(uint8_t y=16; y<57; y+=13){
					u8g2_DrawHLine(&u8g2, 7, y, 4); 				    // left ticks
				}

				for(uint8_t x=0; x<100; x++){
					uint8_t pos=plot_Index+x;
					if(pos>99) pos-=100;

					uint16_t plotV = plotData[pos];

					if (plotV<180) plotV = 0;
					else plotV = (plotV-180) >> 3; 					      // divide by 8, (500-180)/8=40
					u8g2_DrawVLine(&u8g2, x+13, 56-plotV, plotV);	// data points
				}
				if(t<188){ set = 1; }
				else {
					set=(t-180)>>3;
				}
				set= 56-set;
			}

			u8g2_DrawTriangle(&u8g2, 122, set-4, 122, set+4, 115, set);		// set temp marker
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
  editable_widget_t* edit;

	//iron tip temperature display
	w=&Widget_IronTemp;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_display, &display_IronTemp);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=5;
	dis->textAlign=align_center;
	dis->dispAlign=align_center;
	dis->font=u8g2_font_iron;
	w->posY = 17;
	dis->getData = &main_screen_getIronTemp;

	// Tip temperature setpoint
	w=&Widget_SetPoint;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_editable, &editable_SetPoint);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=5;
	w->posY = Widget_IronTemp.posY-2;
	dis->getData = &getTemp;
	dis->dispAlign=align_center;
	dis->textAlign=align_center;
	dis->font=display_IronTemp.font;
	edit->selectable.tab = 1;
	edit->setData = (void (*)(void *))&setTemp;
	w->frameType=frame_solid;
	w->radius=0;
	w->enabled=0;

	#ifdef USE_VIN
	//V input display
	w = &Widget_Vsupply;
	screen_addWidget(w, scr);
	widgetDefaultsInit(w, widget_display, &display_Vsupply);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->endString="V";
	dis->reservedChars=5;
	dis->textAlign=align_center;
	dis->number_of_dec=1;
	dis->font=u8g2_font_labels;
	w->posX = voltXBM[0]+2;
	w->posY= 2;
	dis->getData = &main_screen_getVin;
	//w->width = 40;
	#endif

	#ifdef USE_NTC
	//Ambient temperature display
	w=&Widget_AmbTemp;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_display, &display_AmbTemp);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=7;
	dis->dispAlign=align_right;
	dis->textAlign=align_center;
	w->posY= 2;
	dis->number_of_dec=1;
	dis->font=u8g2_font_labels;
	dis->getData = &main_screen_getAmbTemp;
	#endif

	// Tips
	w=&Widget_TipSelect;
	screen_addWidget(w,scr);
	widgetDefaultsInit(w, widget_multi_option, &editable_TipSelect);
	dis=extractDisplayPartFromWidget(w);
	edit=extractEditablePartFromWidget(w);
	dis->reservedChars=TipCharSize-1;
	w->posY = 32;
	dis->dispAlign=align_center;
	dis->textAlign=align_center;
	edit->inputData.getData = &getTip;
	edit->inputData.number_of_dec = 0;
	edit->big_step = 0;
	edit->step = 0;
	edit->selectable.tab = 2;
	edit->setData = (void (*)(void *))&setTip;
	edit->options = tipName;
	w->enabled=0;
	w->frameType=frame_disabled;

	setMainWidget(&Widget_IronTemp);
	u8g2_SetFont(&u8g2,u8g2_font_mainBig);
	sleepWidth=u8g2_GetStrWidth(&u8g2, "SLEEP")+2;
	sleepHeigh= u8g2_GetMaxCharHeight(&u8g2)+3;
}

