
#ifndef SETUP_H_
#define SETUP_H_
/*
 * Setup.h file
*/
//               __attribute__((optimize("O0")))

//#warning OJO no se cargan las opciones pid al cambiar de punta!

/*
 * ******************************
 * 			Software Settings
 * ******************************
 */

#define Adc_Buffer_Size 		16	+2		// ADC DMA buffer size Buffer[ADC_Buffer_Size][Adc_Buffer_Elements](+2 to compensate min/max value discard in filtering)
#define Process_Refresh_ms 		20			// Graphic system refresh interval (for processing button input, etc)
#define GUI_Update_ms 			20			// Graphic values refresh interval (for showing temperatures, etc)
#define PID_Refresh_ms			20			// PID calculation interval

#define USE_FILTER							// Comment to disable
#define FILTER_N				2			// For IIR filter coefficient (Higher, more filtering, also more delay in the filter output, can make the system oscillate)

#define No_Iron_Delay_mS		300			// In mS. Increase if it bounces between "NO IRON" and "iron temperature" screen when there is no iron plugged
#define CurrTemp_Save_Time_s	10			// Minimum time in secs for saving temperature setpoint changes to store the value in flash. Warning, too often will degrade the flash quickly!
											// If your screen doesn't work, and want to discard a SPI problem
//#define Soft_SPI							// Uncomment to disable Hardware SPI with DMA and use software SPI
#define SH1106_FIX							// For 1.3" OLED
//#define JBC


/*
 * ******************************
 * 			PWM Settings
 * ******************************
 */
#define PWM_PRE				(48		-1)		// Prescaler
#define PWM_PERIOD			(20000	-1)		// in uS. PWM Frequency. F = 48MHz / (48pre*20000period) = 50Hz
#define ADC_MEASURE_TIME	(1000	-1)		// in uS. Time to subtract from the Period where PWM output will be low, so the ADC can measure the tip
#define DEAD_TIME			(1000 	-1)		// in uS. After PWM Stopped, delay before starting ADC conversion for the iron tip. Cannot be higher that Measure Time
#define DELAY_TIMER			htim15			// Timer for the dead time
#define PWM_DUTY			(PWM_PERIOD - (ADC_MEASURE_TIME + DEAD_TIME))	// Max PWM duty cycle, rest is left for ADC measurement after each PWM cycle.
#define PWM_TIMER 			htim17			// PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_1	// PWM Timer Channel
#define CHxN								// Using CHxN Output type
//#define CHx								// Using CHx Output type

/*
 * ******************************
 * 			SPI Settings
 * ******************************
 */
#define SPI_DEVICE 			hspi2			// SPI device

/*
 * ******************************
 * 			ADC Settings
 * ******************************
 */
#define ADC_DEVICE 			hadc			// ADC device

#define ADC_TIP				ADC_CHANNEL_5	//  CH5 = IRON TIP (Sampled independently)

// Order for secondary measurements, ADC channels not requiring sampling in the PWM low period. Order as ADC rank order (usually ch0-ch18)
#define ADC_1st				VREF			// ADC 1st used channel (CH1)
#define ADC_2nd				NTC				// ADC 2nd used channel (CH2)
#define ADC_3rd				VIN				// ADC 3rd used channel (CH3)
#define ADC_AuxNum			3				// Number of other secondary elements

											// Channel assignment
#define ADC_VREF			ADC_CHANNEL_1	//  CH1 = VREF
#define ADC_NTC				ADC_CHANNEL_2	//  CH2 = NTC
#define ADC_VIN				ADC_CHANNEL_3	//  CH3 = VIN


//#define TEST				// Enable test features for debugging purposes
/*
 *
 *		 <·············· PERIOD ······················>(20mS)
 *		 <····· PWM_DUTY ···><····· MEASURE TIME ·····>(19.8+0.2 mS)
 *   	 ___________________						    ________________________________________
 * PWM _|		 			|__________________________|
 *  				   		 <  Dead Time >	_________		________________
 * ADC ____________________________________|  ADC ON |_____| 	ADC ON		|
 				^			   ^		 ^     ^							^ADC interrupt. Update averages. Restart ADC in trigger mode.
 * 							| 	 100uS	   |  100uS  |	   |_ ADC manual conversion for the rest of channels(not requiring measure during low PWM output)
 * 							|			   |		 |_______ ADC interrupt. Update averages. Handle Iron. Start ADC Manual conversion.
 * 							|			   |_________________ Tim15 fires ADC via TRGO and disables (One pulse mode). Measure tip only
 * 							|________________________________ Tim17 PWM compare interrupt. Inits Tim15 and starts it (Dead time)
*/





#endif
