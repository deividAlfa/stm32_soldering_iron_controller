import re
import subprocess
import os
import time

# Universal file reading function
def read_file(file_path):
    """Read the content of a file."""
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as file:
            return file.read()

# Universal file writing function
def write_file(file_path, content):
    """Write content to a file."""
    with open(file_path, "w", encoding="utf-8") as file:
        file.write(content)

# Function to extract specific characters
def extract_chars(code, filter_condition):
    """Extract specific variables and their values from the code based on a filter condition."""
    regex_string = r'(\b[a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.*)'
    matches = re.findall(regex_string, code)
    filtered_assignments = [(var, val) for var, val in matches if filter_condition(var)]
    chars = "".join(value for var, value in filtered_assignments)
    filtered_chars = set(filter(lambda c: ord(c) >= 19968, chars))
    return filtered_chars

# Function to generate a mapping file
def make_map_file(file_path):
    """Generate a mapping file from the content of a text file."""
    if os.path.exists(file_path):
        data = read_file(file_path)
        unicode_text = [hex(ord(c)).upper() + "\t" + hex(ord(c)).upper() for c in data]
        unicode_text.sort()
        return "\n".join(unicode_text)

# Function to run external commands
def run_command(command):
    """Execute an external command in the shell."""
    subprocess.run(command, shell=True)

# Function to replace pixels in a file
def replace_pixel(file_path, ustr, nstr):
    """Replace a specific string with another in a file."""
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()
        lines[7] = lines[7].replace(ustr, nstr)
        with open(file_path, 'w', encoding='utf-8') as file:
            file.writelines(lines)

# Function to merge mapping files
def mux_map_file(eng_path, ch_path, out_path):
    """Merge two mapping files into one."""
    eng_data = read_file(eng_path)
    start = eng_data.find("CHARS ") + 6
    end = eng_data.find("STARTCHAR")
    eng_chars = int(eng_data[start:end])
    ch_data = read_file(ch_path)
    start = ch_data.find("CHARS ") + 6
    end = ch_data.find("STARTCHAR")
    ch_chars = int(ch_data[start:end])
    index = eng_data.find("ENDFONT")
    start = ch_data.find("STARTCHAR")
    end = ch_data.find("ENDFONT")
    out_comt = "COMMENT Traditional Chinese Fonts start from here!!!\n"
    out_data = eng_data[:index] + out_comt + ch_data[start:end] + eng_data[index:]
    out_data = out_data.replace(f"CHARS {eng_chars}", f"CHARS {eng_chars + ch_chars}")
    write_file(out_path, out_data)

# Main function
def main():
    """Main function to organize the flow of operations."""
    gui_strings_path = os.path.join("..", "..", "..", "..", "gui", "screens", "gui_strings.c")
    output_dir = "output"
    
    # Ensure the output directory exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Generate menu files and mapping files
    menu_chars = extract_chars(read_file(gui_strings_path), lambda var: 'main_mode_' not in var and 'main_error_' not in var)
    small_chars = extract_chars(read_file(gui_strings_path), lambda var: 'main_mode' in var or 'main_error' in var)

    write_file(os.path.join(output_dir, "font_menu_ch.txt"), "".join(menu_chars) + f"简繁└")
    write_file(os.path.join(output_dir, "font_small_ch.txt"), "".join(small_chars))

    map_file_data_menu = make_map_file(os.path.join(output_dir, "font_menu_ch.txt"))
    map_file_data_small = make_map_file(os.path.join(output_dir, "font_small_ch.txt"))

    write_file(os.path.join(output_dir, "font_menu_ch.map"), map_file_data_menu)
    write_file(os.path.join(output_dir, "font_small_ch.map"), map_file_data_small)
    
    # Call external programs to generate BDF files
    run_command(f"..\\otf2bdf\\otf2bdf.exe -m {os.path.join(output_dir, 'font_menu_ch.map')} -p 9 fireflysung.ttf -o {os.path.join(output_dir, 'font_menu_ch.bdf')}")
    run_command(f"..\\otf2bdf\\otf2bdf.exe -m {os.path.join(output_dir, 'font_small_ch.map')} -p 8 fireflysung.ttf -o {os.path.join(output_dir, 'font_small_ch.bdf')}")
    
    # Merge mapping files
    mux_map_file(os.path.join("..", "bdf", "font_menu.bdf"), os.path.join(output_dir, "font_menu_ch.bdf"), os.path.join(output_dir, "font_menu_all.bdf"))
    mux_map_file(os.path.join("..", "bdf", "font_small.bdf"), os.path.join(output_dir, "font_small_ch.bdf"), os.path.join(output_dir, "font_small_all.bdf"))
    
    # Call external programs to generate C files
    run_command(f"..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535\" -o {os.path.join(output_dir, 'font_menu.c')} -n u8g2_font_menu {os.path.join(output_dir, 'font_menu_all.bdf')}")
    run_command(f"..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"32-126,160-255,268-271,282-283,286,287,304,305,327,328,344,345,350-353,356,357,366,367,381,382,937,1040-1103,5000-65535\" -o {os.path.join(output_dir, 'font_small.c')} -n u8g2_font_small {os.path.join(output_dir, 'font_small_all.bdf')}")
    run_command(f"..\\bdfconv\\bdfconv.exe -v -b 0 -f 1 -m \"45,48-57,67,70,176\" \"../bdf/ITC Avant Garde Gothic Medium_31.bdf\" -o {os.path.join(output_dir, 'font_iron_temp.c')}  -n u8g2_font_iron_temp")
    
    # Replace specific characters
    replace_pixel(os.path.join(output_dir, "font_iron_temp.c"), "\\42\\", "\\45\\")
    replace_pixel(os.path.join(output_dir, "font_menu.c"), "\\21\\", "\\17\\")
    replace_pixel(os.path.join(output_dir, "font_small.c"), "\\16\\", "\\12\\")
    
    # Merge C files
    font_files = [os.path.join(output_dir, "font_iron_temp.c"),
                  os.path.join(output_dir, "font_menu.c"),
                  os.path.join(output_dir, "font_small.c")]
    combined_font_file = "u8g2_aio.c"
    with open(combined_font_file, "w") as combined_file:
        for font_file in font_files:
            with open(font_file, "r") as f:
                combined_file.write(f.read())
    
    # Clean up temporary files
    for font_file in font_files:
        os.remove(font_file)

if __name__ == '__main__':
    start_time = time.perf_counter()
    main()
    end_time = time.perf_counter()
    execution_time = end_time - start_time
    print(f"u8g2_aio.c Generation Finished!\nExecution Time {execution_time:.3f} seconds!")
    os.system("pause")
