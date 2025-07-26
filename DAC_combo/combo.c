#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#define SPI_PORT spi1
#define SCK_PIN  10
#define MOSI_PIN 11
#define CS_PIN    9

#define PULSE_PIN 28
#define ADC_PIN 26  // GP26 = ADC0

#define MAX_DAC_VAL     4095
#define SAMPLE_DELAY_US 5000
#define STEP_SIZE       1
#define BUF_SIZE    10

#define DAC_STEP        400

uint16_t adc_data[BUF_SIZE];
static uint16_t last_val;
int dma_chan;

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


void setup_dma() {
    // DMA config
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);  // read from &last_val
    channel_config_set_write_increment(&cfg, true);  // write into buffer[i]
    //channel_config_set_dreq(&cfg, DREQ_ADC); //DMA Request from ADC, Only transfer data when the ADC is ready and requests it.

    // Pre‑configure for 1 sample; dest/src are filled at run time
    dma_channel_configure(
        dma_chan,
        &cfg,
        NULL,               // dst addr set at runtime
        &last_val,          // src addr fixed
        1,                  // One sample at a time
        false               // Do not start yet
    );

    // // IRQ on complete
    // dma_channel_set_irq0_enabled(dma_chan, true);
    // irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    // irq_set_enabled(DMA_IRQ_0, true);

    // //dma_done = false;
    // dma_channel_start(dma_chan);  // start transfer
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to be ready

    // SPI initialization
    spi_init(SPI_PORT, 10 * 1000 * 1000); // 10 MHz
    spi_set_format(SPI_PORT, 8, 0, 0, SPI_MSB_FIRST);

    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);

    // DAC CS pin setup
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1); // CS idle high

    // // Scope‐trigger pin
    // gpio_init(PULSE_PIN);
    // gpio_set_dir(PULSE_PIN, GPIO_OUT);
    // gpio_put(PULSE_PIN, 1);
    // sleep_ms(10);

    setup_adc();
    setup_dma();

    uint16_t dac = 0;
    //while (1) {
        for (int i = 0; i < BUF_SIZE; i++) {
            // 1) write to the DAC
            mcp4921_write(dac);
            dac = (dac + DAC_STEP) & 0x0FFF;
            // give the DAC output time to settle
            sleep_us(SAMPLE_DELAY_US);
            
            // 2) CPU read the ADC
            last_val = adc_read(); // CPU grabs one sample


            // 3) DMA copy last_val → adc_data[i]
            dma_channel_set_write_addr(dma_chan, &adc_data[i], false);
            dma_channel_start(dma_chan);
            // wait for that single-sample DMA to complete
            dma_channel_wait_for_finish_blocking(dma_chan);
        }

        // when buffer is full → send it in one burst
        putchar(0xAA);
        fwrite(adc_data, sizeof(uint16_t), BUF_SIZE, stdout);
        fflush(stdout);

        // small pause before next round
        sleep_ms(500);
    //}


    return 0;
}


