#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/dma.h"

#define UART_ID uart0
#define BAUD_RATE 85200  // 1 Mbps
#define UART_TX_PIN 12  // TX
//#define UART_RX_PIN 13  // RX - not used here

#define SAMPLE_COUNT 5000
uint16_t data[SAMPLE_COUNT];
#define PIN 27


// ************************************* Setup UART + DMA ****************************************
int dma_chan;

void init_uart_dma_tx() {
    // UART setup
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    //gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);  // optional
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_ID, false, false);

    // DMA setup
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);  // 8-bit transfer
    channel_config_set_read_increment(&c, true);            // increment read pointer
    channel_config_set_write_increment(&c, false);          // write to fixed UART FIFO
    channel_config_set_dreq(&c, uart_get_dreq(UART_ID, true));  // UART TX DREQ

    dma_channel_configure(
        dma_chan,
        &c,
        &uart_get_hw(UART_ID)->dr,   // UART TX FIFO
        NULL,                        // source: set later
        0,                           // transfer count: set later
        false                        // don't start yet
    );
}

// ************************************* Send Data via DMA ****************************************
void send_data_dma_async() {
    // Fill sample data
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        data[i] = i;
    }

    // Cast 16-bit data to byte stream
    uint8_t *byte_data = (uint8_t *)data;
    int total_bytes = SAMPLE_COUNT * 2;

    // Later start DMA:
    dma_channel_set_read_addr(dma_chan, byte_data, false); // read data from buffer
    dma_channel_set_trans_count(dma_chan, total_bytes, true);  // start DMA now

}

// ************************************* main() Function ****************************************
int main() {
    stdio_init_all();
    sleep_ms(2000);  // Give time to connect

    init_uart_dma_tx();

    gpio_init(PIN);
    gpio_set_dir(PIN, GPIO_OUT);

    while (true) {
        gpio_put(PIN, 1);
        send_data_dma_async();
        gpio_put(PIN, 0);
        // Wait for DMA to finish before next transfer
        dma_channel_wait_for_finish_blocking(dma_chan);
        sleep_ms(13);  // send once per second (or whatever rate you like)
    }
}
