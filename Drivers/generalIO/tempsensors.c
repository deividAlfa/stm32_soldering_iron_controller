    /*
 * tempsensors.c
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#include "tempsensors.h"
#include "math.h"
#define temp_minC  0                 // Minimum system temperature in degrees of Celsius
#define temp_maxC  480                // Maximum calibration temperature in degrees of Celsius
static tipData_t *currentTipData;
static uint32_t detect_error_timer=1000;
static uint8_t detected=0;
int16_t last_TIP_Raw;
int16_t last_TIP;
int16_t last_NTC_F;
int16_t last_NTC_C;

void detectNTC(void){
  detected=0;
  detect_error_timer = HAL_GetTick();
}

int16_t readColdJunctionSensorTemp_x10(bool update, bool tempUnit){
  static uint32_t error_timer=0;
  static uint8_t detected=0;
#ifdef USE_NTC
  if(update){
    uint8_t error = (Iron.Error.Flags & _ACTIVE);
    uint32_t current_time = HAL_GetTick();
    float NTC_res;
    float pull_res=systemSettings.settings.Pull_res*100;
    float NTC_Beta;
    float adcValue=NTC.last_avg;
    float result;

    if(systemSettings.settings.NTC_detect){                         // NTC Autodetect enabled?
      NTC_res = systemSettings.settings.NTC_detect_low_res*100;     // Set lower by default
      NTC_Beta = systemSettings.settings.NTC_detect_low_res_beta;
      if(!error && (current_time-error_timer>1000)){                // If no errors for 1000mS (Stable reading), check value
        if(!detected){                                              // If not done detection yet (Only detect once after error is gone)
          detected=1;                                               // Set detected flag
          if(last_NTC_C<0){                                         // If temp negative, set higher res
            NTC_res = systemSettings.settings.NTC_detect_high_res*100;
            NTC_Beta = systemSettings.settings.NTC_detect_high_res_beta;
          }
        }
      }
      else{
        error_timer = current_time;                                 // If error, refresh timer
      }
    }
    else{
      NTC_res = systemSettings.settings.NTC_res*100;
      NTC_Beta = systemSettings.settings.NTC_Beta;
    }

    if(systemSettings.settings.Pullup){
      if(adcValue == 4095) return 999;
      result = (1/((log(((pull_res * adcValue) / (4095.0 - adcValue))/NTC_res)/NTC_Beta) + (1 / (273.15 + 25.000)))) - 273.15;
    }
    else{
      if(adcValue == 0) return -999;
      result = (1/((log(((pull_res * (4095.0 - adcValue)) / adcValue)/NTC_res)/NTC_Beta) + (1 / (273.15 + 25.000)))) - 273.15;
    }
    result*=10;
    last_NTC_C = result;
    last_NTC_F = TempConversion(result, mode_Farenheit, 1);
  }
#else
  if(update){
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
int16_t readTipTemperatureCompensated(bool update, bool ReadRaw){
  int16_t temp, temp_Raw;
  if(systemSettings.setupMode==enable){
    return 0;
  }
  if(update){
    temp = adc2Human(TIP.last_avg,1,systemSettings.settings.tempUnit);
    temp_Raw = adc2Human(TIP.last_raw,1,systemSettings.settings.tempUnit);

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
  if(ReadRaw){
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
uint16_t human2adc(int16_t t) {
  volatile int16_t temp = t;
  volatile int16_t tH;
  volatile int16_t ambTemp = readColdJunctionSensorTemp_x10(stored_reading, mode_Celsius) / 10;

  // If using Farenheit, convert to Celsius
  if(systemSettings.settings.tempUnit==mode_Farenheit){
    t = TempConversion(t,mode_Celsius,0);
  }
  t-=ambTemp;
  if (t < temp_minC){ return 0; }                                 // If requested temp below min, return 0
  else if (t > temp_maxC){ t = temp_maxC; }                       // If requested over max, apply limit

  // If t>350, map between ADC values Cal_350 - Cal_450
  if (t >= 350){
    temp = map(t, 350, 450, currentTipData->calADC_At_350, currentTipData->calADC_At_450);
  }
  // Else, map between ADC values Cal_250 - Cal_350
  else{
     temp = map(t, 250, 350, currentTipData->calADC_At_250, currentTipData->calADC_At_350);
  }
  tH = adc2Human(temp,0,mode_Celsius);
  if (tH < (t-1)) {
    while(tH < t){
      tH = adc2Human(++temp,0,mode_Celsius);
    }
  }
  else if (tH > (t+1)) {
    while(tH > t){
      tH = adc2Human(--temp,0,mode_Celsius);
    }
  }
  return temp;
}

// Translate temperature from internal units to the human readable value
int16_t adc2Human(uint16_t adc_value,bool correction, bool tempUnit) {
  int16_t tempH = 0;
  int16_t ambTemp = readColdJunctionSensorTemp_x10(stored_reading, mode_Celsius) / 10;
  if (adc_value >= currentTipData->calADC_At_350) {
    tempH = map(adc_value, currentTipData->calADC_At_350, currentTipData->calADC_At_450, 350, 450);
  }
  else{
    tempH = map(adc_value, currentTipData->calADC_At_250, currentTipData->calADC_At_350, 250, 350);
  }
  if(correction){ tempH+= ambTemp; }
  if(tempUnit==mode_Farenheit){
    tempH=TempConversion(tempH,mode_Farenheit,0);
  }
  return tempH;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  long ret;
  ret = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (ret < 0)
    ret = 0;
  return ret;
}

// Fixed point calculation
// 2E20*1.8 = 1887437 , 2E20/1.8 = 582542
// So (temp*1887437)>>20 == temp*1.8 (Real: 1,800000191)
// (temp*582542)>>20 == temp/1.8 (Real: 1,800000687)
// Max input = 1100°C / 3700°F, otherwise we will overflow the signed int32

int16_t TempConversion(int16_t temperature, bool conversion, bool x10mode){
  if(conversion==mode_Farenheit){  // Input==Celsius, Output==Farenheit
    temperature=(((int32_t)temperature*1887437)>>20);// F = (C*1.8)+32
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
    temperature=((int32_t)temperature*582542)>>20;// C = (F-32)/1.8
  }
  return temperature;
}
