import asyncio
import serial_asyncio
import numpy as np
import binascii

SAMPLE_COUNT = 5000
# the data from pico is a uint16_t (16-bit unsigned integer)
# each sample is 2 bytes, in total 10,000 bytes
# Frame size: wait until recevied 10,000 bytes, treat one complete frame of 5,000 samples
FRAME_SIZE = SAMPLE_COUNT * 2

class BinaryReader(asyncio.Protocol):
    def __init__(self):
        self.buffer = bytearray()

    def connection_made(self, transport):
        print("✅ Connected to USB serial.")
        self.transport = transport

    def data_received(self, data):
        #print("****")
        self.buffer.extend(data)
        #print(f"📥 Received {len(data)} bytes, total buffer = {len(self.buffer)}")
        while len(self.buffer) >= FRAME_SIZE:
            #print("start to receive arr....1")
            # Extract the frame 
            frame = self.buffer[:FRAME_SIZE]
            self.buffer = self.buffer[FRAME_SIZE:]

            #print("start to receive arr....2")
            # Decode to uint16 array
            # '<u2' means "little-endian unsigned 2-byte integer" → exactly what Pico sent.
            arr = np.frombuffer(frame, dtype='<u2')  # Little-endian uint16
            #print("🔄 Received array (first 100):", arr[:100], "...")
            print("🔄 Received array:", arr)
            
            # # === Send confirmation ===
            # self.transport.write(b'C')  # Send 'C' back to Pico
            # self.transport.serial.flush()  # Force the bytes to go out immediately

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
