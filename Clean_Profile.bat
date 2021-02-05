@echo off
del .\Core\Inc\stm*.* /Q >nul
del .\Core\Inc\board.h /Q >nul
del .\Core\Src\stm*.* /Q >nul
del .\Core\Src\system_stm*.* /Q >nul
del .\Core\Startup\*.s /Q >nul
del .\.cproject /Q >nul
del .\.project /Q >nul
del .\STM32SolderingStation.ioc /Q >nul
rmdir .\Drivers\STM32F0xx_HAL_Driver /S /Q >nul
rmdir .\Drivers\STM32F1xx_HAL_Driver /S /Q >nul
rmdir .\Debug /S /Q >nul
rmdir .\Release /S /Q >nul
echo Done cleaning profile files!
echo Press any key to close the window...
pause >nul