#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // for usleep

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define I2C_PORT i2c0
#define MCP4725_ADDR 0x60 // defaulte I2C address
#define SDA_PIN 0 // SDA - data
#define SCL_PIN 1 // SCL - clock
#define PULSE_PIN 28 // channel 1 - scope

#define MAX_DAC_VAL 4095

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
    stdio_init_all();
    
     // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000); // 100kHz or 400kHz I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    // Initialize GPIO
    gpio_init(PULSE_PIN);
    gpio_set_dir(PULSE_PIN, GPIO_OUT);

    while (true) {
        gpio_put(PULSE_PIN, 1);
        mcp4725_write(MAX_DAC_VAL);
        sleep_us(200);

        gpio_put(PULSE_PIN, 0);
        mcp4725_write(0);
        sleep_us(300);
    }

    return 0;
}




