## Building the firmware

<h3>
⚠️ Use CubeIDE 1.12.1! ⚠️ <br><br>
Newer versions (Currently 1.13.0) are causing lots of issues.<br>
Neither the firmware or the Build script work properly.<br>
Until the issues are addressed up, keep using 1.12.1.<br><br>
</h3>


Video of building steps:<br>
[![IMAGE ALT TEXT](http://img.youtube.com/vi/8oeGVSSxudk/0.jpg)](https://www.youtube.com/watch?v=8oeGVSSxudk "Firmware build")<br><br>

First of all, download and install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).<br>
Clone or download the source from this repository.<br>
The source is stripped from ST own libraries and unnecessary stuff, only includes the very basic code owning to the project.<br>
CubeMX will add the STM32 and CMSIS libraries automatically after a code generation.<br><br>
There's a new automated build script for Windows (Building_script.bat) that allows a simple and fast way of copying and building the desired profile.<br>
With it, all you need is to have CubeIDE installed in C:\ST (It's the default installation folder), it will search and execute the tools without requiring any user intervention.<br>
JDK is no longer required, as I recently discovered CubeIDE packs its own.<br>
Just open it, choose your profile, then select whether you want to only copy the files, run CubeMX (Generate the libraries) and/or compile the firmware.<br>
After compiling, the binaries will be placed in their respective BOARDS/... folders.<br><br>
<img src="/Readme_files/build_script_0.png?raw=true"><br><br>
<img src="/Readme_files/build_script_1.png?raw=true"><br><br>

If you want to build it within CubeIDE, first run Building_script.bat, choose your profile and select Copy files / Run CubeMX.<br>
Open STM32CUBE IDE, click on Import/Existing project and select the project folder.<br>
Disable "Search for nested projects", select only the project at the root of the folder.<br>
After this, it'll be ready for compiling, click in the right arrow of the build button (Hammer icon) and select [Release]:<br>
<img src="/Readme_files/release.jpg?raw=true"><br><br>
After a while you'll have the compiled bin/hex files inside Release folder.<br><br>

If you want to modify the hardware initialization, double-click on [STM32SolderingStation.ioc] file:<br>
<img src="/Readme_files/open_ioc.png?raw=true"><br><br>
CubeMX will open. Make you changes, then run code generation:<br>
<img src="/Readme_files/gen.png?raw=true"><br><br>
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

At some point, the firmware might not fit into the flash when compiling for debugging, as running without optimizations will use a lot more space.<br>
In that case, you'll need to force some optimization level, starting with "Optimize for debug" (Og), and going to higher levels if still being too big (O1, O2, Osize).<br>
The settings can be changed in project Properties / Build / Settings / MCU GCC Compiler / Optimizations.<br>
However, when debugging, it's desirable to completely disable optimizations to see the program flow clearly.<br>
If you had to enable any level of global optimizations, you can still selectively disable build optimizations for any function, making debugging easier.<br>
A line of code can be found at the start of main.h:<br>

  __attribute__((optimize("O0")))

Copy that line before a function to disable optimization, like this:<br>

   __attribute__((optimize("O0"))) void ThisFunctionWillNotBeOptimized(...)
   

If you want to retarget the project, avoid mixing different profile files, run Building_script.bat again so it cleans the project and copies the new files.<br>

If you use an existing project template and modify it, the changes must be reflected in /Core/Inc/board.h.<br>
All the project code takes the data from there. The file it's pretty much self-explaining.<br>
So, any changes you make in CubeMX, ex. use PWM timer6 intead timer2, or SPI1 instead SPI2...all that should be configured in their respective define.<br>
As long as the GPIO names are called the same way, no further changes are needed.<br>
