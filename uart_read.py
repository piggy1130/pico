import serial
import struct
import time

SERIAL_PORT = '/dev/ttyAMA0'  # or /dev/ttyUSB0, /dev/ttyACM0 depending on setup
BAUD_RATE = 115200
SAMPLE_COUNT = 5000
BYTES_PER_FRAME = SAMPLE_COUNT * 2  # 2 bytes per uint16

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

def read_frame():
    """Read exactly one frame (5000 uint16 samples)"""
    buffer = bytearray()
    while len(buffer) < BYTES_PER_FRAME:
        chunk = ser.read(BYTES_PER_FRAME - len(buffer))
        if not chunk:
            print("⚠️ Timeout or no data")
            return None
        buffer.extend(chunk)
    
    # Unpack the binary data into Python integers
    return struct.unpack('<' + 'H'*SAMPLE_COUNT, buffer)

try:
    while True:
        data = read_frame()
        if data:
            #print("✅ Got frame:", data)
            print("✅ Got frame:", data[:10], "...", data[-5:])  # Print first/last samples
        else:
            print("⚠️ Skipped a frame due to timeout")
        # Optional: sleep or throttle here if needed
        # time.sleep(0.01)

except KeyboardInterrupt:
    print("🔚 Stopped by user")
finally:
    ser.close()
