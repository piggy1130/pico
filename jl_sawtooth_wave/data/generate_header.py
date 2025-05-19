import os

# Input and output paths
input_file = "waveforms.txt"
output_file = "waveform_data.h"

sawtooth_values = []
square_values = []

# Read the .txt file and extract columns
with open(input_file, "r") as f:
    for line in f:
        parts = line.strip().split(",")
        if len(parts) == 3:
            try:
                saw = int(float(parts[1]))
                square = int(float(parts[2]))
                sawtooth_values.append(saw)
                square_values.append(square)
            except ValueError:
                continue  # Skip malformed lines

# Write to header file
with open(output_file, "w") as f:
    f.write("#ifndef WAVEFORM_DATA_H\n#define WAVEFORM_DATA_H\n\n")
    f.write(f"#define WAVEFORM_SIZE {len(sawtooth_values)}\n\n")

    # Sawtooth array
    f.write("const uint16_t sawtooth_wave[WAVEFORM_SIZE] = {\n")
    for i, val in enumerate(sawtooth_values):
        f.write(f"  {val},")
        if (i + 1) % 10 == 0:
            f.write("\n")
    f.write("\n};\n\n")

    # Square wave array
    f.write("const uint16_t square_wave[WAVEFORM_SIZE] = {\n")
    for i, val in enumerate(square_values):
        f.write(f"  {val},")
        if (i + 1) % 10 == 0:
            f.write("\n")
    f.write("\n};\n\n")

    f.write("#endif // WAVEFORM_DATA_H\n")

print(f"✅ Generated '{output_file}' with {len(sawtooth_values)} values.")
