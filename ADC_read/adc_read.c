#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <stdio.h>
#include <string.h>

#define NUM_SAMPLES 5000
#define ADC_PIN 26  // GP26 = ADC0

// Double buffers
uint16_t adc_buffer_A[NUM_SAMPLES];
uint16_t adc_buffer_B[NUM_SAMPLES];
volatile bool buffer_A_ready = false;
volatile bool buffer_B_ready = false;
bool using_buffer_A = true;

int dma_chan;

void dma_handler() {
    dma_hw->ints0 = 1u << dma_chan;  // Clear interrupt

    // Swap buffers first
    using_buffer_A = !using_buffer_A;

    // Now mark the just-completed buffer as ready
    if (!using_buffer_A) {
        buffer_B_ready = true;
    } else {
        buffer_A_ready = true;
    }

    // Set up DMA to write to the newly selected buffer
    uint16_t* next_buf = using_buffer_A ? adc_buffer_A : adc_buffer_B;
    dma_channel_set_read_addr(dma_chan, &adc_hw->fifo, false);
    dma_channel_set_write_addr(dma_chan, next_buf, false);
    dma_channel_set_trans_count(dma_chan, NUM_SAMPLES, true);  // Restart DMA
}


// Set up ADC and DMA
void setup_adc_dma() {
    // ADC init
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    // ADC FIFO config
    adc_fifo_setup(
        true,   // Enable FIFO
        true,   // Enable DMA request (DREQ)
        1,      // DREQ threshold
        false,  // No IRQ
        false   // 12-bit result, no shift
    );

    // DMA config
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(
        dma_chan,
        &cfg,
        adc_buffer_A,        // Destination buffer, start with buffer A
        &adc_hw->fifo,     // Source: ADC FIFO
        NUM_SAMPLES,
        false              // Do not start yet
    );

    // IRQ on complete
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

int main() {
    stdio_usb_init();
    sleep_ms(2000); // Give time for USB to connect

    setup_adc_dma();
    adc_run(true);             // Start ADC once, and it keeps running
    dma_channel_start(dma_chan);     // Start first DMA

    char cmd[16];

    while (true) {
        // Wait for "GET" command from Pi
        if (fgets(cmd, sizeof(cmd), stdin)) {
            if (strcmp(cmd, "GET\n") == 0 || strcmp(cmd, "get\n") == 0) {

                // Wait until one buffer is ready
                while (!buffer_A_ready && !buffer_B_ready) {
                    tight_loop_contents(); // small wait
                }

                if (buffer_A_ready) {
                    fwrite(adc_buffer_A, sizeof(uint16_t), NUM_SAMPLES, stdout);
                    buffer_A_ready = false;
                } else if (buffer_B_ready) {
                    fwrite(adc_buffer_B, sizeof(uint16_t), NUM_SAMPLES, stdout);
                    buffer_B_ready = false;
                }

                fflush(stdout);  // ensure full transmission
            }
        }
    }

    return 0;
}
