#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define OUTPUT_PIN 16
#define ADC_PIN 26
#define SAMPLE_COUNT 5000

#define PIN 27
#define PIN_trigger 28

static uint16_t data[SAMPLE_COUNT];


int main() {
    stdio_usb_init();  // Only USB
    stdio_set_translate_crlf(&stdio_usb, false);  // Disable CRLF translation
    stdio_set_driver_enabled(&stdio_usb, true);  // optional in most cases
    sleep_ms(2000);

    gpio_init(PIN);
    gpio_init(PIN_trigger);

    gpio_set_dir(PIN, GPIO_OUT);
    gpio_set_dir(PIN_trigger, GPIO_OUT);

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
        
        // // === Wait for confirmation byte ===
        // int c;
        // printf("start to wait...\n");
        // do {
        //     c = getchar_timeout_us(5 * 1000 * 1000); // wait up to 5 sec
        //     if (c == PICO_ERROR_TIMEOUT) {
        //         printf("❌ No confirmation received.\n");
        //     } else {
        //         printf("✅ Got byte: %c (%d)\n", c, c);
        //     }
        // } while (c != 'C');  // Expecting 'C' as confirmation

        // printf("✅ Confirmation received.\n");
        // sleep_ms(2000);



        gpio_put(PIN_trigger, 0);
        gpio_put(PIN, 0);
        sleep_ms(20);  // Wait before next cycle
    }

    return 0;
}
