
#ifndef SETUP_H_
#define SETUP_H_

/*
 * Software adjustments
 *
 * Adjust all the delays in steps of 1mS
 *
 */

#define Adc_Buffer_Size 	16				// ADC DMA buffer size Buffer[ADC_Buffer_Size][Adc_Buffer_Elements]
#define Adc_Buffer_Elements 4				// Set the number of values of "adc_measures_t" , in "adc_global.h"
#define Adc_Measure_Delay	1				// Delay in mS after PWM stopped to start ADC Conversion
#define Process_Refresh_ms 	10				// Graphic system refresh interval (for processing button input, etc)
#define GUI_Update_ms 		200				// Graphic values refresh interval (for showing temperatures, etc)
#define PID_Refresh_ms		100				// PID calculation and ADC update interval

#define RollingBufferSize 	1				// Buffer size for storing rolling averages
											// 1= Takes average from ADC buffer but doesn't make use of rolling buffer
											// Leave like this, if enabled the no iron detection doesn't work

#define No_Iron_Delay		500				// Increase if it bounces between "NO IRON" and "iron temperature" screen when there is no iron plugged
#define CurrTemp_Save_Time	10000			// Time without temp setpoint changes to store the value in flash. Warning, too often will degrade the flash quickly!
											// Set to 0 for no saving. Always start in default temp.
/*
 * Hardware adjustments
 * Change these settings as configured in CubeMX to a
 */

#define SH1106_FIX							// For 1.3" OLED
#define PWM_TIMER 			htim17			// PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_1	// PWM Timer Channel
#define CHxN								// Using CHxN Output
//#define CHx								// Using CHx Output
#define ADC_DEVICE 			hadc			// ADC device
#define SPI_DEVICE 			hspi2			// SPI device

//#define Soft_SPI							// Uncomment to disable Hardware SPI with DMA and use software SPI

#define WAKE_input			HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin)
#define BUTTON_input		HAL_GPIO_ReadPin(ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_Pin)





#define _Error_Handler(__FILE__, __LINE__); Error_Handler();


#endif
