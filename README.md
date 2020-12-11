# stm32_soldering_iron_controller

<!-- MarkdownTOC -->

* [Status](#status)
* [Warning](#warning)
* [Backup First!](#backup-first)
* ~~[Setup instructions](#Setup_instructions)~~ Needs Update!
* [Creating a .ioc file from scratch](#Creating_ioc)
* [History](#history)
  * [Working](#working)
  * [Todo](#todo)
* [Where are Docs?](#where-are-docs)
* [Compatibility](#compatibility)

<!-- /MarkdownTOC -->

Video can be seen there: (Project in active development, the features will change continuously)
[![IMAGE ALT TEXT](https://img.youtube.com/vi/XhJvmsmOdC0/0.jpg)](https://www.youtube.com/watch?v=XhJvmsmOdC0 "Quicko T12 custom firmware")

<a id="status"></a>
## Status
* Developed on STM32CubeIDE - download and use that. This IDE includes the MX code generator
* Tested on STM32F072 (on a QUICKO T12 board).
* ~~Included config for STM32F103 but needs testing~~ No longer until fw is ready 
* Should be easily ported to other stm32f0xx/stm32f1xx MCUs.
* Already compiled bins in Debug folder! (Test at your own risk) 
* Intended to serve as an unified codebase that's easier to share across different boards / hardware

<a id="warning"></a>
## Warning

Newer hardware is often inferior! With bad low quality component(s) such as:

* Bad Op-Amp
* Bad 3v3 Regulator

Which can then result in bad performance / bad temperature regulation.

* It is recommended to check these above suspect components. And if necessary order better alternatives parts and replace them (before proceeding further).
* This is especially true for many of the newer v2.1 boards and v3.x boards by KSGER.

<a id="backup-first"></a>
## Backup First!
Usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.

The simplest way to not loose the originl FW is actually to buy a new MCU, then remove the original MCU with hot air, or chipquik, or bizmuth low temperature solder, and store it in a safe place in case you want to revert back.

Keep in mind that there are many versions of the hardware PCB, KSGER and friends keeps changing them all the time, so it might not work even in the MCU is the same.

Any difference in the pinout will require firmware tuning!
Although that is one of the proposits of this fork, to ease that part.

~<a id="Setup_instructions"></a>~ Needs update!
## Setup instructions
DEFINE YOUR HARDWARE FIRST!!

Create a new, empty project in STM32CUBE IDE, selecting your MCU. Name it "T12" for easiness.

If your MCU is not in the included templates list (see below), follow [Creating a .ioc file from scratch](#Creating_ioc) and then continue [Configuring setup.h](#setup_h)

When CUBEMX configurator opens, just close it.

Delete the T12.ioc file, checkout or download the code in a zip.

Go to the project folder, and paste Core and Drivers folders, overwriting all files.
There are two .ioc (CUBEMX templates) included by default:
   
        - T12-STM32F072.ioc : For Quicko T12 with STM32F072
        - T12-STM32F103.ioc : For the STM32F103, as the original FW.
 
If you are OK with any of these, copy any to the root of the project and rename it to "T12.ioc"
Open it, make any small change, ex. take an unused pin and set is as GPIO_Input, then revert it to reset state.
This will trigger the code generation.
Close saving changes and the code will be generated.

Add the new folders to the include search path!
Right click on project -> Properties -> C/C++ Build -> Settings ->  Tool Settings -> MCU GCC Compiler -> Include paths

On the upper menu, Configuration, Select [All configurations]

Click on Add... Select Workspace and select these folder while holding Control key:

    /Core/Inc
    /Core/Src
    /Drivers/generalIO
    /Drivers/graphics
    /Drivers/graphics/gui
![Alt text](/Readme_files/Includes.jpg?raw=true "Title")

<a id="setup_h"></a>
## Configuring setup.h
Open setup.h and define your board. There are 3 included, uncomment the correct one:

    //#define T12_STM072
    //#define T12_STM103
    //#define YOUR_CUSTOM_CONFIG
    
   If your board use the same MCU but different pinout, you will need to adjust the .ioc and the setup.h config!
   As long as you use the same PIN labels, you will only need to touch setup.h.
     
## Building

Click in the right arrow of the build button (Hammer icon), select Release, then click on the build button and should build right away.

![Alt text](/Readme_files/release.jpg?raw=true "Title")


Videos of the complete build steps here:

STM32F072

[![IMAGE ALT TEXT](http://img.youtube.com/vi/dtz3Nw_tk-4/0.jpg)](https://www.youtube.com/watch?v=dtz3Nw_tk-4 "STM32F072 Build")

STM32F103

[![IMAGE ALT TEXT](http://img.youtube.com/vi/fdoPI83p_gw/0.jpg)](https://www.youtube.com/watch?v=fdoPI83p_gw "STM32F103 Build")


Keep in mind that the flash is almost full, so it will not build in debug mode unless you change Build optimizations to -Os(Optimize for size)

<a id="Creating_ioc"></a>
## Creating a .ioc file from scratch

If you make a new .ioc file, ex. for a different MCU, follow this guide:

    * MISC
        -  Wake signal from handle: GPIO EXTI*, User label: WAKE, No pull
           GPIO config: Rising/falling edge interrupt mode. 
           Ensure that NVIC interrupt is enabled for it!
          
        -  Buzzer signal: GPIO Output, User label: BUZZER,  No pull

	* ENCODER
        -  Rotatory encoder right signal: GPIO EXTI*, name: ROT_ENC_R
        -  Rotatory encoder left signal: GPIO EXTI*, name: ROT_ENC_L
        -  Rotatory encoder button signal: GPIO EXTI*,name: ROT_ENC_BUTTON
        -  GPIO config: All EXTI inputs set in rising/falling edge interrupt mode, all no pull
        -  Ensure that NVIC interrupts are enabled for them!
        
    * OLED 
        -  Oled CS signal: GPIO Output, name: OLED_CS
        -  Oled DC signal: GPIO Output, name: OLED_DC
        -  Oled RESET signal: GPIO Output, name: OLED_RST
        
    * SPI
        -  GPIO Settings:
             * Oled SPI CLOCK signal: SPI*_SCK
                User Label: SCK
                No pull
                Speed: High
             * Oled SPI DATA signal: SPI*_MOSI
                User Label: SDO
                No pull
                Speed: High
        -  Parameter settings:
             * Mode: Half-Duplex master
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
            * Direction: Memory to pheripheral
            * Piority: Low
            * DMA request mode: Normal
            * Data width: Byte in both
        - NVIC Settings:
            * DMAx channel interrupt enabled.
            * SPIx global interrupt disabled
            
    * ADC
       -   GPIO config:
            * NTC pin label: NTC
            * V Supply pin label: VIN
            * VRef pin label: VREF
            * Iron Temp pin label: IRON
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
            * Sampling time: 13.5 cycles
            * External Trigger Conversion Source: Regular Conversion Launched by software
            * External trigger Conversion Edge: None
            * Watchdog disabled
            * IF ADJUSTABLE regular channels:
              Set channels 1 to 4 to the inputs, all 13.5 cycles.
            
            * IMPORTANT: Configure in setup.h the order of the channels and set their labels accordingly!
              The ADC channel order goes from 0 to 15 (unless otherwise set in regular config), skipping the disabled channels.
            *   #define ADC_CH1    VREF      // ADC 1st used channel, ex. ch0
                #define ADC_CH2    NTC       // ADC 2nd used channel, ex. ch2
                #define ADC_CH3    VIN   // ADC 3rd used channel, ex. ch3
                #define ADC_CH4    IRON // ADC 4th used channel, ex. ch7
        - DMA settings:
            * Pheripheral to memory
            * Mode: Circular
            * Data width: Half word on both.
        - NVIC Settings:
            * DMAx channel interrupt enabled.
            * ADC and COMP*** interrupts disabled
            
    * BASE TIMER
        - Base timer: Usually Timer 3, you might take any.
            * Internal clock
            * Internal clock division: No division
            * Auto-reload preload: Disable
            * Master/Slave mode: Disable
            * Trigger event selection: Reset (UG bit from TIMx_EGR)
            * Set Prescaler and Counter period for 1mS period. 
              Ex @ 48MHz Ftimer: Prescaler: 8000, Period 6.
              8000*6 = 48,000. 48Mhz/48K=1000Hz, 1mS period
            * NVIC settings: Enable TIMx Global interrupt.
              Consider that some timers will run at CPU speed, while other may take a slower clock.
              Check the Clock config in CUBEMX!
            
    * PWM
        - GPIO:
           * User label: PWM_OUTPUT
           * Mode: TIMxCHx(N) ("x" and "N" depends on the selected pin)
           
        - TIMER: Select timer assigned to the pin.
            - Check Activated 
            - Select channel assigned to the pin
            - Mode: PWM Generation. Select CHx(N), as assigned to PIN. Ensure to select "N" if the pin has it!
            - Counter period: 2000
            - Prescaler: Adjust is accordingly to the wanted PWM frequency: Fpwm = FTimer/(2000*prescaler)
              Ex: 48MHZ/(2000x480) = 50Hz (There's no need of high PWM frequency in this application)
              Consider that some timers will run at CPU speed while other may take a slower clock.
              Check the Clock config in CUBEMX!
            
 
<a id="history"></a>
## History
* Original version by PTDreamer
* Fixed a lot of bugs, lots of small improvements, specially the SPI DMA transfer
* Much easier to port between devices
* 
* Commentary of changes in the forum thread - [Posts starting here](https://www.eevblog.com/forum/reviews/stm32-oled-digital-soldering-station-for-t12-handle/msg3284800/#msg3284800)

**WARNING:** Tip temperature measurement is still unreliable, this firmware might heat up your iron far above the temperature on display. Use with caution and at own risk!

<a id="working"></a>
### Working
* OLED Display
* Rotary Encoder
* Buzzer
* Wake switch
* Supply voltage sensor
* Ambient temperature sensor (might need calibration)
* Tip temperature read out (might also need calibration)
* T12 PWM Control (might burn your tip)

<a id="todo"></a>
### Todo
* I2C eeprom
* 
<a id="where-are-docs"></a>
## Where are Docs?

* Docs have been split out into seperate 'docs' repo. Because the size of assets kept ballooning in git
* This repo is just only for source code now. This is simply code fork of PTDreamer meant for v2.1s Hardware

Well which Docs are you looking for?

* **[New Docs, Full Repo](https://github.com/dreamcat4/t12-t245-controllers-docs)**
* **[Backup of PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)** 
* **[Hardware Compatibility Docs](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/controllers/stm32-t12-oled)**
* **[How to Compile & Flash This Code with Official ST Toolchain](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE)**
* *Docs for Compiling / Flashing with VSCode and PlatformIO* - TBD. Not done.

<a id="compatibility"></a>
## Compatibility







