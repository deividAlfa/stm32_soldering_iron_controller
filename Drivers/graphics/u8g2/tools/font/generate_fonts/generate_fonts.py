import os
import subprocess
import traceback

def hasChineseChar(data: str):
    chr_list = set(data)
    truth_table = map(lambda elm : ord(elm) > 5000, chr_list)
    return any(truth_table)

def makeBDFMapFile(data: list, file: str):
    map_list = map(lambda elm : f"0x{ord(elm):02X}\t0x{ord(elm):02X}\n", data)
    open(file, "w", encoding="utf-8").write("".join(map_list))
    return

def exec_command(parameters: str | list):
    subprocess.run(parameters, shell=True)
    return

def muxBDFMapFile(engFile: str, chiFile: str, outFile: str):
    eng_data = open(engFile, encoding="utf-8").read()
    i = eng_data.find("CHARS ") + 6
    j = eng_data.find("STARTCHAR")
    eng_nums = int(eng_data[i:j])

    chi_data = open(chiFile, encoding="utf-8").read()
    i = chi_data.find("CHARS ") + 6
    j = chi_data.find("STARTCHAR")
    cht_nums = int(chi_data[i:j])

    ei = eng_data.rfind("ENDFONT")
    ci = chi_data.find("STARTCHAR")
    cj = chi_data.rfind("ENDFONT")

    out_comt = "COMMENT Chinese Fonts start from here!!!\n"
    # out_comt = out_comt + "COMMENT Please add fonts above for better maintenance\n"
    out_data = eng_data[:ei] + out_comt + chi_data[ci:cj] + eng_data[ei:]
    out_data = out_data.replace(f"CHARS {eng_nums}", f"CHARS {eng_nums + cht_nums}")

    open(outFile, "w", encoding="utf-8").write(out_data)
    return

def replaceFileString(file: str, old: str, new: str):
    data = open(file, encoding="utf-8").read()
    data = data.replace(old, new, 1)
    open(file, "w", encoding="utf-8").write(data)
    return

if __name__ == "__main__":

    try:
        os.chdir(os.path.dirname(__file__))
        os.makedirs("temp", exist_ok=True)

        # Read all chinese lines from gui_strings.c
        gui_lines = open(r"..\..\..\..\gui\screens\gui_strings.c", encoding="utf-8").readlines()
        chi_lines = list(filter(lambda elm : hasChineseChar(elm), gui_lines))

        # Separate menu and small lines from all chinese lines
        chi_menu_lines = list(filter(lambda elm : ".main_error_" not in elm and ".main_mode_" not in elm, chi_lines))
        chi_smol_lines = list(filter(lambda elm : elm not in chi_menu_lines, chi_lines))

        # Extract only chinese characters from lines
        chi_menu_chars = sorted(set(filter(lambda elm : ord(elm) > 5000, "".join(chi_menu_lines))))
        chi_smol_chars = sorted(set(filter(lambda elm : ord(elm) > 5000, "".join(chi_smol_lines))))
        open(r"temp\font_menu_chi.txt", "w", encoding="utf-8").write("\n".join(chi_menu_chars))
        open(r"temp\font_smol_chi.txt", "w", encoding="utf-8").write("\n".join(chi_smol_chars))

        # Generate chinese BDF files
        makeBDFMapFile(chi_menu_chars, r"temp\font_menu_chi.map")
        makeBDFMapFile(chi_smol_chars, r"temp\font_smol_chi.map")
        exec_command([r"..\otf2bdf\otf2bdf.exe", "-m", r"temp\font_menu_chi.map", "-p", "9", "-o", r"temp\font_menu_chi.bdf", "fireflysung.ttf"])
        exec_command([r"..\otf2bdf\otf2bdf.exe", "-m", r"temp\font_smol_chi.map", "-p", "8", "-o", r"temp\font_smol_chi.bdf", "fireflysung.ttf"])

        # Merge english and chinese BDF files
        muxBDFMapFile(r"..\bdf\font_menu.bdf", r"temp\font_menu_chi.bdf", r"temp\font_menu_all.bdf")
        muxBDFMapFile(r"..\bdf\font_small.bdf", r"temp\font_smol_chi.bdf", r"temp\font_smol_all.bdf")

        # Generate C language font source
        uni_mapset = "32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535"
        exec_command([r"..\bdfconv\bdfconv.exe", "-v", "-b", "0", "-f", "1", "-m", uni_mapset, "-o", r"temp\font_menu.c", "-n", "u8g2_font_menu", r"temp\font_menu_all.bdf"])
        exec_command([r"..\bdfconv\bdfconv.exe", "-v", "-b", "0", "-f", "1", "-m", uni_mapset, "-o", r"temp\font_smol.c", "-n", "u8g2_font_small", r"temp\font_smol_all.bdf"])
        exec_command([r"..\bdfconv\bdfconv.exe", "-v", "-b", "0", "-f", "1", "-m", "45,48-57,67,70,176", "-o", r"temp\font_iron_temp.c", "-n", "u8g2_font_iron_temp", r"..\bdf\ITC Avant Garde Gothic Medium_31.bdf"])

        # Patch incorrect font height
        replaceFileString(r"temp\font_iron_temp.c", "\\42\\", "\\45\\")
        replaceFileString(r"temp\font_menu.c", "\\21\\", "\\17\\")
        replaceFileString(r"temp\font_smol.c", "\\16\\", "\\12\\")

        # Combine all font sources into one file
        exec_command(["copy", "/b", r"temp\font_iron_temp.c", "+", r"temp\font_menu.c", "+", r"temp\font_smol.c", "u8g2_aio.c"])

    except Exception:
        # Print error message
        traceback.print_exc()

    finally:
        # Cleanup unnecessary files
        exec_command("rmdir /s /q temp")
        os.system("timeout 3")