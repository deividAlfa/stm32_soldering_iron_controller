# Operating Instructions

## Initial Startup
When an unconfigured (new or fully reset) controller starts, it first offers a choice of which tip profile to use. Select the one appropriate for your iron. The JBC irons require electrical changes to the controller:

* Hakko __T12__ (T15 Series in North America & EU) This is the handle normally used with these controllers.
* JBC __C210__
* JBC __C245__

## Basic Controls
Changes are made using the rotary encoder and its push button. On the main screen, a quick press will alternate between a tip temperature display and a graph showing the recent history of tip temperature. The power supply voltage is shown upper left, room temperature in the upper right, bar graph of relative power going to the tip on the bottom.

Rotating the encoder will change the desired tip temperature, which will then quickly revert to showing the measured temperature.

Pressing, then turning while pressed will allow a different tip to be chosen (more on that later).

Pressing, and holding will enter the Menus. The controller will return to the main screen with another long press, scrolling and selecting "BACK" from each menu, or 15 seconds of inactivity.

To operate the menus, rotate to scroll to the desired selection, quick press to select it, rotate to change. Quick press again to exit the setting.

## Theory of Operation
Before covering the settings, it is useful to understand how it works. 

Tip temperature is measured by using an ADC (Analog-Digital Converter) to measure the voltage output by a thermocouple in the tip. The voltage is basically proportional to the difference between tip temperature and room temperature.

The controller uses PWM (Pulse Width Modulation) to control the tip temperature. PWM basically varies the amount of time that the tip is powered. By default, PWM uses a 200 ms (5 times per second) period. If the tip is too hot, the controller will turn on power to the tip little or not at all during each cycle. If the tip is too cold, it will turn the power on for more of the period. The time the tip is powered on during each period is called the duty cycle. 
<pre>
|<------ PWM PERIOD ------>|
| POWER |____ no power ____|
</pre>
The PID (Proportional, Integral, Derivitive) algorithm determines the PWM duty cycle based on the difference between desired and measured tip temperatures.

## Menus

### PID

The PID menu allows fine tuning of the algorithm. PID tuning is an advanced topic, _**incorrect settings here can result in instability and damage to tips and possibly the controller**_.  Most users should not change these settings, but here are the basics. Kp, Ki, and Kd are the coefficients which control the PID's behavior. In simple terms:
  * #### _Kp_ 
The proportional term, changes the PWM duty cycle based on how far the measured temperature is from the desired temperature.
  * #### _Ki_ 
The integral term, changes the duty cycle based on how long the temperatures have been different.
  * #### _Kd_ 
The differential term, changes the duty cycle based on how fast the measured temperature has changed.
  * #### _Time_
PID is called at the end of every PWM period. The _time_ preset will delay the PID calculation. 
Settings this value lower than the PWM period will cause the PID to be done every time. Setting it higher will delay the PID update to a later PWM period. Default is 0 (no delay, update every PWM period)

### IRON
Iron settings control the operation of the handle/tips. 
  * #### _Sleep_
If there is no soldering activity for this period, the controller will "sleep" and stop providing power to the tip, allowing it to cool. This helps increase tip lifetime. Activity (e.g. shaking the handle for a T12) will wake it up and heating will resume.
  * #### _Heater R(esistance)._
The resistance of the tip's heating element. There is normally no need to change this from the default.
  * #### _Power_
The maximum power which will be delivered to the tip. This sets a maximum for the PWM duty cycle, based on the power supply voltage and the heater resistance.
  * #### _PWM Time_
Sets the PWM period. The controller will check and adjust the tip temperature once each period. Default 200 ms.
  * #### _ADC Delay_
Near the end of each PWM period, the temperature is measured by the ADC. _ADC Delay_ controls how soon before the end of a period the temperature is measured. The measurement can only be taken when the power to the tip is off, so the effective PWM duty cycle is limited to (_PWM Time_ - _ADC Delay_). Default 20 ms.
<pre>
|<----------------- PWM TIME -------------------->|
|_____________________________[ADC READ]|<-DELAY->| NEXT CYCLE
|<- POWER ->|_____________________________________|
</pre>
  * #### _Filtering_
Used to filter the temperature measurements before they are passed to the PID. This helps remove noise and provides more stability.
   * * ##### _Avg_
This uses a simple moving average, and is the default filter.
   * * ##### _EMA_
Exponential Moving Average. A more sophisticated filter, adds a delay, but tracks changes more closely.
   * * ##### _DEMA_
Double Exponential Moving Average. A more sophisticated filter, double _EMA_.
  * #### _Factor_
Only for EMA/DEMA. 
  * #### _No iron_
The ADC reading which signals that no iron is present. When no iron is plugged in, the measured temperature will read at or near maximum (ADC = 4095). Default 4000, max 4100 (don't detect "no iron").
  * #### _Detection_
Time an iron must be plugged in before it is considered present.

### SYSTEM
General settings for the controller.
  * #### _Profile_
Sets which iron profile (__T12__, __C210__, __C245__) to use.
  * #### _Contrast_
Screen Contrast/brightness.
  * #### _Offset_
Screen offset. This can accomodate the different screens which the controllers have come with. Use it to center the display on the screen. Default = 2.
  * #### _Encoder_
If rotating the encoder moves in the wrong direction, change this.
  * #### _Boot_
Operation mode when powered on. __RUN__ or __SLEEP__.
  * #### _Wake mode_
How to detect activity. SHAKE or STAND. SHAKE uses a motion sensor present in T12 handles, shake or hold the handle tip up to wake. STAND can use the same handle shake wire, but must be disconnected from the handle and connected so when the tip is in the stand, this wire is shorted to GND. Shorted = sleep, open = wake.
  * #### _Btn. Wake_
Allow waking the controller by moving or pressing the encoder.
  * #### _Buzzer_
Buzz/beep when notable conditions occur.
    * Changing operating mode (sleep, run)
    * Temperature reached after the setpoint was changes
    * Alarm when no iron is detected or system error happens
  * #### _Unit_
Temperature scale, Celsius or Fahrenheit
  * #### _Step_
Temperature step when adjusting tip temperature.
  * #### _GUI Time_
How often the main screen readings are updated (voltage, temperatures). Should be greater than _PWM Time_. Use a higher setting for less "flicker" in the display.
  * #### _Save time_
How long before storing changed settings in flash memory. Flash has a limited number of write cycles (~100,000). Higher values reduce writes, but settings changes could be forgotten when the controller is powered off or reset. Default: 5 seconds.
  * #### _RESET MENU_
Reset various configuration sections:
  * * ##### _Settings_
Reset Settings menu items to default.
  * * ##### _Profile_
Reset the current profile (iron/tips) to default.
  * * ##### _Profiles_
Reset all profiles to default.
  * * ##### _All_
Reset everything.

  * #### _SW:_
Displays the current software version. The version number is from a hash - higher is not necessarily newer.
  * #### _HW:_
Displays the hardware type.

### EDIT TIPS
Different tips may have different characteristics. Tips may be added or edited here. Select a tip to edit, or add new to create and name a new tip. The new tip will be created by copying the settings from the first tip listed. Return to the main display, press and turn to select the tip, then make changes for the new tip as desired.

Individual tip data includes info from both the *IRON* and *PID* menus. 

### CALIBRATION
Requires a tip thermometer (e.g. Hakko FG-100 or similar). Calibrates the current tip at temperatures of 200, 300 and 400C. Wait for tip temperature to settle, then enter temperature as measured by the thermometer for each step.

