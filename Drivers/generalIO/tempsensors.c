    /*
 * tempsensors.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "tempsensors.h"
#include "math.h"

#ifdef ENABLE_INT_TEMP
uint8_t use_int_temp;
#endif

static tipData_t *currentTipData;
int16_t last_TIP_Raw;
int16_t last_TIP;
int16_t last_NTC_F;
int16_t last_NTC_C;

#ifdef USE_NTC
static uint32_t detect_error_timer=1000;
static uint8_t detected=0;

void detectNTC(void){
  #ifdef ENABLE_INT_TEMP
  use_int_temp=0;
  #endif
  detected=0;
  detect_error_timer = HAL_GetTick();
}
#endif

int16_t readColdJunctionSensorTemp_x10(bool new, bool tempUnit){
#ifdef USE_NTC
  static uint32_t error_timer=0;
  static uint8_t detected=0;
  if(new){

    #ifdef ENABLE_INT_TEMP
    if(systemSettings.settings.enableNTC){                               // If NTC enabled
      if(Iron.Error.active && Iron.Error.noIron){                        // If handle removed, always force external NTC mode
        use_int_temp = 0;
      }
      else if (!use_int_temp){
        if( (Iron.Error.active && (Iron.Error.NTC_high || Iron.Error.NTC_low))  &&                      // If handle connected, but NTC error is still active
            (!systemSettings.settings.NTC_detect || (systemSettings.settings.NTC_detect && detected))){ // and NTC autodetection is done or disabled

          use_int_temp = 1;                                                                             // Use internal sensor
        }
      }
    }
    else{                                                                                               // If NTC disabled
      use_int_temp = 1;                                                                                 // Use internal sensor
    }

    if(!use_int_temp){                                                                                  // Compute NTC temperature
    #else
    if(systemSettings.settings.enableNTC){
    #endif

      uint32_t current_time = HAL_GetTick();
      float NTC_res;
      float pull_res=systemSettings.settings.Pull_res*100;
      float NTC_Beta;
      float adcValue=NTC.last_avg;
      float result;

      if(systemSettings.settings.NTC_detect){                                                           // NTC Autodetect enabled?
        NTC_res = systemSettings.settings.NTC_detect_low_res*100;                                       // Set lower by default
        NTC_Beta = systemSettings.settings.NTC_detect_low_res_beta;
        if(!(Iron.Error.Flags & _ACTIVE) && (current_time-error_timer>1000)){                           // If no errors for 1000mS (Stable reading), check value
          if(!detected){                                                                                // If not done detection yet (Only detect once after error is gone)
            detected=1;                                                                                 // Set detected flag
            if(last_NTC_C<0){                                                                           // If temp negative, set higher res
              NTC_res = systemSettings.settings.NTC_detect_high_res*100;
              NTC_Beta = systemSettings.settings.NTC_detect_high_res_beta;
            }
          }
        }
        else{
          error_timer = current_time;                                                                   // If error, refresh timer
        }
      }
      else{
        NTC_res = systemSettings.settings.NTC_res*100;
        NTC_Beta = systemSettings.settings.NTC_Beta;
      }

      if(systemSettings.settings.Pullup){
        if(adcValue > 4094){
          result = (float)-99.9;
        }
        else if(adcValue == 0){
          result = (float)99.9;
        }
        else{
          result = (1/((log(((pull_res * adcValue) / (4095.0 - adcValue))/NTC_res)/NTC_Beta) + (1 / (273.15 + 25.000)))) - 273.15;
        }
      }
      else{
        if(adcValue > 4094){
          result = (float)99.9;
        }
        else if(adcValue == 0){
          result = (float)-99.9;
        }
        else{
          result = (1/((log(((pull_res * (4095.0 - adcValue)) / adcValue)/NTC_res)/NTC_Beta) + (1 / (273.15 + 25.000)))) - 273.15;
        }
      }
      result*=10;
      last_NTC_C = result;
      if(last_NTC_C < -200){
        last_NTC_C = -999;
        last_NTC_F = -999;
      }
      else{
        last_NTC_F = TempConversion(result, mode_Farenheit, 1);
      }
    }
    #ifdef ENABLE_INT_TEMP
    else{                                                                                               // Compute internal temperature
      #if defined STM32F101xB || defined STM32F102xB || defined STM32F103xB
      last_NTC_C = (((1.43f-(INT_TMP.last_avg*3.3f/4096))/0.00439f)+25)*10;
      #else
      float adcCalValue30 = *((uint16_t*)((uint32_t)0x1FFF7A2E));
      float adcCalValue110 = *((uint16_t*)((uint32_t)0x1FFF7A2C));
      last_NTC_C = (((110-30)*(INT_TMP.last_avg-adcCalValue30)/(adcCalValue110-adcCalValue30))+30)*10;
      #endif
      last_NTC_F = TempConversion(last_NTC_C, mode_Farenheit, 1);
    }
    #else
    else{                                                                                              // NTC disabled in options, use always 35.0ºC
      last_NTC_C = 350;
      last_NTC_F = 950;
    }
    #endif
  }
#else
  if(new){                                                                                              // NTC disabled in build options, use always 35.0ºC
    last_NTC_C = 350;
    last_NTC_F = 950;
  }
#endif

  if(tempUnit==mode_Celsius){
    return last_NTC_C;
  }
  else{
    return last_NTC_F;
  }
}

// Read tip temperature
int16_t readTipTemperatureCompensated(bool new, bool mode){
  int16_t temp, temp_Raw;
  if(systemSettings.setupMode==enable){
    return 0;
  }
  if(new){
    temp = adc2Human_x10(TIP.last_avg,1,systemSettings.settings.tempUnit)/10;
    temp_Raw = adc2Human_x10(TIP.last_raw,1,systemSettings.settings.tempUnit)/10;

    // Limit output values
    if(temp>999){
      temp=999;
    }
    else if(temp<-99){
      temp = -99;
    }
    if(temp_Raw>999){
      temp_Raw=999;
    }
    else if(temp_Raw<0){
      temp_Raw = 0;
    }
    last_TIP = temp;
    last_TIP_Raw = temp_Raw;
  }
  if(mode==read_unfiltered){
    return last_TIP_Raw;
  }
  else{
    return last_TIP;
  }
}

void setCurrentTip(uint8_t tip) {
  currentTipData = &systemSettings.Profile.tip[tip];
  setupPID(&currentTipData->PID);
}

tipData_t *getCurrentTip() {
  return currentTipData;
}

// Translate the human readable t into internal value
int16_t human2adc(int16_t t) {
  t = t*10;
  // If using Farenheit, convert to Celsius
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    t = TempConversion(t,mode_Celsius,1);
  }
  int16_t temp = t;
  t -= readColdJunctionSensorTemp_x10(old_reading, mode_Celsius);

  if (t < 0){ return 0; }                                 // If requested temp below min, return 0

  // If t>350, map between ADC values Cal_350 - Cal_450
  if (t >= 3500){
    temp = map(t, 3500, 4500, currentTipData->calADC_At_350, currentTipData->calADC_At_450);
  }
  // Else, map between ADC values Cal_250 - Cal_350
  else{
     temp = map(t, 2500, 3500, currentTipData->calADC_At_250, currentTipData->calADC_At_350);
  }

  int16_t tH = adc2Human_x10(temp,0,mode_Celsius);                // Find +0.5ºC to provide better reading stability
  if (tH < (t+4)) {
    while(tH < (t+4)){
      tH = adc2Human_x10(++temp,0,mode_Celsius);
    }
  }
  else if (tH > (t+6)) {
    while(tH > (t+6)){
      tH = adc2Human_x10(--temp,0,mode_Celsius);
    }
  }
  if(temp>4090){                                                // Safety check to avoid exceeding ADC range
    temp=0;
  }
  return temp;
}

// Translate temperature from internal units to the human readable value
int16_t adc2Human_x10(uint16_t adc_value,bool correction, bool tempUnit) {
  int16_t temp;
  int16_t ambTemp = readColdJunctionSensorTemp_x10(old_reading, mode_Celsius);
  if (adc_value >= currentTipData->calADC_At_350) {
    temp = map(adc_value, currentTipData->calADC_At_350, currentTipData->calADC_At_450, 3500, 4500);
  }
  else{
    temp = map(adc_value, currentTipData->calADC_At_250, currentTipData->calADC_At_350, 2500, 3500);
  }
  if(correction){ temp += ambTemp; }
  if(tempUnit==mode_Farenheit){
    temp=TempConversion(temp,mode_Farenheit,1);
  }
  return temp;
}

int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  uint32_t ret;
  ret = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (ret < 0)
    ret = 0;
  return ret;
}

// Fixed point calculation
// So (temp*117965)>>10 == temp*1.8 (Real: 1,800003052)
// (temp*36409)>>10 == temp/1.8 (Real: 1,799994507)
// Max input: 110.000°F, 35.000°C (risk of uint32_t overflow)
int16_t TempConversion(int16_t temperature, bool conversion, bool x10mode){
  if(conversion==mode_Farenheit){  // Input==Celsius, Output==Farenheit
    temperature=(((int32_t)temperature*117965)>>16);// F = (C*1.8)+32
    if(x10mode){
      temperature += 320;
    }
    else{
      temperature += 32;
    }
  }
  else{// Input==Farenheit, Output==Celsius
    if(x10mode){
      temperature -= 320;
    }
    else{
      temperature -= 32;
    }
    temperature=((int32_t)temperature*36409)>>16;// C = (F-32)/1.8
  }
  return temperature;
}
