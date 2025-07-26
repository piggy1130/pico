import serial
import struct
import numpy as np
import time

# ─── CONFIG ─────────────────────────────────────────────────────────────────
PORT        = '/dev/ttyACM0'    # e.g. 'COM3' on Windows, '/dev/ttyACM0' on Linux
BAUDRATE    = 115200           # USB‑CDC ignores baud, but pyserial needs something
BUF_SIZE    = 4096
PACKET_BYTES = BUF_SIZE * 2

# ─── OPEN SERIAL PORT ────────────────────────────────────────────────────────
ser = serial.Serial(PORT, BAUDRATE, timeout=1)
time.sleep(2)                 # give host time to reset and enumerate
ser.reset_input_buffer()      # flush any old data
# Read and discard until we've purged up to a full packet alignment
discard = ser.read(ser.in_waiting or PACKET_BYTES)

def read_full(ser, size):
    buf = bytearray()
    while len(buf) < size:
        chunk = ser.read(size - len(buf))
        if not chunk:
            break
        buf += chunk
    return bytes(buf)



while True:
    data = ser.read(PACKET_BYTES)
    if len(data) < PACKET_BYTES:
        # Possibly a timeout or disconnect
        print(f"⚠️  Incomplete packet: got {len(data)} bytes")
        continue

    # ─ unpack into a tuple of BUF_SIZE uint16s ─
    samples = struct.unpack('<' + 'H'*BUF_SIZE, data)

    # ─ convert to numpy for easy math/plotting ─
    arr = np.frombuffer(data, dtype=np.uint16)

    # ─ example processing: compute the mean and max ─
    mean_val = arr.mean()
    max_val  = arr.max()
    print(f"Buffer received:  mean={mean_val:.1f}, max={max_val}, size={arr.size}")

    # ─ or hand off to some plotting / storage routine ─
    # plot(arr)  # if you want to visualize

