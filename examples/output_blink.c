// output_blink — cycles P0, demos write_byte / write_pin / toggle
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pcf8574.h"

int main(void) {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4); gpio_pull_up(5);
    pcf8574_t dev;
    if (!pcf8574_init(&dev, i2c0, 0x20, 0xFF)) { printf("pcf8574 not found at 0x20\n"); return 1; }
    while (true) {
        pcf8574_write_pin(&dev, 0, false);
        sleep_ms(500);
        pcf8574_write_pin(&dev, 0, true);
        sleep_ms(500);
        pcf8574_toggle_pin(&dev, 1);
        sleep_ms(200);
    }
}
