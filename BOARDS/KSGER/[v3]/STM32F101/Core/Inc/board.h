
#ifndef SETUP_H_
#define SETUP_H_
/*
 * Setup.h file
*/

/*
    BOARD PROFILE FOR KSGER v3.x
*/

#define HWSTRING "HW: KSGER v3.x"

/********************************
 *       Display Settings    *
 ********************************/
#define OLED_SPI                                              // SPI display
//#define OLED_I2C                                            // I2C display
//#define I2C_TRY_HW                                          // Try I2C HW first, use I2C SW if not detected
//#define OLED_ADDRESS      (0x3c<<1)                         // Only used for i2c
#define OLED_DEVICE         hspi2                             // SPI / I2C handler if used. Enables HW mode, otherwise SW mode is used
#define FILL_DMA            hdma_memtomem_dma1_channel2       // DMA mem2mem for filling
#define USE_RST                                               // Reset pin is used
#define USE_DC                                                // DC pin is used
//#define USE_CS                                              // CS pin is used
#define OLED_OFFSET         2                                 // Display offset

/********************************
 *      PWM Settings        *
 ********************************/
//#define PWM_TIMER_HALFCLOCK                                 // Timer runs at FCY/2
//#define READ_TIMER_HALFCLOCK                                // Timer runs at FCY/2
#define READ_TIMER          htim4                             // Timer for the dead time
#define PWM_TIMER           htim3                             // PWM Timer
#define PWM_CHANNEL         TIM_CHANNEL_1                     // PWM Timer Channel
//#define PWM_CHxN                                            // Using CHxN Output type
#define PWM_CHx                                               // Using CHx Output type
//#define DEBUG_PWM                                           // To enable a test signal and some printing through SWO (Create a output GPIO called PWM_DBG)


/********************************
 *       ADC Settings        *
 ********************************/
#define ADC_DEVICE          hadc1                             // ADC device
#define ADC_BFSIZ           (16+2)                            // ADC DMA buffer size Buffer[ADC_BFSIZ][Adc_Buffer_Elements](+2 to compensate min/max value discard in filtering)

// Order for secondary measurements, ADC channels not requiring sampling in the PWM low period. Order as ADC rank order (usually ch0-ch18)
#define ADC_1st             NTC                               // ADC 1st used channel (CH2)
#define ADC_2nd             VIN                               // ADC 2nd used channel (CH3)
#define ADC_3rd             TIP                               // ADC 3nd used channel (CH5)
#define ADC_4th             INT_TMP                           // ADC 4th used channel (CH16)
//#define ADC_5th           VREF                              // ADC 5th used channel
#define ADC_Num             4                                 // Number of channels

// Channel assignment
#define ADC_CH_1ST          ADC_CHANNEL_5                     // CH5 = NTC
#define ADC_CH_2ND          ADC_CHANNEL_8                     // CH8 = VIN
#define ADC_CH_3RD          ADC_CHANNEL_9                     // CH9 = IRON TIP
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
#define T12_Cal250        1600
#define T12_Cal350        2300
#define T12_Cal450        3000

#define C210_Cal250       300
#define C210_Cal350       400
#define C210_Cal450       500

#define C245_Cal250       900
#define C245_Cal350       1000
#define C245_Cal450       1100

#endif


/********************************
 *       Buzzer        *
 ********************************/
#define BUZZER_OFF          HAL_GPIO_WritePin(BUZ0_GPIO_Port, BUZ0_Pin, GPIO_PIN_SET);  \
                            HAL_GPIO_WritePin(BUZ1_GPIO_Port, BUZ1_Pin, GPIO_PIN_SET);

#define BUZZER_ON           HAL_GPIO_WritePin(BUZ0_GPIO_Port, BUZ0_Pin, GPIO_PIN_RESET); \
                            HAL_GPIO_WritePin(BUZ1_GPIO_Port, BUZ1_Pin, GPIO_PIN_RESET);

#define BUZZER_TOGGLE       HAL_GPIO_TogglePin(BUZ0_GPIO_Port, BUZ0_Pin); \
                            HAL_GPIO_TogglePin(BUZ1_GPIO_Port, BUZ1_Pin);


/********************************
 *       Misc    *
 ********************************/
#define FLASH_SZ            128                           // Flash Size (KB)
//#define NOSAVESETTINGS                                  // Don't use flash to save or load settings. Always use defaults (for debugging purposes)
//#define SWO_PRINT                                           // To enable printing through SWO


#ifdef USE_NTC

#define NTC_RES       10000
#define NTC_BETA      3950
#define PULL_RES      4700
#define PULLUP

#endif

// To stop peripherals when debugging
#define DebugOpts()         __HAL_DBGMCU_FREEZE_IWDG(); \
                            __HAL_DBGMCU_FREEZE_TIM3(); \
                            __HAL_DBGMCU_FREEZE_TIM4()

#endif
