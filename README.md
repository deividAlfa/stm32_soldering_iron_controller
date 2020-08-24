# stm32_soldering_iron_controller

<!-- MarkdownTOC -->

* [Where are Docs?](#where-are-docs)
* [Compatibility](#compatibility)
* [Critique](#critique)
* [Backup First!](#backup-first)
* [Status](#status)
    * [Working](#working)
    * [Todo](#todo)

<!-- /MarkdownTOC -->

* Custom firmware for the chinese kseger soldering iron controller
* Original version by PTDreamer
* Updated branch - for v2.1s, from newer fork: `LuckyTomas`, flawless_testing` branch
* Merged to master, with Docs, and minor update by Dreamcat4

<a id="where-are-docs"></a>
## Where are Docs?

* Docs have been split out into seperate 'docs' repo. Because the size of assets kept ballooning in git
* This repo is just only for source code now. This is simply code fork of PTDreamer meant for v2.1s Hardware

Well which Docs are you looking for?

* **New Docs, Full Repo:** --> https://github.com/dreamcat4/t12-t245-controllers-docs
* **Backup of PTDreamer Blog** --> https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/research/ptdreamer
* Hardware Compatibility Docs --> https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/controllers/stm32-t12-oled
* **Docs for Compiling / Flashing with ST Official Toolchain** --> https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/tools/software/STM32CubeIDE
* *Docs for Compiling / Flashing with VSCode and PlatformIO* - TBD. Not done.

<a id="compatibility"></a>
## Compatibility

Moved to:

* Hardware Compatibility Docs --> https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/controllers/stm32-t12-oled

This fork is meant for `v2.1s` versions of hardware. And maybe even some `v3.x` who are actually rebranded `2.x` underneath. Mis labeled.

However there are many different versions of 'v2.1S' PCB hardware. Non original PCBs ***might*** work, to a greater or lesser extent. We do not know enough about them yet.

Please read carefully the [Hardware Compatibility Docs](https://github.com/dreamcat4/t12-t245-controllers-docs/tree/master/controllers/stm32-t12-oled). Be sure to:

* **Check hardware before Buying**
* **Check hardware before Flashing**

Otherwise you might end up with incompatible hardware. Newer hardware is often inferior! With bad low quality component(s) such as:

* Bad Op-Amp
* Bad 3v3 Regulator

Which can then result in bad performance / a bad temperature regulation.

<a id="critique"></a>
## Critique

It has been observed (by others) that:

* PTDreamer Open Source Firmware might be inferior. To the CFW firmwares (closed source)
* PTDreamer Open Source Firmware might be buggy. To the CFW firmwares (closed source)
* PTDreamer Open Source Firmware might be missing features. To the CFW firmwares (closed source)

It is not clear exactly what the situation is. So make sure to replace MCU before flashing. To preserve the CFW. In case you want to get back to the Original Commercial Firmware (CFW).

* Perhaps some of these issues are because of bad hardware
* Perhaps some of these issues are because of early PTDreamer version (some new code now, some stuff is improved!)

Otherwise there may be some kind of issue or problem, that's worse for this Open Source version... So you have been warned to replace (backup) the MCU! See next section below:

<a id="backup-first"></a>
## Backup First!

The simplest way to backup is actually to buy a new MCU. Then remove the original MCU with hot air, or chipquik, or bizmuth low temperature solder. And set it aside in case the Open Firmware here fails to work. Since there are so many versions of the hardware PCB. KSGER and friends keeps changing them all the time.

Unfortunately we cannot backup the firmware with programmer PC link. This is because the program in the MCU memory is security protected. It cannot be read fully. So the original commercial firmware cannot be backup over SWD protocol, or JTAG whatever. Not for these STMF1xx series MCUs. Sorry to say that. 

<a id="status"></a>
## Status

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








