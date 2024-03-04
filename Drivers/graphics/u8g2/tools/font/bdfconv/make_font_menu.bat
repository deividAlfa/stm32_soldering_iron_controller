echo.
md out>nul
bdfconv.exe -v -b0 -f1 -m "32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103" "../bdf/font_menu.bdf" -o ./out/font_menu.c  -n u8g2_font_menu
