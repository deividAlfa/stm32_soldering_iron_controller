## Font generation notes

<br>

If the characters are not present in the bdf font (For example, when adding a new language), the bdf font must be modified.<br>
Current approach is to copy the characters from another font.<br>

**Font files:** (Outdated, since new languages were added, but still valid for explaining the procedure)<br>

- `font_menu`:    bdf/font_menu.bdf    (Modded from t0-16-uni.bdf).<br>
        Uses cyrillic chars 1040-1103, turkish 286, 287, 350, 351 from unifont.bdf<br>
        Ohm symbol 937 is custom made.<br>

- `font_small`:     bdf/font_small.bdf    (Modded from bdf/Wizzard12.bdf)<br>
        Uses cyrillic chars 1040-1103, turkish 199, 231, 286, 287, 350, 351 from 6x13.bdf<br>

- `font_iron_temp`: bdf/ITC Avant Garde Gothic Medium_31.bdf<br>
        Only displays 0-9, C, F and ° (ASCII 176), no special characters.<br>
<br>

**Modification:**<br>

To modify and insert data from a different bdf font:<br>
 - Open the both BDFs with a text editor.<br>

 - The number of chars are defined in this line: `CHARS 64`<br>

 - Each character uses this structure:<br>

    STARTCHAR 0411        -> Start character block. The name can be anything (ex. STARTCHAR A_Letter)<br>
    ENCODING 1041         -> Actual unicode in decimal (1041 = 0x411)<br><br>
    SWIDTH 675 0<br>
    DWIDTH 15 0<br>
    BBX 12 16 2 0<br>
    BITMAP                -> Start of bitmap data<br>
    ---                   -> Bitmap data<br>
    ---<br>
    ENDCHAR               -> End of character block<br>

<br>

To add a character from one font to another, copy the STARTCHAR--->ENDCHAR blocks and paste them in the destination font.<br>
Take care to not duplicate them, always search the ENCODING number and replace the whole block.<br>
If it doesn't exists, the `CHARS` number must be increased.<br>
Now the font will have the new symbol, and be ready for conversion using bdfconv.<br>
<br>

**Generation:**<br>

Still, the new chars won't be added automatically, their Unicode numbers must be added to `make_u8g2.bat`, like this:<br>
    `-m "32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,..."`<br>
It can be single chars (32, 40, 64) or ranges (32-64).<br> 
<br>

Then run `make_u8g2.bat` to generate the new fonts.<br>
The script also inserts the required chinese characters from the font `fireflysung.ttf` using python scripts.<br>
`u8g2_aio.c` will be generated.<br>

<br>
U8g2 font structure mix octal and char representation to be as compact as possible.<br>
So when a characters starts with escape(\) the next 1,2,3 numbers will be the octal representation, a single byte.<br>
But you might also find any other symbol, like spaces, letters, these are a single character.<br>

<br>
The default fonts report wrong height and need to be patched manually.<br>

For u8g2_font_menu, replace the 10th byte, in this case \21, with \17:<br>

    Original:  "\257\0\3\2\4\4\4\4\5\10\21\0\375\12\375\13\377\1\207\3\42\5' \5\0\210\30!\7\241\214"
    Modified:  "\257\0\3\2\4\4\4\4\5\10\17\0\375\12\375\13\377\1\207\3\42\5' \5\0\210\30!\7\241\214"
                                        ^^
<br>
For u8g2_font_small, replace the 10th byte, in this case \17, with \12.<br>

    Original:  "\257\0\3\2\4\4\2\4\5\11\17\0\375\10\376\10\376\1T\2\274\4k \5\0b\5!\6\201\343"
    Modified:  "\257\0\3\2\4\4\2\4\5\11\12\0\375\10\376\10\376\1T\2\274\4k \5\0b\5!\6\201\343"
                                        ^^
<br>
For u8g2_font_iron_temp, replace the 10th byte, in this case \42 with \45.<br>

    Original:  "\16\0\5\4\5\6\4\6\7 \42\0\377 \367 \365\0\0\0\0\2Q-\11\214@\225\11~`\0"
    Modified:  "\16\0\5\4\5\6\4\6\7 \45\0\377 \367 \365\0\0\0\0\2Q-\11\214@\225\11~`\0"
                                     ^^

Copy the contents of `u8g2_aio.c` (Not the file), and replace the existing sections at the beginning of [u8g2_fonts.c](https://github.com/deividAlfa/stm32_soldering_iron_controller/blob/master/Drivers/graphics/u8g2/u8g2_fonts.c).
<br>

After that, the font modification will be done.<br>