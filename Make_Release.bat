@echo off

:: Adjust this to 7-Zip executable

set ZIP="C:\Program Files\7-Zip\7z.exe"

:: Check that 7-Zip exists        

if not exist %ZIP% (
  echo 7-Zip not installed or wrong path set!
  echo You might need to modify "ZIP" variable in this bat file.
  GOTO :ERROR
)

:: Compile all profiles
echo Compiling...
start /w Building_script.bat 10
IF %ERRORLEVEL% NEQ 0 echo Build error! && GOTO :ERROR
echo OK
echo.

:: Copy files
md Release 2>nul
cd Release
del *.zip *.bin *.list 2>nul >nul

copy "..\BOARDS\KSGER\v1.5\STM32F103\SSD1306.bin"	"KSGER_v1_5_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v1.5\STM32F103\SSD1306.list"	"KSGER_v1_5_OLED.list" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v1.5\STM32F103\ST7565.bin" 	"KSGER_v1_5_LCD_ST7565.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v1.5\STM32F103\ST7565.list" 	"KSGER_v1_5_LCD_ST7565.list" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v2\STM32F101\SSD1306.bin" 	"KSGER_v2_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v2\STM32F101\SSD1306.list" 	"KSGER_v2_OLED.list" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\SSD1306.bin" 	"KSGER_v3_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\SSD1306.list" 	"KSGER_v3_OLED.list" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\ST7565.bin" 		"KSGER_v3_LCD_ST7565.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\ST7565.list" 	"KSGER_v3_LCD_ST7565.list" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\SSD1306.bin" 		"Quicko_STM32F072_OLED.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\SSD1306.list" 		"Quicko_STM32F072_OLED.list" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\ST7565.bin" 		"Quicko_STM32F072_LCD_ST7565.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\ST7565.list" 		"Quicko_STM32F072_LCD_ST7565.list" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\SSD1306.bin" 		"Quicko_STM32F103_OLED.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\SSD1306.list" 		"Quicko_STM32F103_OLED.list" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\ST7565.bin" 		"Quicko_STM32F103_LCD_ST7565.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\ST7565.list" 		"Quicko_STM32F103_LCD_ST7565.list" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES

echo Creating zip files...
echo.
:: Create zips
for %%f in (*.bin) do (
  set FILE=%%~nf
  call :ZIP
)
goto :DONE

:ZIP
echo %FILE%
%ZIP% a -mx=9 -tzip %FILE%.zip %FILE%.* -y >nul
::%ZIP% a -mx=9 -tzip FLASH_TEST_%FILE%.zip %FILE%.bin -y >nul
IF %ERRORLEVEL% NEQ 0 echo Unknown 7-ZIP Error! && GOTO :ERROR
exit /B


:DONE
echo.
echo OK
echo Files placed in "Release" folder
TIMEOUT 3 >NUL
GOTO :END


:NO_FILES
echo Missing bin files!
echo First run Building_script.bat and select "Compile all"

:ERROR
pause

:END