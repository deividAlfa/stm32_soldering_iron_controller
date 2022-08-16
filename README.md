# STM32 Soldering Station Firmware

<!-- MarkdownTOC -->

* [Project details](#Details)
* [Compatibility](#Compatibility)
* [Programming](Readme_files/Programming.md)
* [Operating instructions](Readme_files/Operation.md)
* [Frequently asked questions](#faq)
* [Building the firmware](#build)
* [Translations](#translations)
* [Creating a .ioc file from scratch](Readme_files/Creating_ioc.md)
* [Additional Documentation](#docs)
* [Pending or non working features](#pending)


<!-- /MarkdownTOC -->

 If you liked the firmware, you can send me a beer with [PAYPAL](https://www.paypal.me/davidalfistas) 🙂<br>

--- 

Video of operation here: (Project in active development, the features will change continuously)<br>

[![IMAGE ALT TEXT](https://img.youtube.com/vi/j0HQq4aRiXw/0.jpg)](https://www.youtube.com/watch?v=j0HQq4aRiXw "STM32 T12 custom firmware")<br><br>


---

<a id="Details"></a>
## Project details
* This project started by forking [PTDreamer's firmware](https://github.com/PTDreamer/stm32_soldering_iron_controller). Since then it became a separate project.
* Developed on STM32Cube IDE. Basic configuration is easily done in CubeMx (Included in STM32Cube IDE).
* Unified codebase, different hardware support based on profiles, very few files need to be changed.
* Supports all display modes: I2C, SPI, software and hardware+DMA (When connected to hardware pins).
* Uses u8g2 graphics library. 
* Dynamic screen loading to save RAM resources.
* Extremely customizable, lots of options available.
* Code highly optimized to avoid wasting cpu power, slow devices still run great.    

---

<a id="Compatibility"></a>
## Compatibility

The actual requirements are 10KB RAM and 64KB **(\*)** flash.<br>
**(\*)** Currently the firmware has surpassed the 64KB limit, and uses the additional undocumented 64KB flash block.<br>
**(\*)** All 64KB devices have 128KB, with the 2nd 64KB block untested from the factory, so not guaranteed to work.<br>
**(\*)** To date, I have found zero issues. Original KSGER firmware also does this.<br>
**(\*)** ST-Link checks the written data, and the firmware uses checksums to protect the settings, any error will be detected.<br>

**CLONES** Some controllers began to put stm32 clones due the chip shortage.<br>
CKS32 works well, but MM32 and CH32 doesn't!<br>
The CH32 almost works but the ADC makes strange things.<br>
The MM32 is cortex-M0, but KSGER originally used STM32F10x (Cortex-M3), that's what this firmware uses, so it won't work at all.<br>
If your board came with MM32 or CH32, replace it with a STM32F101/102/103.<br>


The [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder has the board code profile, schematics and/or board pictures for quickly identify your hardware.<br>
Currently supported controllers (Click to download the latest build):<br>
* **Quicko T12-072** : For STM32F072 variant. Current build is broken for this model, use [this one](https://github.com/deividAlfa/stm32_soldering_iron_controller/blob/9f4b7f9565344e30a6ce1394d28350f82089488b/BOARDS/Quicko/STM32F072_SSD1306/STM32SolderingStation.bin).<br>
* **Quicko T12-103** For STM32F103 variant.
* **KSGER v1.5** : Profile for STM32F103 (There are no other known CPUs used in this board).
* **KSGER v2.x**, **JCD T12**, **T12-955**, **Handskit** : Profile compatible with all STM32F101/2/3xx models.
* **KSGER v3.x**, **T12-958** : Profile compatible with all STM32F101/2/3xx models.

For KSGER v2/v3: As long as use the correct firmware, any STM32 variant (101/102/103/C8/R8/CB/RB) will work.<br>
Check [Releases](https://github.com/deividAlfa/stm32_soldering_iron_controller/releases) for downloads.<br>

Actually, the easiest way to quickly identify your KGSER version is by looking at the Oled screen connection:<br>
- **4 pin** (I2C) = v2.x<br>
- **6 pin** (SPI) = v3.x<br>

Also keep in mind that you can't trust the version shown in the original firmware to identify your board.<br>
Go to [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS)/... schematics folder and compare the pictures.<br>
There are several compatible/cloned boards in the market that will work fine with Ksger profiles.<br>

T12-951, T12-952, T12-956, T12-959 use STC mcu, not supported by this firmware.<br>

---

<a id="faq"></a>
## Frequently asked questions<br>

First, make sure to read the [Operating instructions](Readme_files/Operation.md)!<br>

 
### Changelog<br>
You can check the [commit history](https://github.com/deividAlfa/stm32_soldering_iron_controller/commits/master) to see what have been changed between builds.

### Backing up the original firmware
The original firmwares are available [[HERE]](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/Original_FW)<br>
Some KSGER firmwares require an activation code which can be generated [[HERE]](http://t12.omegahg.com/keygen.htm)  [[Alternative link]](https://rawcdn.githack.com/deividAlfa/stm32_soldering_iron_controller/3f48a9c4c9586f89503ce763b1c6a73b9b73b55a/Original_FW/KSGER/Gen/gen.htm)<br>

Be warned, usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.<br> 
The simplest way to not loose the original FW is actually to buy a new MCU, replace it, and store the original MCU in a safe place.<br>
Any difference in the pinout will require firmware tuning, although one of the main proposits of this firmware is easing that.<br>
There are some hacks / vulnerabilities that can be used to backup protected firmware, more details here:<br>
**[STM32 power glitching timing attack](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE#option-2-power-glitching-timing-attack
)**<br>

### Display issues<br>
If the display has right/left line like this picture: Go to [System menu](Readme_files/Operation.md#system) / Offset and adjust the value until it's centered.<br>
<img src="/Readme_files/oled_offset.jpg?raw=true" width="320"><br>

### Temperature unstability<br>
By default, never modify any PWM / Delay settings in the [Iron menu](Readme_files/Operation.md#menu). Doing so may cause such issues.<br>
Also, new tips are often unstable, leading to temperature jumps.<br>
Don't try to calibrate the tip in this state, neither set a high temperature, because it could go under control.<br>
They usually settle after some burning time. It's recommended to set a medium temperature (250-300ºC) and leave like that for 15-20 minutes until it stabilizes.<br>
If the temps are still unstable, try increasing the Iron/Delay option, allowing more time for the temp signal to settle.<br>
A damaged, loose or defective connection in the handle will also cause this issues. Ensure the contacts are clean.<br>
There have been problems with some board/stations like:<br>
* Noisy power supply
* Broken / badly soldered capacitors
* Bad Op-Amp
* Bad 3v3 Regulator

### Temperature accuracy
Buying a cheap high temperature meter is highly recommended!<br>
These boards can have pretty different readings and tolerances. Even between T12 tips.<br>
So the factory calibration is intentionally set lower than real, to avoid possible overheating problems.<br>
Once you set the firmware, go to calibration and set there the real temperature measured with the external probe.<br>

### Calibration issues<br>
Ensure to read [Calibration menu](Readme_files/Operation.md#calibration) first!.<br>
To calibrate, go into Calibration / Start.<br>
Attach the temperature probe before proceeding!<br>
If the difference between measured and real is higher than 50ºC, the calibration will be aborted, telling you to go into Calibration / Settings and manually adjust the values.<br>
The calibration settings menu has 3 calibration steps: Zero set, 250 and 400°C.<br>
When you edit 250/400ºC value, the power will be enabled and the value applied in real time, so be careful!<br>
The power will be removed when no settings are being edited.<br>
Adjust each value until it's close to the target temperature. Repeat for each step and save.<br>
This values are only used by the calibration process, to prevent burning the tip if your board reads too low.<br>
After adjusting, repeat calibration, this time it should work correctly.<br>
The calibration results for the current tip can be seen in the tip settings menu.<br>
Tip settings menu calibration values aren't meant to be another calibration menu, only for viewing (Ex. reporting calibration results) and for backup/restore purposes.<br>
In the case you lose, wipe or reset the data, you can go back into that menu and adjust the values based on previous calibration results.<br>
Zero calibration can't be manually restored, but it only takes few seconds to adjust.<br>

### Cold tip not showing ambient temperature
Some amplifiers can introduce a small voltage offset that will translate into the cold tip reading 30-50°C higher than ambient temperature.<br>
To fix that, enter the [Calibration menu](Readme_files/Operation.md#calibration), insert a completely cold tip, enter Settings, adjust Zero set calibration and save.<br>
After that, the offset will be compensated and the cold temperature will be normal.<br>
It's highly recommended to recalibrate after changing this value.<br>

### KSGER self-resetting<br>
Some KSGER controllers use a linear regulator to convert 24V to 3.3V, which is a very bad design and generates a lot of heat.<br>
With the oled displays, each pixel turned on consumes more power, and this firmware uses much larger numbers for the display.<br>
Thus, this firmware uses some more power. The design is so bad that the regulator will overload and shut down, resetting the board.<br>
There're some options to fix this:<br>
- Lower the display brightness to reduce the power consumption.<br>
- Put a 100-150Ω 2W resistor in series with the regulator (24V->Resistor->LDO input). The resistor will drop part of the voltage and reduce the stress on the regulator.<br>
- Replace the LDO with a better one, or modify the board, adding a LDO that accepts a small heatsink to take away the heat.<br>
- Use a small DC/DC step-down module to convert 24V to 5V, and feed 5V to the 3.3V LDO (best option, barely makes any heat).<br>

### Other issues<br>
After fully reading the documentation, if you still have problems or doubts, please ask in the EEVblog thread:<br>
https://www.eevblog.com/forum/reviews/stm32-oled-digital-soldering-station-for-t12-handle/<br>

---

<a id="build"></a>     
## Building the firmware

Video of building steps:<br>
[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")<br><br>

There's a new automated build script for Windows (_Building_script.bat) that allows a simple and fast way of copying and building the desired profile.<br>
With it, all you need is to have CubeIDE installed in C:\ST... (It's the default installation folder), it will search and execute the tools without requiring any user intervention.<br>
Just open it, choose your profile, and if you want to build it or not.<br>
The compiled binaries will be placed in their respective BOARDS/... folders.<br><br>

If you want to build your own, clone or download the source.<br>
The source is stripped from ST own libraries and unnecesary stuff, only includes the very basic code owning to the project.<br>
CubeMX will add the STM32 and CMSIS libraries automatically after a code generation.<br>
Open the [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder, find your board (or take any to work with) and copy all the contents to the root of the project.<br><br>
Open STM32CUBE IDE, click on Import/Existing project and select the project folder.<br>
Important: Disable "Search for nested projects", select only the project in the root of the folder.<br>
After that, double-click on [STM32SolderingStation.ioc] file:<br>
<img src="/Readme_files/open_ioc.png?raw=true"><br><br>
CubeMX will open, then click on generate code:<br>
<img src="/Readme_files/gen.png?raw=true"><br><br>
After this, it'll be ready for compiling, click in the right arrow of the build button (Hammer icon) and select [Release]:<br>
<img src="/Readme_files/release.jpg?raw=true"><br><br>
After a while you'll have the compiled bin/hex files inside Release folder.<br><br>
If the build fails with files no found or undeclared functions errors, check the Include search path:<br>
Right click on project -> [Properties] -> [C/C++ Build] -> [Settings] ->  [Tool Settings] -> [MCU GCC Compiler] -> [Include paths]<br>
Select [All configurations] in [Configuration] dropdown menu.<br>
Now ensure these are present:<br>

      /Core/Inc
      /Core/Src
      /Drivers/generalIO
      /Drivers/graphics
      /Drivers/graphics/gui
      /Drivers/graphics/gui/screens
      /Drivers/graphics/u8g2
      /Drivers/STM32Fxxx_HAL_Driver/Inc
      /Drivers/STM32Fxxx_HAL_Driver/Inc/Legacy
      /Drivers/CMSIS/Device/ST/STM32Fxxx/Include
      /Drivers/CMSIS/Include
      
(STM32Fxxx matches your current mcu family, ex. STM32F0xx, STM32F1xx)<br><br>
If any is missing, click on Add... Select Workspace and select the missing ones.<br>
You can make multiple selection  while holding the Control key:<br>      
<img src="/Readme_files/Includes.jpg?raw=true">

At some point, the firmware might not fit into the flash when compiling for debugging, as it'll skip optimizations, and use much more space.<br>
In that case, you'll need to force some optimization level, starting with "Optimize for debug" (Og), and going to higher levels if still being too big (O1,O2,Osize).<br>
The settings can be changed in project Properties / Build / Settings / MCU GCC Compiler / Optimizations.<br>
However, when debugging, it's desirable to completely disable optimizations to see the program flow clearly.<br>
If you had to enable any level of global optimizations, you can still selectively disable build optimizations for any function.<br>
A line of code can be found at the start of main.h:<br>

  __attribute__((optimize("O0")))

Copy that line before a function to disable optimization, like this:<br>

   __attribute__((optimize("O0"))) void ThisFunctionWillNotBeOptimized(...)
   

If you want to retarget the project, avoid mixing different profile files.<br>
Run the included script "Clean_Profile.bat", or manually delete these files:<br>

    /Core/Inc/*stm32*
    /Core/Src/*stm32*
    /Core/Src/system_stm32*
    /Core/Startup/*

And then copy the board profile files overwriting any existing files.<br><br>

If you use an existing project template and modify it, the changes must be reflected in /Core/Inc/board.h.<br>
All the project code takes the data from there. The file it's pretty much self-explaining.<br>
So, any changes you make in CubeMX, ex. use PWM timer6 intead timer2, or SPI1 instead SPI2...all that should be configured in their respective define.<br>
As long as the GPIO names are called the same way, no further changes are needed.<br>
 
---

<a id="translations"></a>     
## Translations

For adding new languages, you have to modify these files:<br>
* Src/settings.h<br>
  LANGUAGE_COUNT, lang_english,lang_xxxx in system_types enum<br><br>
* Drivers/gui/gui_strings.c<br>
  strings_t strings, Langs[LANGUAGE_COUNT]<br>

For adding new characters to the existing fonts symbols, there're some instructions here:<br>
* Drivers/graphics/u8g2/tools/font/bdfconv/Notes.txt<br>

---
           
<a id="pending"></a>
### Non working features
* I2C eeprom. Some boards have it, some doesn't. So internal flash storage is used for all.<br>
Also, the current settings don't fit in the commonly used 24C08 memory.<br>
* RTC clock. There's very little space in the screen. Use it for what matters, instead for showing a clock!

---

<a id="docs"></a>
## Additional Documentation

* Dreamcat4 has made a great research and documentation of T12 and STM32 related stuff:
* **[Dreamcat4 documentation repo](https://github.com/dreamcat4/t12-t245-controllers-docs)**
* **[Backup of original PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)** 
