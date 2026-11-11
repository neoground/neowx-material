#!/usr/bin/env python3
"""
Source: https://github.com/seehase/python-config-beautifier

usage: config_beautifier.py [-h] [-o OUTPUT] [--indent INDENT] [--version] source


Config file beautifier for Python config files in dictionary format.

This script lints and beautifies configuration files with sections, subsections,
and key-value pairs, ensuring proper indentation and formatting.

Format specifications:
- Sections: [section_name]
- Subsections: [[subsection_name]], [[[nested_subsection]]], etc.
- Key-value pairs: key = value
- Comments: # comment
- Indentation: 4 spaces per level
- Maximum 1 blank line between sections
"""

import argparse
import re
import sys
from typing import List, Optional

__version__ = "1.0.0"


class ConfigLine:
    """Represents a line in the config file with its type and content."""

    def __init__(self, content: str, line_type: str, indent_level: int = 0):
        self.content = content.strip()
        self.line_type = line_type
        self.indent_level = indent_level


class ConfigParser:
    """Parses config file content into a list of ConfigLine objects."""

    def __init__(self):
        self.section_stack: List[int] = []

    def parse_line(self, line: str) -> ConfigLine:
        stripped = line.strip()
        if not stripped:
            return ConfigLine("", 'blank')

        if stripped.startswith('#'):
            indent = len(self.section_stack)
            return ConfigLine(stripped, 'comment', indent)

        match = re.match(r'^(\[+)(.*?)(\]+)$', stripped)
        if match:
            left_brackets, name, right_brackets = match.groups()
            if len(left_brackets) != len(right_brackets):
                raise ValueError(f"Mismatched brackets in section: {stripped}")

            level = len(left_brackets) - 1
            while self.section_stack and self.section_stack[-1] >= level:
                self.section_stack.pop()
            self.section_stack.append(level)
            return ConfigLine(stripped, 'section', level)

        if '=' in stripped:
            key, value = [p.strip() for p in stripped.split('=', 1)]
            return ConfigLine(f'{key} = {value}', 'key_value', len(self.section_stack))

        return ConfigLine(stripped, 'content', len(self.section_stack))

    def parse_file(self, content: str) -> List[ConfigLine]:
        self.section_stack = []
        lines = []
        for line_num, line in enumerate(content.splitlines(), 1):
            try:
                lines.append(self.parse_line(line))
            except ValueError as e:
                sys.exit(f"Error on line {line_num}: {e}")
        return lines


class ConfigFormatter:
    """Formats a list of ConfigLine objects into a string."""

    def __init__(self, indent_spaces: int = 4):
        self.indent_string = ' ' * indent_spaces

    def format_line(self, line: ConfigLine) -> str:
        if line.line_type == 'blank':
            return ''
        return f"{self.indent_string * line.indent_level}{line.content}"

    def format_content(self, lines: List[ConfigLine]) -> str:
        return '\n'.join(self.format_line(line) for line in lines) + '\n'


class ConfigBeautifier:
    """Applies beautification rules to a list of ConfigLine objects."""

    def beautify(self, content: str, indent_spaces: int) -> str:
        parser = ConfigParser()
        lines = parser.parse_file(content)

        lines = self._adjust_header_comments(lines)
        lines = self._apply_spacing_rules(lines)
        self._validate_config(lines)

        formatter = ConfigFormatter(indent_spaces)
        return formatter.format_content(lines)

    def _adjust_header_comments(self, lines: List[ConfigLine]) -> List[ConfigLine]:
        i = 0
        while i < len(lines):
            if lines[i].line_type == 'comment':
                next_section_idx = -1
                for j in range(i + 1, len(lines)):
                    if lines[j].line_type == 'section':
                        next_section_idx = j
                        break
                    if lines[j].line_type not in ['comment', 'blank']:
                        break

                if next_section_idx != -1:
                    section_level = lines[next_section_idx].indent_level
                    for k in range(i, next_section_idx):
                        if lines[k].line_type == 'comment':
                            lines[k].indent_level = section_level
                    i = next_section_idx
                    continue
            i += 1
        return lines

    def _apply_spacing_rules(self, lines: List[ConfigLine]) -> List[ConfigLine]:
        if not lines:
            return []

        result: List[ConfigLine] = []
        for i, line in enumerate(lines):
            prev_line = result[-1] if result else None

            # Rule: Skip blank lines before comments that precede sections
            if line.line_type == 'blank':
                next_line_idx = i + 1
                while next_line_idx < len(lines) and lines[next_line_idx].line_type == 'blank':
                    next_line_idx += 1
                if next_line_idx < len(lines):
                    next_line = lines[next_line_idx]
                    if next_line.line_type == 'comment' and self._is_comment_before_section(lines, next_line_idx):
                        continue

            # Rule: Add blank line after section content ends
            if prev_line and prev_line.line_type == 'key_value' and self._next_line_is_section_or_comment(lines, i):
                if not (result and result[-1].line_type == 'blank'):
                    result.append(ConfigLine("", 'blank'))

            result.append(line)

            # Rule: Add blank line before top-level sections
            if i < len(lines) - 1:
                next_line = lines[i+1]
                if (next_line.line_type == 'section' and next_line.indent_level == 0 and
                        line.line_type not in ['blank', 'comment']):
                    result.append(ConfigLine("", 'blank'))
                elif (next_line.line_type == 'comment' and line.line_type not in ['blank', 'comment'] and
                      self._is_comment_before_toplevel_section(lines, i + 1)):
                    result.append(ConfigLine("", 'blank'))

        # Rule: Remove consecutive blank lines
        final_result: List[ConfigLine] = []
        for i, line in enumerate(result):
            if line.line_type == 'blank' and i > 0 and result[i-1].line_type == 'blank':
                continue
            final_result.append(line)

        # Rule: Remove leading/trailing blank lines
        while final_result and final_result[0].line_type == 'blank':
            final_result.pop(0)
        while final_result and final_result[-1].line_type == 'blank':
            final_result.pop()

        return final_result

    def _is_comment_before_section(self, lines: List[ConfigLine], comment_index: int) -> bool:
        for j in range(comment_index + 1, len(lines)):
            if lines[j].line_type == 'blank':
                continue
            return lines[j].line_type == 'section'
        return False

    def _is_comment_before_toplevel_section(self, lines: List[ConfigLine], comment_index: int) -> bool:
        for j in range(comment_index + 1, len(lines)):
            if lines[j].line_type == 'blank':
                continue
            return lines[j].line_type == 'section' and lines[j].indent_level == 0
        return False

    def _next_line_is_section_or_comment(self, lines: List[ConfigLine], start_idx: int) -> bool:
        for i in range(start_idx, len(lines)):
            line = lines[i]
            if line.line_type == 'blank':
                continue
            return line.line_type == 'section' or (line.line_type == 'comment' and self._is_comment_before_section(lines, i))
        return False

    def _validate_config(self, lines: List[ConfigLine]):
        sections = {}
        current_path = []
        for line in lines:
            if line.line_type == 'section':
                level = line.indent_level
                current_path = current_path[:level]

                match = re.match(r'\[+([^\]]+)\]+', line.content)
                if match:
                    section_name = match.group(1)
                    current_path.append(section_name)
                    path_key = '/'.join(current_path)
                    if path_key in sections:
                        print(f"Warning: Duplicate section found: {path_key}", file=sys.stderr)
                    sections[path_key] = True

def main():
    print(f"config_beautifier.py {__version__}")
    parser = argparse.ArgumentParser(
        description='Beautify Python config files.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="Example: python config_beautifier.py input.conf -o output.conf"
    )
    parser.add_argument('source', help='Source config file')
    parser.add_argument('-o', '--output', help='Output file (default: overwrite source)')
    parser.add_argument('--indent', type=int, default=4, help='Number of spaces for indentation')
    parser.add_argument('--version', action='version', version=f'%(prog)s {__version__}')
    args = parser.parse_args()

    try:
        with open(args.source, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        sys.exit(f"Error: File '{args.source}' not found.")

    beautifier = ConfigBeautifier()
    beautified_content = beautifier.beautify(content, args.indent)

    output_file = args.output or args.source
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(beautified_content)

    print(f"Successfully beautified '{args.source}' -> '{output_file}'")

if __name__ == '__main__':
    main()
