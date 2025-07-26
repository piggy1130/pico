import serial
import time
import struct
import numpy as np
import matplotlib.pyplot as plt


PORT = "/dev/ttyACM0"     # <– make sure this matches the Pico’s device!
BAUD = 115200
NUM_SAMPLES = 5000
BYTES_TO_READ = NUM_SAMPLES * 2

# open serial
ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(0.1)  # let the port settle

# flush any leftover
ser.reset_input_buffer()
ser.reset_output_buffer()

# trigger a read
ser.write(b"GET\n")
ser.flush()

# read exactly BYTES_TO_READ bytes
data = bytearray()
while len(data) < BYTES_TO_READ:
    chunk = ser.read(BYTES_TO_READ - len(data))
    if not chunk:
        break  # timeout or disconnect
    data.extend(chunk)

print("received bytes:", len(data))
print(list(data[:20]), "...")  # print first 20 byte values

# convert to uint16 if you want
if len(data) == BYTES_TO_READ:
    import struct
    samples = struct.unpack("<" + "H"*NUM_SAMPLES, data)
    #print("first 10 samples:", samples[:10])
    print(samples)

plt.figure(figsize=(10, 4))
plt.plot(samples, linewidth=1)
plt.title("ADC Samples from Pico")
plt.xlabel("Sample Index")
plt.ylabel("ADC Value")
plt.grid(True)

# Autoscale y-axis based on your data:
plt.gca().relim()
plt.gca().autoscale_view()

# Save the figure
plt.savefig("samples_plot.png")
print("Plot saved as samples_plot.png")
