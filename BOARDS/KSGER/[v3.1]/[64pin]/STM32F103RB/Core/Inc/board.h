
#ifndef SETUP_H_
#define SETUP_H_
/*
 * Setup.h file
*/
// This is left here just to have it handy for copying when debugging a specific function
// Don't uncomment!!
//               __attribute__((optimize("O0")))

/********************************
 * 		Software Settings       *
 ********************************/
#define PID_Refresh_ms			0			                    // PID calculation interval in mS, 0=After each PWM cycle
//#define USE_FILTER							                // Comment to totally disable filtering (Only make average of the last buffer)
//#define FILTER_N				3			                    // For filter coefficient (Higher, more filtering, also more delay in the filter output, can make the system oscillate)

                                                                // If your screen doesn't work, and want to discard a SPI problem
//#define Soft_SPI							                    // Uncomment to disable Hardware SPI with DMA and use software SPI

/********************************
 * 			PWM Settings        *
 ********************************/
#define DELAY_TIMER			htim4			                    // Timer for the dead time
#define PWM_TIMER 			htim3			                    // PWM Timer
#define PWM_CHANNEL 		TIM_CHANNEL_1	                    // PWM Timer Channel
//#define PWM_CHxN							                    // Using CHxN Output type
#define PWM_CHx								                    // Using CHx Output type

/********************************
 * 			Display Settings    *
 ********************************/
#define OLED_SPI												// Hardware DMA SPI
//#define OLED_I2C												// Hardware DMA I2C
//#define OLED_SOFT_SPI											// Software bitbang SPI
//#define OLED_SOFT_I2C											// Software bitbang I2C
#define OLED_DEVICE			hspi2								// SPI / I2C handler if used
#define OLED_ADDRESS 		(0x3c<<1)						    // Only used for i2c
#define FILL_DMA			hdma_memtomem_dma1_channel2		    // DMA mem2mem for filling
#define USE_RST
#define USE_DC
//#define USE_CS

/********************************
 * 			ADC Settings        *
 ********************************/
#define ADC_DEVICE 			hadc1			                    // ADC device
#define ADC_MEASURE_TIME	60 				                    // in uS
#define ADC_TRGO			ADC_EXTERNALTRIGCONV_T3_TRGO		// TRGO source for ADC trigger
#define ADC_BFSIZ 	        16	+2		                        // ADC DMA buffer size Buffer[ADC_BFSIZ][Adc_Buffer_Elements](+2 to compensate min/max value discard in filtering)

// Order for secondary measurements, ADC channels not requiring sampling in the PWM low period. Order as ADC rank order (usually ch0-ch18)
#define ADC_1st				NTC			                    	// ADC 1st used channel (CH1)
#define ADC_2nd				VIN				                    // ADC 2nd used channel (CH2)
//#define ADC_3rd			VREF			                    // ADC 2nd used channel (CH2)
#define ADC_AuxNum			2				                    // Number of secondary elements

// Channel assignment
#define ADC_CH_1ST			ADC_CHANNEL_5	                    //  CH5 = NTC
#define ADC_CH_2ND			ADC_CHANNEL_8	                    //  CH8 = VIN
//#define ADC_CH_3RD		ADC_CHANNEL_						//  NO VREF
#define ADC_TIP				ADC_CHANNEL_9	                    //  CH9 = IRON TIP (Sampled independently)

// To enable specific functions in code
//#define USE_VREF
#define USE_VIN
#define USE_NTC
/********************************
 * 			Buzzer				*
 ********************************/
#define BUZZER_OFF 			HAL_GPIO_WritePin(BUZ0_GPIO_Port, BUZ0_Pin, GPIO_PIN_SET);\
							HAL_GPIO_WritePin(BUZ1_GPIO_Port, BUZ1_Pin, GPIO_PIN_SET);

#define BUZZER_ON 			HAL_GPIO_WritePin(BUZ0_GPIO_Port, BUZ0_Pin, GPIO_PIN_RESET);\
							HAL_GPIO_WritePin(BUZ1_GPIO_Port, BUZ1_Pin, GPIO_PIN_RESET);

#define BUZZER_TOGGLE 		HAL_GPIO_TogglePin(BUZ0_GPIO_Port, BUZ0_Pin);\
							HAL_GPIO_TogglePin(BUZ1_GPIO_Port, BUZ1_Pin);

/********************************
 * 			Misc		*
 ********************************/
#define HIWDG				hiwdg							// iwdg used
#define HCRC				hcrc							// crc used
#define FLASH_SZ			128								// Flash Size (KB)
//#define NOSAVESETTINGS									// Don't use flash to save or load settings. Always use defaults (for debugging purposes)

// To stop peripherals when debugging
#define DebugOpts()			__HAL_DBGMCU_FREEZE_IWDG();\
							__HAL_DBGMCU_FREEZE_TIM3();\
							__HAL_DBGMCU_FREEZE_TIM4()

#endif
