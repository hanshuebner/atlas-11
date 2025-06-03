import sys
import re

def update_kicad_layers(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        content = f.read()

    # Change all fp_text reference from F.SilkS to F.Fab
    content = re.sub(
        r'(\(fp_text\s+reference\s+".+?"\s+.+?\(layer\s+")F\.SilkS(")',
        r'\1F.Fab\2',
        content,
        flags=re.DOTALL
    )

    # Change all fp_text value from F.Fab to F.SilkS
    content = re.sub(
        r'(\(fp_text\s+value\s+".+?"\s+.+?\(layer\s+")F\.Fab(")',
        r'\1F.SilkS\2',
        content,
        flags=re.DOTALL
    )

    backup = filename + '.bak'
    with open(backup, 'w', encoding='utf-8') as f:
        f.write(content)  # Save original as backup

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Layer updates complete. Backup saved as: {backup}")

# Usage:
# python update_layers.py atlas-11.kicad_pcb
