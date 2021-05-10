# Operating Instructions<br>

## Initial Startup<br>
When an unconfigured (new or fully reset) controller starts, it first offers a choice of which tip profile to use.<br>
Select the one appropriate for your iron. (JBC cartidges usually require electrical changes to the controller).<br>

- **Hakko T12** (T15 Series in North America & EU) This is the handle normally used with these controllers.<br>
- **JBC C210**<br>
- **JBC C245**<br>

---

## Basic Controls<br>
Changes are made using the rotary encoder and its push button.<br>
The power supply voltage is shown upper left, room temperature in the upper right, bar graph of relative power going to the tip on the bottom.<br>
While in normal operation, shake sensor activity will be shown briefly in the top center.<br>
To operate the menus, rotate to scroll to the desired selection, quick press to select it, rotate to change.<br>
Quick press again to stop editing the setting.<br>
Widgets have fine and coarse adjustment.<br>
While editing the widget, normal rotation will make fine adjustement. Click and rotate will do coarse adjustment.<br>

---

## Theory of Operation<br>
Before covering the settings, it is useful to understand how it works.<br>
Tip temperature is measured by using an ADC (Analog-Digital Converter) to measure the voltage output by a thermocouple in the tip.<br>
The voltage is basically proportional to the difference between tip temperature and room temperature.
The controller uses PWM (Pulse Width Modulation) to control the tip temperature.<br>
PWM basically varies the amount of time that the tip is powered.<br>
By default, PWM uses a 200 ms (5 times per second) period.<br>
If the tip is too hot, the controller will turn on power to the tip little or not at all during each cycle.<br>
If the tip is too cold, it will turn the power on for more of the period.<br>
The time the tip is powered on during each period is called the duty cycle.<br> 
<pre>
|<------ PWM PERIOD ------>|
| POWER |____ no power ____|
</pre>
The PID (Proportional, Integral, Derivative) algorithm determines the PWM duty cycle based on the difference between desired and measured tip temperatures.<br>

---

## Main screen<br>

  - **Temperature display modes**<br>
While in run mode, a single click will switch between numeric and graph (10 second history).<br>
  - **Temperature setpoint adjustment**<br>
Rotate the encoder, the setpoint will be shown, continue rotating to adjust it.<br>  
After 1 second of inactivity it will return to normal mode.<br>
  - **Sleep mode**<br>
You can force entering sleep mode by clicking and rotating counter-clockwise.<br>  
To wake up you can make a click (If button wake is enabled) or move the handle (If shake wake is enabled or in stand mode).<br>
If the display brighntess is dimmed, you can wake up the display by rotating the encoder.<br>
If the tip reading is higher than 120ºC, it will show a "HOT!" warning.<br>
  - **Tip selection**<br>
Click and rotate clockwise to show the tip selection. Then, long-pressing will enter the selected tip settings.<br>  
It will return to normal mode after 5 seconds of inactivity.<br>
  - **System menu**<br>
A long click will enter the system menu (Except while in tip selection).<br>

---

## System menu<br>
Most screens will return to main screen after 15 seconds of inactivity.<br>
Also, long clicking will have the same effect.<br>

### IRON

Iron settings control the operation of the handle/tips. <br>
  - **Max temp**<br>
Upper adjustable temperature limit.<br>
  - **Min temp**<br>
Lower adjustable temperature limit.<br>
  - **Sleep**<br>
If there is no soldering activity for this period, the controller will "sleep" and stop providing power to the tip, allowing it to cool. This helps increase tip lifetime. Activity (e.g. shaking the handle for a T12) will wake it up and heating will resume.<br>
  - **Heater ohm**<br>
The resistance of the tip's heating element. There is normally no need to change this from the default.<br>
  - **Power**<br>
The maximum power which will be delivered to the tip. This sets a maximum for the PWM duty cycle, based on the power supply voltage and the heater resistance.<br>
  - **PWM Time**<br>
Sets the PWM period. The controller will check and adjust the tip temperature once each period. Default 200 ms.<br>
Lower values will increase the PWM frequency and also the audible switching noise.<br>
  - **ADC Delay**<br>
Near the end of each PWM period, the temperature is measured by the ADC.<br>
_ADC Delay_ controls how soon before the end of a period the temperature is measured.<br> 
This delay is needed to have a clean reading of the thermocouple. If the delay is too low, it will read switching noise and be very unstable.<br>
The measurement can only be taken when the power to the tip is off, so the effective PWM duty cycle is limited to (_PWM Time_ - _ADC Delay_).<br>
If you get random spikes in the temperature reading, try increasing this value. 20mS is usually more than enough.<br>
There are other factors that could cause unstability, like poor circuit design, power supply noise or bad quality parts.<br>
Default 20 ms.<br>
<pre>
|<----------------- PWM TIME -------------------->|
|_____________________________|<-DELAY->[ADC READ]| NEXT CYCLE
|<- POWER ->|_____________________________________|
</pre>
  - **Filtering**<br>
Used to filter the temperature measurements before they are passed to the PID. This helps remove noise and provides more stability.<br>
     - **Avg**<br>
This uses a simple moving average, and is the default filter.<br>
     - **EMA**<br>
Exponential Moving Average. A more sophisticated filter.<br>
  - **Factor**<br>
Only for EMA. The higher, the heavier the filtering (also more delay).<br>
  - **No iron**<br>
The ADC reading which signals that no iron is present. When no iron is plugged in, the measured temperature will read at or near maximum (ADC = 4095).
Default 4000, max 4100 (Over 4095 will disable "no iron" detection).<br>
  - **Detection**<br>
Time in miliSeconds that an iron must be plugged in before it is considered present.<br>
  - **Back**<br>
Return to system menu.<br>

---

### SYSTEM
General settings for the controller.<br>
  - **Profile**<br>
Sets which iron profile (__T12__, __C210__, __C245__) to use.<br>
  - **Contrast**<br>
Screen Contrast/brightness.<br>
  - **Auto dim**<br>
Fade the display after 15s in skleep or error modes to prevent display burning.<br>
  - **Offset**<br>
Screen offset. This can accomodate the different screens which the controllers have come with. Use it to center the display on the screen. Default = 2.<br>
  - **Encoder**<br>
If rotating the encoder moves in the wrong direction, change this.<br>
  - **Boot**<br>
Operation mode when powered on. __RUN__ or __SLEEP__.<br>
  - **Wake mode**<br>
How to detect activity. SHAKE or STAND.<br>
SHAKE uses a motion sensor present in T12 handles, shake or hold the handle tip up to wake.
STAND can use the same handle shake wire, but must be disconnected from the handle and connected so when the tip is in the stand, this wire is shorted to GND. Shorted = sleep, open = wake.<br>
  - **Btn. Wake**<br>
Allow waking the controller by pressing the encoder button.<br>
  - **Shake Wake**<br>
Allow waking the controller by the shake sensor.<br>
  - **Active det.**<br>
Use iron active detection by leaving the PWM slightly on all the time. If your amp has a pullup resistor it can be disabled.<br>
  - **Buzzer**<br>
Buzz/beep when notable conditions occur.<br>
   - Changing operating mode (sleep, run)<br>
   - Temperature reached after the setpoint was changed<br>
   - Alarm when no iron is detected or system error happens<br>
  - **Unit**<br>
Temperature scale, Celsius or Fahrenheit<br>
  - **Step**<br>
Temperature step when adjusting tip temperature.<br>
  - **GUI Time**<br>
To offer maximum respoiveness, , the screen is updated between few 10s to more than 100 times per second (depends on the MCU used and display interface).<br> 
If the display reading were updated at the same speed, it would be impossible to read anything.<br> 
This setting defines the time in mS that the main screen readings are updated (voltage, temperatures).<br>
The real update rate will be limited by the PWM frequency (ADC is read at the every PWM cycle end),<br> 
Use a higher setting for less "flicker" in the display (more steady values).<br>
  - **Save time**<br>
Defines the delay with no changes before storing changed settings in flash memory.<br>
Flash has a limited number of write cycles (~100,000).<br>
Higher values reduce writes, but settings changes could be forgotten when the controller is powered off or reset.<br>
Default: 5 seconds.
  - **RESET MENU**<br>
Reset various configuration sections:<br>
    - **Settings**<br>
Reset Settings menu items to default.<br>
    - **Profile**<br>
Reset the current profile (iron/tips) to default.<br>
    - **Profiles**<br>
Reset all profiles to default.<br>
    - **All**<br>
Reset everything.<br>
  - **SW:**<br>
Displays the current software version. The version number is from a hash - higher is not necessarily newer.<br>
  - **HW:**<br>
Displays the hardware type.<br>
  - **Back**<br>
Return to system menu.<br>

---

### EDIT TIPS
Different tips may have different characteristics. Tips may be added or edited here.<br>
Select a tip to enter tip settings edit screen, or select Add New to create a new tip.<br>
The new tip will be created by copying the PID/calibration settings from the first in the system.<br>
Individual tip data includes info from both the *IRON- and *PID- menus.<br>

### EDIT TIP SETTINGS
This menu allows Tip editing/removing, PID tuning and adjustment of stored tip calibration values.<br>
PID tuning is an advanced topic, _**incorrect settings here can result in instability and damage to tips and possibly the controller**_.<br>
Most users should not change these settings, but here are the basics. Kp, Ki, Kd, Imax, Imin are the coefficients which control the PID's behavior.<br>
Calibration values are not meant for manual adjustment. Only to restore a previous calibration result.<br>
Use calibration for optimal results.<br>
  - **TIP NAME**<br>
Shows the tip name, click on it to access tip name editing/removing.<br>
  - **PID Kp**<br> 
The proportional term, changes the PWM duty cycle based on how far the measured temperature is from the desired temperature.<br>
  - **PID Ki**<br>
The integral term, changes the duty cycle based on how long the temperatures have been different.<br>
  - **PID Kd**<br>
PID differential term, changes the duty cycle based on how fast the measured temperature has changed.<br>
  - **PID Imax**<br>
The integral accumulator higher limit.<br>
  - **PID Imin**<br>
The integral accumulator lower limit.<br>
  - **Cal250**<br>
The stored value for 250ºC calibration.<br>
  - **Cal350**<br>
The stored value for 350ºC calibration.<br>
  - **Cal450**<br>
The stored value for 450ºC calibration.<br>
  - **Back**<br>
Return to system menu.<br>

### EDIT TIP NAME
This screen allows Tip name editing/removing.<br>
Click on the tip name to edit each character indvidually by scrolling up and down.<br>
Delete button will only appear if there's more than one tip in the system.<br>
Save button will only appear if the name was modified and there're no other tips with the same name. Empty names are not allowed.<br>
Back button will return discarding any the changes.<br>

---

### CALIBRATION
  - **Start**<br>
Requires a tip thermometer (e.g. Hakko FG-100 or similar). Calibrates the current tip at temperatures of 250, 350 and 450C.<br>
Wait for tip temperature to settle (When the thermomether reading stops moving), it can take up to 20 seconds in some cases.<br>
Then enter temperature as measured by the thermometer for each step.<br>
If the entered temperature is more than 50ºC higher than the target calibration value, the process will be aborted and you will have to adjust it manually<br>
  - **Adjust**<br>
Here you can adjust the default calibration values. For every step (250,350,450ºC) adjust the value until it's close to the target temperature.<br>
This is a coarse adjustment, made to avoid burning the tip in the calibration process if your controllers reads too low values.<br> 
This values have nothing to do with the Tip Settings calibration values (Those are temperature-compensated).<br>
Click on save to apply and store the changes, or cancel to discard.<br>
The Save button will be hidden if no changes were made, or the entered data is invalid.<br>
  - **Back**<br>
Return to system menu.<br>

---

### ERROR REPORTING
A lot of effort was done to protect the tips from overheating.<br>
Any detected error will disable PWM and show a message on the display.<br>
To recover from an error, simply press the button and it will reboot.<br>
There are multiple error types:<br>
  - **Iron warning**<br>
Non critical errors, a warning will be shown: iron not detected, supply voltage too low, ambient temperature too high or too low.<br>
  - **Iron runaway**<br>
If by any means the iron temperature is higher than requested and the PWM is still active, it will trigger a timer depending on the temperature diference.<br>
The condition must dissapear within the specified time, otherwise it will trigger a critical runaway error, shutting down the power stage.<br>
This is very useful to protect the tip from wrong PID adjustemnts (Ex. high integral accumulator).<br>
  - **Internal function errors**<br>
If any internal function detects undefined or not expected state, it will lock the station and show a message trying to show where the error happened (File, line).<br>
  - **Hardware exceptions**<br>
If a hardware exception happens, the station will lock up and display an error message about the exception.<br>
  - **Data error detection**<br>
The data is stored as separate blocks: System settings, profile 1, profile 2 and profile 3.<br>
Each one has it's own CRC checksum. When a block is read, the checksum is computed and compared.<br>
If a mismatch occurs, the block will be erased and resetted to defaults, trying to preserve the rest of the data.<br>
An error will be shown, detailing if the error detected was on the system settings data, or in any of the profiles.<br>
Also, the flash storage is checked carefully before and after writes, any issue will trigger a flash error message.<br>
  
 ### HARD RESET
If for any reason the station is unable to boot, you can't access the reset menu or you want to reset everything up quickly, there's a hard reset method.<br>
Power the station off, push and hold the button, then power the station on.<br>
A message will appear in the screen. "Hold button to restore defaults".<br>
Keep pressing the button for another 5 seconds, until the next message appears, "Release button now".
Release the button, the station will wipe everything and reboot.<br>

If you accidentally pushed the button, just release it before the 5 second timeout to resume the boot process. 
