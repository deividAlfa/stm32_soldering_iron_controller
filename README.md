# stm32_soldering_iron_controller

<!-- MarkdownTOC -->

* [Status](#status)
* [Compatibility](#Compatibility)
* [Warning](#warning)
* [Backup First!](#backup-first)
* [Setup instructions](#Setup_instructions)
* [Working with board profiles](#boards)
* [Building](#build)
* [Creating a .ioc file from scratch](#Creating_ioc)
* [Operations guide](Readme_files/Operation.md)
* [Additional Documentation](#docs)
* [Pending or non working features](#pending)

<!-- /MarkdownTOC -->

Video can be seen there: (Project in active development, the features will change continuously)
[![IMAGE ALT TEXT](https://img.youtube.com/vi/l7mDah2jRSI/0.jpg)](https://www.youtube.com/watch?v=l7mDah2jRSI "STM32 T12 custom firmware")




<a id="status"></a>
## Status
* Developed on STM32Cube IDE - download and use that for compiling.
* Basic configuration is easily done in CubeMx (Included in STM32Cube IDE)
* Already compiled bins in each board folder (Test at your own risk) 
* Intended to serve as an unified codebase that's easier to share across different boards / hardware.
* Different hardware support based on profiles, very few files need to be changed.
* Supports all display modes:

  	Boards where the display is connected to dedicated hardware:
  	- Hardware SPI with DMA
  	- Hardware I2C with DMA
 
  	Boards where the display is not connected to dedicated hardware:
  	- Software SPI
  	- Software I2C 




<a id="compatibility"></a>
## Compatibility

The actual requirements are 10KB RAM and 64KB flash. Don't even try if your MCU has less than that.

The BOARDS folder has the board code profile, schematics and/or board pictures for quickly identify your hardware.  

These board profiles are being tested:
* Quicko T12 [STABLE]: Profiles compatible with STM32F072 C8/CB and STM32F103 C8/CB. 
* KSGER v2.1 [STABLE]: Profile compatible with STM32F101/102/103 C8/CB/R8/RB 48/64-pin. Use 101R8 profile.
* KSGER v3.0 [STABLE]: Profile compatible with STM32F101/102/103 R8/RB. Use 101R8 profile.




<a id="warning"></a>
## Warning

#### Temperature accuracy

Buying a cheap high temperature meter is highly recommended!

These boards can have pretty different readings and tolerances. It can even change a lot between T12 tips.

So the factory calibration is intentionally set lower than real, to avoid possible overheating problems.

Once you set the firmware, go to calibration and set there the real temperature measured with the external probe.


#### Hardware issues

Newer hardware is often inferior! With bad low quality component(s) such as:

* Bad Op-Amp
* Bad 3v3 Regulator

Which can then result in bad performance / bad temperature regulation.

* It is recommended to check these above suspect components. And if necessary order better alternatives parts and replace them (before proceeding further).
* This is especially true for many of the newer v2.1 boards and v3.x boards by KSGER.




<a id="backup-first"></a>
## Backup First!
Be warned, usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.

The simplest way to not loose the original FW is actually to buy a new MCU, replace it, and store the original MCU in a safe place.

Keep in mind that there are many revisions, specially with KSGER, that can make the firmware not compatible even sharing the same MCU. 

Any difference in the pinout will require firmware tuning, although easing that that is one of the main proposits of this firmware.

There are some hacks / vulnerabilities that can be used to backup protected firmware, more details here:

**[STM32 power glitching timing attack](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE#option-2-power-glitching-timing-attack
)**


<a id="Setup_instructions"></a>
## Setup instructions
Download the binary already compiled from the /BOARDS folder, flash it using stlink, and it would be done.

There's no support for custom bootloaders, and there won't be, as flash is almost full in 64KB devices.

Use one of these ST-LINK clones ($3 or less)




If you want to build your own, clone or download the source.

The source is stripped from ST own libraries and unnecesary stuff, only includes the very basic code owning to the project.

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




<a id="boards"></a>
## Working with board profiles
If you use an existing project template and modify it, the changes must be reflected in /Core/Inc/board.h.
All the project code takes the data from there. The file it's pretty much self-explaining.

So, any changes you make in CubeMX, ex. use PWM timer6 intead timer2, or SPI1 instead SPI2...all that should be configured in their respective define.
As long as the GPIO names are called the same way, no further changes are needed.




<a id="build"></a>     
## Building

Click in the right arrow of the build button (Hammer icon), select Release, then click on the build button and should build right away.

![Alt text](/Readme_files/release.jpg?raw=true "Title")


Video of building steps:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")

Keep in mind that in 64KB devices the flash is almost full and will not fit unless optimization is set to "Optimize for size".

To debug MCUs where the flash space is unsufficient to store a unoptimized build, you can selectively disable build optimizations.

A line of code can be found at the start of board.h: "__attribute__((optimize("O0")))"

Copy that line before the function like this:

	 __attribute__((optimize("O0"))) void ThisFunctionWillNotBeOptimized(...)
	 

If you want to retarget the project, avoid mixing different profile files.

Run the included script "Clean_Profile.bat", or manually delete these files:

	/Core/Inc/*stm32*
	/Core/Src/*stm32*
	/Core/Src/system_stm32*
	/Core/Startup/*


And then copy the board profile files overwriting any existing files.




<a id="Creating_ioc"></a>
## Creating a .ioc file from scratch

If you make a new .ioc file, ex. for a different MCU, follow this guide:

    * MISC
        -  Wake signal from handle: GPIO EXTI*, User label: WAKE, No pull
           GPIO config: Rising/falling edge interrupt mode. 
           Ensure that NVIC interrupt is enabled for it!
          
        -  Buzzer signal: GPIO Output, User label: BUZZER,  No pull
        -  DMA stream mem2mem, Mode: Normal, Size: Word, increase only dest address.
         
	* CRC
        -  Enabled, default settings
        
	* ENCODER
        -  Rotatory encoder right signal: GPIO INPUT, name: ROT_ENC_R
        -  Rotatory encoder left signal: GPIO INPUT, name: ROT_ENC_L
        -  Rotatory encoder button signal: GPIO INPUT,name: ROT_ENC_BUTTON
        -  GPIO config: All inputs no pull
        
    * OLED 
        -  Oled CS signal: GPIO Output, name: OLED_CS
        -  Oled DC signal: GPIO Output, name: OLED_DC
        -  Oled RESET signal: GPIO Output, name: OLED_RST
    
    * Software SPI/I2C (If used)
        -  GPIO Settings:
             * Oled CLOCK signal
                User Label: OLED_SCL
                No pull
                Speed: High
             * Oled DATA signal
                User Label: OLED_SDA
                No pull
                Speed: High

    * Hardware SPI (If used)
        -  GPIO Settings:
             * Oled SPI CLOCK signal
                User Label: OLED_SCL (Don't care actually)
                No pull
                Speed: High
             * Oled SPI MOSI signal
                User Label: OLED_SDA (Don't care actually)
                No pull
                Speed: High
        -  Parameter settings:
             * Mode: Half-Duplex master or master transmit only
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
            
   	* Hardware I2C (If used)
         -  GPIO Settings:
             * Oled I2C CLOCK signal
                User Label: OLED_SCL (Don't care actually)
                No pull
                Speed: High
             * Oled I2C DATA signal
                User Label: OLED_SDA (Don't care actually)
                No pull
                Speed: High
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
            * NTC pin label: NTC
            * V Supply pin label: VINPUT
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
           * User label: PWM_OUTPUT (Don't care actually)
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
       
       
       
            
<a id="pending"></a>
### Pending or non working features
* I2C eeprom not implemented yet. Internal flash storage is used. (1KB free needed)
* RTC clock is not implemented and probably never will.




<a id="docs"></a>
## Additional Documentation

* Dreamcat4 has made a great research and documentation of T12 and STM32 related stuff:
* **[Dreamcat4 documentation repo](https://github.com/dreamcat4/t12-t245-controllers-docs)**
* **[Backup of original PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)** 








