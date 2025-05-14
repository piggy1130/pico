#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // for usleep

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define I2C_PORT i2c0
#define MCP4725_ADDR 0x60 // defaulte I2C address
#define SDA_PIN 0 // SDA - data
#define SCL_PIN 1 // SCL - clock


// MCP4725 takes 12-bit value for output voltage
void mcp4725_write(uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = 0x40; // Command to update DAC (write DAC register)
    buffer[1] = (value >> 4) & 0xFF; // Upper 8 bits
    buffer[2] = (value & 0x0F) << 4; // Lower 4 bits
    // sends these 3 bytes to the DAC via function i2c_write_blocking
    i2c_write_blocking(I2C_PORT, MCP4725_ADDR, buffer, 3, false);
}

int main() {
    // initializes standard I/O
    stdio_init_all();
    
    i2c_init(I2C_PORT, 100 * 1000); // 100kHz I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    const uint16_t max_val = 4095;
    const uint16_t slow_step = 16;      // Small step = slow rise
    const uint16_t fast_step = 256;     // Big step = fast fall
    const int slow_delay_us = 1000;     // Longer delay for slow rise. (in microseconds → sets frequency of sawtooth)
    const int fast_delay_us = 500;      // Short delay for fast fall

    // This produces a repeating sawtooth wave, where voltage increases linearly from 0V to max Vout(3.3V), then resets.
    while (1) {
        // Slow rise
        for (uint16_t val = 0; val <= max_val; val += slow_step) {
            mcp4725_write(val);
            sleep_us(slow_delay_us);
        }

        // Fast drop
        for (uint16_t val = max_val; val >= fast_step; val -= fast_step) {
            mcp4725_write(val);
            sleep_us(fast_delay_us);
        }

        // Final step to reach 0 cleanly
        mcp4725_write(0);
        sleep_us(fast_delay_us);
    }

    return 0;
}


