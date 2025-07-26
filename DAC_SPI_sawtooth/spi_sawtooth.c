#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // for usleep

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "data/waveform_data.h"

#define SPI_PORT spi1
#define SCK_PIN  10
#define MOSI_PIN 11
#define CS_PIN    9
#define PULSE_PIN 28

#define MAX_DAC_VAL     4095
#define SAMPLE_DELAY_US 20
#define STEP_SIZE       1

// Function to write to MCP4921 (12-bit SPI DAC)
void mcp4921_write(uint16_t value) {
    uint8_t buf[2];
    value &= 0x0FFF; // Ensure 12-bit
    buf[0] = 0x30 | ((value >> 8) & 0x0F); // DAC A, buffered, gain=1x, active
    buf[1] = value & 0xFF;

    gpio_put(CS_PIN, 0); // CS low to start communication
    spi_write_blocking(SPI_PORT, buf, 2);
    gpio_put(CS_PIN, 1); // CS high to end communication
}

int main() {
    stdio_init_all();

    // SPI initialization
    spi_init(SPI_PORT, 10 * 1000 * 1000); // 1 MHz
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    // CS pin setup
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); // CS idle high

    // Scope‐trigger pin
    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_OUT);
    //gpio_put(PULSE_PIN, 0);

    gpio_put(PULSE_PIN, 1);
    sleep_ms(10);
    while (true) {
        gpio_put(PULSE_PIN, 0);
        for (int i = 0; i < MAX_DAC_VAL; i += STEP_SIZE) {
            mcp4921_write(i);  // DAC output
            sleep_us(20);
            if (i == MAX_DAC_VAL){
                mcp4921_write(0);
            }
        }
        gpio_put(PULSE_PIN, 1);
        sleep_us(100);

    }

    return 0;
}
