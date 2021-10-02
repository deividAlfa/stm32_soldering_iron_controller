@echo off
del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul	
xcopy /e /k /h /i /s /q /y "BOARDS\Quicko\STM32F072" 2>nul >nul
