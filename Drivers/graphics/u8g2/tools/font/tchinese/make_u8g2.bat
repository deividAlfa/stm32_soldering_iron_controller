@echo off
pushd "%~dp0"
python make_bdfmap.py
..\otf2bdf\otf2bdf.exe -m font_menu_cht.map -p 9 -o font_menu_cht.bdf fireflysung.ttf
..\otf2bdf\otf2bdf.exe -m font_small_cht.map -p 8 -o font_small_cht.bdf fireflysung.ttf
python make_bdfmux.py
..\bdfconv\bdfconv.exe -v -b0 -f1 -m "32-126,176,196,197,199,214,220,228,229,231,246,252,286,287,304,305,350,351,937,1040-1103,5000-65535" -o font_menu.c -n u8g2_font_menu font_menu_all.bdf
..\bdfconv\bdfconv.exe -v -b0 -f1 -m "32-126,176,196,197,199,200,214,220,228,229,231,246,252,286,287,304,305,350,351,937,1040-1103,5000-65535" -o font_small.c -n u8g2_font_small font_small_all.bdf
copy /b font_menu.c + font_small.c u8g2_aio.c
del *cht.map
del *cht.bdf
del *all.bdf
del font_*.c
