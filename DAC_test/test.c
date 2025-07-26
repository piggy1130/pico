#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "pico/multicore.h"

#define SPI_PORT            spi1
#define SCK_PIN             10
#define MOSI_PIN            11
#define CS_PIN              9

#define TRIGGER_PULSE_PIN   8
#define ADC_PIN             26  // GP26 = ADC0

#define MAX_DAC_VAL         4095
#define DAC_STEP            1
#define SAMPLE_DELAY_US     20

#define BUF_SIZE            4096

// ─── BUFFERS & STATE ─────────
static uint16_t adc_buf[2][BUF_SIZE];
static volatile uint8_t buf_index = 0;

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

void setup_adc() {
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);
}

void core1_entry(){
    while (true){
        // Wait for a buffer ID from core 0
        uint32_t id = multicore_fifo_pop_blocking();
        // Send the entire buffer over USB‑CDC
        fwrite(adc_buf[id], sizeof(adc_buf[id][0]), BUF_SIZE, stdout);
        fflush(stdout);
    }
}

int main() {
    stdio_init_all(); // initialize USB serial
    sleep_ms(2000); // Wait for USB serial to be ready

    // SPI initialization
    spi_init(SPI_PORT, 10 * 1000 * 1000); // 10 MHz
    spi_set_format(SPI_PORT, 8, 0, 0, SPI_MSB_FIRST);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    // DAC CS pin setup
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); // CS idle high

    // Scope‐trigger pin
    gpio_init(TRIGGER_PULSE_PIN);
    gpio_set_dir(TRIGGER_PULSE_PIN, GPIO_OUT);
    gpio_put(TRIGGER_PULSE_PIN, 1);
    sleep_ms(10);

    setup_adc();

    // launch 2nd core
    multicore_launch_core1(core1_entry);

    // the 1st core
    while (true) {
        gpio_put(TRIGGER_PULSE_PIN, 0);
        
        for (int i = 0; i <= MAX_DAC_VAL; i += DAC_STEP) {
            // 1) write to the DAC
            mcp4921_write(i);  
            sleep_us(10);
            // 2) CPU read the ADC
            adc_buf[buf_index][i] = adc_read();
            
            if (i == MAX_DAC_VAL){
                mcp4921_write(0);
            }

        }

        // Hand off the full buffer to core 1, then swap
        multicore_fifo_push_blocking(buf_index);
        buf_index ^= 1;

        gpio_put(TRIGGER_PULSE_PIN, 1);
        // small pause before next round
        sleep_ms(20);
    }


    return 0;
}


