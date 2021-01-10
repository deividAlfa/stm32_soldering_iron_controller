# stm32_soldering_iron_controller

<!-- MarkdownTOC -->

* [Status](#status)
* [Warning](#warning)
* [Backup First!](#backup-first)
* [Setup instructions](#Setup_instructions)
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
* Tested on QUICKO T12 with STM32F072 and STM32F103.
* Should be easily ported to other stm32f0xx/stm32f1xx MCUs.
* Already compiled bins in each board folder (Test at your own risk) 
* Intended to serve as an unified codebase that's easier to share across different boards / hardware. Very few files need to be changed.

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

Any difference in the pinout will require firmware tuning, although easing that that is one of the main proposits of this firmware.

<a id="Setup_instructions"></a>
## Setup instructions
Clone or download the source.

The source is stripped from ST own libraries ans unnecesary stuff, only includes the very basic code owning to the project.

CubeMX will add the STM32 and CMSIS libraries automatically after a code generation.

Open the BOARDS folder, find your board (or take any to work with) and copy all the contents to the root of the project.

Now you're ready to open STM32CUBE IDE and import the project.

Open the .ioc file,  make any small change, ex. take an unused pin and set is as GPIO_Input, then revert it to reset state.

This will trigger the code generation. Close saving changes and the code will be generated. And it's ready for building.

CubeMX should care of adding the new folders to the search path, if it fails follow this steps.

Right click on project -> Properties -> C/C++ Build -> Settings ->  Tool Settings -> MCU GCC Compiler -> Include paths

On the upper menu, Configuration, Select [All configurations]

Click on Add... Select Workspace and select these folder while holding Control key:
Ensure these are present:

    /Core/Inc
    /Core/Src
    /Drivers/generalIO
    /Drivers/graphics
    /Drivers/graphics/gui    
    /Drivers/STM32Fxxx_HAL_Driver/Inc
    /Drivers/STM32Fxxx_HAL_Driver/Inc/Legacy
    /Drivers/CMSIS/Device/ST/STM32Fxxx/Include
    /Drivers/CMSIS/Include
    (STM32Fxxx matches your current mcu family, ex. STM32F0xx, STM32F1xx)
![Alt text](/Readme_files/Includes.jpg?raw=true "Title")

<a id="board_h"></a>
## Configuring board.h
If you use an existing project template and modify it, the changes must be reflected in /Core/Inc/board.h.
All the project code takes the data from there. The file it's pretty much self-explaining.

So, any changes you make in CubeMX, ex. use PWM timer6 intead timer2, or SPI1 instead SPI2...all that should be configured in their respective define.
Remember that the GPIO changes need no changes as long as the name is the same.
     
## Building

Click in the right arrow of the build button (Hammer icon), select Release, then click on the build button and should build right away.

![Alt text](/Readme_files/release.jpg?raw=true "Title")


Video of building steps:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")

Keep in mind that the flash is almost full, so it will not fit with optimizations disabled.

It needs to have build optimizations to -Os(Optimize for size).

To debug a specific function you can use the "__attribute" directive, found at the start of board.h.
Copy that line before the function like this:

**__attribute__((optimize("O0"))) void ThisFunctionIsNotOptimized(...)**

If you want to retarget the project, you can't mix the files, it will make a conflict.
Delete these files:
- /Core/Inc/\*stm32\*
- /Core/Src/\*stm32\*
- /Core/Startup/\*

And then copy the board profile files overwriting any files.

<a id="Creating_ioc"></a>
## Creating a .ioc file from scratch

If you make a new .ioc file, ex. for a different MCU, follow this guide:

    * MISC
        -  Wake signal from handle: GPIO EXTI*, User label: WAKE, No pull
           GPIO config: Rising/falling edge interrupt mode. 
           Ensure that NVIC interrupt is enabled for it!
          
        -  Buzzer signal: GPIO Output, User label: BUZZER,  No pull
        -  DMA stream mem2mem, Mode: Normal, Size: Word, increase only dest address. 

	* ENCODER
        -  Rotatory encoder right signal: GPIO INPUT, name: ROT_ENC_R
        -  Rotatory encoder left signal: GPIO INPUT, name: ROT_ENC_L
        -  Rotatory encoder button signal: GPIO INPUT,name: ROT_ENC_BUTTON
        -  GPIO config: All inputs no pull
        
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
            * Sampling time: Don't care, adjusted within the program.
            * External Trigger Conversion Source: Don't care, adjusted within the program.
            * External trigger Conversion Edge: None
            * Watchdog disabled.
            
            * IMPORTANT: Configure in board.h the order of the channels and set their labels accordingly!
              The ADC channel order goes from 0 to 15 (unless otherwise set in regular config), skipping the disabled channels.
              You must define the ADC channels in these lines:
                #define ADC_VREF            ADC_CHANNEL_1                       //  CH1 = VREF
                #define ADC_NTC             ADC_CHANNEL_2                       //  CH2 = NTC
                #define ADC_VIN             ADC_CHANNEL_3                       //  CH3 = VIN
                #define ADC_TIP             ADC_CHANNEL_5                       //  CH5 = IRON TIP (Sampled independently)
				
              Also, as the secondary channels are samples together in sequence, they must be correctly ordered as follows:
			
                #define ADC_1st             VREF                                // ADC 1st used channel (CH1)
                #define ADC_2nd             NTC                                 // ADC 2nd used channel (CH2)
                #define ADC_3rd             VIN                                 // ADC 3rd used channel (CH3)
                #define ADC_AuxNum          3                                   // Number of secondary elements
				
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
            * Prescaler: Don't care, adjusted within the program.
                         Consider that the routine is designed for the timer running at CPU speed. Some timers may take haf the clock speed, depending on the bus!
                         If the timer uses FCY/2, use #define DELAY_TIMER_HALFCLOCK in board.h!
                         Check the Clock config in CUBEMX!
            * Period: Don't care, it's adjusted within the program.
            * NVIC settings: General enabled.
              
            
    * PWM
        - GPIO:
           * User label: PWM_OUTPUT
           * Mode: TIMxCHx(N) ("x" and "N" depends on the selected pin)
           
        - TIMER: Select timer assigned to the pin.
            - Check Activated 
            - Select channel assigned to the pin
            - Mode: PWM Generation. Select CHx(N), as assigned to PIN. Ensure to select "N" if the pin has it!
            - Prescaler: Don't care, it's adjusted within the program.
                         Consider that the routine is designed for the timer running at CPU speed. Some timers may take haf the clock speed, depending on the bus!
                         If the timer uses FCY/2, use #define PWM_TIMER_HALFCLOCK in board.h!
                         Check the Clock config in CUBEMX!
            - Period: Don't care, it's adjusted within the program.
            - Pulse: Don't care, it's adjusted within the program.
            - NVIC settings: Depending on the timer type, enable TIMx "Capture compare" if available, else use "Global interrupt".
              Consider that some timers will run at CPU speed while other may take a slower clock.
              Check the Clock config in CUBEMX!
            
 
<a id="history"></a>
## History
* Original version by PTDreamer
* Fixed a lot of bugs, lots of small improvements, specially the SPI DMA transfer
* Much easier to port between devices
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







