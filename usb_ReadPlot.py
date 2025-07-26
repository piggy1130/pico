import serial
import numpy as np
import matplotlib.pyplot as plt
import time

# ── Config ────────────────────────────────
PORT = "/dev/ttyACM0"        # Update if different
BAUD = 115200
NUM_SAMPLES = 5000
BYTES_PER_SAMPLE = 2
TOTAL_BYTES = NUM_SAMPLES * BYTES_PER_SAMPLE

# ── Setup Serial ──────────────────────────
ser = serial.Serial(PORT, BAUD, timeout=3)

# ── Setup Live Plot ───────────────────────
plt.ion()  # Interactive mode
fig, ax = plt.subplots()
line, = ax.plot([], [], lw=1)
ax.set_xlim(0, NUM_SAMPLES)
ax.set_title("Live ADC Plot")
ax.set_xlabel("Sample Index")
ax.set_ylabel("ADC Value")

# ── Main Loop ─────────────────────────────
try:
    while True:
        # Send command to Pico
        ser.reset_input_buffer()
        ser.write(b"GET\n")

        # Read 10,000 bytes (5,000 samples)
        raw = bytearray()
        while len(raw) < TOTAL_BYTES:
            chunk = ser.read(TOTAL_BYTES - len(raw))
            if not chunk:
                print("Timeout or incomplete data")
                break
            raw.extend(chunk)

        if len(raw) != TOTAL_BYTES:
            continue  # Skip this frame if incomplete

        # Convert to uint16 array
        data = np.frombuffer(raw, dtype=np.uint16)

        # Update plot
        line.set_ydata(data)
        line.set_xdata(np.arange(NUM_SAMPLES))
        ax.relim()          # Recalculate limits
        ax.autoscale_view() # Apply limits
        fig.canvas.draw()
        fig.canvas.flush_events()
        
        time.sleep(0.5)  # Add this to throttle updates (every 0.5 seconds)

except KeyboardInterrupt:
    plt.savefig("plot.png")
    print("Stopped by user")
    plt.ioff()
    plt.show()
