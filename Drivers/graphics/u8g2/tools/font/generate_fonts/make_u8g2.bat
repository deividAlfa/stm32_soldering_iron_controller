@echo off
pushd "%~dp0"

python make_bdfmap.py
..\otf2bdf\otf2bdf.exe -m font_menu_cht.map -p 9 -o font_menu_cht.bdf fireflysung.ttf
..\otf2bdf\otf2bdf.exe -m font_small_cht.map -p 8 -o font_small_cht.bdf fireflysung.ttf

python make_bdfmux.py
..\bdfconv\bdfconv.exe -v -b 0 -f 1 -m "32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535" -o font_menu.c -n u8g2_font_menu font_menu_all.bdf
..\bdfconv\bdfconv.exe -v -b 0 -f 1 -m "32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535" -o font_small.c -n u8g2_font_small font_small_all.bdf
copy /b font_menu.c + font_small.c u8g2_aio.c

del *cht.map
del *cht.bdf
del *all.bdf
del font_*.c
