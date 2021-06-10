# STM32 Soldering Iron Controller

<!-- MarkdownTOC -->

* [Operations guide](Readme_files/Operation.md)
* [Frequently asked questions](#faq)
* [Status](#status)
* [Compatibility](#Compatibility)
* [Warning](#warning)
* [Backup First!](#backup-first)
* [Flashing the firmware](#flashing)
* [Building](#build)
* [Creating a .ioc file from scratch](Readme_files/Creating_ioc.md)
* [Additional Documentation](#docs)
* [Pending or non working features](#pending)

<!-- /MarkdownTOC -->

Video can be seen there: (Project in active development, the features will change continuously)
[![IMAGE ALT TEXT](https://img.youtube.com/vi/l7mDah2jRSI/0.jpg)](https://www.youtube.com/watch?v=l7mDah2jRSI "STM32 T12 custom firmware")




<a id="status"></a>
## Status
* This project started by forking [PTDreamer's firmware](https://github.com/PTDreamer/stm32_soldering_iron_controller). There was a lot of development since then.
* Developed on STM32Cube IDE - download and use that for compiling.
* Basic configuration is easily done in CubeMx (Included in STM32Cube IDE).
* Already compiled bins in each board folder (Test at your own risk).
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
Actually all the KSGER boards are supported. Some have not been tested yet and need feedback from users.
These board profiles are being tested:
* Quicko T12 [STABLE]: Profiles compatible with STM32F072 C8/CB and STM32F103 C8/CB.
* JCD T12    [STABLE]: Different board layout, but it's 100% the same as the KSGER v2.1. Use that firmware.
* KSGER v1.5 [TESTING]: Recently added. Not tested yet.
* KSGER v2.0 [TESTING]: Seems to use the as the 2.1. Use that firmware, not tested yet.
* KSGER v2.1 [STABLE]: Profile compatible with STM32F101/102/103 C8/CB/R8/RB 48/64-pin. Use 101R8 profile.
* KSGER v3.0 [TESTING]: Seems to use the same as the 3.1. Use that firmware, not tested yet.
* KSGER v3.1 [STABLE]: Profile compatible with STM32F101/102/103 R8/RB. Use 101R8 profile.


<a id="faq"></a>
## Frequently asked questions<br>

### Display issues<br>
If the display has right/left line like this picture: Go to system / Offset and adjust the value until it's centered.
<img src="/Readme_files/oled_offset.jpg?raw=true" width="320">

### Temperature unstabilitit<br>
Never modify any PWM / Delay settings by default. Doing so may cause such issues.<br>
Also, new tips are often unstable, leading to temperature jumps. Don't try to calibrate the tip in this state, neither set a high temperature, because it could go under control.<br>
They usually settle after some burning time. It's recommended to set a middle temperature (250-300ºC) and leave like that for 15-20 minutes until it stabilizes.<br>
If the temps are still unstable, you might be having power supply noise. Try increasing the Iron/Delay option, allowing more time for the temop signal to settle in.<br>
Some boards are defective, with broken/badly soldered capacitors, leading to similar issues.<br>
A damaged, loose or defective connection in the handle will also cause this issues. Make sure you contacts are clean.<br>

### Calibration issues<br>
To calibrate, go into Calibration / Start.<br>
If the difference between measured and real is more than 50ºC, the calibration will be aborted, telling you to go into Calibration / Adjust.<br>
That menu has two fields: Calibration step (for 250, 350 and 450ºC steps), and the internal value associated to that step.<br>
Once you enter this menu, the value will be applied in real time, so be careful. Attach a temperature probe to the tip and change the values.<br>
If you got higher temperatures, it's recommended to lower the value quicky to prevent tip overheating. <br>
Then, slowly rise the values until it gets close to the calibration target.<br>
Repeat for each step and save.<br>
This values are only used by the calibration process. It's there with the only purpose to prevent tip burning if your board reads lower than real.<br>
After adjusting, repeat calibration, this time it should work right away.<br>
The calibration results for the current tip can be seen in the tip settings menu.<br>
In the case you lose, wipe or reset the data, you can go back into that menu and adjust these based on previous calibration results.<br>
These values aren't meant to be another calibration menu! Only for viewing (Ex. reporting calibration results) and making backup/restore of the values.<br>

### Other issues<br>
After fully reading the documentaion, if you still have problems or doubts, please ask in the EEVblog thread:<br>
https://www.eevblog.com/forum/reviews/stm32-oled-digital-soldering-station-for-t12-handle<br>

<a id="warning"></a>
## Warning
#### Temperature accuracy
Buying a cheap high temperature meter is highly recommended!
These boards can have pretty different readings and tolerances. It can even change a lot between T12 tips.
So the factory calibration is intentionally set lower than real, to avoid possible overheating problems.
Once you set the firmware, go to calibration and set there the real temperature measured with the external probe.
#### Hardware issues
Newer hardware is often inferior, causing temperature regulation issues, with low quality components such as:
* Bad Op-Amp
* Bad 3v3 Regulator

It is recommended to check and replace the problematic parts with better alternatives before proceeding.
This is especially true for many of the newer v2.1 boards and v3.x boards by KSGER.

<a id="backup-first"></a>
## Backup First!
Be warned, usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.
The simplest way to not loose the original FW is actually to buy a new MCU, replace it, and store the original MCU in a safe place.
Keep in mind that there are many revisions, specially with KSGER, that can make the firmware not compatible even sharing the same MCU.
Any difference in the pinout will require firmware tuning, although easing that that is one of the main proposits of this firmware.
There are some hacks / vulnerabilities that can be used to backup protected firmware, more details here:
**[STM32 power glitching timing attack](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE#option-2-power-glitching-timing-attack
)**


<a id="flashing"></a>
## Flashing the firmware
Download the binary **STM32SolderingStation.bin** already compiled from the /BOARDS folder and flash it using stlink.
There's no support for custom bootloaders, and there won't be, as flash is almost full in 64KB devices.
Use one of these ST-LINK clones ($3 or less), refer to the schematics for the SWD pinout.


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
      
(STM32Fxxx matches your current mcu family, ex. STM32F0xx, STM32F1xx)<br>
<img src="/Readme_files/Includes.jpg?raw=true">


Click in the right arrow of the build button (Hammer icon), select Release, then click on the build button and should build right away.<br>
<img src="/Readme_files/release.jpg?raw=true">

Video of building steps:<br>

[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")
Keep in mind that in 64KB devices the flash is almost full and will not fit unless optimization is set to "Optimize for size".<br>
To debug MCUs where the flash space is unsufficient to store a unoptimized build, you can selectively disable build optimizations.<br>
A line of code can be found at the start of main.h:<br>

  __attribute__((optimize("O0")))

Copy that line before the function like this:<br>

   __attribute__((optimize("O0"))) void ThisFunctionWillNotBeOptimized(...)
   

If you want to retarget the project, avoid mixing different profile files.<br>
Run the included script "Clean_Profile.bat", or manually delete these files:<br>

    /Core/Inc/*stm32*
    /Core/Src/*stm32*
    /Core/Src/system_stm32*
    /Core/Startup/*

And then copy the board profile files overwriting any existing files.<br>
            
<a id="pending"></a>
### Non working features
* I2C eeprom. Some boards have it, some doesn't. So internal flash storage is used for all.
* RTC clock. And probably never will.




<a id="docs"></a>
## Additional Documentation

* Dreamcat4 has made a great research and documentation of T12 and STM32 related stuff:
* **[Dreamcat4 documentation repo](https://github.com/dreamcat4/t12-t245-controllers-docs)**
* **[Backup of original PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)** 
