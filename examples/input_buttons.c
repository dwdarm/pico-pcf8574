// input_buttons — upper nibble as inputs with INT poll + optional IRQ flag
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pcf8574.h"

#define INT_PIN 22
static volatile bool int_fired = false;
static void int_cb(uint gpio, uint32_t events) {
    (void)gpio; (void)events;
    int_fired = true; // never do I2C in IRQ
}

int main(void) {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4); gpio_pull_up(5);
    gpio_init(INT_PIN); gpio_set_dir(INT_PIN, GPIO_IN); gpio_pull_up(INT_PIN);
    gpio_set_irq_enabled_with_callback(INT_PIN, GPIO_IRQ_EDGE_FALL, true, &int_cb);
    pcf8574_t dev;
    if (!pcf8574_init(&dev, i2c0, 0x20, 0xFF)) { printf("pcf8574 not found\n"); return 1; }
    pcf8574_set_inputs_mask(&dev, 0xF0); // P4-P7 inputs, P0-P3 outputs
    uint8_t last = 0xFF;
    while (true) {
        if (!gpio_get(INT_PIN)) int_fired = true; // poll fallback if IRQ missed
        if (int_fired) {
            int_fired = false;
            uint8_t v = 0;
            if (pcf8574_read_byte(&dev, &v) && v != last) {
                last = v;
                printf("inputs: 0x%02X\n", v);
            }
        }
        sleep_ms(10);
    }
}
