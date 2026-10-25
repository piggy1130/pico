"""Generate a repeating sawtooth using sample counts, without timestamps."""

import argparse
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output", type=Path,
        default=Path(__file__).resolve().with_name("waveform.txt"),
        help="Output path (default: waveform.txt beside this script)",
    )
    args = parser.parse_args()

    num_points = 10000
    fall_fraction = 0.1
    max_value = 4095
    dac_full_scale_voltage = 5.0
    v_min = 0.7
    v_max = 4.3

    fall_points = round(num_points * fall_fraction)
    rise_points = num_points - fall_points
    if rise_points < 1 or fall_points < 1:
        raise ValueError("Both rising and falling sections need at least one point")
    if not 0 <= v_min < v_max <= dac_full_scale_voltage:
        raise ValueError("Voltage range must fit within the DAC full scale")

    dac_min = int(v_min / dac_full_scale_voltage * max_value)
    dac_max = int(v_max / dac_full_scale_voltage * max_value)
    dac_range = dac_max - dac_min

    # Exclude each section's endpoint: the fall starts at the peak, and
    # the next repetition starts at the minimum without duplicating it.
    values = [
        int(dac_min + dac_range * i / rise_points)
        for i in range(rise_points)
    ]
    values.extend(
        int(dac_max - dac_range * i / fall_points)
        for i in range(fall_points)
    )

    # Two columns match generate_header.py's format. The first is a sample
    # index, not time; that script reads only the second column.
    with args.output.open("w") as output:
        for index, value in enumerate(values):
            output.write(f"{index}, {value}\n")

    print(f"Saved {num_points} points to {args.output}")
    print(f"Rise: {rise_points} points; fall: {fall_points} points")


if __name__ == "__main__":
    main()
