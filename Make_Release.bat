@echo off

:: Adjust this to 7-Zip executable

set ZIP=	"C:\Program Files\7-Zip\7z.exe"

:: Check that 7-Zip exists        

if not exist %ZIP% (
  echo 7-Zip not installed or wrong path set!
  echo You might need to modify "ZIP" variable in this bat file.
  GOTO :ERROR
)


:: Copy files

cd Release
del *.zip *.bin 2>nul >nul

copy "..\BOARDS\KSGER\v1.5\STM32F103\SSD1306.bin"	"KSGER_v1_5_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v1.5\STM32F103\ST7565.bin" 	"KSGER_v1_5_LCD_ST7565.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v2\STM32F101\SSD1306.bin" 	"KSGER_v2_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\SSD1306.bin" 	"KSGER_v3_OLED.bin" >nul			&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\KSGER\v3\STM32F101\ST7565.bin" 		"KSGER_v3_LCD_ST7565.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\SSD1306.bin" 		"Quicko_STM32F072_OLED.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F072\ST7565.bin" 		"Quicko_STM32F072_LCD_ST7565.bin" >nul	&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\SSD1306.bin" 		"Quicko_STM32F103_OLED.bin" >nul		&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES
copy "..\BOARDS\Quicko\STM32F103\ST7565.bin" 		"Quicko_STM32F103_LCD_ST7565.bin" >nul	&& IF %ERRORLEVEL% NEQ 0 GOTO :NO_FILES

:: Create zips

for %%f in (*.bin) do (
  if not exist %%~f 		GOTO :NO_FILES
  echo %%~f
  %ZIP% a -tzip %%~nf.zip %%~f -y >nul

  IF %ERRORLEVEL% NEQ 0		echo Unknown 7-ZIP Error!	&& GOTO :ERROR
)

echo.
echo Done!
TIMEOUT 3 >NUL
GOTO :END


:NO_FILES
echo Missing bin files!
echo First run Building_script.bat and select "Compile all"

:ERROR
pause

:END

