import asyncio
import serial_asyncio
import numpy as np
import binascii

SAMPLE_COUNT = 10000
FRAME_SIZE = SAMPLE_COUNT * 2

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
            arr = np.frombuffer(frame, dtype='<u2')  # Little-endian uint16
            #print("🔄 Received array (first 100):", arr[:100], "...")
            print("🔄 Received array:", arr)

    def connection_lost(self, exc):
        print("⚠️ USB serial connection closed.")

async def main():
    port = '/dev/ttyACM0'  # Change if needed
    baudrate = 115200      # CDC USB ignores this, but it's required

    loop = asyncio.get_running_loop()
    await serial_asyncio.create_serial_connection(
        loop,
        lambda: BinaryReader(),
        port,
        baudrate=baudrate
    )

    await asyncio.Event().wait()  # Keep running forever

if __name__ == '__main__':
    asyncio.run(main())
