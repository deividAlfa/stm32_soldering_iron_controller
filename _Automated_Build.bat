@echo off

REM ##################  [PROFILE LIST]  ##################
Rem Sort by MCU type, compiling same cpu profiles together improves the speed, as the compiler cache is kept

set MODELS=    "BOARDS\KSGER\[v1.5]\STM32F103";^
               "BOARDS\Quicko\STM32F103";^
               "BOARDS\KSGER\[v2]\STM32F101";^
               "BOARDS\KSGER\[v3]\STM32F101";^
               "BOARDS\Quicko\STM32F072"


REM ##################  [ENABLE THIS LINES TO MANUALLY SET THE TOOL PATHS]  ##################
REM set IDEPATH="C:\ST\STM32CubeIDE_1.7.0\STM32CubeIDE"
REM set IDE="%IDEPATH:"=%\stm32cubeide.exe"
REM set MX="%IDEPATH:"=%\plugins\com.st.stm32cube.common.mx_6.3.0.202107141111\STM32CubeMX.jar"


REM ##################  [TRY FINDING THE TOOLS IF INVALID PATHS DETECTED]  ##################

if not exist "%IDE%" ( GOTO detect )
if not exist "%MX%" ( GOTO detect )
goto :start

:detect
SET IDEPATH=
SET IDE=
SET MX=
echo Searching tool paths...
echo.

for /f "delims=" %%F in ('dir /b /s "C:\ST\STM32CubeIDE" 2^>nul') do (set IDEPATH=%%~F)
if not defined IDEPATH (goto notfound)

set IDE=%IDEPATH%\stm32cubeide.exe
if not exist "%IDE%" (goto notfound)

for /f "delims=" %%F in ('dir /b /s "%IDEPATH:"=%\STM32CubeMX.jar" 2^>nul') do (set MX=%%~F)
if not defined MX (goto notfound)
goto start

:notfound
echo [91mTools not found! Ensure CubeIDE is installed in C:\ST\[0m
echo [91mElse, edit this script and set the path manually[0m
goto :end

REM ##################  [START]  ##################
:start

echo [32mFound CubeIDE at:[0m
echo %IDE:"=%
echo.
echo [32mFound CubeMX at:[0m
echo %MX:"=%
echo.


REM ##################  [BUILD LOOP]  ##################
for %%M in (%MODELS%) do (
    echo [93mProfile: %%~M[0m
     
    del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q
    xcopy /e /k /h /i /s /q /y %%M >nul
    
    echo [94mRunning CubeMX...[0m
    start /w /min "CubeMX" java -jar "%MX%" -q cubemx_script 2>nul >nul
    IF %ERRORLEVEL% NEQ 0 (
        echo [91mCubeMX error![0m
        goto :end
    )   
    
    echo [94mCompiling...[0m
    start /w /min "CubeIDE" %IDE% --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import %cd% -build STM32SolderingStation/Release 2>nul >nul
    IF %ERRORLEVEL% NEQ 0 (
        echo [91mCompiler error![0m
        goto :end
    )    
    
   copy /Y Release\STM32SolderingStation.bin %%M\ >nul
   echo.
)
echo [32mBuild complete![0m


REM ##################  [END]  ##################
:end
echo Cleaning up...
echo.
rd EWARM /Q /S 2>nul
rd Release /Q /S 2>nul
rd Debug /Q /S 2>nul
del *.bin /Q 2>nul

pause
