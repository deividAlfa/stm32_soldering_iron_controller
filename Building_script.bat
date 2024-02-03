@echo off

:: [Default search path] Installation folder must be named STM32CubeIDE!
SET SEARCH_PATH="C:\ST"

:: [ENABLE THIS LINES TO MANUALLY SET THE TOOL PATHS]
:: set IDEPATH="C:\ST\STM32CubeIDE_1.7.0\STM32CubeIDE"
:: set IDE="%IDEPATH:"=%\stm32cubeide.exe"
:: set MX="%IDEPATH:"=%\plugins\com.st.stm32cube.common.mx_6.3.0.202107141111\STM32CubeMX.jar"
:: [Java path] You also use absolute path, ex "C:\JDK_19\bin\java.exe"
:: set JAVA_CMD="java.exe"


SET MODELS=    "BOARDS\KSGER\v1.5\STM32F103";^
               "BOARDS\KSGER\v2\STM32F101";^
               "BOARDS\KSGER\v3\STM32F101";^
               "BOARDS\Quicko\STM32F072";^
               "BOARDS\Quicko\STM32F103"
SET RUN_CUBEMX="n"
SET COMPILE="n"

SET ERR=0
SET BUILD_OPT=
SET BUILD_TARGET=

IF "%~1"=="" GOTO :BUILD_PROMPT
IF %~1 LSS 1 GOTO :BUILD_HELP
IF %~1 GTR 11 GOTO :BUILD_HELP
SET BUILD_TARGET=%~1

IF "%BUILD_TARGET%"=="10" GOTO :NO_BUILD_PROMPT
IF "%~2"=="" GOTO :BUILD_HELP
IF %~2 LSS 1 GOTO :BUILD_HELP
IF %~2 GTR 4 GOTO :BUILD_HELP
SET BUILD_OPT=%~2
GOTO :NO_BUILD_PROMPT


:BUILD_HELP
cls
echo.
echo Bad arguments given!
echo.
echo Run without any arguments to show the interactive prompt
echo.
echo Usage: Building_script.bat [PROFILE] [OPT]
echo.
echo        PROFILE:
echo                    1  KSGER v1.5     OLED
echo                    2  KSGER v1.5     LCD
echo                    3  KSGER v2       OLED
echo                    4  KSGER v3       OLED
echo                    5  KSGER v3       LCD
echo                    6  Quicko 072     OLED
echo                    7  Quicko 072     LCD
echo                    8  Quicko 103     OLED
echo                    9  Quicko 103     LCD
echo                    10 Build all
echo                    11 Quit
echo.
echo        OPT:
echo                    1  Only copy files
echo                    2  Copy files and run CubeMX
echo                    3  Copy files, run CubeMX and compile
echo                    4  Quit
echo.
echo For example, compile KSGER v3 OLED:    Building_script.bat 4 3
echo.
set ERR=1
GOTO :EXIT


:BUILD_PROMPT
cls
echo.
echo     STM32 Soldering firmware automated builder.
echo.
echo     KEY   PROFILE        DISPLAY
echo.
echo     [1]   KSGER v1.5     OLED
echo     [2]   KSGER v1.5     LCD
echo     [3]   KSGER v2       OLED
echo     [4]   KSGER v3       OLED
echo     [5]   KSGER v3       LCD
echo     [6]   Quicko 072     OLED
echo     [7]   Quicko 072     LCD
echo     [8]   Quicko 103     OLED
echo     [9]   Quicko 103     LCD
echo     [A]   Build all
echo     [Q]   Quit
echo.
CHOICE /C 123456789AQ /N /M "Please select your building target:"
set BUILD_TARGET=%ERRORLEVEL%
cls

:NO_BUILD_PROMPT
IF "%BUILD_TARGET%"=="1" SET PROFILE="BOARDS\KSGER\v1.5\STM32F103" && SET DISPLAY="SSD1306"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="2" SET PROFILE="BOARDS\KSGER\v1.5\STM32F103" && SET DISPLAY="ST7565"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="3" SET PROFILE="BOARDS\KSGER\v2\STM32F101" && SET DISPLAY="SSD1306"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="4" SET PROFILE="BOARDS\KSGER\v3\STM32F101" && SET DISPLAY="SSD1306"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="5" SET PROFILE="BOARDS\KSGER\v3\STM32F101" && SET DISPLAY="ST7565"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="6" SET PROFILE="BOARDS\Quicko\STM32F072" && SET DISPLAY="SSD1306"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="7" SET PROFILE="BOARDS\Quicko\STM32F072" && SET DISPLAY="ST7565"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="8" SET PROFILE="BOARDS\Quicko\STM32F103" && SET DISPLAY="SSD1306"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="9" SET PROFILE="BOARDS\Quicko\STM32F103" && SET DISPLAY="ST7565"&& GOTO :OPT_PROMPT
IF "%BUILD_TARGET%"=="10" SET PROFILE="" && SET RUN_CUBEMX="y" && SET COMPILE="y" && SET DISPLAY=""&& GOTO :TOOLS
IF "%BUILD_TARGET%"=="11" GOTO :EXIT

:OPT_PROMPT
IF NOT "%BUILD_OPT%"=="" GOTO :NO_OPT_PROMPT
echo.
echo     Run options
echo.
echo     [1]   Only copy files
echo     [2]   Copy files and run CubeMX
echo     [3]   Copy files, run CubeMX and compile
echo     [Q]   Quit
echo.
CHOICE /C 123Q /N /M "Please select an option:"
set BUILD_OPT=%ERRORLEVEL%
cls

:NO_OPT_PROMPT
IF "%BUILD_OPT%"=="1" GOTO :ONLYCOPY
IF "%BUILD_OPT%"=="2" SET RUN_CUBEMX="y" && GOTO :TOOLS
IF "%BUILD_OPT%"=="3" SET RUN_CUBEMX="y" && SET COMPILE="y" && GOTO :TOOLS
IF "%BUILD_OPT%"=="4" GOTO :EXIT

:: ##################  [TRY FINDING THE TOOLS IF INVALID PATHS DETECTED]  ##################

:TOOLS
if not exist "%JAVA_CMD%" ( GOTO :DETECT )
if not exist "%IDE%" ( GOTO :DETECT )
if not exist "%MX%" ( GOTO :DETECT )
goto :START

:DETECT
SET IDEPATH=
SET IDE=
SET MX=
SET JAVA_CMD=
echo Searching tool paths...
echo.

for /f "delims=" %%F in ('dir /b /s %SEARCH_PATH%\STM32CubeIDE 2^>nul') do (set IDEPATH=%%~F)
if not defined IDEPATH ( goto :NOTFOUND )

set IDE=%IDEPATH%\stm32cubeidec.exe
if not exist "%IDE%" ( goto :NOTFOUND )

for /f "delims=" %%F in ('dir /b /s "%IDEPATH:"=%\java.exe" 2^>nul') do (set JAVA_CMD=%%~F)
if not defined JAVA_CMD ( goto :NOTFOUND )

for /f "delims=" %%F in ('dir /b /s "%IDEPATH:"=%\STM32CubeMX.jar" 2^>nul') do (set MX=%%~F)
if not defined MX ( goto :NOTFOUND )
goto :START

:NOTFOUND
echo [91mTools not found! Ensure CubeIDE is installed in C:\ST\[0m
echo [91mElse, edit this script and set the path manually[0m
set ERR=1
goto :DONE

:: ##################  [START]  ##################
:START

echo [32mFound CubeIDE at:[0m
echo %IDE:"=%
echo.
echo [32mFound Java at:[0m
echo %JAVA_CMD:"=%
echo.
echo [32mFound CubeMX at:[0m
echo %MX:"=%
echo.
:: ##################  [BUILD LOOP]  ##################
:ONLYCOPY
for %%M in (%MODELS%) do (
    SET CURRENT=""
    :: Profile is empty, build all
    IF %PROFILE%=="" (SET CURRENT=%%M)
    :: Profile defined, build only if matching
    IF %PROFILE%==%%M (SET CURRENT=%%M)
    CALL :BUILD
)
IF %COMPILE%=="y" echo [32mBuild complete![0m
goto :DONE

:BUILD
IF %CURRENT%=="" ( EXIT /B )
echo [93mProfile: %CURRENT%[0m     
del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul >nul
rd /S /Q Drivers\CMSIS Drivers\STM32F0xx_HAL_Driver Drivers\STM32F1xx_HAL_Driver 2>nul >nul
xcopy /e /k /h /i /s /q /y %CURRENT% >nul
IF %RUN_CUBEMX%=="n" ( EXIT /B )

:: Workaround required, CubeMX keeps messing the IOC, changing the project to EWARM, won't work in CubeIDE after generation
copy /Y STM32SolderingStation.ioc STM32SolderingStation.bak >nul

echo [94mRunning CubeMX...[0m
start /w /min "CubeMX" %JAVA_CMD% -jar "%MX%" -q cubemx_script >nul
IF %ERRORLEVEL% NEQ 0 (
  echo [91mCubeMX error![0m : %ERRORLEVEL%
  set ERR=1
  goto :DONE
)       

IF %COMPILE%=="n" ( EXIT /B )

set DISP=%DISPLAY%
:COMPILE
IF %DISPLAY%=="" (
  IF %DISP%=="" (SET DISP="SSD1306")
  IF %DISP%=="SSD1306" (SET DISP="ST7565")  
)
echo [94mCompiling...[0m    DISPLAY:%DISP:"=%
echo start /w /min "CubeIDE" %IDE% --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import %cd% -build STM32SolderingStation/%DISP:"=%_Release 2>nul >nul
start /w /min "CubeIDE" %IDE% --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import %cd% -build STM32SolderingStation/%DISP:"=%_Release 2>nul >nul
IF %ERRORLEVEL% NEQ 0 (
  echo [91mCompiler error![0m : %ERRORLEVEL%
  set ERR=1
  goto :DONE
)
move /Y "%DISP:"=%_Release\STM32SolderingStation.bin" "%CURRENT:"=%\%DISP:"=%.bin" 2>nul >nul
move /Y "%DISP:"=%_Release\STM32SolderingStation.list" "%CURRENT:"=%\%DISP:"=%.list" 2>nul >nul
IF %ERRORLEVEL% EQU 0 ( echo                 Binary placed at %CURRENT:"=%\%DISP:"=%.bin
) ELSE ( echo                 %CURRENT:"=% doesn't contain %DISP:"=% profile ?)
IF %DISPLAY%=="" ( IF %DISP%=="SSD1306" ( GOTO :COMPILE ) )
echo.
exit /B

:DONE
echo Cleaning up...
timeout 1 >nul
:: Restore unmodified IOC file so it keeps working in CubeIDE
move /Y STM32SolderingStation.bak STM32SolderingStation.ioc >nul 2>nul
:: Cleanup
rd /Q /S SSD1306_Release ST7565_Release EWARM Application 2>nul

IF "%BUILD_TARGET%"=="10" GOTO :EXIT
pause

:EXIT
exit %ERR%