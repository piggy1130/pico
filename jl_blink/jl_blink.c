#include "pico/stdlib.h"

int main() {
    const uint LED_1 = 28; // 25 is for built-in LED
    const uint LED_2 = 27;

    gpio_init(LED_1);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);

    while (true) {
        gpio_put(LED_1, 1);
        sleep_us(100); // 100 microsecond = 0.1 ms
        gpio_put(LED_2, 1);
        sleep_us(100); // 100 microsecond = 0.1 ms
        //sleep_ms(500);
        gpio_put(LED_1, 0);
        gpio_put(LED_2, 0);
        sleep_ms(100); // 100,000 microsecond = 100 ms 
        //sleep_ms(500);
    }
}

