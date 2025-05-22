#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define OUTPUT_PIN 16
#define ADC_PIN 26
#define SAMPLE_COUNT 10000

uint16_t adc_buffer[SAMPLE_COUNT];

int main() {
    stdio_usb_init();
    while (!stdio_usb_connected()) {
        sleep_ms(100); // Wait until USB is ready
    }

    gpio_init(OUTPUT_PIN);
    gpio_set_dir(OUTPUT_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    sleep_ms(1000); // Give host time to start reading
    
    printf("ADC Test\n");
    sleep_ms(2000);

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        gpio_put(OUTPUT_PIN, i % 2);
        //sleep_us(100); // 10kHz
        sleep_us(10); // 100kHz
        adc_buffer[i] = adc_read();
    }

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        printf("%d\n", adc_buffer[i]);
        fflush(stdout); // Force the output buffer to flush
    }

    while (1) {
        printf("idle...\n");
        sleep_ms(1000);
    }
}
