#!/usr/bin/env python

import sys

ARRAY_SIZE = 256

def read_pdp11_words(filename):
    with open(filename, "rb") as f:
        data = f.read()

    words = []
    for i in range(0, len(data), 2):
        low = data[i]
        high = data[i+1] if i+1 < len(data) else 0
        word = low | (high << 8)
        words.append(word)

    return words

def generate_cpp_initializer(words, size):
    padded = words + [0] * (size - len(words))
    lines = []
    for i in range(0, size, 8):
        line = ", ".join(f"0x{w:04x}" for w in padded[i:i+8])
        lines.append("    " + line + ",")
    return "\n".join(lines).rstrip(",")

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_file> <array_name>", file=sys.stderr)
        sys.exit(1)

    input_file = sys.argv[1]
    array_name = sys.argv[2]

    words = read_pdp11_words(input_file)
    if len(words) > ARRAY_SIZE:
        print(f"Error: file contains more than {ARRAY_SIZE} words", file=sys.stderr)
        sys.exit(1)

    print(f"const uint16_t {array_name}[{ARRAY_SIZE}] = {{")
    print(generate_cpp_initializer(words, ARRAY_SIZE))
    print("};")

if __name__ == "__main__":
    main()

