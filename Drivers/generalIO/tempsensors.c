    /*
 * tempsensors.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "tempsensors.h"
#include "calibration_screen.h"

#ifdef ENABLE_INT_TEMP
uint8_t use_int_temp;
#endif

static tipData_t *currentTipData;
volatile int16_t last_TIP_C, last_TIP_F, last_TIP_Raw_C, last_TIP_Raw_F, last_NTC_F, last_NTC_C;

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
  static uint8_t detected = 0;  
  if(new){
  
    #ifdef ENABLE_INT_TEMP
    if(systemSettings.settings.enableNTC){                                                              // If NTC enabled
      if(Iron.Error.Flags&(FLAG_ACTIVE | FLAG_NO_IRON)){                                                // If handle removed, always force external NTC mode
        use_int_temp = 0;
      }
      else if (!use_int_temp){																																					// Else, handle connected 
        detected |= !(systemSettings.settings.NTC_detect && 1);                                         // If NTC detection disabled, set detected flag to skip further checks
        if(detected && ((Iron.Error.Flags&(FLAG_ACTIVE | FLAG_NTC_HIGH | FLAG_NTC_LOW ))>FLAG_ACTIVE)){ // If NTC error active and NTC detection is done
          use_int_temp = 1;                                                                             // Use internal sensor
        }
      }
    }
    else{                                                                                               // If NTC disabled
      use_int_temp = 1;                                                                                 // Use internal sensor
    }

    if(!use_int_temp){                                                                                  // Compute external NTC temperature
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
        if(!(Iron.Error.Flags & FLAG_ACTIVE) && (current_time-error_timer>1000)){                       // If no errors for 1000mS (Stable reading), check value
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
int16_t readTipTemperatureCompensated(bool new, bool mode, bool tempUnit){
  int16_t temp, temp_Raw;
  if(systemSettings.setupMode==enable){
    return 0;
  }
  if(new){
    temp = (adc2Human_x10(TIP.last_avg,1,systemSettings.settings.tempUnit)+5)/10;
    temp_Raw = (adc2Human_x10(TIP.last_raw,1,systemSettings.settings.tempUnit)+5)/10;

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
    if(systemSettings.settings.tempUnit==mode_Celsius){
      last_TIP_C = temp;
      last_TIP_Raw_C = temp_Raw;
      last_TIP_F = TempConversion(last_TIP_C,mode_Farenheit,0);
      last_TIP_Raw_F = TempConversion(last_TIP_Raw_C,mode_Farenheit,0);
    }
    else{
      last_TIP_F = temp;
      last_TIP_Raw_F = temp_Raw;
      last_TIP_C = TempConversion(last_TIP_F,mode_Celsius,0);
      last_TIP_Raw_C = TempConversion(last_TIP_Raw_F,mode_Celsius,0);
    }
  }
  if(tempUnit==mode_Celsius){
    if(mode==read_unfiltered){
      return last_TIP_Raw_C;
    }
    else{
      return last_TIP_C;
    }
  }
  else{
    if(mode==read_unfiltered){
      return last_TIP_Raw_F;
    }
    else{
      return last_TIP_F;
    }
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
uint16_t human2adc(int16_t t) {
  t = t*10;
  // If using Farenheit, convert to Celsius
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    t = TempConversion(t,mode_Celsius,1);
  }
  int16_t temp = t;
  t -= last_NTC_C;

  if (t < 0){ return 0; }                                 // If requested temp below min, return 0
  if(t< state_temps[cal_250]){
    temp = map(t, 0, state_temps[cal_250], systemSettings.Profile.calADC_At_0, currentTipData->calADC_At_250);
  }
  else{
    temp = map(t, state_temps[cal_250], state_temps[cal_400], currentTipData->calADC_At_250, currentTipData->calADC_At_400);
  }

  int16_t tH = adc2Human_x10(temp,0,mode_Celsius);                // Find +0.5ºC to provide better reading stability
  if (tH < (t)) {
    while(tH < (t)){
      tH = adc2Human_x10(++temp,0,mode_Celsius);
    }
  }
  else if (tH > (t)) {
    while(tH > (t)){
      tH = adc2Human_x10(--temp,0,mode_Celsius);
    }
  }
  if(temp>4090 || temp<0){                                                // Safety check to avoid exceeding ADC range
    temp=0;
  }
  return temp;
}

// Translate temperature from internal units to the human readable value
int16_t adc2Human_x10(uint16_t adc_value,bool correction, bool tempUnit) {
  int16_t temp;
  if(adc_value<currentTipData->calADC_At_250){
    temp = map(adc_value, systemSettings.Profile.calADC_At_0, currentTipData->calADC_At_250, 0, state_temps[cal_250]);
  }
  else{
    temp = map(adc_value, currentTipData->calADC_At_250, currentTipData->calADC_At_400, state_temps[cal_250], state_temps[cal_400]);
  }
  if(correction){ temp += last_NTC_C; }
  if(tempUnit==mode_Farenheit){
    temp=TempConversion(temp,mode_Farenheit,1);
  }
  return temp;
}

int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  uint32_t ret;
  ret = (((x - in_min) * (out_max - out_min)) + ((in_max - in_min)/2)) / (in_max - in_min) + out_min;
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
