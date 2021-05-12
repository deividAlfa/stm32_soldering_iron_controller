# STM32 Soldering Iron Controller

<!-- MarkdownTOC -->

* [Operations guide](Readme_files/Operation.md)<br>
* [Status](#status)<br>
* [Compatibility](#Compatibility)<br>
* [Warning](#warning)<br>
* [Backup First!](#backup-first)<br>
* [Flashing the firmware](#flashing)<br>
* [Building](#build)<br>
* [Creating a .ioc file from scratch](#Creating_ioc)<br>
* [Additional Documentation](#docs)<br>
* [Pending or non working features](#pending)<br>

<!-- /MarkdownTOC -->

Video can be seen there: (Project in active development, the features will change continuously)<br>
[![IMAGE ALT TEXT](https://img.youtube.com/vi/l7mDah2jRSI/0.jpg)](https://www.youtube.com/watch?v=l7mDah2jRSI "STM32 T12 custom firmware")




<a id="status"></a>
## Status<br>
* This project started by forking [PTDreamer's firmware](https://github.com/PTDreamer/stm32_soldering_iron_controller). There was a lot of development since then.
* Developed on STM32Cube IDE - download and use that for compiling.<br>
* Basic configuration is easily done in CubeMx (Included in STM32Cube IDE).<br>
* Already compiled bins in each board folder (Test at your own risk).<br>
* Intended to serve as an unified codebase that's easier to share across different boards / hardware.<br>
* Different hardware support based on profiles, very few files need to be changed.<br>
* Supports all display modes:<br>
    Boards where the display is connected to dedicated hardware:<br>
    - Hardware SPI with DMA<br>
    - Hardware I2C with DMA<br>
 
    Boards where the display is not connected to dedicated hardware:<br>
    - Software SPI<br>
    - Software I2C<br>

<a id="compatibility"></a>
## Compatibility

The actual requirements are 10KB RAM and 64KB flash. Don't even try if your MCU has less than that.<br>
The BOARDS folder has the board code profile, schematics and/or board pictures for quickly identify your hardware.<br>
Actually all the KSGER boards are supported. Some have not been tested yet and need feedback from users.<br>
These board profiles are being tested:<br>
* Quicko T12 [STABLE]: Profiles compatible with STM32F072 C8/CB and STM32F103 C8/CB.<br>
* JCD T12    [STABLE]: Different board layout, but it's 100% the same as the KSGER v2.1. Use that firmware.
* KSGER v1.5 [TESTING]: Recently added. Not tested yet.<br>
* KSGER v2.0 [TESTING]: Seems to use the as the 2.1. Use that firmware, not tested yet.<br>
* KSGER v2.1 [STABLE]: Profile compatible with STM32F101/102/103 C8/CB/R8/RB 48/64-pin. Use 101R8 profile.<br>
* KSGER v3.0 [TESTING]: Seems to use the same as the 3.1. Use that firmware, not tested yet.<br>
* KSGER v3.1 [STABLE]: Profile compatible with STM32F101/102/103 R8/RB. Use 101R8 profile.<br>




<a id="warning"></a>
## Warning<br>
#### Temperature accuracy
Buying a cheap high temperature meter is highly recommended!<br>
These boards can have pretty different readings and tolerances. It can even change a lot between T12 tips.<br>
So the factory calibration is intentionally set lower than real, to avoid possible overheating problems.<br>
Once you set the firmware, go to calibration and set there the real temperature measured with the external probe.<br>
#### Hardware issues<br>
Newer hardware is often inferior, causing temperature regulation issues, with low quality components such as:<br>
* Bad Op-Amp<br>
* Bad 3v3 Regulator<br>

It is recommended to check and replace the problematic parts with better alternatives before proceeding.<br>
This is especially true for many of the newer v2.1 boards and v3.x boards by KSGER.<br>

<a id="backup-first"></a>
## Backup First!<br>
Be warned, usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.<br>
The simplest way to not loose the original FW is actually to buy a new MCU, replace it, and store the original MCU in a safe place.<br>
Keep in mind that there are many revisions, specially with KSGER, that can make the firmware not compatible even sharing the same MCU.<br>
Any difference in the pinout will require firmware tuning, although easing that that is one of the main proposits of this firmware.<br>
There are some hacks / vulnerabilities that can be used to backup protected firmware, more details here:<br>
**[STM32 power glitching timing attack](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE#option-2-power-glitching-timing-attack
)**


<a id="flashing"></a>
## Flashing the firmware<br>
Download the binary **STM32SolderingStation.bin** already compiled from the /BOARDS folder and flash it using stlink.<br>
There's no support for custom bootloaders, and there won't be, as flash is almost full in 64KB devices.<br>
Use one of these ST-LINK clones ($3 or less), refer to the schematics for the SWD pinout.<br>


<a id="build"></a>     
## Building

#### Board profiles<br>
If you use an existing project template and modify it, the changes must be reflected in /Core/Inc/board.h.<br>
All the project code takes the data from there. The file it's pretty much self-explaining.<br>
So, any changes you make in CubeMX, ex. use PWM timer6 intead timer2, or SPI1 instead SPI2...all that should be configured in their respective define.<br>
As long as the GPIO names are called the same way, no further changes are needed.<br>

#### Building the code<br>
If you want to build your own, clone or download the source.<br>
The source is stripped from ST own libraries and unnecesary stuff, only includes the very basic code owning to the project.<br>
CubeMX will add the STM32 and CMSIS libraries automatically after a code generation.<br>
Open the BOARDS folder, find your board (or take any to work with) and copy all the contents to the root of the project.<br>
Now you're ready to open STM32CUBE IDE and import the project.<br>
Open the .ioc file,  make any small change, ex. take an unused pin and set is as GPIO_Input, then revert it to reset state.<br>
This will trigger the code generation. Close saving changes and the code will be generated. And it's ready for building.<br>
CubeMX should care of adding the new folders to the search path, if it fails follow this steps.<br>
Right click on project -> Properties -> C/C++ Build -> Settings ->  Tool Settings -> MCU GCC Compiler -> Include paths<br>
On the upper menu, Configuration, Select [All configurations]<br>
Click on Add... Select Workspace and select these folder while holding Control key:<br>
Ensure these are present:<br>

      /Core/Inc
      /Core/Src
      /Drivers/generalIO
      /Drivers/graphics
      /Drivers/graphics/gui    
      /Drivers/graphics/u8g2
      /Drivers/STM32Fxxx_HAL_Driver/Inc
      /Drivers/STM32Fxxx_HAL_Driver/Inc/Legacy
      /Drivers/CMSIS/Device/ST/STM32Fxxx/Include
      /Drivers/CMSIS/Include
      
(STM32Fxxx matches your current mcu family, ex. STM32F0xx, STM32F1xx)
![Alt text](/Readme_files/Includes.jpg?raw=true "Title")


Click in the right arrow of the build button (Hammer icon), select Release, then click on the build button and should build right away.<br>
![Alt text](/Readme_files/release.jpg?raw=true "Title")


Video of building steps:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")<br>
Keep in mind that in 64KB devices the flash is almost full and will not fit unless optimization is set to "Optimize for size".<br>
To debug MCUs where the flash space is unsufficient to store a unoptimized build, you can selectively disable build optimizations.<br>
A line of code can be found at the start of main.h:

  __attribute__((optimize("O0")))

Copy that line before the function like this:

   __attribute__((optimize("O0"))) void ThisFunctionWillNotBeOptimized(...)
   

If you want to retarget the project, avoid mixing different profile files.<br>
Run the included script "Clean_Profile.bat", or manually delete these files:<br>

    /Core/Inc/*stm32*
    /Core/Src/*stm32*
    /Core/Src/system_stm32*
    /Core/Startup/*

And then copy the board profile files overwriting any existing files.<br>

<a id="Creating_ioc"></a>
## Creating a .ioc file from scratch

If you make a new .ioc file, ex. for a different MCU, follow this guide:<br>

* MISC
        *  Wake signal from handle: GPIO Input, User label: WAKE, No pull          
        *  Buzzer signal: GPIO Output, User label: BUZZER,  No pull
        *  DMA stream mem2mem, Mode: Normal, Size: Word, increase only dest address.
         
* CRC
        *  Enabled, default settings
        
* ENCODER
        *  Rotatory encoder right signal: GPIO INPUT, name: ROT_ENC_R
        *  Rotatory encoder left signal: GPIO INPUT, name: ROT_ENC_L
        *  Rotatory encoder button signal: GPIO INPUT,name: ROT_ENC_BUTTON
        *  GPIO config: All inputs no pull
        
* OLED 
        -  Oled CS signal: GPIO Output, name: OLED_CS
        -  Oled DC signal: GPIO Output, name: OLED_DC
        -  Oled RESET signal: GPIO Output, name: OLED_RST
    
* Software SPI/I2C (If used)
        -  GPIO Settings:
             * Oled CLOCK signal: 
             	- User Label: OLED_SCL
                - No pull
                - Speed: High
                
             * Oled DATA signal
                - User Label: OLED_SDA
                - No pull
                - Speed: High

* Hardware SPI (If used)
        -  GPIO Settings:
             * Oled SPI CLOCK signal
                - User Label: OLED_SCL (Don't care actually)<br>
                - No pull
                - Speed: High
             * Oled SPI MOSI signal
                - User Label: OLED_SDA (Don't care actually)<br>
                - No pull
                - Speed: High
                
        -  Parameter settings:
             * Mode: Half-Duplex master or master transmit only<br>
             * NSS: Disabled
             * Data size: 8
             * MSB First
             * Prescaler: Adjust for max speed, usually the limit is 18Mbit.
             * Clock Polarity: Low
             * Clock Phase: 1 Edge
             * CRC Calculation: Disabled
             * NSSP Mode: Disabled
             * NSSP SIgnal Type: Software
             
        - DMA Settings:
            * SPIx_Tx
            * Direction: Memory to peripheral
            * Piority: Low
            * DMA request mode: Normal
            * Data width: Byte in both
            
        - NVIC Settings:
            * DMAx channel interrupt enabled.
            * SPIx global interrupt disabled
            
* Hardware I2C (If used)
         -  GPIO Settings:
             * Oled I2C CLOCK signal
                User Label: OLED_SCL (Don't care actually)<br>
                No pull<br>
                Speed: High<br>
             * Oled I2C DATA signal
                User Label: OLED_SDA (Don't care actually)<br>
                No pull<br>
                Speed: High<br>
                
        -  Parameter settings:
             * I2C Speed Mode: Fast mode or Fast-mode Plus (The fastest the better).
               (Try lower speeds if it doen't work)
             * I2C Clock Speed: 100KHz...1MHz
             * Slave features: Don't care.
             
        - DMA Settings:
            * I2Cx_Tx
            * Direction: Memory to pheripheral
            * Piority: Low
            * DMA request mode: Normal
            * Data width: Byte in both
            
        - NVIC Settings:
            * DMAx channel interrupt enabled.
            * I2Cx global interrupt disabled
            
* ADC
       -   GPIO config:
            * NTC pin label: NTC (Don't care)
            * V Supply pin label: VINPUT (Don't care)
            * VRef pin label: VREF  (Don't care)
            * Iron Temp pin label: IRON  (Don't care)
            
        - Parameter settings:
            * Select channels assigned to the used inputs
            * Clock prescaler: Asynchronous clock mode
            * Resolution: 12 bit resolution
            * Data Alignment: Right alignment
            * Scan Conversion Mode: Forward
            * Continuous Conversion: Enabled
            * DMA Continuous Requests: Enabled
            * End Of Conversion Selection: End of Single Conversion
            * Overrun behavior: Overrun data preserved
            * Low Power Auto Wait: Disabled
            * Low Power Auto Power Off: Disabled
            * Sampling time: Don't care, adjusted within the program.
            * External Trigger Conversion Source: Don't care, adjusted within the program.
            * External trigger Conversion Edge: None
            * Watchdog disabled.
            
            * IMPORTANT: Configure in board.h the order of the channels and set their labels accordingly!
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
        
        - DMA settings:
            * Pheripheral to memory
            * Mode: Circular
            * Data width: Half word on both.
            
        - NVIC Settings:
            * DMAx channel interrupt enabled.
            * ADC and COMP*** interrupts disabled
            
* DELAY TIMER

        - Base timer: Take any.
            * Internal clock
            * Internal clock division: No division
            * Auto-reload preload: Enable
            * Master/Slave mode: Disable
            * Trigger event selection: Reset
            * Prescaler: Don't care, it's adjusted within the program. It asumes the timer runs at CPU speed.
                         Some timers may take haf the clock speed, depending on the bus!
                         In that case use #define DELAY_TIMER_HALFCLOCK in board.h!
                         Check the Clock config in CUBEMX!
            * Period: Don't care, it's adjusted within the program.
            * NVIC settings: General enabled.
              
            
* PWM

        - GPIO:
            * User label: PWM_OUTPUT (Don't care actually)
            * Mode: TIMxCHx(N) ("x" and "N" depends on the selected pin)
           
        - TIMER: Select timer assigned to the pin.
            * Check Activated 
            * Select channel assigned to the pin
            * Mode: PWM Generation. Select CHx(N), as assigned to PIN. Ensure to select "N" if the pin has it!
            * Prescaler: Don't care, it's adjusted within the program. It asumes the timer runs at CPU speed.
                         Some timers may take haf the clock speed, depending on the bus!
                         In that case use #define PWM_TIMER_HALFCLOCK in board.h!
                         Check the Clock config in CUBEMX!
            * Period: Don't care, it's adjusted within the program.
            * Pulse: Don't care, it's adjusted within the program.
            * NVIC settings: Depending on the timer type, enable TIMx "Capture compare" if available, else use "Global interrupt".
       
       
            
<a id="pending"></a>
### Non working features
* I2C eeprom. Some boards have it, some doesn't. So internal flash storage is used for all.
* RTC clock. And probably never will.




<a id="docs"></a>
## Additional Documentation

* Dreamcat4 has made a great research and documentation of T12 and STM32 related stuff:
* **[Dreamcat4 documentation repo](https://github.com/dreamcat4/t12-t245-controllers-docs)**
* **[Backup of original PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)** 








