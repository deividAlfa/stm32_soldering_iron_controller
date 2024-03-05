def MuxMapFile(eng, cht, out):
	eng_data = open(eng).read()
	start = eng_data.find("CHARS ") + 6
	end = eng_data.find("STARTCHAR")
	eng_chars = int(eng_data[start:end])
	
	cht_data = open(cht).read()
	start = cht_data.find("CHARS ") + 6
	end = cht_data.find("STARTCHAR")
	cht_chars = int(cht_data[start:end])
	
	index = eng_data.find("ENDFONT")
	start = cht_data.find("STARTCHAR")
	end = cht_data.find("ENDFONT")
	
	out_comt = "COMMENT Traditional Chinese Fonts start from here!!!\n"
	#out_comt = out_comt + "COMMENT Please add fonts above for better maintenance\n"
	out_data = eng_data[:index] + out_comt + cht_data[start:end] + eng_data[index:]
	out_data = out_data.replace(f"CHARS {eng_chars}", f"CHARS {eng_chars + cht_chars}")
	
	open(out, "w").write(out_data)
	pass

if __name__ == "__main__":
	MuxMapFile("../bdf/font_menu.bdf", "font_menu_cht.bdf", "font_menu_all.bdf")
	MuxMapFile("../bdf/font_small.bdf", "font_small_cht.bdf", "font_small_all.bdf")