@echo off
del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul	
xcopy /e /k /h /i /s /q /y "BOARDS\KSGER\[v1.5]\STM32F103" 2>nul >nul
