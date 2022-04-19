## Programmer

Get a cheap ST-Link clones (Around $4), refer to the schematics for the SWD pinout.<br>
There's no support for custom bootloaders.<br>

## Connection

Usually the board will have a connector with pn named "G V C D" (GND, VDD,SWCLK, SWDIO).<br>
If not named, just use a multimeter and check which pins goes where.<br>
Then connection is as follows.<br>
**Connect VDD only if not being powered by station power supply!**<br>
<img src="/Readme_files/stlink_connection.jpg?raw=true"><br>

## Programming

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

### STM32 not recognized error
Some STM32 have the debug port disabled, in this case the only way to access is connecting nRST pin to the ST-Link.<br>
There're several ST-Link clones with a flaw in nRST pin, which is wired wrong by design, and won't able to connect.<br> 
Follow this instructions for manual reset method.<br>
<img src="/Readme_files/stlink_force_rst.jpg?raw=true"><br>
