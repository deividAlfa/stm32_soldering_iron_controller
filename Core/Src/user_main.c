/*
 * addon_fume_extractor.c
 *
 *  Created on: Apr 22, 2022
 *      Author: KocsisV
 */

#include "main.h"
#include "user_main.h"

#include "iron.h"
#include "pid.h"
#include "settings.h"
#include "adc_global.h"
#include "buzzer.h"
#include "rotary_encoder.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "display.h"
#include "gui.h"
#include "screen.h"
#include "myTest.h"
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
#include "addon_fume_extractor.h"
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
#include "addon_switch_off_reminder.h"
#endif

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "user_main.c"
#endif

extern ADC_HandleTypeDef ADC_DEVICE;
extern DMA_HandleTypeDef FILL_DMA;
extern TIM_HandleTypeDef PWM_TIMER;
extern TIM_HandleTypeDef READ_TIMER;

#if defined(DISPLAY_SPI) && defined(DISPLAY_DEVICE)
extern SPI_HandleTypeDef DISPLAY_DEVICE;
#endif
#if defined(DISPLAY_I2C) && defined(DISPLAY_DEVICE)
#error fix the code, copy i2c device handle type here
extern I2C??? DISPLAY_DEVICE;
#endif

#if defined DEBUG_PWM && defined SWO_PRINT
volatile uint16_t dbg_prev_TIP_Raw, dbg_prev_TIP, dbg_prev_VIN, dbg_prev_PWR;
volatile int16_t dbg_prev_NTC;
volatile bool dbg_newData;

int _write(int32_t file, uint8_t *ptr, int32_t len)
{
  for (int i = 0; i < len; i++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
#endif

#ifdef DEBUG_ALLOC
struct mallinfo mi;
uint32_t max_allocated;
#endif

// Allocate max possible ram, then release it. This fill the heap pool and avoids internal fragmentation due (ST's?) poor malloc implementation.
void malloc_fragmentation_fix(void){
  uint32_t *ptr = NULL;
  uint32_t try=20480; // Current ram usage is ~5KB, so we should have have another 5/15KB free, depending on the stm32 used.
  while(!ptr && try){
    ptr = _malloc(try);
    try-=16;
  }
  _free(ptr);
  #ifdef DEBUG_ALLOC
  max_allocated=0; //Clear max allocated result
  #endif
}

void initBeforeMCUConfiguration(void)
{
  malloc_fragmentation_fix();

  #if defined DEBUG && !defined STM32F072xB
    DebugOpts();          // Enable debug options in Debug build
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable DWT
    DWT->CYCCNT = 0;                                // Clear counter
    DWT->CTRL = DWT_CTRL_CYCCNTENA_Msk;             // Enable counter
    // Now CPU cycles can be read in DWT->CYCCNT;
  #endif
}

void InitAfterMCUConfiguration(void){
  configurePWMpin(output_Low);

  // Wait 500ms for voltage to stabilize? (Before calibrating ADC)
  for(uint32_t t=HAL_GetTick(); (HAL_GetTick()-t)<500u; ){
    HAL_IWDG_Refresh(&hiwdg);
  }

#ifdef HAS_BATTERY
 RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN; // power the BKP peripheral
 PWR->CR      |= PWR_CR_DBP;                            // enable access to the BKP registers
 BKP->CR       = 0u;                                    // disable tamper pin, just to be sure
#endif

#if (defined DISPLAY_SPI || defined DISPLAY_I2C) && defined DISPLAY_DEVICE
  lcd_init(&DISPLAY_DEVICE, &FILL_DMA);
#elif defined DISPLAY_SPI || defined DISPLAY_I2C
  lcd_init(&FILL_DMA);
#endif

    guiInit();
    ADC_Init(&ADC_DEVICE);
    buzzer_init();
    restoreSettings();
    setDisplayContrastOrBrightness(systemSettings.settings.contrastOrBrightness);
    setDisplayXflip(systemSettings.settings.displayXflip);
    setDisplayYflip(systemSettings.settings.displayYflip);
#ifdef ST7565
    setDisplayResRatio(systemSettings.settings.displayResRatio);
#endif
    ironInit(&READ_TIMER, &PWM_TIMER,PWM_CHANNEL);
    RE_Init((RE_State_t *)&RE1_Data, ENC_L_GPIO_Port, ENC_L_Pin, ENC_R_GPIO_Port, ENC_R_Pin, ENC_SW_GPIO_Port, ENC_SW_Pin);
    oled_init(&RE_Get,&RE1_Data);
#ifdef HAS_BATTERY
    restoreLastSessionSettings();
#endif
}

void mainCycle(void)
{
#if defined DEBUG_PWM && defined SWO_PRINT
  if(dbg_newData){
    dbg_newData=0;

    printf("     LAST  VAL    RAW   VAL      PREV  VAL    RAW   VAL\n"
           "TIP: %4u  %3u\260C  %4u  %3u\260C    %4u  %3u\260C  %4u  %3u\260C \n"
           "PID: %3u%%                        %3u%%\n\n",
          TIP.last_avg, last_TIP, TIP.last_raw, last_TIP_Raw, TIP.prev_avg, dbg_prev_TIP, TIP.prev_raw, dbg_prev_TIP_Raw,
          Iron.CurrentIronPower, dbg_prev_PWR);

  }
#endif

  checkSettings(); // Check if settings were modified
  oled_handle();   // Handle oled drawing
}

// Called from SysTick IRQ every 1mS
void Program_Handler(void) {
  handle_buzzer();                                                    // Handle buzzer state
  RE_Process(&RE1_Data);                                              // Handle Encoder
#ifdef ENABLE_ADDON_FUME_EXTRACTOR
  handleAddonFumeExtractor();
#endif
#ifdef ENABLE_ADDON_SWITCH_OFF_REMINDER
  handleAddonSwitchOffReminder();
#endif
  if(systemSettings.Profile.WakeInputMode!=mode_stand){
    readWake();
  }
}

/*
 *  Timer working in load preload mode! The value loaded now is loaded on next event. That's why the values are reversed!
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *_htim){
  TIM_HandleTypeDef* const ironReadTimer = getIronReadTimer();
  if(_htim == ironReadTimer){
    __HAL_TIM_CLEAR_FLAG(ironReadTimer,TIM_FLAG_UPDATE);

    if(ADC_Status==ADC_Idle){
      __HAL_TIM_SET_AUTORELOAD(ironReadTimer,systemSettings.Profile.readPeriod-(systemSettings.Profile.readDelay+1)); // load (period-delay) time

      if(systemSettings.settings.activeDetection && !getIronErrorFlags().safeMode){
        configurePWMpin(output_High);                                                   // Force PWM high for a few uS (typically 5-10uS)
        while(__HAL_TIM_GET_COUNTER(ironReadTimer)<(TIP_DETECT_TIME/5));
      }
      configurePWMpin(output_Low);                                                      // Force PWM low
      ADC_Status = ADC_Waiting;
    }
    else if(ADC_Status==ADC_Waiting){
      __HAL_TIM_SET_AUTORELOAD(ironReadTimer,systemSettings.Profile.readDelay);       // Load Delay time
      ADC_Start_DMA();
    }
  }
}

void CrashErrorHandler(char * file, int line)
{
#ifdef DEBUG_ERROR

  Oled_error_init();
  
  char strOut[16];
  uint8_t outPos=0;
  uint8_t inPos=0;
  uint8_t ypos=16;

  sprintf(strOut,"Line %u",line);
  u8g2_DrawStr(&u8g2, 0, 0, strOut);

  while(1){                                                  // Divide string in chuncks that fit teh screen width
    strOut[outPos] = file[inPos];                            // Copy char
    strOut[outPos+1] = 0;                                    // Set out string null terminator
    uint8_t currentWidth = u8g2_GetStrWidth(&u8g2, strOut);  // Get width
    if(currentWidth<displayWidth){                              // If less than oled width, increase input string pos
      inPos++;
    }
    if( (currentWidth>displayWidth) || (strOut[outPos]==0) ){  // If width bigger than oled width or current char null(We reached end of input string)
      char current = strOut[outPos];                        // Store current char
      strOut[outPos]=0;                                     // Set current out char to null
      u8g2_DrawStr(&u8g2, 0, ypos, strOut);                 // Draw string
      if(current==0){                                       // If current is null, we reached end
        break;                                              // Break
      }
      outPos=0;                                             // Else, reset output position
      ypos += 12;                                           // Increase Y position and continue copying the input string
    }
    else{
      outPos++;                                              // Output buffer not filled yet, increase position
    }
  };

  #if (defined DISPLAY_I2C || defined DISPLAY_SPI) && defined DISPLAY_DEVICE

  #ifdef I2C_TRY_HW
  if(oled.use_sw){
    update_display();
  }
  else{
    update_display_ErrorHandler();
  }
  #else
  update_display_ErrorHandler();
  #endif

  #else
  update_display();
  #endif
  buttonReset();
#endif
  while(1){
  }
}
