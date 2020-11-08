
#ifndef SETUP_H_
#define SETUP_H_
/*
 * Setup.h file
 *
 * The firmware will not build automatically. You have to set few things first:
 *  - Set the C
 */
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

#define No_Iron_Delay_mS		500				// In mS. Increase if it bounces between "NO IRON" and "iron temperature" screen when there is no iron plugged
#define CurrTemp_Save_Time_S	10				// Minimum time in secs for saving temperature setpoint changes to store the value in flash. Warning, too often will degrade the flash quickly!
											// Set to 0 for no saving. Always start in default temp.
/*
 * HARDWARE SETTINGS
 * If you change the timers, pwm channel, adc, spi devices,
 * you will have to set them here too for the rest of the program to adjust.
 *
 * Config settings for Quicko T12 STM32F072 and PTDreamer STM32F103 alrready included.
 * Uncomment the correct one
 *
 * After building the .ioc file, go to Project properties -> C/C++ Build -> Settings -> MCU GCC Compiler -> Preprocessor
 * And add one of these to the Define symbols
 * STM32F103xB		// For STM103
 * STM32F072xB		// For STM072
 *
 */

											// If your screen doesn't work, and want to discard a SPI problem
//#define Soft_SPI							// Uncomment to disable Hardware SPI with DMA and use software SPI




//#define QUICKO_T12_STM072
#define T12_STM103
//#define YOUR_CUSTOM_CONFIG



#ifdef QUICKO_T12_STM072
//-----------------------------
// For STM32F072 in Quicko T12
//-----------------------------

#define SH1106_FIX							// For 1.3" OLED
#define BASE_TIMER			htim3			// 1mS base timer for timing functions
#define PWM_TIMER 			htim17			// PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_1	// PWM Timer Channel
#define CHxN								// Using CHxN Output type
//#define CHx								// Using CHx Output type
#define ADC_DEVICE 			hadc			// ADC device
#define SPI_DEVICE 			hspi2			// SPI device

#define ADC_CH1 			VREF			// ADC 1st used channel
#define ADC_CH2 			NTC				// ADC 2nd used channel
#define ADC_CH3 			V_INPUT			// ADC 3rd used channel
#define ADC_CH4 			IRON_TEMP		// ADC 4th used channel

#elif defined T12_STM103
//-----------------------------
// For STM32F103, original controller from PTDreamer
//-----------------------------

#define SH1106_FIX							// For 1.3" OLED
#define BASE_TIMER			htim3			// 1mS base timer for timing functions
#define PWM_TIMER 			htim4			// PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_3	// PWM Timer Channel
//#define CHxN								// Using CHxN Output type
#define CHx								// Using CHx Output type
#define ADC_DEVICE 			hadc1			// ADC device
#define SPI_DEVICE 			hspi1			// SPI device


#define ADC_CH1 			IRON_TEMP		// ADC 1st used channel
#define ADC_CH2 			VREF			// ADC 2nd used channel
#define ADC_CH3 			NTC				// ADC 3rd used channel
#define ADC_CH4 			V_INPUT			// ADC 4th used channel

#elif defined YOUR_CUSTOM_CONFIG
//-----------------------------
// For STM32___, Your custom board
//-----------------------------

#define SH1106_FIX							// For 1.3" OLED
#define BASE_TIMER			htim?			// 1mS base timer for timing functions
#define PWM_TIMER 			htim?		// PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_?	// PWM Timer Channel
//#define CHxN								// Using CHxN Output type
#define CHx									// Using CHx Output type
#define ADC_DEVICE 			hadc?			// ADC device
#define SPI_DEVICE 			hspi?			// SPI device



#define ADC_CH1 			VREF			// ADC 1st used channel
#define ADC_CH2 			NTC				// ADC 2nd used channel
#define ADC_CH3 			V_INPUT			// ADC 3rd used channel
#define ADC_CH4 			IRON_TEMP		// ADC 4th used channel

#else

	#error You must configure settings first in setup.h

#endif



#define WAKE_input			HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin)
#define BUTTON_input		HAL_GPIO_ReadPin(ROT_ENC_BUTTON_GPIO_Port, ROT_ENC_BUTTON_Pin)





#define _Error_Handler(__FILE__, __LINE__); Error_Handler();


#endif
