# STM32 Soldering Station Firmware

<!-- MarkdownTOC -->

* [Project details](#project-details)
* [Compatibility](#compatibility)
* [Bug reporting](#bug-reporting)
* [Programming](Readme_files/Programming.md)
* [Operating instructions](Readme_files/Operation.md)
* [Frequently asked questions](#frequently-asked-questions)
* [Building the firmware](Readme_files/Build.md)
* [Translations](#translations)
* [Creating a .ioc file from scratch](Readme_files/Creating_ioc.md)
* [Non-working features](#non-working-features)
* [Additional Documentation](#additional-documentation)

<!-- /MarkdownTOC -->

If you liked the firmware, you can send me a beer with [PAYPAL](https://www.paypal.me/davidalfistas) 🙂

---

Video of operation here: (Project in active development, the features will change continuously)<br>

[![STM32 T12 custom firmware](https://img.youtube.com/vi/j0HQq4aRiXw/0.jpg)](https://www.youtube.com/watch?v=j0HQq4aRiXw "STM32 T12 custom firmware")


## Project details
* This project started by forking [PTDreamer's firmware](https://github.com/PTDreamer/stm32_soldering_iron_controller). Since then it became a separate project.
* Developed on STM32Cube IDE v1.12.1. Basic configuration made in CubeMX (Included in CubeIDE).<br>
ST has removed this version from their site, so [here's a download mirror](https://github.com/deividAlfa/stm32_soldering_iron_controller/releases/tag/CubeIDE_v1.12.1). 
* Unified codebase, different hardware support based on profiles, very few files need to be changed.
* Supports all display modes: I2C, SPI, software and hardware+DMA (When connected to hardware pins).
* Uses U8g2 graphics library.
* Dynamic screen loading to save RAM resources.
* Extremely customizable, lots of options available.
* Code highly optimized to avoid wasting CPU power, slow devices still run great.
* Check [Releases](https://github.com/deividAlfa/stm32_soldering_iron_controller/releases) for downloads.


## Compatibility

Check [Boards readme](Readme_files/boards.md) for quick board identification.<br>
Visit [Dreamcat4 T12 controllers](https://github.com/dreamcat4/t12-t245-controllers-docs) for more pictures and schematics.<br>
[BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder contains the build profiles.<br>
**KSGER Combo station is not supported!**<br><br>
The actual requirements are 10KB RAM and 64KB **(\*)** flash.<br>
**(\*)** Currently the firmware has surpassed the 64KB limit, and uses the additional undocumented 64KB flash block.<br>
**(\*)** All 64KB devices have 128KB, with the second 64KB block untested from the factory, so not guaranteed to work.<br>
**(\*)** To date, I have found zero issues. Original KSGER firmware also does this.<br>
**(\*)** ST-Link checks the written data and the firmware uses CRC for settings, any error will be detected.

**Clones / fakes:** <br>
Some controllers are using fake STM32 or compatibles, sometimes relabeled as genuine STM32, causing problems.<br>
Check [STM32 clone detection](Readme_files/Programming.md#clone-detection) section to find out how to detect a genuine STM32.<br>
- The only known working clone is CKS32.
- GD32, MM32 and CH32 have issues with the ADC converter.
- APM32 hasn't been tested yet.

Some issues caused by clones:<br>
- Showing strange values in temperatures and voltage (ADC DMA issue), and/or getting NTC High/Low error even when NTC is disabled.
- Bootlooping / freezing / dying or going black after the initial setup screen (Uncompatible flash layout).
- Hardfault, Checksum Error bootloop.

Clones bring all kind of issues and there're too many of them, lots are relabeled and they even copy a genuine STM32 device ID, so in some cases it's almost impossible to figure out the actual device.<br>
Some fakes worked well until recently, when the flash storage layout was updated.<br>
The older [**v1.10.8**](https://github.com/deividAlfa/stm32_soldering_iron_controller/releases/tag/v1.10.8) release is more compatible with fake devices, try it out in case you suspect having one.<br>
No effort will be done to support fake / clones!<br>
<br> 

If your board came with a fake/clone, you can replace it with a STM32F103, they're pin-compatible:
* **48-pin**: STM32F103C8T6, STM32F103CBT6.
* **64-pin**: STM32F103R8T6, STM32F103RBT6, STM32F103RCT6, STM32F103RDT6, STM32F103RET6.
<br>

Currently supported controllers:
* **Quicko T12-072**: First gen Quicko, STM32F072 variant. Compatibility issues were fixed since v1.04.
* **Quicko T12-103** First gen Quicko, same board but mounting a STM32F103.
* **KSGER v1.5**: Profile for STM32F103 (There are no other known CPUs used in this board).
* **KSGER v2**,   **JCD T12**, **T12-955**, **Handskit**: Profile compatible with all STM32F101/2/3xx models.
* **KSGER v3**,   **T12-958**: Profile compatible with all STM32F101/2/3xx models.
* **T12-958 v2**: Profile compatible with STM32F103. Needs a [mod](https://raw.githubusercontent.com/deividAlfa/stm32_soldering_iron_controller/master/BOARDS/Quecoo/vbat_mod.jpg) for battery to work.

Don't follow the version reported in the original firmware to identify your board.<br>
To this day, the easiest way to quickly identify your controller version is by checking the OLED screen connection:
- **4 pin** (I2C) = Generic v2 (KSGER/Quecoo/Handskit/etc.)
- **6 pin** (SPI) = Generic v3
- **7 pin** (SPI) = Only used by KSGER v1.5 or first gen Quicko, easy to differentiate.

For KSGER v2/v3: As long as use the correct firmware, any STM32 variant (101/102/103/C8/R8/CB/RB) will work.<br>
There are several compatible/cloned boards in the market that will work fine with KSGER profiles.<br>

T12-951, T12-952, T12-956, T12-959 use STC MCU, not supported by this firmware.


## Bug reporting
If you encounter any error or bug:<br>
- Ensure your STM32 is genuine, check [STM32 Clones](Readme_files/Programming.md#clone-detection).
- Throughly read the readme and manual and check if your issue is explained somewhere.
- If still no solution for it, open a new [`issue`](https://github.com/deividAlfa/stm32_soldering_iron_controller/issues) specifying: 
  - Firmware version (v.XX) and type (KSGER V2, etc).
  - If showing a screen with "Error: file xx.c, line n", mention this or attach a picture.
  - If getting a Hardfault, attach a picture of the data shown on the screen.<br>
    If you want to check yourself, check the `PC` address shown in the screen and search it in the listing file (.list).<br>
    This will show where it happened in the program.
- You might also ask at [Eevblog](https://www.eevblog.com/forum/reviews/stm32-oled-digital-soldering-station-for-t12-handle/) (English), [4PDA](https://4pda.to/forum/index.php?showtopic=1009098) (Russian), [Radiokot](https://www.radiokot.ru/forum/viewtopic.php?t=178399) (Russian) forums.    


## Frequently asked questions

First, make sure to read the [Operating Instructions](Readme_files/Operation.md)!

### Changelog
You can check the [commit history](https://github.com/deividAlfa/stm32_soldering_iron_controller/commits/master) to see what have been changed between builds.

### Backing up the original firmware
The original firmwares are available [[HERE]](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/Original_FW)<br>
Some KSGER firmwares require an activation code which can be generated with these keygens:<br>
    [`gen.htm`](https://rawcdn.githack.com/deividAlfa/stm32_soldering_iron_controller/3f48a9c4c9586f89503ce763b1c6a73b9b73b55a/Original_FW/KSGER/Gen/gen.htm) [`stm32-ss-keygen-drz.py`](https://raw.githubusercontent.com/deividAlfa/stm32_soldering_iron_controller/master/Original_FW/KSGER/Gen/stm32-ss-keygen-drz.py).

Be warned, usually the MCU will be read-protected, so you won't be able to read its contents, only erase it.<br>
The simplest way to not loose the original firmware is actually to buy a new MCU, replace it, and store the original MCU in a safe place.<br>
Any difference in the pinout will require firmware tuning, although one of the main proposits of this firmware is easing that.<br>
There are some vulnerabilities that can be used to backup protected firmware:
- [STM32 power glitching timing attack](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE#option-2-power-glitching-timing-attack).
- [PicoPwner](https://github.com/CTXz/stm32f1-picopwner/). Tested, works very well.

### Battery mod
Some commonly changed settings (Temperature setpoint, tip/profile selection) can be saved to the RTC SRAM, reducing flash wear.<br>
The RTC needs a battery connected to STM32's VBAT pin, most boards have a battery connector for this.<br>
- Install a 3.3V cell battery. Some boards have a resistor connected between VBAT pin and GND, will drain the battery, remove it.<br>
- Enable [`SYSTEM`](Readme_files/Operation.md#system) > `Battery`.

### Display issues
If the display has right/left line or is vertically shifted:<br>
- Go to [`System`](Readme_files/Operation.md#system) > `Display` menu.<br>
- Adjust `X/Y Offsets` until it's centered.<br>
<img src="/Readme_files/oled_offset.jpg" width="320">

### Temperature unstability
By default, never modify any PWM or Delay settings in the [`Iron`](Readme_files/Operation.md#iron) menu. Doing so may cause such issues.<br>
Also, new tips are often unstable, leading to temperature jumps.<br>
Don't try to calibrate the tip in this state, neither set a high temperature, because it could go under control.<br>
They usually settle after some burn-in time. It's recommended to set a medium temperature (250-300ºC) and leave it like that for 15-20 minutes until it stabilizes.<br>
If the temperature is still unstable, try increasing the `Iron` > `Delay` value, allowing more time for the temperature signal to settle.<br>
A damaged, loose or defective connection in the handle will also cause this issues. Ensure the contacts are clean.<br>
There have been problems with some board/stations like:<br>
* Noisy power supply
* Broken / badly soldered capacitors
* Bad Op-Amp
* Bad 3v3 regulator

If you're getting "NTC high/low" error even when disabling the NTC in settings, then your STM32 is fake.<br>
Check Clones in [Compatibility](#compatibility) section and `Clone fix` option in [`System menu`](https://github.com/deividAlfa/stm32_soldering_iron_controller/blob/master/Readme_files/Operation.md#system).<br>


### Temperature accuracy
Buying a cheap high temperature meter is highly recommended!<br>
These boards can have pretty different readings and tolerances. Even between T12 tips.<br>
So the factory calibration is intentionally set lower than real, to avoid possible overheating problems.<br>
Once you set the firmware, go to calibration and set there the real temperature measured with the external probe.

### Calibration issues
Ensure to read [Calibration](Readme_files/Operation.md#calibration) first!<br>
To calibrate, go into `Calibration` > `Start`.<br>
Attach the temperature probe before proceeding!<br>
If the difference between measured and real is higher than 50ºC, the calibration will be aborted, telling you to go into `Calibration` > `Settings` and manually adjust the values.<br>
The calibration settings menu has 3 calibration options: `Zero set`, `Cal 250ºC` and `Cal 400ºC`.<br>
When you edit 250 or 400ºC value, the power will be enabled and the value applied in real time, so be careful!<br>
The power will be removed when no settings are being edited.<br>
Adjust each value until it's close to the target temperature. Repeat for each step and save.<br>
Those values are only used by the calibration process, to prevent burning the tip if your board reads too low.<br>
After adjusting, repeat calibration, this time it should work correctly.<br>
The calibration results for the current tip can be seen in the tip settings menu.<br>
Tip settings menu calibration values aren't meant to be another calibration menu, only for viewing (ex. reporting calibration results) and for backup/restore purposes.<br>
In case you lose, wipe or reset the data, you can go back into that menu and adjust the values based on previous calibration results.<br>
Zero calibration can't be manually restored, but it only takes few seconds to adjust.

### Cold tip not showing ambient temperature
Some amplifiers can introduce a small voltage offset that will translate into the cold tip reading 30-50°C higher than ambient temperature.<br>
To fix that, follow this order exactly!<br>
Tip power is removed in Calibration menu, inserting the tip before will heat it up and make cold calibration impossible.<br>
Enter the [`Calibration`](Readme_files/Operation.md#calibration) menu, **insert a completely cold tip now**, enter `Settings`, adjust `Zero set` calibration and save.<br>
After that, the offset will be compensated and the cold temperature will be normal.<br>
It's highly recommended to recalibrate after changing this value.

### KSGER self-resetting
Some KSGER controllers use a linear regulator to convert 24V to 3.3V, which is a very bad design and generates a lot of heat.<br>
With the OLED displays, each pixel turned on consumes more power, and this firmware uses a larger font for displaying the temperature.<br>
Thus, this firmware uses some more power. The design is so bad that the regulator will overload and shut down, resetting the board.<br>
There're some options to fix this:<br>
- Lower the display brightness to reduce the power consumption.
- Put a 100-150Ω 2W resistor in series with the regulator (24V->Resistor->LDO input). The resistor will drop part of the voltage and reduce the stress on the regulator.
- Replace the LDO with a better one or modify the board, adding a small heatsink to take away the heat.
- Use a small DC/DC step-down module to convert 24V to 5V, and feed 5V to the 3.3V LDO (best option, barely makes any heat).

### Other issues
After fully reading the documentation, if you still have problems or doubts, there're several forums with threads about this firmare:<br>
- [Eevblog](https://www.eevblog.com/forum/reviews/stm32-oled-digital-soldering-station-for-t12-handle/) (English).
- [4PDA](https://4pda.to/forum/index.php?showtopic=1009098) (Russian).
- [Radiokot](https://www.radiokot.ru/forum/viewtopic.php?t=178399) (Russian).


## Translations
For adding new languages, you have to modify these files:<br>
* [`Core/Inc/settings.h`](/Core/Inc/settings.h)<br>
  - increment the value of `LANGUAGE_COUNT_WITH_CHINESE` by one, 
  - add your language identifier (`lang_xxxx`) in `languages_t` enum.
* [`Drivers/graphics/gui/screens/gui_strings.c`](/Drivers/graphics/gui/screens/gui_strings.c)<br>
  - copy the whole `[lang_english] = { … }` section at the bottom of `strings_t` strings,
  - replace `lang_english` with `lang_xxxx` of the section you just copied and translate,
  - add your `lang_xxxx` to `Langs` near the bottom of the same file.<br>

For adding new characters to the existing fonts symbols, there're some instructions here:
* [Font build README](/Drivers/graphics/u8g2/tools/font/generate_fonts/README.md)


## Non-working features
* I2C EEPROM - some boards have it, some doesn't. So internal flash storage is used for all.
Also, the current settings don't fit in the commonly used 24C08 memory.
* Realtime clock - there's very little screen space. Use it for what matters, instead for showing a clock!


## Additional Documentation
@Dreamcat4 has made a great research and documentation of T12 and STM32 related stuff:
* [Dreamcat4 documentation repo](https://github.com/dreamcat4/t12-t245-controllers-docs)
* [Backup of original PTDreamer Blog](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer)
