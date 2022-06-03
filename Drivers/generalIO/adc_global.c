/*
 * adc_global.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "adc_global.h"
#include "buzzer.h"
#include "iron.h"
#include "tempsensors.h"
#include "voltagesensors.h"
#include "board.h"

#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "adc_global.c"
#endif


#define SELECTIVE_FILTERING

volatile adc_measures_t ADC_measures[ADC_BFSIZ];
volatile ADC_Status_t ADC_Status;

volatile ADCDataTypeDef_t TIP = {
    .adc_buffer = &ADC_measures[0].TIP,
};

#ifdef USE_VIN
volatile ADCDataTypeDef_t VIN = {
    .adc_buffer             = &ADC_measures[0].VIN,
    .filter.coefficient     = 95,
    .filter.threshold       = 20,
    .filter.reset_threshold = 60,
    .filter.count_limit     = 0,
    .filter.min             = 50,
    .filter.step            = 5,
};
#endif

#ifdef USE_NTC
volatile ADCDataTypeDef_t NTC = {
    .adc_buffer             = &ADC_measures[0].NTC,
    .filter.coefficient     = 95,
    .filter.threshold       = 20,
    .filter.reset_threshold = 60,
    .filter.count_limit     = 0,
    .filter.min             = 50,
    .filter.step            = 5,
};
#endif

#ifdef USE_VREF
volatile ADCDataTypeDef_t VREF = {
    .adc_buffer             = &ADC_measures[0].VREF,
    .filter.coefficient     = 95,
    .filter.threshold       = 20,
    .filter.reset_threshold = 60,
    .filter.count_limit     = 0,
    .filter.min             = 50,
    .filter.step            = 5,
};
#endif


#ifdef ENABLE_INT_TEMP
volatile ADCDataTypeDef_t INT_TMP = {
    .adc_buffer             = &ADC_measures[0].INT_TMP,
    .filter.coefficient     = 95,
    .filter.threshold       = 10,
    .filter.reset_threshold = 30,
    .filter.count_limit     = 0,
    .filter.min             = 50,
    .filter.step            = 5,
};
#endif
static ADC_HandleTypeDef *adc_device;
volatile uint8_t reset_measures;


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

  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;                                         // More sampling time to compensate high input impedances

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

    #if defined ENABLE_INT_TEMP && !defined ADC_CH_4TH
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;                                      // Last channel is internal temperature, requires min. 10uS sampling time
    #endif

    if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_4TH
  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
    sConfig.Rank = ADC_REGULAR_RANK_4;
  #endif
  sConfig.Channel = ADC_CH_4TH;

  #if defined ENABLE_INT_TEMP && !defined ADC_CH_5TH
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  #endif

  if (HAL_ADC_ConfigChannel(adc_device, &sConfig) != HAL_OK){Error_Handler();}
  #endif

  #ifdef ADC_CH_5TH
  #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
    sConfig.Rank = ADC_REGULAR_RANK_5;
  #endif
  sConfig.Channel = ADC_CH_5TH;

  #if defined ENABLE_INT_TEMP && !defined ADC_CH_6TH
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  #endif

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
/*
 // TEST CODE, DOESN'T WORK!
#ifdef ENABLE_INT_TEMP
  static uint8_t sampling_temp=0;
  static uint32_t last_temp_time=0;
  uint32_t tmp_sqr1 = 0U;

  if(sampling_temp){                                                                      // If last conversion sampled temperature
    sampling_temp=0;                                                                      // Disable In Temp channel
    tmp_sqr1 = ADC_SQR1_L_SHIFT(ADC_Num-1);                                               // Assume Int. temp is last channel, thus matching ADC_Num
    MODIFY_REG(adc_device->Instance->SQR1, ADC_SQR1_L, tmp_sqr1 );
  }
  else if(!last_temp_time || (HAL_GetTick()-last_temp_time)>999){                        // If never sampled or every 10 seconds (It's pretty slow)
    sampling_temp=1;                                                                      // Enable temp channel
    last_temp_time=HAL_GetTick();
    tmp_sqr1 = ADC_SQR1_L_SHIFT(ADC_Num);
    MODIFY_REG(adc_device->Instance->SQR1, ADC_SQR1_L, tmp_sqr1 );
  }
#endif
*/
  if(HAL_ADC_Start_DMA(adc_device, (uint32_t*)ADC_measures, sizeof(ADC_measures)/ sizeof(uint16_t) )!=HAL_OK){  // Start ADC conversion now
    Error_Handler();
  }
}


void ADC_Stop_DMA(void){
  HAL_ADC_Stop_DMA(adc_device);
}

void ADC_Reset_measures(void){
  reset_measures=1;                                                                           // Set the ADC flag to reset all averages
  while(reset_measures);                                                                      // Cleared after new ADC conversion
}

/*
 * Some credits: https://kiritchatterjee.wordpress.com/2014/11/10/a-simple-digital-low-pass-filter-in-c/
 */
void DoAverage(volatile ADCDataTypeDef_t* InputData){

  volatile uint16_t *inputBuffer=InputData->adc_buffer;
  int32_t adc_sum,avg_data;
  uint16_t max=0, min=0xffff;
  volatile filter_t *f = &InputData->filter;
  int8_t factor = f->coefficient;
  float k;

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
  avg_data = adc_sum  / (ADC_BFSIZ-2);
  InputData->last_raw = avg_data;

  // Reset measures active?
  if(reset_measures){
    InputData->last_avg = avg_data;
    InputData->EMA_of_Input = avg_data;
    return;
  }

#ifdef SELECTIVE_FILTERING

#if defined DEBUG_PWM && defined SWO_PRINT
  extern bool dbg_newData;
#endif

  if(factor>0 && factor<100){
    int32_t diff = avg_data - (int32_t)InputData->last_avg;
    int32_t abs_diff=abs(diff);

    if(abs_diff > f->reset_threshold){                                         // If diff greater reset_threshold, reset the filter
      factor = 0;
      f->counter = 0;
    }
    else if(abs_diff > f->threshold){                                         // diff greater than threshold
      if(f->counter < f->count_limit){                                        // If counter below limit, just increase it, use normal filter coefficient
        f->counter++;
      }
      else{                                                                   // Else,
        factor = factor + ((f->counter - f->count_limit)*f->step);            // compute new coefficient (Note adding because step is negative)
        if(factor > f->min){                                                  // if factor greater than limit
          f->counter++;                                                       // Keep decreasing
        }
        else{
          factor = f->min;                                                    // Else, use min
        }
      }
    }
    else{                                                                     // Below threshold limit, use normal coefficient
      f->counter = 0;
    }
  }

  if(factor==0 || factor==100){                                             // Factor 100 would use 100% of old value, never updating, so we force 0% if that happens (shouldn't)
    InputData->EMA_of_Input = avg_data;                                     // Use last average (No filtering)
  }
  else{
    k=((float)factor/100);
    InputData->EMA_of_Input = (InputData->EMA_of_Input*k) + ((float)avg_data*(1.0-k));
 }
#endif
  InputData->last_avg = InputData->EMA_of_Input+0.5f;
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
  #ifdef ENABLE_INT_TEMP
  DoAverage(&INT_TMP);
  #endif
  reset_measures = 0;
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* _hadc){

#if defined DEBUG_PWM && defined SWO_PRINT
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
    if(systemSettings.Profile.WakeInputMode==mode_stand){
      readWake();
    }

    __HAL_TIM_SET_COUNTER(getIronPwmTimer(),0);                                             // Synchronize PWM
    if((!getIronErrorFlags().safeMode) && (getCurrentMode() != mode_sleep) && getBootCompleteFlag()){
    #ifndef DISABLE_OUTPUT
      configurePWMpin(output_PWM);
    #endif
    }

    handle_ADC_Data();


#if defined DEBUG_PWM && defined SWO_PRINT
    if(dbg_t!=dbg_newData){                                                                 // Save values before handleIron() updates them
      dbg_prev_TIP_Raw=last_TIP_Raw;                                                        // If filter was resetted, print values
      dbg_prev_TIP=last_TIP;
      dbg_prev_VIN=last_VIN;
      dbg_prev_NTC=last_NTC_C;
      dbg_prev_PWR=Iron.CurrentIronPower;
    }
#endif

    handleIron();
    runAwayCheck();

    HAL_IWDG_Refresh(&hiwdg);
  }
}
