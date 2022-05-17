@echo off

REM ##################  [ENABLE THIS LINES TO MANUALLY SET THE TOOL PATHS]  ##################
REM set IDEPATH="C:\ST\STM32CubeIDE_1.7.0\STM32CubeIDE"
REM set IDE="%IDEPATH:"=%\stm32cubeide.exe"
REM set MX="%IDEPATH:"=%\plugins\com.st.stm32cube.common.mx_6.3.0.202107141111\STM32CubeMX.jar"

SET MODELS=    "BOARDS\KSGER\[v1.5]\STM32F103_SSD1306";^
               "BOARDS\KSGER\[v1.5]\STM32F103_ST7565";^
               "BOARDS\Quicko\STM32F103_SSD1306";^
               "BOARDS\Quicko\STM32F103_ST7565";^
               "BOARDS\KSGER\[v2]\STM32F101_SSD1306";^
               "BOARDS\KSGER\[v3]\STM32F101_SSD1306";^
               "BOARDS\KSGER\[v3]\STM32F101_ST7565";^
               "BOARDS\Quicko\STM32F072_SSD1306";^
               "BOARDS\Quicko\STM32F072_ST7565"
SET PROFILE=""
SET RUN_CUBEMX="n"
SET COMPILE="n"
cls
echo.
echo     STM32 Soldering firmware automated builder.
echo.
echo     KEY   PROFILE        DISPLAY
echo.
echo     [1]   KSGER v1.5     SSD1306
echo     [2]   KSGER v1.5     ST7565
echo     [3]   KSGER v2       SSD1306
echo     [4]   KSGER v3       SSD1306
echo     [5]   KSGER v3       ST7565
echo     [6]   Quicko 072     SSD1306
echo     [7]   Quicko 072     ST7565
echo     [8]   Quicko 103     SSD1306
echo     [9]   Quicko 103     ST7565
echo     [A]   Build all
echo     [Q]   Quit
echo.
CHOICE /C 123456789AQ /N /M "Please select your building target:"
cls
IF "%ERRORLEVEL%"=="1" SET PROFILE="BOARDS\KSGER\[v1.5]\STM32F103_SSD1306" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="2" SET PROFILE="BOARDS\KSGER\[v1.5]\STM32F103_ST7565" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="3" SET PROFILE="BOARDS\KSGER\[v2]\STM32F101_SSD1306" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="4" SET PROFILE="BOARDS\KSGER\[v3]\STM32F101_SSD1306" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="5" SET PROFILE="BOARDS\KSGER\[v3]\STM32F101_ST7565" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="6" SET PROFILE="BOARDS\Quicko\STM32F072_SSD1306" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="7" SET PROFILE="BOARDS\Quicko\STM32F072_ST7565" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="8" SET PROFILE="BOARDS\Quicko\STM32F103_SSD1306" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="9" SET PROFILE="BOARDS\Quicko\STM32F103_ST7565" && GOTO :ASKCUBEMX
IF "%ERRORLEVEL%"=="10" SET PROFILE="" && SET RUN_CUBEMX="y" && SET COMPILE="y" && GOTO :TOOLS
IF "%ERRORLEVEL%"=="11" GOTO :END

:ASKCUBEMX
echo.
echo     Run options
echo.
echo     [1]   Only copy files
echo     [2]   Copy files and run CubeMX
echo     [3]   Copy files, run CubeMX and compile
echo     [Q]   Quit
echo.
CHOICE /C 123Q /N /M "Please select an option:"
cls
IF "%ERRORLEVEL%"=="1" GOTO :ONLYCOPY
IF "%ERRORLEVEL%"=="2" SET RUN_CUBEMX="y" && GOTO :TOOLS
IF "%ERRORLEVEL%"=="3" SET RUN_CUBEMX="y" && SET COMPILE="y" && GOTO :TOOLS
IF "%ERRORLEVEL%"=="4" GOTO :END

REM ##################  [TRY FINDING THE TOOLS IF INVALID PATHS DETECTED]  ##################
:TOOLS
if not exist "%IDE%" ( GOTO :DETECT )
if not exist "%MX%" ( GOTO :DETECT )
goto :START

:DETECT
SET IDEPATH=
SET IDE=
SET MX=
echo Searching tool paths...
echo.

for /f "delims=" %%F in ('dir /b /s "C:\ST\STM32CubeIDE" 2^>nul') do (set IDEPATH=%%~F)
if not defined IDEPATH ( goto :NOTFOUND )

set IDE=%IDEPATH%\stm32cubeide.exe
if not exist "%IDE%" ( goto :NOTFOUND )

for /f "delims=" %%F in ('dir /b /s "%IDEPATH:"=%\STM32CubeMX.jar" 2^>nul') do (set MX=%%~F)
if not defined MX ( goto :NOTFOUND )
goto :START

:NOTFOUND
echo [91mTools not found! Ensure CubeIDE is installed in C:\ST\[0m
echo [91mElse, edit this script and set the path manually[0m
goto :END

REM ##################  [START]  ##################
:START

echo [32mFound CubeIDE at:[0m
echo %IDE:"=%
echo.
echo [32mFound CubeMX at:[0m
echo %MX:"=%
echo.
REM ##################  [BUILD LOOP]  ##################
:ONLYCOPY
for %%M in (%MODELS%) do (
    SET CURRENT=""
    REM Profile is empty, build all
    IF %PROFILE%=="" (SET CURRENT=%%M)
    REM Profile defined, build only if matching
    IF %PROFILE%==%%M (SET CURRENT=%%M)
    CALL :BUILD
)
IF %COMPILE%=="y" echo [32mBuild complete![0m && echo Binaries placed in their respective BOARDS folder (STM32SolderingStation.bin)
goto :DONE

:BUILD
IF %CURRENT%=="" ( EXIT /B )

echo [93mProfile: %CURRENT%[0m     
del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul >nul
xcopy /e /k /h /i /s /q /y %CURRENT% >nul

IF %RUN_CUBEMX%=="n" ( EXIT /B )

echo [94mRunning CubeMX...[0m
start /w /min "CubeMX" java -jar "%MX%" -q cubemx_script 2>nul >nul

IF %ERRORLEVEL% NEQ 0 (
  echo [91mCubeMX error![0m
  goto :DONE
)       

IF %COMPILE%=="n" ( EXIT /B )

echo [94mCompiling...[0m
start /w /min "CubeIDE" %IDE% --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import %cd% -build STM32SolderingStation/Release 2>nul >nul

IF %ERRORLEVEL% NEQ 0 (
  echo [91mCompiler error![0m
  goto :DONE
)    

copy /Y Release\STM32SolderingStation.bin %CURRENT%\ >nul

echo.
exit /B

REM ##################  [END]  ##################
:DONE
echo Cleaning up...
echo.
rd EWARM /Q /S 2>nul
rd Release /Q /S 2>nul
rd Debug /Q /S 2>nul
del *.bin /Q 2>nul
pause
:END