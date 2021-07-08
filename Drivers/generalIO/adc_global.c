/*
 * adc_global.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose (PTDreamer), 2017
 */

#include "adc_global.h"
#include "buzzer.h"
#include "iron.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "board.h"


volatile adc_measures_t ADC_measures[ADC_BFSIZ];
volatile ADC_Status_t ADC_Status;

volatile ADCDataTypeDef_t TIP = {
    adc_buffer: &ADC_measures[0].TIP
};

#ifdef USE_VIN
volatile ADCDataTypeDef_t VIN = {
    adc_buffer: &ADC_measures[0].VIN
};
#endif

#ifdef USE_NTC
volatile ADCDataTypeDef_t NTC = {
    adc_buffer: &ADC_measures[0].NTC
};
#endif

#ifdef USE_VREF
volatile ADCDataTypeDef_t VREF = {
    adc_buffer: &ADC_measures[0].VREF
};
#endif

static ADC_HandleTypeDef *adc_device;



uint8_t ADC_Cal(void){
  return HAL_ADCEx_Calibration_Start(adc_device);
}


void ADC_Init(ADC_HandleTypeDef *adc){
  adc_device=adc;
  ADC_ChannelConfTypeDef sConfig = {0};

  #ifdef STM32F072xB
    adc_device->Instance->CHSELR &= ~(0x7FFFF);                                             // Disable all regular channels
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  #endif

  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
    adc_device->Init.NbrOfConversion = ADC_Num;
  #endif

  adc_device->Init.ExternalTrigConv = ADC_SOFTWARE_START;                                   // Set software trigger
  if (HAL_ADC_Init(adc_device) != HAL_OK) { Error_Handler(); }

  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;                                         // More sampling time to compensate high input impedances

  #ifdef ADC_CH_1ST
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_1;
    #endif
    sConfig.Channel = ADC_CH_1ST;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_2ND
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_2;
    #endif
    sConfig.Channel = ADC_CH_2ND;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_3RD
    #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      sConfig.Rank = ADC_REGULAR_RANK_3;
    #endif
    sConfig.Channel = ADC_CH_3RD;
    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_4TH
  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
    sConfig.Rank = ADC_REGULAR_RANK_4;
  #endif
  sConfig.Channel = ADC_CH_4TH;
  if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  if(ADC_Cal() != HAL_OK ){
    Error_Handler();
  }

  ADC_Status = ADC_Idle;
  buzzer_short_beep();
}

void ADC_Start_DMA(){

  if(ADC_Status!=ADC_Waiting){
    Error_Handler();
  }
  if(systemSettings.isSaving){                                                              // If saving, skip ADC conversion (PWM pin disabled)
    ADC_Status=ADC_Idle;
    HAL_IWDG_Refresh(&hiwdg);
    return;
  }
  #ifdef DEBUG_PWM
  PWM_DBG_GPIO_Port->BSRR=PWM_DBG_Pin;                                                    // Set TEST to 1
  #endif

  ADC_Status=ADC_Sampling;
  if(HAL_ADC_Start_DMA(adc_device, (uint32_t*)ADC_measures, sizeof(ADC_measures)/ sizeof(uint16_t) )!=HAL_OK){  // Start ADC conversion now
    Error_Handler();
  }
}


void ADC_Stop_DMA(void){
  HAL_ADC_Stop_DMA(adc_device);
}

/*
 * Some credits: https://kiritchatterjee.wordpress.com/2014/11/10/a-simple-digital-low-pass-filter-in-c/
 */
void DoAverage(volatile ADCDataTypeDef_t* InputData){
  volatile uint16_t *inputBuffer=InputData->adc_buffer;
  uint32_t adc_sum,avg_data;
  uint16_t max=0, min=0xffff;
  uint8_t shift;

  #ifdef DEBUG_PWM
  InputData->prev_avg=InputData->last_avg;
  InputData->prev_raw=InputData->last_raw;
  #endif

  // Make the average of the ADC buffer
  adc_sum = 0;
  for(uint16_t x = 0; x < ADC_BFSIZ; x++) {
    adc_sum += *inputBuffer;
    if(*inputBuffer > max){
      max = *inputBuffer;
    }
    if(*inputBuffer < min){
      min = *inputBuffer;
    }
    inputBuffer += ADC_Num;
  }
  //Remove highest and lowest values
  adc_sum -=  (min + max);

  // Calculate average
  avg_data = adc_sum / (ADC_BFSIZ -2) ;
  InputData->last_raw = avg_data;

  if(systemSettings.Profile.filterFactor > 0) {                                             // Advanced filtering enabled?

    if(systemSettings.Profile.filterFactor>8){                                              // Limit coefficient, more than 8 will cause overflow
      systemSettings.Profile.filterFactor=8;
    }
    shift = systemSettings.Profile.filterFactor;                                            // Set EMA factor setting from system settings

    // Fixed point shift
    uint32_t RawData = avg_data << 12;

    // Compute EMA of input
#define LIMIT_FILTERING

#ifdef LIMIT_FILTERING
#define SMOOTH_START  50       // Start difference to apply partial filtering override
#define SMOOTH_END    150      // Max difference to completely override filter
#define SMOOTH_DIFF  (SMOOTH_END-SMOOTH_START)
#if defined DEBUG_PWM && SWO_PRINT
    extern bool dbg_newData;
#endif

    int32_t diff = (int32_t)avg_data - (int32_t)(InputData->EMA_of_Input>>12);              // Check difference between stored EMA and last average
    int32_t abs_diff=abs(diff);

    if(abs_diff>SMOOTH_END){                                                                // If huge (Filtering will delay too much the response)
      InputData->EMA_of_Input = RawData;                                                    // Reset filter
      #if defined DEBUG_PWM && SWO_PRINT
      dbg_newData=1;                                                                        // Enable flag to debug the data
      #endif
    }
    else if(abs_diff>SMOOTH_START){                                                         // If medium, smoothen the difference
      InputData->EMA_of_Input += ((diff*(abs_diff-SMOOTH_START))/SMOOTH_DIFF)<<12;
      #if defined DEBUG_PWM && SWO_PRINT
      //dbg_newData=1;                                                                      // Meh, just some noise, not important
      #endif
    }
    else{
      InputData->EMA_of_Input = ( ((InputData->EMA_of_Input << shift) - InputData->EMA_of_Input) + RawData +(1<<(shift-1)))>>shift;
    }
#else
    InputData->EMA_of_Input = ( ((InputData->EMA_of_Input << shift) - InputData->EMA_of_Input) + RawData +(1<<(shift-1)))>>shift;
#endif
    InputData->last_avg = InputData->EMA_of_Input>>12;
  }
  else {
    InputData->last_avg=avg_data;
  }
}

uint16_t ADC_to_mV (uint16_t adc){
  /*
   * Instead running ( ADC*(3300/4095) ),
   * We previously multiply (3300/4095)*2^20 = 845006
   * Then we can use the fast hardware multiplier and
   * divide just with bit rotation.
   *
   * So it becomes Vadc = (ADC * 845006) >>20
   * Max possible input = 20 bit number, more will cause overflow to the 32 bit variable
   * Calculated to use  12 bit max input from ADC (4095)
   * Much, much faster than floats!
   */

  return( ((uint32_t)845006*adc)>>20 );
}


// Don't call this function, only the ADC ISR should use it.
void handle_ADC_Data(void){
  DoAverage(&TIP);
  #ifdef USE_VREF
  DoAverage(&VREF);
  #endif
  #ifdef USE_NTC
  DoAverage(&NTC);
  #endif
  #ifdef USE_VIN
  DoAverage(&VIN);
  #endif
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc){

#if defined DEBUG_PWM && SWO_PRINT
    extern bool dbg_newData;
    extern uint16_t dbg_prev_TIP_Raw, dbg_prev_TIP, dbg_prev_VIN, dbg_prev_PWR;
    extern int16_t dbg_prev_NTC;
    bool dbg_t=dbg_newData;
#endif

  if(_hadc == adc_device){
    if(ADC_Status!=ADC_Sampling){
      Error_Handler();
    }
    ADC_Stop_DMA();
    ADC_Status = ADC_Idle;

    #ifdef DEBUG_PWM
    PWM_DBG_GPIO_Port->BSRR=PWM_DBG_Pin<<16;                                                // Set TEST to 0
    #endif

    __HAL_TIM_SET_COUNTER(Iron.Pwm_Timer,0);                                                // Synchronize PWM

    if(!Iron.Error.safeMode && Iron.CurrentMode!=mode_sleep){
      configurePWMpin(output_PWM);
    }

    HAL_IWDG_Refresh(&hiwdg);
    handle_ADC_Data();

#if defined DEBUG_PWM && defined SWO_PRINT
    if(dbg_t!=dbg_newData){                                                                 // Save values before handleIron() updates them
      dbg_prev_TIP_Raw=last_TIP_Raw;                                                        // If filter was resetted, print values
      dbg_prev_TIP=last_TIP;
      dbg_prev_VIN=last_VIN;
      dbg_prev_NTC=last_NTC;
      dbg_prev_PWR=Iron.CurrentIronPower;
    }
#endif

    handleIron();
    runAwayCheck();
  }
}
