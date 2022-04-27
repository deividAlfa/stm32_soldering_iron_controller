## Creating a .ioc file from scratch

If you make a new .ioc file, ex. for a different MCU, follow this guide:<br>

**MISC**<br>

    * DMA stream mem2mem, Mode: Normal, Size: Word, increase only dest address.   
    * Buzzer:                   GPIO Output, User label: BUZZER,  No pull
    * Wake:                     GPIO Input, User label: WAKE, No pull       
      
      
**CRC**<br>

    * Enabled, default settings
        
        
**ENCODER**<br>

    * Rotatory encoder right:  GPIO INPUT, name: ENC_R, no pull
    * Rotatory encoder left:   GPIO INPUT, name: ENC_L, no pull
    * Rotatory encoder button: GPIO INPUT, name: ENC_SW, no pull
        
        
**OLED**<br>

    * Oled CS:      GPIO Output, name: DISPLAY_CS, no pull
    * Oled DC:      GPIO Output, name: DISPLAY_DC, no pull
    * Oled RESET:   GPIO Output, name: DISPLAY_RST, no pull


**SOFTWARE SPI/I2C** (If used)<br>

    * OLED CLOCK
        Label: SW_SCL
        No pull
        Speed: high
        
    * OLED DATA signal
        Label: SW_SDA
        No pull
        Speed: High
        
        
**HARDWARE SPI** (If used)<br>

    * GPIO Settings
        * Oled SPI CLOCK
            No pull
            Speed: High
            
        * Oled SPI MOSI
            No pull
            Speed: High
            
    * Parameter settings
        Mode: Half-Duplex master or master transmit only
        NSS: Disabled
        Data size: 8
        MSB First
        Prescaler: Adjust for max speed, usually the limit is 18Mbit
        Clock Polarity: Low
        Clock Phase: 1 Edge
        CRC Calculation: Disabled
        NSSP Mode: Disabled
        NSSP SIgnal Type: Software
        
    * DMA Settings
        SPIx_Tx
        Direction: Memory to peripheral
        Piority: Low
        DMA request mode: Normal
        Data width: Byte in both
        
    * NVIC Settings
        DMAx channel interrupt enabled
        SPIx global interrupt disabled
        
        
**HARDWARE I2C** (If used)<br>

    * GPIO Settings
        Oled I2C CLOCK
            No pull
            Speed: High
            
        Oled I2C DATA
            No pull
            Speed: High

    Parameter settings
        I2C Speed Mode: Fast mode or Fast-mode Plus (The fastest the better).
        (Try lower speeds if it doen't work)
        I2C Clock Speed: 100KHz...1MHz
        Slave features: Don't care             
        
    DMA Settings
        I2Cx_Tx
        Direction: Memory to peripheral
        Piority: Low
        DMA request mode: Normal
        Data width: Byte in both           
        
    NVIC Settings
        DMAx channel interrupt enabled
        I2Cx global interrupt disabled  
        
        
**ADC**<br>

    * GPIO config
        NTC pin label: NTC (Don't care)
        V Supply pin label: VINPUT (Don't care)
        VRef pin label: VREF  (Don't care)
        Iron Temp pin label: IRON  (Don't care)
        
    * Parameter settings
        Select channels assigned to the used inputs
        Clock prescaler: Asynchronous clock mode
        Resolution: 12 bit resolution
        Data Alignment: Right alignment
        Scan Conversion Mode: Forward
        Continuous Conversion: Enabled
        DMA Continuous Requests: Enabled
        End Of Conversion Selection: End of Single Conversion
        Overrun behavior: Overrun data preserved
        Low Power Auto Wait: Disabled
        Low Power Auto Power Off: Disabled
        Sampling time: Don't care, adjusted within the program.
        External Trigger Conversion Source: Don't care, adjusted within the program.
        External trigger Conversion Edge: None
        Watchdog disabled
            
        **IMPORTANT: Configure in board.h the order of the channels and set their labels accordingly!**
        
        The ADC channel order goes from 0 to 15 (unless otherwise set in regular config), skipping the disabled channels.
        You must define the ADC channels in these lines:
              
            #define ADC_CH_1ST          ADC_CHANNEL_1             // First used channel:  CH1
            #define ADC_CH_2ND          ADC_CHANNEL_4             // Second used channel: CH4
            #define ADC_CH_3RD          ADC_CHANNEL_7             // Third used channel:  CH7
            #define ADC_CH_4TH          ADC_CHANNEL_9             // Fourth used channel: CH9
        
        Also, they must be adjusted depending on the signal connected to them:
      
            #define ADC_1st             VREF                     // First used channel measures VREF
            #define ADC_2nd             NTC                      // Second used channel measures NTC
            #define ADC_3rd             VIN                      // Third used channel measures VIN
            #define ADC_3rd             TIP                      // Fourth used channel measures TIP
                
        Set the number for active ADC channels:
              
            #define ADC_Num            4                         // Number of active channels
                  
        Except the tip ADC input, all the others can be enabled or disabled:
        
            #define USE_VREF
            #define USE_VIN
            #define USE_NTC
       
        Power limit will not be available if VIN is disabled.
        When disabling NTC, ambient temperature is internally set to 35ºC.
		
    * DMA settings
        Pheripheral to memory
        Mode: Circular
        Data width: Half word on both.           
		
    * NVIC Settings
        DMAx channel interrupt enabled.
        ADC and COMP*** interrupts disabled 


**DELAY TIMER**<br>

    * Base timer
        Internal clock
        Internal clock division: No division
        Auto-reload preload: Enable
        Master/Slave mode: Disable
        Trigger event selection: Reset
        Prescaler: Don't care, it's adjusted within the program. It asumes the timer runs at CPU speed
                    Some timers may take haf the clock speed, depending on the bus!
                    In that case use #define TIMER_HALFCLOCK in board.h!
                    Check the Clock config in CUBEMX!
        Period: Don't care, it's adjusted within the program
        NVIC settings: General enabled


**PWM**<br>

    * GPIO
        User label: PWM
        Mode: TIMxCHx(N) ("x" and "N" depends on the selected pin)
        
    * TIMER
        Select timer assigned to the pin
        Check Activated
        Select channel assigned to the pin
        Mode: PWM Generation. Select CHx(N), as assigned to PIN. Ensure to select "N" if the pin has it!
        Prescaler: Don't care, it's adjusted within the program. It asumes the timer runs at CPU speed.
                     Some timers may take haf the clock speed, depending on the bus!
                     In that case use #define PWM_TIMER_HALFCLOCK in board.h!
                     Check the Clock config in CUBEMX!
        Period: Don't care, it's adjusted within the program.
        Pulse: Don't care, it's adjusted within the program.
        NVIC settings: Depending on the timer type, enable TIMx "Capture compare" if available, else use "Global interrupt".
