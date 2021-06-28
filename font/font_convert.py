import glob
import os
import re
from PIL import Image

header_body = "#include <stdint.h>\n\n"
header_body += "#define FONT_WIDTH 5\n\n"
header_body += "static const uint8_t font_error_symbol[FONT_WIDTH] = {0xfe, 0xaa, 0x92, 0xaa, 0xfe};"

image_list = glob.glob("*.png")

char_table = []

def utf8_code(ch):
    return int.from_bytes(ch.encode("utf-8"), "big")

for filename in image_list:
    basename = os.path.splitext(filename)[0]
    match_multi = re.search(r"sprites_(.)-(.).png", filename)
    match_single = re.search(r"sprite_(.).png", filename)

    if match_multi is not None:
        start_character = match_multi.group(1)
        end_character = match_multi.group(2)
    elif match_single is not None:
        start_character = match_single.group(1)
        end_character = match_single.group(1)
    else: continue

    start_code = utf8_code(start_character)
    end_code =  utf8_code(end_character)

    print("Processing %s (0x%08x - 0x%08x)" % (basename, start_code, end_code))

    image = Image.open(filename)
    image_rgb = image.convert("RGB")

    table_size = ord(end_character) - ord(start_character) + 1

    print("Table size: %d" % table_size)

    if table_size < 1 or table_size > image.width / 5:
        print("invalid table size")
        continue

    for i in range(0, table_size):
        char_bytes = [];
        for col in range(0, 5):
            col_byte = 0
            for row in range(0, 8):
                r, g, b = image_rgb.getpixel((i*5 + col, row))
                if (r + g + b) == 0:
                    col_byte |= (1 << row)

            char_bytes.append(col_byte)

        char_char = chr(ord(start_character) + i)

        char_table.append({
            "char": char_char,
            "code": utf8_code(char_char),
            "bytes": char_bytes
        })

    image.close()

if not char_table:
    print("Nothing found")
    exit(0)


char_table.sort(key=lambda x: x["code"])


table_blocks = []
current_block = {}
prev_code = 0
# split to blocks if character in different ranges
for idx, item in enumerate(char_table):
    code = item["code"]
    if code - prev_code > 1:
        if len(current_block) > 0:
            current_block["end_code"] = prev_code
            table_blocks.append(current_block)
        current_block = {"start_code": code, "data": []}

    current_block["data"].append(item)
    prev_code = code

current_block["end_code"] = prev_code
table_blocks.append(current_block)

array_body = "static const uint8_t font_data[FONT_WIDTH * %d] = {\n" % len(char_table)
function_body = "void ssd1306_mbchar(uint32_t ch) {\n";
function_body += "  uint16_t mapped_idx;\n\n";
idx = 0

for block in table_blocks:
    start_code = block["start_code"]
    end_code = block["end_code"]
    array_body += "  // 0x%08x - 0x%08x \n" % (start_code, end_code)
    first_case = function_body.endswith("idx;\n\n")

    function_body += "  "
    if not first_case:
        function_body += "} else "

    if start_code == end_code:
        function_body += "if (ch == 0x%08x) {\n" % start_code
        function_body += "    mapped_idx = %d;\n" % idx
    else:
        function_body += "if (ch >= 0x%08x && ch <= 0x%08x) {\n" % (start_code, end_code)
        function_body += "    mapped_idx = %d + (ch - 0x%08x);\n" % (idx, start_code)



    for char_data in block["data"]:
        b = char_data["bytes"]
        array_body += "  0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, // %s (0x%08x) #%d\n" % (
            b[0], b[1], b[2], b[3], b[4], char_data["char"], char_data["code"], idx
        )
        idx += 1

function_body += "  } else {\n"
function_body += "    draw_font_bytes(font_error_symbol);\n"
function_body += "    return;\n"
function_body += "  }\n\n"
function_body += "  draw_font_bytes(font_data + mapped_idx * FONT_WIDTH);\n"

function_body += "}"

array_body += "};"

# out = open("font.json", "w", encoding="utf-8")
# out.write(json.dumps(table_blocks))
# out.close()

out = open("font.c", "w", encoding="utf-8")
out.write(header_body)
out.write("\n\n")
out.write(array_body)
out.write("\n\n")
out.write(function_body)
out.write("\n\n")
out.close()
