import numpy as np

# Parameters
duration = 0.1                    # seconds for each period
num_points = 5000                # samples
max_value = 4095                  # peak DAC value
max_voltage = 5.0
fall_fraction = 0.1               # 10% for falling edge

# Voltage range: 0.7V to 4.3V
v_min = 0.7
v_max = 4.3
dac_min = int(v_min / max_voltage * max_value)  # ~573
dac_max = int(v_max / max_voltage * max_value)  # ~3521
dac_range = dac_max - dac_min

# Time array
t = np.linspace(0, duration, num_points, endpoint=False)

# Generate asymmetric sawtooth wave
sawtooth_wave = np.zeros_like(t)
rise_time = duration * (1 - fall_fraction)
fall_time = duration * fall_fraction

# Slow rise
rise_indices = t < rise_time
sawtooth_wave[rise_indices] = dac_min + dac_range * (t[rise_indices] / rise_time)

# Fast fall
fall_indices = ~rise_indices
sawtooth_wave[fall_indices] = dac_min + dac_range * (1 - (t[fall_indices] - rise_time) / fall_time)

# # Generate square wave: high between 0.02s and 0.05s
# square_wave = np.where((t >= 0.02) & (t < 0.05), 1, 0)

output_path = "waveforms.txt"
# Save to file
with open(output_path, "w") as f:
    # for time, saw_val, square_val in zip(t, sawtooth_wave, square_wave):
    #     f.write(f"{time:.6f}, {int(saw_val)}, {int(square_val)}\n")
    for time, saw_val in zip(t, sawtooth_wave):
        f.write(f"{time:.6f}, {int(saw_val)}\n")
print("Asymmetric waveforms saved to 'waveforms.txt'")
