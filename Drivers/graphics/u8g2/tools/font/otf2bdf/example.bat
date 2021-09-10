@echo off  
set MIN=20
set MAX=31
set FONTS=	"ITC Avant Garde Gothic Medium";^
 		"ITC Avant Garde Gothic CE Book";^
 		"ITC Avant Garde Gothic Book Condensed"

for %%F in (%FONTS%) do (
     for /l %%S in (%MIN%, 1, %MAX%) do (
	otf2bdf.exe -o "..\bdf\%%~F_%%S.bdf" -p %%S "..\ttf\%%~F.otf"
	echo %%~F %%S
    )
)