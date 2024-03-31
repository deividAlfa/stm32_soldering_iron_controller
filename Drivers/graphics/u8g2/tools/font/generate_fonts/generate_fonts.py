import os
def to_unicode(string):
        ret = ""
        for v in string:
                ret = ret + hex(ord(v)).upper()
        return ret
def MakeChFile(infile, outfile):
        data = open(infile, "r", encoding="utf-8").read()
        #Filter duplicates
        atomText = set(data)
        file = open(outfile, "w", encoding="utf-8")
        for c in atomText:
                #If it is not a Chinese character, skip it
                if (ord(c) < 19968):
                        continue
                file.write(c)
        file.close
        
#def ReplacePixe(infile):
        #data = open(infile, "r", encoding="utf-8").read()
        # ...??
        # ...??
        #file = open(infile, "w", encoding="utf-8")
        #file.write(data)
        #file.close
        
def MakeMapFile(infile, outfile):
        data = open(infile, "r", encoding="utf-8").read()
        #Filter duplicates
        atomText = set(data)
        unicodeText = []
        for c in atomText:
                #If it is not a Chinese character, skip it
                if (ord(c) < 19968):
                        continue
                unicodeText.append(to_unicode(c))
        unicodeText.sort()
        mapFileDatas = ""
        for u in unicodeText:
                mapFileDatas += u
                if (len(mapFileDatas) > 0):
                        mapFileDatas += "{0}{1}\n".format("	", u)
        #Additional characters                
        mapFileDatas += "{1}{0}{1}\n".format("	", hex(ord("└")))
        file = open(outfile, "w", encoding="utf-8")
        file.write(mapFileDatas)
        file.close()

def MuxMapFile(eng, ch, out):
	eng_data = open(eng).read()
	start = eng_data.find("CHARS ") + 6
	end = eng_data.find("STARTCHAR")
	eng_chars = int(eng_data[start:end])
	
	ch_data = open(ch).read()
	start = ch_data.find("CHARS ") + 6
	end = ch_data.find("STARTCHAR")
	ch_chars = int(ch_data[start:end])
	
	index = eng_data.find("ENDFONT")
	start = ch_data.find("STARTCHAR")
	end = ch_data.find("ENDFONT")
	
	out_comt = "COMMENT Traditional Chinese Fonts start from here!!!\n"
	#out_comt = out_comt + "COMMENT Please add fonts above for better maintenance\n"
	out_data = eng_data[:index] + out_comt + ch_data[start:end] + eng_data[index:]
	out_data = out_data.replace(f"CHARS {eng_chars}", f"CHARS {eng_chars + ch_chars}")
	open(out, "w").write(out_data)
	
if __name__ == '__main__':
        #Read the "gui_strings.c" file under the project, extract the list of Chinese characters without repeated Chinese characters
        MakeChFile("..\\..\\..\\..\\gui\\screens\\gui_strings.c", "font_menu_ch.txt")
        
        MakeMapFile("font_menu_ch.txt", "font_menu_ch.map")
        MakeMapFile("font_small_ch.txt", "font_small_ch.map")
        os.system("..\\otf2bdf\\otf2bdf.exe -m font_menu_ch.map -p 9 fireflysung.ttf -o font_menu_ch.bdf")
        os.system("..\\otf2bdf\\otf2bdf.exe -m font_small_ch.map -p 8 fireflysung.ttf -o font_small_ch.bdf")
        MuxMapFile("../bdf/font_menu.bdf", "font_menu_ch.bdf", "font_menu_all.bdf")
        MuxMapFile("../bdf/font_small.bdf", "font_small_ch.bdf", "font_small_all.bdf")
        os.system("..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535\" -o font_menu.c -n u8g2_font_menu font_menu_all.bdf")
        os.system("..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535\" -o font_small.c -n u8g2_font_small font_small_all.bdf")
        os.system("..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"45,48-57,67,70,176\" \"../bdf/ITC Avant Garde Gothic Medium_31.bdf\" -o font_iron_temp.c  -n u8g2_font_iron_temp")
        os.system("copy /b font_iron_temp.c + font_menu.c + font_small.c u8g2_aio.txt")

        os.system("del *ch.map && del *ch.bdf && del *all.bdf && del font_*.c")
