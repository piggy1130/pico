#include "pico/stdlib.h"

#define PIN_trigger 28 // channel 1
#define PIN_signal 27  // channel 2

int main() {

    gpio_init(PIN_trigger);
    gpio_set_dir(PIN_trigger, GPIO_OUT);
    gpio_init(PIN_signal);
    gpio_set_dir(PIN_signal, GPIO_OUT);

    while (true) {
        gpio_put(PIN_trigger, 1);
        sleep_us(5); // 100 microsecond = 0.1 ms
        gpio_put(PIN_signal, 1);
        sleep_us(10); // 100 microsecond = 0.1 ms
        gpio_put(PIN_trigger, 0);
        gpio_put(PIN_signal, 0);
        sleep_us(20); // 100,000 microsecond = 100 ms 
    }
}

