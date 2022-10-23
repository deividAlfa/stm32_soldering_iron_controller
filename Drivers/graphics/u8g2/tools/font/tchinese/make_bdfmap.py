def MakeMapFile(infile, outfile):
	data = open(infile, "rb").read()
	file = open(outfile, "w")
	for i in range(0, len(data), 2):
		elm = int.from_bytes(data[i:i + 2], 'little')
		if elm == 10 or elm == 13:
			continue
		file.write(f"0x{elm:02X}\t0x{elm:02X}\n")
	file.close()

if __name__ == "__main__":
	MakeMapFile("font_menu_cht.txt", "font_menu_cht.map")
	MakeMapFile("font_small_cht.txt", "font_small_cht.map")