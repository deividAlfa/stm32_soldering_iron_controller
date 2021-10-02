echo.
md out>nul
bdfconv.exe -v -b0 -f1 -m "32-126,176,196,197,199,214,220,228,229,231,246,252,286,287,304,305,350,351,937,1040-1103" "../bdf/font_menu.bdf" -o ./out/font_menu.c  -n u8g2_font_menu
