@echo off  
set MIN=18
set MAX=20
set FONTS=	"arial.ttf";^
 		"ITC Avant Garde Gothic Book Cyrillic.otf"

for %%F in (%FONTS%) do (
     for /l %%S in (%MIN%, 1, %MAX%) do (
	otf2bdf.exe -o "..\bdf\%%~F_%%S_cyrillic.bdf" -p %%S "..\ttf\%%~F" -l '1040_1103'
	echo %%~F %%S
    )
)
timeout 3 >nul