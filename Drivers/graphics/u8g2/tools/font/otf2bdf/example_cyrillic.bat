@echo off  
set MIN=16
set MAX=18
set FONTS=	"arial.ttf"

for %%F in (%FONTS%) do (
     for /l %%S in (%MIN%, 1, %MAX%) do (
	otf2bdf.exe -o "..\bdf\%%~F_%%S_cyrillic.bdf" -p %%S "..\ttf\%%~F" -l '1040_1103'
	echo %%~F %%S
    )
)
timeout 3 >nul