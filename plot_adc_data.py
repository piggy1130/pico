import serial
import struct
from collections import deque
import matplotlib.pyplot as plt
import matplotlib.animation as animation

ser = serial.Serial('/dev/ttyACM1', 115200, timeout=1)
data = deque(maxlen=200)

fig, ax = plt.subplots()
line, = ax.plot([], [])
ax.set_ylim(0, 4095)
ax.set_xlim(0, 200)

def update(frame):
    while ser.in_waiting >= 2:
        raw = ser.read(2)
        value = struct.unpack('<H', raw)[0]
        print("Reading:", value);
        data.append(value)
    line.set_data(range(len(data)), list(data))
    return line,

ani = animation.FuncAnimation(fig, update, interval=50)
plt.show()
