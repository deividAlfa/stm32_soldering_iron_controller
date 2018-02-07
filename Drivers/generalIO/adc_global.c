/*
 * adc_global.c
 *
 *  Created on: Jul 27, 2017
 *      Author: jose
 */

#include "adc_global.h"
#define ADC_SAMPLE_TIME ADC_SAMPLETIME_239CYCLES_5
volatile adc_measures_t adc_measures[10];
volatile iron_temp_measure_state_t iron_temp_measure_state = iron_temp_measure_idle;
volatile uint16_t iron_temp_adc_avg = 0;

/* ADC1 init function */
void MX_ADC1_Init(ADC_HandleTypeDef *hadc1)
{

  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

    /**Common config
    */
  hadc1->Instance = ADC1;
  hadc1->Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1->Init.ContinuousConvMode = ENABLE;
  hadc1->Init.DiscontinuousConvMode = DISABLE;
  hadc1->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1->Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode
    */
  multimode.Mode = ADC_DUALMODE_REGSIMULT;
  if (HAL_ADCEx_MultiModeConfigChannel(hadc1, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLE_TIME;
  if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* ADC2 init function */
void MX_ADC2_Init(ADC_HandleTypeDef * hadc2)
{
  ADC_ChannelConfTypeDef sConfig;

    /**Common config
    */
  hadc2->Instance = ADC2;
  hadc2->Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc2->Init.ContinuousConvMode = ENABLE;
  hadc2->Init.DiscontinuousConvMode = DISABLE;
  hadc2->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2->Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(hadc2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLE_TIME;
  if (HAL_ADC_ConfigChannel(hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel
    */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLE_TIME;
  if (HAL_ADC_ConfigChannel(hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
