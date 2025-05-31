def count_numbers(filename):
    greater = 0
    smaller = 0

    with open(filename, 'r') as file:
        for line in file:
            try:
                num = int(line.strip())
                if num > 2000:
                    greater += 1
                elif num < 2000:
                    smaller += 1
            except ValueError:
                continue  # skip any lines that are not valid integers

    print(f"Numbers > 2000: {greater}")
    print(f"Numbers < 2000: {smaller}")

# Example usage
count_numbers("ADC/build/data.txt")
