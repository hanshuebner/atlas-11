#!/usr/bin/env python3
import sys
import svgwrite

# Configuration
font_size_in = 0.07  # inches
line_height_in = 0.1  # inches
dpi = 96  # typical screen DPI
font_size_px = font_size_in * dpi
line_height_px = line_height_in * dpi
margin = 10  # px

def apply_overline(text):
    return ''.join(c + '\u0305' for c in text)

# Read and parse input
labels = []
for line in sys.stdin:
    line = line.rstrip('\n')
    if not line.strip():
        labels.append("")
    elif line.startswith('/'):
        labels.append(apply_overline(line[1:]))
    else:
        labels.append(line)

# SVG size
num_lines = len(labels)
height = int(line_height_px * num_lines + 2 * margin)
width = 300  # generous width for a single column

# Create SVG drawing
dwg = svgwrite.Drawing(size=(width, height))

# Draw single column
for i, label in enumerate(labels):
    y = margin + i * line_height_px + font_size_px
    dwg.add(dwg.text(label, insert=(margin, y), font_size=font_size_px, font_family="monospace"))

# Output to stdout
dwg.write(sys.stdout)
