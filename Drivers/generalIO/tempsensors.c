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
volatile int16_t last_TIP_C, last_TIP_F, last_TIP_F_Raw, last_TIP_C_Raw;
#ifdef USE_NTC
volatile int16_t last_NTC_C=-999, last_NTC_F=-999;
#else
volatile int16_t last_NTC_C=350, last_NTC_F=950;
#endif

#ifdef USE_NTC

#define _DETECTED           0x80      // Detected flag
#define _FIRST              0x10      // First pass flag
#define _USE_EXTERNAL       0x00      // For normal NTC: Use single NTC values, NTC autodetect mode: Use lower NTC values
#define _USE_EXTERNAL_HIGH  0x01      // For NTC autodetect mode: Use higher NTC values
#define _USE_INTERNAL       0x02      // Use internal sensor flag

static uint8_t ntc_status;

void detectNTC(void){
  ntc_status=_USE_EXTERNAL;           // Force NTC detection
}

#endif

int16_t readColdJunctionSensorTemp_x10(bool new, bool tempUnit){
#ifdef USE_NTC

  uint8_t error = (Iron.Error.Flags&(FLAG_ACTIVE | FLAG_NO_IRON))==(FLAG_ACTIVE | FLAG_NO_IRON);

  if(new){
    do{                                                                                                 // Detection loop
      if(systemSettings.Profile.ntc.enabled){                                                            // If NTC enabled
        if(error){                                                                                      // If errors (handle removed)
          ntc_status = _USE_EXTERNAL;                                                                   // Force external NTC mode, clear detected and first pass flag
        }
        else if (!(ntc_status&_DETECTED)){                                                              // If no errors and not detected yet
          if(!(ntc_status&_FIRST)){                                                                     // If first pass not done
            ntc_status=_FIRST;                                                                          // Set first pass flag, wait for next call to get new readings
          }
          else if(last_NTC_C<-100 || last_NTC_C>900){                                                   // If temp <-10.0ºC or >90.0ºC
            if(systemSettings.Profile.ntc.detection){                                                     // If NTC detection enabled
              if(ntc_status&_USE_EXTERNAL_HIGH){                                                        // If already trying higher values
                ntc_status = (_DETECTED | _USE_INTERNAL);                                               // NTC invalid, use internal sensor
              }
              else{                                                                                     // This reading was with lower NTC values
                //ntc_status |= _USE_EXTERNAL_HIGH;                                                       // Now try with higher ones
                ntc_status |= _DETECTED;                                                                    // Set detected bit (NTC External/External_high bit remains unchanged)
              }
            }
            else{                                                                                       // If NTC detection disabled, use internal sensor
              //ntc_status = (_DETECTED | _USE_INTERNAL);
              ntc_status |= _DETECTED;                                                                    // Set detected bit (NTC External/External_high bit remains unchanged)
            }
          }
          else{                                                                                         // If temp valid
            ntc_status |= _DETECTED;                                                                    // Set detected bit (NTC External/External_high bit remains unchanged)
            break;
          }
        }
      }
      else{                                                                                             // If NTC disabled
        ntc_status = (_DETECTED | _USE_INTERNAL);                                                       // Use internal sensor
      }

      if(!(ntc_status&_USE_INTERNAL)){                                                                  // Compute external NTC temperature (if internal sensor not selected)
        float NTC_res;
        float pull_res=systemSettings.Profile.ntc.pull_res*100;
        float NTC_Beta;
        float adcValue=NTC.last_avg;
        float result;

        if(systemSettings.Profile.ntc.detection){                                                       // NTC Autodetect enabled?
          if(ntc_status & _USE_EXTERNAL_HIGH){
            NTC_res = systemSettings.Profile.ntc.high_NTC_res*100;                                      // Second stage, use higher NTC values
            NTC_Beta = systemSettings.Profile.ntc.high_NTC_beta;
          }
          else{
            NTC_res = systemSettings.Profile.ntc.low_NTC_res*100;                                       // First stage, use lower NTC values
            NTC_Beta = systemSettings.Profile.ntc.low_NTC_beta;
          }
        }
        else{                                                                                           // NTC Autodetect disabled, use normal values
          NTC_res = systemSettings.Profile.ntc.NTC_res*100;
          NTC_Beta = systemSettings.Profile.ntc.NTC_beta;
        }

        if(systemSettings.Profile.ntc.pullup){
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
        if(last_NTC_C < -200){                                                                          // If temperature <-20.0ºC, assume it's wrong
          last_NTC_C = -999;
          last_NTC_F = -999;
        }
        else if(last_NTC_C > 900){                                                                      // If temperature >90.0ºC, also assume it's wrong
          last_NTC_C = 999;
          last_NTC_F = 1999;
        }
        else{
          last_NTC_F = TempConversion(result, mode_Farenheit, 1);
        }
      }
      #ifdef ENABLE_INT_TEMP
      else{                                                                                             // Compute internal temperature if int temp enabled
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
      else{                                                                                             // If internal temperature disabled in options, use always 35.0ºC
        last_NTC_C = 350;
        last_NTC_F = 950;
      }
      #endif
    }while(!error && !(ntc_status&_DETECTED));                                                          // Repeat if no errors and not detected yet, keep trying the next options until finding a valid one
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
      last_TIP_C_Raw = temp_Raw;
      last_TIP_F = TempConversion(last_TIP_C,mode_Farenheit,0);
      last_TIP_F_Raw = TempConversion(last_TIP_C_Raw,mode_Farenheit,0);
    }
    else{
      last_TIP_F = temp;
      last_TIP_F_Raw = temp_Raw;
      last_TIP_C = TempConversion(last_TIP_F,mode_Celsius,0);
      last_TIP_C_Raw = TempConversion(last_TIP_F_Raw,mode_Celsius,0);
    }
  }
  if(tempUnit==mode_Celsius){
    if(mode==read_unfiltered){
      return last_TIP_C_Raw;
    }
    else{
      return last_TIP_C;
    }
  }
  else{
    if(mode==read_unfiltered){
      return last_TIP_F_Raw;
    }
    else{
      return last_TIP_F;
    }
  }
}

void setCurrentTip(uint8_t tip) {
  systemSettings.Profile.currentTip = tip;
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

// Absolute temperature conversion
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

// Relative temperature conversion (ex. increment or difference)
int16_t TempIncrementConversion(int16_t temperature, bool conversion){
  if(conversion==mode_Farenheit){  // Input==Celsius, Output==Farenheit
    temperature=(((int32_t)temperature*117965)>>16);// F = (C*1.8)+*9/5
  }
  else{// Input==Farenheit, Output==Celsius
    temperature=((int32_t)temperature*36409)>>16;// C = (F-32)/1.8
  }
  return temperature;
}
