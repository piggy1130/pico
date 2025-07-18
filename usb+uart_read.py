import asyncio
import serial_asyncio
import numpy as np
import serial
import time

SAMPLE_COUNT = 5000
# the data from pico is a uint16_t (16-bit unsigned integer)
# each sample is 2 bytes, in total 10,000 bytes
# Frame size: wait until recevied 10,000 bytes, treat one complete frame of 5,000 samples
FRAME_SIZE = SAMPLE_COUNT * 2

uart = None  # Global handle

class BinaryReader(asyncio.Protocol):
    def __init__(self):
        self.buffer = bytearray()

    def connection_made(self, transport):
        print("✅ Connected to USB serial.")
        self.transport = transport

    def data_received(self, data):
        self.buffer.extend(data)
        while len(self.buffer) >= FRAME_SIZE:
            # Extract the frame 
            frame = self.buffer[:FRAME_SIZE]
            self.buffer = self.buffer[FRAME_SIZE:]
            # Decode to uint16 array
            # '<u2' means "little-endian unsigned 2-byte integer" → exactly what Pico sent.
            arr = np.frombuffer(frame, dtype='<u2')  # Little-endian uint16
            print("🔄 Received array:", arr)
            time.sleep(0.1) # 10 ms
            uart.write(b'C')
            uart.flush()
            
            # # Send confirmation byte over UART
            # try:
            #     self.uart.write(b'C')
            #     self.uart.flush()  # Ensure byte is sent immediately
            #     print("✅ Sent confirmation 'C' to Pico via UART.")
            # except Exception as e:
            #     print(f"⚠️ Failed to send confirmation via UART: {e}")

    def connection_lost(self, exc):
        print("⚠️ USB serial connection closed.")

# === Main async setup ===
async def main():
    global uart
    usb_port = '/dev/ttyACM0'   # USB CDC port from Pico
    uart_port = '/dev/ttyAMA0'  # UART adapter (Pi-to-Pico confirmation)
    baudrate = 115200

    # Open UART for writing confirmation
    uart = serial.Serial(uart_port, baudrate, timeout=1)

    # Open USB for reading binary data
    loop = asyncio.get_running_loop()
    await serial_asyncio.create_serial_connection(
        loop,
        BinaryReader,
        usb_port,
        baudrate=baudrate
    )

    await asyncio.Event().wait()  # Keep running forever

if __name__ == '__main__':
    asyncio.run(main())
