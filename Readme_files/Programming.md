## Programmer

Get a cheap ST-Link clones (Around $4), refer to the schematics for the SWD pinout.<br>
There's no support for custom bootloaders.<br>

## Connection

Usually the board will have a connector with pn named "G V C D" (GND, VDD,SWCLK, SWDIO).<br>
If not named, just use a multimeter and check which pins goes where.<br>
Then connection is as follows.<br>
**Connect VDD only if not being powered by station power supply!**<br>
<img src="/Readme_files/stlink_connection.jpg?raw=true"><br>

## Programming with STM32 ST-LINK Utility

Download the binary **STM32SolderingStation.bin** for your station (Listed in the Readme), also found in [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder, and flash it using ST-Link.<br>

### Coming from original firmware

Usually the STM32 comes read-protected. Follow this workflow to program it.<br>
(Click for bigger picture)<br>
<img src="/Readme_files/st-link_programming.png?raw=true"><br>

### Upgrading

Just open and program the new binary.<br>
Because this firmware stores the settings in the flash, don't make a full chip erase!<br>
<img src="/Readme_files/upgrade.png?raw=true"><br>

In any case, the firmware will check the settings and reset them if not valid.<br>

### STM32 not recognized error
Some STM32 have the debug port disabled, in this case the only way to access is connecting nRST pin to the ST-Link.<br>
There're several ST-Link clones with a flaw in nRST pin, which is wired wrong by design, and won't able to connect.<br> 
Temporaly short nRST (STM32 pin 7) to gnd, click "connect" button, wait 1-2 seconds and release nRST, now it should recognice it.<br>
Usually nRST is connected to a capacitor, only requiring to short the capacitor as showed in this picture:<br>
<img src="/Readme_files/stlink_force_rst.jpg?raw=true"><br>

## Programming with **OpenOCD**

1. Download the binary **STM32SolderingStation.bin** for your station (Listed in the Readme), also found in [BOARDS](https://github.com/deividAlfa/stm32_soldering_iron_controller/tree/master/BOARDS) folder. Rename it **fw.bin**
1. Install [OpenOCD](https://openocd.org/)<br>
   If running Ubuntu/Debian: sudo apt install openocd<br>
1. create a file with the following content or [download it](/ocd-program.cfg?raw=true)
```
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]

# Set RDP to level 0

init
reset halt
stm32f1x unlock 0
reset halt

#Program
program fw.bin 0x08000000
exit
```
1. Connect the st-link programmer and from the console
```
$ openocd -f ocd-program.cfg 
Open On-Chip Debugger 0.11.0-rc2
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
Info : auto-selecting first available session transport "hla_swd". To override use 'transport select <transport>'.
Info : The selected transport took over low-level target control. The results might differ compared to plain JTAG/SWD
Info : clock speed 1000 kHz
Info : STLINK V2J37S7 (API v2) VID:PID 0483:3748
Info : Target voltage: 3.232297
Info : stm32f1x.cpu: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for stm32f1x.cpu on 3333
Info : Listening on port 3333 for gdb connections
target halted due to debug-request, current mode: Thread 
xPSR: 0x01000000 pc: 0x080033f0 msp: 0x20002800
Info : device id = 0x20036410
Warn : STM32 flash size failed, probe inaccurate - assuming 128k flash
Info : flash size = 128kbytes
target halted due to debug-request, current mode: Thread 
xPSR: 0x01000000 pc: 0x080033f0 msp: 0x20002800
target halted due to debug-request, current mode: Thread 
xPSR: 0x01000000 pc: 0x080033f0 msp: 0x20002800
** Programming Started **
** Programming Finished **
** Verify Started **
** Verified OK **

```
