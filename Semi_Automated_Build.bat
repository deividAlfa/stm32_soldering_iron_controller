@echo off
setlocal enabledelayedexpansion 
echo.


REM ##################  [PROFILE LIST]  ##################
Rem Sort by MCU type, compiling same cpu profiles together improves the speed, as the compiler cache is kept

set MODELS=    "BOARDS\KSGER\[v1.5]\STM32F103";^
               "BOARDS\Quicko\STM32F103";^
               "BOARDS\KSGER\[v2]\STM32F101";^
               "BOARDS\KSGER\[v3]\STM32F101";^
               "BOARDS\Quicko\STM32F072"



REM ##################  [ENABLE THIS LINES TO MANUALLY SET THE PATHS]  ##################

REM set IDE=C:\ST\STM32CubeIDE_1.7.0\STM32CubeIDE
REM set MX="%IDE%\plugins\com.st.stm32cube.common.mx_6.3.0.202107141111\STM32CubeMX.jar"




REM ##################  [FIND CUBEIDE IF NOT DEFINED ALREADY]  ##################

if "%IDE%"=="" (
    for /f "delims=" %%F in ('dir /b /s "C:\ST\stm32cubeide.exe" 2^>nul') do (
        set IDE=%%F
    )
    if "!IDE!"=="" (
        echo [91mCubeIDE not found![0m
        echo Edit this file and set the path manually
        goto :end
    ) else (
        echo [32mFound CubeIDE:[0m
        echo "!IDE!"
        echo.
    )    
)


REM ##################  [FIND CUBEMX IF NOT DEFINED ALREADY]  ##################

if "%MX%"=="" (
    for /f "delims=" %%F in ('dir /b /s "C:\ST\STM32CubeMX.jar" 2^>nul') do (
        set MX=%%F
    )
    if "!MX!"=="" (
        echo [91mCubeMX not found![0m
        echo Edit this file and set the path manually
        goto :end
    ) else (
        echo [32mFound CubeMX:[0m
        echo "!MX!"
        echo.
    )    
)
echo.


REM ##################  [BUILD PROFILES]  ##################
echo [93mEach time CubeMX is launched, click on "Generate code", wait until finishes, and exit[0m
echo.
echo.
for %%M in (%MODELS%) do (
    echo [93mProfile: %%~M[0m
     
    del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul	
    xcopy /e /k /h /i /s /q /y %%M 2>nul >nul
    
    echo [94mLaunching CubeMX...[0m
    start /w /min java -jar %MX% STM32SolderingStation.ioc
    IF /I "!ERRORLEVEL!" NEQ "0" (
        echo [91mCubeMX error![0m
        goto :end
    )   
    
    echo [94mCompiling...[0m
    start /w /min %IDE% --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import %cd% -build STM32SolderingStation/Release
    IF /I "!ERRORLEVEL!" NEQ "0" (
        echo [91mCompiler error![0m
        goto :end
    )    
    
   copy /Y Release\STM32SolderingStation.bin %%M\ >nul 2>nul
   echo.
)
echo [32mBuild complete![0m

:end
timeout 2 >nul