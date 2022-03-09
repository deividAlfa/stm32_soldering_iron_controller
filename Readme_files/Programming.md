
## Programming

Get a cheap ST-Link clones (Around $4), refer to the schematics for the SWD pinout. There's no support for custom bootloaders.<br>
Download the binary **STM32SolderingStation.bin** for your station (Listed in the Readme), also found in [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder, and flash it using ST-Link.<br>

### Coming from original firmware

Usually the STM32 comes read-protected. Follow this workflow to program it.<br>
(Click for bigger picture)<br>
<img src="/Readme_files/st-link_programming.png?raw=true"><br>

### Upgrading

Because this firmware stores the settings in the flash, doing so when updating to a newer version would wipe the settings.<br>
Follow this pictures to keep them when updating. Don't do this when coming from original firmware!<br>
Important: STM32F072 has 2KB flash sector size, so only de-select the last sector.<br>
(Click for bigger picture)<br>
<img src="/Readme_files/stlink_upgrade_erase.png?raw=true"><br>
<img src="/Readme_files/stlink_upgrade_program.png?raw=true"><br>

In any case, the firmware will check the settings and reset them if not valid.<br>
