WARNING: Tip temperature measurement is still unreliable, this firmware might heat up your iron far above the temperature on display. Use with caution and at own risk!

This is a fork and first attempt to get the firmare of ptdreamer working on a slightly different STM32 soldering controller.
See schematic/Ver2.1S/ for hardware details

Working:
  * OLED Display
  * Rotary Encoder
  * Buzzer
  * Wake switch
  * Supply voltage sensor
  * Ambient temperature sensor (might need calibration)
  * Tip temperature read out (might also need calibration)
  * T12 PWM Control (might burn your tip)

TODO:
  * I2C eeprom

# stm32_soldering_iron_controller
Custom firmware for the chinese kseger soldering iron controller

Check http://www.ptdreamer.com/chinese-stm32-oled-soldering-controller-reverse-engineer-custom-firmware/ for more information
