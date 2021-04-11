# Operating Instructions<br>

## Initial Startup<br>
When an unconfigured (new or fully reset) controller starts, it first offers a choice of which tip profile to use.
Select the one appropriate for your iron. (JBC cartidges usually require electrical changes to the controller)<br>

* Hakko __T12__ (T15 Series in North America & EU) This is the handle normally used with these controllers.<br>
* JBC __C210__<br>
* JBC __C245__<br>

## Basic Controls<br>
Changes are made using the rotary encoder and its push button.<br>
On the main screen, a quick press will alternate between a tip temperature display and a graph showing the recent history of tip temperature.<br>
The power supply voltage is shown upper left, room temperature in the upper right, bar graph of relative power going to the tip on the bottom.<br>

Rotating the encoder will change the desired tip temperature, which will then quickly revert to showing the measured temperature.<br>
Press and turn clockwise (while pressed) to choose the tip to be used (more on that later).<br>
Press and turn anticlockwise (while pressed) to force entering sleep mode.<br>
A long press will enter the system menu. The controller will return to the main screen with another long press, scrolling and selecting "BACK" from each menu, or 15 seconds of inactivity.<br>
To operate the menus, rotate to scroll to the desired selection, quick press to select it, rotate to change. Quick press again to exit the setting.<br>

## Theory of Operation<br>
Before covering the settings, it is useful to understand how it works.<br>
Tip temperature is measured by using an ADC (Analog-Digital Converter) to measure the voltage output by a thermocouple in the tip.<br>
The voltage is basically proportional to the difference between tip temperature and room temperature.
The controller uses PWM (Pulse Width Modulation) to control the tip temperature.<br>
PWM basically varies the amount of time that the tip is powered. By default, PWM uses a 200 ms (5 times per second) period.<br>
If the tip is too hot, the controller will turn on power to the tip little or not at all during each cycle.<br>
If the tip is too cold, it will turn the power on for more of the period.<br>
The time the tip is powered on during each period is called the duty cycle.<br> 
<pre>
|<------ PWM PERIOD ------>|
| POWER |____ no power ____|
</pre>
The PID (Proportional, Integral, Derivitive) algorithm determines the PWM duty cycle based on the difference between desired and measured tip temperatures.<br>

## Menus<br>

### PID

The PID menu allows fine tuning of the algorithm.<br>
PID tuning is an advanced topic, _**incorrect settings here can result in instability and damage to tips and possibly the controller**_.<br>
Most users should not change these settings, but here are the basics. Kp, Ki, and Kd are the coefficients which control the PID's behavior. In simple terms:
  * #### _Kp_ 
The proportional term, changes the PWM duty cycle based on how far the measured temperature is from the desired temperature.<br>
  * #### _Ki_ 
The integral term, changes the duty cycle based on how long the temperatures have been different.<br>
  * #### _Kd_ 
The differential term, changes the duty cycle based on how fast the measured temperature has changed.<br>

### IRON
Iron settings control the operation of the handle/tips. <br>
  * #### _Sleep_
If there is no soldering activity for this period, the controller will "sleep" and stop providing power to the tip, allowing it to cool. This helps increase tip lifetime. Activity (e.g. shaking the handle for a T12) will wake it up and heating will resume.<br>
  * #### _Heater R(esistance)._
The resistance of the tip's heating element. There is normally no need to change this from the default.<br>
  * #### _Power_
The maximum power which will be delivered to the tip. This sets a maximum for the PWM duty cycle, based on the power supply voltage and the heater resistance.<br>
  * #### _PWM Time_
Sets the PWM period. The controller will check and adjust the tip temperature once each period. Default 200 ms.<br>
  * #### _ADC Delay_
Near the end of each PWM period, the temperature is measured by the ADC.<br>
_ADC Delay_ controls how soon before the end of a period the temperature is measured.<br> 
The measurement can only be taken when the power to the tip is off, so the effective PWM duty cycle is limited to (_PWM Time_ - _ADC Delay_).<br>
Default 20 ms.<br>
<pre>
|<----------------- PWM TIME -------------------->|
|_____________________________|<-DELAY->[ADC READ]| NEXT CYCLE
|<- POWER ->|_____________________________________|
</pre>
  * #### _Filtering_
Used to filter the temperature measurements before they are passed to the PID. This helps remove noise and provides more stability.<br>
   * * ##### _Avg_
This uses a simple moving average, and is the default filter.<br>
   * * ##### _EMA_
Exponential Moving Average. A more sophisticated filter.<br>
  * #### _Factor_
Only for EMA. The higher, the heavier the filtering (also more delay).<br>
  * #### _No iron_
The ADC reading which signals that no iron is present. When no iron is plugged in, the measured temperature will read at or near maximum (ADC = 4095).
Default 4000, max 4100 (Over 4095 will disable "no iron" detection).<br>
  * #### _Detection_
Time in miliSeconds that an iron must be plugged in before it is considered present.<br>

### SYSTEM
General settings for the controller.<br>
  * #### _Profile_
Sets which iron profile (__T12__, __C210__, __C245__) to use.<br>
  * #### _Contrast_
Screen Contrast/brightness.<br>
  * #### _Offset_
Screen offset. This can accomodate the different screens which the controllers have come with. Use it to center the display on the screen. Default = 2.<br>
  * #### _Encoder_
If rotating the encoder moves in the wrong direction, change this.<br>
  * #### _Boot_
Operation mode when powered on. __RUN__ or __SLEEP__.<br>
  * #### _Wake mode_
How to detect activity. SHAKE or STAND.<br>
SHAKE uses a motion sensor present in T12 handles, shake or hold the handle tip up to wake.
STAND can use the same handle shake wire, but must be disconnected from the handle and connected so when the tip is in the stand, this wire is shorted to GND. Shorted = sleep, open = wake.<br>
  * #### _Btn. Wake_
Allow waking the controller by pressing the encoder button.<br>
  * #### _Buzzer_
Buzz/beep when notable conditions occur.<br>
  * * Changing operating mode (sleep, run)<br>
  * * Temperature reached after the setpoint was changes<br>
  * * Alarm when no iron is detected or system error happens<br>
  * #### _Unit_
Temperature scale, Celsius or Fahrenheit<br>
  * #### _Step_
Temperature step when adjusting tip temperature.<br>
  * #### _GUI Time_
To offer maximum respoiveness, , the screen is updated between few 10s to more than 100 times per second (depends on the MCU used and display interface).<br> 
If the display reading were updated at the same speed, it would be impossible to read anything.<br> 
This setting defines the time in mS that the main screen readings are updated (voltage, temperatures).<br>
The real update rate will be limited by the PWM frequency (ADC is read at the every PWM cycle end),<br> 
Use a higher setting for less "flicker" in the display (more steady values).<br>
  * #### _Save time_
Defines the delay with no changes before storing changed settings in flash memory.<br>
Flash has a limited number of write cycles (~100,000).<br>
Higher values reduce writes, but settings changes could be forgotten when the controller is powered off or reset.<br>
Default: 5 seconds.
  * #### _RESET MENU_
Reset various configuration sections:<br>
  * * ##### _Settings_
Reset Settings menu items to default.<br>
  * * ##### _Profile_
Reset the current profile (iron/tips) to default.<br>
  * * ##### _Profiles_
Reset all profiles to default.<br>
  * * ##### _All_
Reset everything.<br>

  * #### _SW:_
Displays the current software version. The version number is from a hash - higher is not necessarily newer.<br>
  * #### _HW:_
Displays the hardware type.<br>

### EDIT TIPS
Different tips may have different characteristics. Tips may be added or edited here.<br>
Select a tip to edit, or add new to create and name a new tip.<br>
The new tip will be created by copying the PID/calibration settings from the first tip listed.<br>
To edit any tip settings, first select it: Go to the main display, press and turn clockwise to select the tip.<br>
Individual tip data includes info from both the *IRON* and *PID* menus.<br> 

### CALIBRATION
  * #### _Start_
Requires a tip thermometer (e.g. Hakko FG-100 or similar). Calibrates the current tip at temperatures of 250, 350 and 450C.<br>
Wait for tip temperature to settle, then enter temperature as measured by the thermometer for each step.<br>
If the entered temperature is more than 50ºC higher than the target calibration value, the process will be aborted and you will have to adjust it manually<br>
* #### _Adjust_
Here you can adjust the default calibration values. For every step (250,350,450ºC) adjust the value until it's close to the target temperature.
Click on save to apply and store the changes, or cancel to discard.<br>
The Save button will be hidden if no changes were made, or the entered data is invalid.<br>


### ERROR REPORTING
A lot of effort was done to protect the tips from overheating.<br>
Any detected error will disable PWM and show a message on the display.<br>
There are multiple error types:<br>
* #### _Iron warning_
Non critical errors, a warning will be shown about iron not connected, supply voltyage too low, ambient temperature too high or too low.<br>
* #### _Iron runaway_
If by any means the iron temperature is higher than requested and the PWM is still active, it will trigger a timer depending on the temperature diference.<br>
The condition must dissapear in the specified time, otherwise it will trigger a critical runaway error a lock the station, shutting down the power stage<br>
* #### _Internal function errors_
If any internal function detects undefined or not expected state, it will also lock the station and show a message trying to show where the error happened (File, line).<br>
* #### _Hardware exceptions_
If a hardware exception happens, it will also lock the station and try to show an error message.<br>
 * #### _Data error detection_
The data is stored as separate blocks: System settings, profile 1, profile 2 and profile 3.<br>
Each one has it's own CRC checksum. Everytime the data is read, the checksum are computed and compared.<br>
If a mismatch happens, it will erase that block, trying to preserve the rest.<br>
An error will be shown, detailing if the error detected belongs to the system settings data, or any of the profiles.<br>
Also, the flash storage is checked carefully before and after writes, any issue will trigger a flash error message.<br>
 
 ### HARD RESET
If for any reason the station is unable to boot, you can't access the reset menu or you want to reset everything up quickly, there's a hard reset method.<br>
Power the station off, push and hold the button, then power the station on.<br>
A message will appear in the screen. "Hold button to restore defaults".<br>
Keep pressing the button for another 5 seconds, until the next message appears, "Release button now".
Release the button, the station will reset and wipe everything.<br>

If you accidentally pushed the button, just release it before the 5 second timeout to resume the boot process. 