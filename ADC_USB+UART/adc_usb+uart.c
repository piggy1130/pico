#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#define OUTPUT_PIN 16
#define ADC_PIN 26
#define SAMPLE_COUNT 5000

#define PIN 27
#define PIN_trigger 28

// setup UART for channel 2 (read data from pi)
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 12  // TX
#define UART_RX_PIN 13  // RX - not used here


static uint16_t data[SAMPLE_COUNT];

// Add this function:
int uart_getc_timeout(uart_inst_t *uart, uint32_t timeout_ms) {
    uint32_t start = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        if (uart_is_readable(uart)) {
            return uart_getc(uart);
        }
        sleep_us(100);  // avoid busy-wait
    }
    return -1;
}

int main() {
    stdio_usb_init();  // Only USB
    stdio_set_translate_crlf(&stdio_usb, false);  // Disable CRLF translation
    stdio_set_driver_enabled(&stdio_usb, true);  // optional in most cases
    sleep_ms(2000);

    gpio_init(PIN);
    gpio_init(PIN_trigger);

    gpio_set_dir(PIN, GPIO_OUT);
    gpio_set_dir(PIN_trigger, GPIO_OUT);

    // initialize for UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);   

    // Generate dummy data: ramp [0, 4095]
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        data[i] = i;  // example data (wraps around at 4096)
    }

    while(true){

        gpio_put(PIN_trigger, 1);
        sleep_ms(10);
        gpio_put(PIN, 1);
        //sleep_ms(50);
        
        // Send binary data (little-endian)
        fwrite(data, sizeof(uint16_t), SAMPLE_COUNT, stdout); // sends raw binary
        fflush(stdout);// forces the buffered data to be sent right away.
        // sleep_ms(2000);
        
        // === Wait for confirmation byte from UART ===
        int c = uart_getc_timeout(UART_ID, 5);  // timeout in ms


        // ***** NOTE!!!!!!*******
        // printf() and USB CDC is single-threaded!!!!!!!!!!
        // use print will mess up the data sending task
        // if (c >= 0) {
        //     printf("✅ Got confirmation on UART: %c\n", c);
        // } else {
        //     printf("❌ No UART confirmation.\n");
        // }    
        //sleep_ms(1000);

        gpio_put(PIN_trigger, 0);
        gpio_put(PIN, 0);
        sleep_ms(50);  // Wait before next cycle
    }

    return 0;
}
