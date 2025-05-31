#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define OUTPUT_PIN 16
#define ADC_PIN 26
#define SAMPLE_COUNT 20000

#define PIN 27

static uint16_t data[SAMPLE_COUNT];


int main() {
    stdio_usb_init();  // Only USB
    stdio_set_translate_crlf(&stdio_usb, false);  // Disable CRLF translation
    sleep_ms(2000);

    gpio_init(PIN);
    gpio_set_dir(PIN, GPIO_OUT);

    // Generate dummy data: ramp [0, 4095]
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        data[i] = i;  // example data (wraps around at 4096)
    }

    while(true){
        gpio_put(PIN, 1);
        //sleep_ms(50);
        
        // Send binary data (little-endian)
        fwrite(data, sizeof(uint16_t), SAMPLE_COUNT, stdout); // sends raw binary
        fflush(stdout);// forces the buffered data to be sent right away.
        
        gpio_put(PIN, 0);
        sleep_ms(50);
    }

    return 0;
}








// // Capture ADC samples and send them as raw binary over USB
// void capture_and_print_adc() {
//     for (int i = 0; i < SAMPLE_COUNT; i++) {
//         gpio_put(OUTPUT_PIN, i % 2);
//         sleep_us(20);
//         adc_buffer[i] = adc_read();
//     }

//     uint16_t sync = 0xAAAA;
//     fwrite(&sync, sizeof(sync), 1, stdout);  // send 2 bytes = sync
//     // sends 10,000 bytes (2bytes * 5000 samples) in one go
//     fwrite(adc_buffer, sizeof(uint16_t), SAMPLE_COUNT, stdout); // sends raw binary
//     //fflush(stdout);
//     stdio_flush(); // forces the buffered data to be sent right away.


// }

// int main() {
//     stdio_usb_init();
//     while (!stdio_usb_connected()) {
//         sleep_ms(10); // Wait until USB is ready
//     }

//     gpio_init(OUTPUT_PIN);
//     gpio_set_dir(OUTPUT_PIN, GPIO_OUT);

//     adc_init();
//     adc_gpio_init(ADC_PIN);
//     adc_select_input(0);

//     sleep_ms(1000); // Give host time to start reading
    
//     //printf("ADC Test\n");
//     sleep_ms(1000);

//     while (1) {

//         capture_and_print_adc();
//     }

// }
