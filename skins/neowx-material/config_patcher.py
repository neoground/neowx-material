#!/usr/bin/env python3

import argparse
import collections.abc
import subprocess
import copy
"""
Source: https://github.com/seehase/python-config-patcher

Usage: usage: config_patcher.py [-h] source patch [-o OUTFILE]
"""
__version__ = "1.0.2"

def parse_config(file_path):
    config = {}
    current_section_path = []
    try:
        with open(file_path, 'r') as f:
            for line in f:
                stripped = line.strip()
                if stripped.startswith('[') and stripped.endswith(']'):
                    level = stripped.count('[')
                    if stripped.count(']') != level: continue
                    section_name = stripped.strip('[]')
                    current_section_path = current_section_path[:level - 1] + [section_name]
                    d = config
                    for part in current_section_path:
                        if part not in d: d[part] = {}
                        d = d[part]
                elif '=' in stripped and not stripped.startswith('#'):
                    try:
                        key, value = [p.strip() for p in stripped.split('=', 1)]
                        d = config
                        for part in current_section_path: d = d[part]
                        d[key] = value
                    except (ValueError, KeyError):
                        continue
    except FileNotFoundError:
        return {}
    return config

def merge_configs(source, patch):
    merged = copy.deepcopy(source)
    for key, patch_value in patch.items():
        source_value = merged.get(key)
        if isinstance(source_value, dict) and isinstance(patch_value, dict):
            merged[key] = merge_configs(source_value, patch_value)
        else:
            merged[key] = copy.deepcopy(patch_value)
    return merged

def get_section(config_dict, path):
    d = config_dict
    for part in path:
        if isinstance(d, dict) and part in d: d = d[part]
        else: return None
    return d

def format_new_items(items, level, indent_char='    '):
    lines = []
    base_indent = indent_char * level

    kv_items = {k: v for k, v in items.items() if not isinstance(v, dict)}
    for key, value in kv_items.items():
        lines.append(f"{base_indent}{key} = {value}\n")

    section_items = {k: v for k, v in items.items() if isinstance(v, dict)}
    for key, value in section_items.items():
        if value:
            header_indent = indent_char * (level)
            lines.append(f"\n{header_indent}{'[' * (level + 1)}{key}{']' * (level + 1)}\n")
            lines.extend(format_new_items(value, level + 1))
    return lines

def write_config(config, patch_config, source_file_path, output_file_path):
    source_config = parse_config(source_file_path)
    with open(source_file_path, 'r') as f:
        source_lines = f.readlines()

    output_lines = []
    section_stack = []
    deleted_section_level = float('inf')
    comment_buffer = []

    def add_new_items_for_section(section_path, section_level):
        patch_section = get_section(patch_config, section_path)
        if not patch_section:
            return

        source_section = get_section(source_config, section_path)

        new_items = {}
        for pkey, pvalue in patch_section.items():
            if not source_section or pkey not in source_section:
                new_items[pkey] = pvalue

        if new_items:
            output_lines.extend(format_new_items(new_items, section_level))

    for line in source_lines:
        stripped = line.strip()
        is_section = stripped.startswith('[') and stripped.endswith(']') and stripped.count('[') == stripped.count(']')

        if is_section:
            level = stripped.count('[')
            name = stripped.strip('[]')

            while section_stack and section_stack[-1][0] >= level:
                old_level, old_name = section_stack.pop()
                path = tuple(s[1] for s in section_stack) + (old_name,)
                add_new_items_for_section(path, old_level)

            output_lines.extend(comment_buffer)
            comment_buffer = []

            if level <= deleted_section_level: deleted_section_level = float('inf')
            if deleted_section_level < float('inf'): continue

            path = tuple(s[1] for s in section_stack) + (name,)
            if get_section(config, path) is None:
                deleted_section_level = level
                continue

            section_stack.append((level, name))
            output_lines.append(line)

        elif deleted_section_level < float('inf'):
            continue
        elif '=' in stripped and not stripped.startswith('#'):
            output_lines.extend(comment_buffer)
            comment_buffer = []

            try:
                key, _ = [p.strip() for p in stripped.split('=', 1)]
                path = tuple(s[1] for s in section_stack)

                merged_section = get_section(config, path)
                if merged_section and key in merged_section:
                    indent = len(line) - len(line.lstrip(' '))
                    output_lines.append(f"{' ' * indent}{key} = {merged_section[key]}\n")
            except (ValueError, KeyError):
                output_lines.append(line)
        else:
            comment_buffer.append(line)

    while section_stack:
        level, name = section_stack.pop()
        path = tuple(s[1] for s in section_stack) + (name,)
        add_new_items_for_section(path, level)

    output_lines.extend(comment_buffer)

    # Add new top-level sections from the patch
    for section_name, section_content in patch_config.items():
        if section_name not in source_config:
            output_lines.append(f"\n[{section_name}]\n")
            output_lines.extend(format_new_items(section_content, 1))

    with open(output_file_path, 'w') as f:
        f.writelines(output_lines)

if __name__ == '__main__':
    print(f"config_patcher.py {__version__}")
    parser = argparse.ArgumentParser(
        usage="%(prog)s [-h] source patch [-o OUTFILE]",
        description='Patch a config file and format the output.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Examples:
  python3 config_patcher.py source.conf source.patch
  python3 config_patcher.py source.conf source.patch -o patched.conf"""
    )
    parser.add_argument('source', help='The source config file.')
    parser.add_argument('patch', help='The patch config file.')
    parser.add_argument('-o', '--outfile', help='The output file.')
    parser.add_argument('-v', '--version', action='version', version=f"%(prog)s {__version__}")
    args = parser.parse_args()

    source_config = parse_config(args.source)
    patch_config = parse_config(args.patch)
    merged_config = merge_configs(source_config, patch_config)
    output_file = args.outfile if args.outfile else args.source
    write_config(merged_config, patch_config, args.source, output_file)
