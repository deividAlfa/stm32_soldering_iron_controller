
#ifndef SETUP_H_
#define SETUP_H_
/*
 * Setup.h file
*/

/*
    BOARD PROFILE FOR Quicko STM32F072
*/

#define HWSTRING "HW: Quicko F072"

/********************************
 *       Display Settings    *
 ********************************/
//#define SSD1306
#define ST7565
#define DISPLAY_SPI                                              // SPI display
//#define DISPLAY_I2C                                            // I2C display
//#define I2C_TRY_HW                                          // Try I2C HW first, use I2C SW if not detected
//#define DISPLAY_ADDRESS      (0x3c<<1)                         // Only used for i2c
#define DISPLAY_DEVICE         hspi2                             // SPI / I2C handler if used. Enables HW mode, otherwise SW mode is used
#define FILL_DMA            hdma_memtomem_dma1_channel2       // DMA mem2mem for filling
#define USE_RST                                               // Reset pin is used
#define USE_DC                                                // DC pin is used
#define USE_CS                                                // CS pin is used
#define DISPLAY_OFFSET         0                                 // Display offset

/********************************
 *       PWM Settings        *
 ********************************/
//#define PWM_TIMER_HALFCLOCK                                 // Timer runs at FCY/2
//#define READ_TIMER_HALFCLOCK                                // Timer runs at FCY/2
#define READ_TIMER          htim15                            // Timer for the dead time
#define PWM_TIMER           htim17                            // PWM Timer
#define PWM_CHANNEL         TIM_CHANNEL_1                     // PWM Timer Channel
#define PWM_CHxN                                              // Using CHxN Output type
//#define PWM_CHx                                             // Using CHx Output type
//#define DEBUG_PWM                                           // To enable a test signal and some printing through SWO (Create a output GPIO called PWM_DBG)
#define PWM_ALT_PIN         GPIO_AF2_TIM17

/********************************
 *       ADC Settings    *
 ********************************/
#define ADC_DEVICE          hadc                              // ADC device
#define ADC_BFSIZ           (16+2)                            // ADC DMA buffer size Buffer[ADC_BFSIZ][Adc_Buffer_Elements](+2 to compensate min/max value discard in filtering)

// Order for secondary measurements, ADC channels not requiring sampling in the PWM low period. Order as ADC rank order (usually ch0-ch18)
#define ADC_1st             NTC                               // ADC 1st used channel (CH2)
#define ADC_2nd             VIN                               // ADC 2nd used channel (CH3)
#define ADC_3rd             TIP                               // ADC 3nd used channel (CH5)
#define ADC_4th             INT_TMP                           // ADC 4th used channel (CH16)
//#define ADC_5th           VREF                              // ADC 5th used channel
#define ADC_Num             4                                 // Number of channels

// Channel assignment
#define ADC_CH_1ST          ADC_CHANNEL_2                     // CH2 = NTC
#define ADC_CH_2ND          ADC_CHANNEL_3                     // CH3 = VIN
#define ADC_CH_3RD          ADC_CHANNEL_5                     // CH5 = IRON TIP
#define ADC_CH_4TH          ADC_CHANNEL_TEMPSENSOR            // CH16 = INT TEMP
//#define ADC_CH_5TH        ADC_CHANNEL__                     // CH? = ?

// To enable specific functions in code
//#define USE_VREF
#define USE_VIN
#define USE_NTC
#define ENABLE_INT_TEMP                                       // Enable internal temperature if NTC fails or disabled in options (Depends on USE_NTC)


/********************************
 *       TIP calibration    *
 ********************************/
#define PROFILE_VALUES                                        // Enable profile values

#ifdef PROFILE_VALUES
#define T12_Cal250        1300
#define T12_Cal400        1900

#define C210_Cal250       450
#define C210_Cal400       700

#define C245_Cal250       1200
#define C245_Cal400       1750
#endif


/********************************
 *       Buzzer        *
 ********************************/
#define BUZZER_ON           HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET)
#define BUZZER_OFF          HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET)
#define BUZZER_TOGGLE       HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin)

/********************************
 *       Addons/Extras    *
 ********************************/
//#define ENABLE_ADDON_FUME_EXTRACTOR      // addon for controlling a fume extractor, must define a gpio output pin "EXTRACTOR" to use
//#define ENABLE_ADDON_SWITCH_OFF_REMINDER // addon to beep periodically in sleep mode

// logical or together all the addons
#if defined(ENABLE_ADDON_FUME_EXTRACTOR)      || \
    defined(ENABLE_ADDON_SWITCH_OFF_REMINDER)
#define ENABLE_ADDONS
#endif

/********************************
 *       Misc    *
 ********************************/
//#define NOSAVESETTINGS                                      // Don't use flash to save or load settings. Always use defaults (for debugging purposes)
//#define SWO_PRINT                                           // To enable printing through SWO

// Define if the board has a battery. Adds "remember set temp" feature.
//#define HAS_BATTERY


#ifdef USE_NTC

#define NTC_RES       10000
#define NTC_BETA      3450
#define PULL_RES      10000
#define PULLUP

#endif

// To stop peripherals when debugging
#define DebugOpts()         __HAL_DBGMCU_FREEZE_IWDG();   \
                            __HAL_DBGMCU_FREEZE_TIM15();  \
                            __HAL_DBGMCU_FREEZE_TIM17()

#endif
