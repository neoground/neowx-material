#!/usr/bin/env python3
import argparse
import os
import re
"""
https://github.com/seehase/cheetah_template_beautifier

This script reformats Cheetah templates to improve readability by adjusting indentation
for HTML, Cheetah directives, and embedded Javascript code. 
It helps maintain a consistent code style across your templates, making them easier to read and maintain.

usage: cheetah_template_beautifier.py [-h] [-o OUTFILE] [-r] [--version] [source]

Beautifies Cheetah templates by reformatting indentation.

positional arguments:
  source                The path to the source file or directory.

options:
  -h, --help            show this help message and exit
  -o, --outfile OUTFILE
                        The path to the output file. Not valid with --recursive.
  -r, --recursive       Recursively find and reformat all *.tmpl and *.inc files in the source directory.
  --version             show program's version number and exit
"""
# Define the script version
__version__ = "1.2.1"

def reformat_cheetah_template(source_code):
    """
    Reformats a Cheetah template by adjusting indentation for HTML, Cheetah, and Javascript.

    Args:
        source_code (str): The source code of the Cheetah template.

    Returns:
        str: The reformatted code.
    """
    lines = source_code.split('\n')
    reformatted_lines = []
    indent_level = 0
    indent_size = 4
    in_multiline_tag = False

    # --- Constants ---
    void_elements = {'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'param', 'source', 'track', 'wbr'}
    tag_re = re.compile(r'<(/?)(\w+)([^>]*)>')

    for line in lines:
        stripped_line = line.strip()

        if not stripped_line:
            if not reformatted_lines or reformatted_lines[-1].strip() != "":
                reformatted_lines.append("")
            continue

        # --- 1. Calculate opens and closes for the current line ---
        opens = 0
        closes = 0

        # Determine line type by content (Cheetah > JS > HTML)
        if stripped_line.startswith('#'):
            if stripped_line.startswith(('#if', '#for', '#def', '#try')):
                opens = 1
            elif stripped_line.startswith(('#else', '#else if', '#elif', '#except')):
                opens = 1
                closes = 1
            elif stripped_line.startswith(('#end', '#end try')):
                closes = 1
        elif in_multiline_tag:
            # If we are in a multi-line tag, we don't change indentation, just look for the end
            if '>' in stripped_line:
                in_multiline_tag = False
        else:
            # Check for JS characters first
            line_no_cheetah_vars = re.sub(r'\$\{[^}]*\}', '', stripped_line)
            js_opens = line_no_cheetah_vars.count('{') + line_no_cheetah_vars.count('[')
            js_closes = line_no_cheetah_vars.count('}') + line_no_cheetah_vars.count(']')

            if js_opens > 0 or js_closes > 0:
                opens = js_opens
                closes = js_closes
            else: # Fallback to HTML rules
                for is_closing, tag, attrs in tag_re.findall(stripped_line):
                    if tag in void_elements:
                        continue
                    if is_closing:
                        closes += 1
                    elif not attrs.strip().endswith('/'): # Not a self-closing tag
                        opens += 1

                # Check if the line ends with an unclosed tag
                last_open_bracket = stripped_line.rfind('<')
                if last_open_bracket != -1:
                    # Make sure it's not a cheetah placeholder
                    if not stripped_line[last_open_bracket-1:last_open_bracket] == '$':
                        last_close_bracket = stripped_line.rfind('>')
                        if last_open_bracket > last_close_bracket:
                            in_multiline_tag = True

        # --- 2. Determine indentation for the CURRENT line ---
        current_indent_level = indent_level
        if closes > opens:
            current_indent_level -= (closes - opens)

        if stripped_line.startswith(('#else', '#elif', '#else if', '#except')):
            current_indent_level -= 1

        current_indent_level = max(0, current_indent_level)
        indent = ' ' * (current_indent_level * indent_size)
        reformatted_lines.append(indent + stripped_line)

        # --- 3. Update master indent level for the NEXT line ---
        indent_level += (opens - closes)
        indent_level = max(0, indent_level)

    # --- Final cleanup: Remove multiple blank lines ---
    final_lines = []
    for i, l in enumerate(reformatted_lines):
        is_blank = not l.strip()
        is_prev_blank = i > 0 and not reformatted_lines[i-1].strip()
        if is_blank and is_prev_blank:
            continue
        final_lines.append(l)

    return '\n'.join(final_lines)

def process_file(file_path, outfile=None):
    """Reads a file, reformats it, and writes it back out."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            source_code = f.read()
    except FileNotFoundError:
        print(f"Error: Source file not found at {file_path}")
        return

    reformatted_code = reformat_cheetah_template(source_code)

    output_file = outfile if outfile else file_path

    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(reformatted_code)
        print(f"Successfully reformatted {file_path} and saved to {output_file}")
    except IOError as e:
        print(f"Error writing to output file: {e}")

def main():
    """
    Main function to parse arguments and process the template file(s).
    """
    print(f"Cheetah Template Beautifier version {__version__}")

    parser = argparse.ArgumentParser(description="Beautifies Cheetah templates by reformatting indentation.")
    parser.add_argument("source", nargs='?', default=None, help="The path to the source file or directory.")
    parser.add_argument("-o", "--outfile", help="The path to the output file. Not valid with --recursive.")
    parser.add_argument("-r", "--recursive", action="store_true", help="Recursively find and reformat all *.tmpl and *.inc files in the source directory.")
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")
    args = parser.parse_args()

    if not args.source:
        parser.print_help()
        return

    if args.recursive:
        if not os.path.isdir(args.source):
            print("Error: --recursive requires a valid directory path.")
            return
        if args.outfile:
            print("Error: -o/--outfile cannot be used with --recursive.")
            return

        for root, _, files in os.walk(args.source):
            for file in files:
                if file.endswith((".tmpl", ".inc")):
                    file_path = os.path.join(root, file)
                    process_file(file_path)
    else:
        if not os.path.isfile(args.source):
            print(f"Error: Source file not found at {args.source}")
            return
        process_file(args.source, args.outfile)

if __name__ == "__main__":
    main()
