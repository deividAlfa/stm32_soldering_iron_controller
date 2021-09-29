@echo off
del Core\Inc\board.h Core\Inc\*stm32*.* Core\Src\*stm32*.* Core\Startup\*.s .cproject .project *.ioc *.bin /Q 2>nul
rd Debug /Q /S 2>nul
rd Release /Q /S /Q 2>nul

if /I not "%1"=="-s" (
    echo Done cleaning profile files!
    timeout 3 >nul
)
