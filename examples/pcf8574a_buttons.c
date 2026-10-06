// pcf8574a_buttons — PCF8574A with A0=A1=A2=GND (addr 0x38), all pins as buttons.
//
// Buttons wire between each P0-P7 pin and GND. The driver keeps the output
// latch at 0xFF so the quasi-bidirectional pins weakly pull high; a pressed
// button reads 0. The port is polled and changes are printed over USB serial.
// The INT pin is left unconnected.
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pcf8574.h"

#define PCF8574A_ADDR_A0_A1_A2_GND 0x38u

static void print_pressed(uint8_t v) {
    // v is the raw port byte: 0 bit = pressed (pulled to GND)
    printf("buttons: 0x%02X pressed:", v);
    bool any = false;
    for (uint8_t pin = 0; pin < 8; pin++) {
        if ((v & (uint8_t)(1u << pin)) == 0) {
            printf(" P%u", pin);
            any = true;
        }
    }
    if (!any) printf(" (none)");
    printf("\n");
}

int main(void) {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4); gpio_pull_up(5);

    pcf8574_t dev;
    if (!pcf8574_init(&dev, i2c0, PCF8574A_ADDR_A0_A1_A2_GND, 0xFF)) {
        printf("pcf8574A not found at 0x38 (check A0-A2 -> GND)\n");
        return 1;
    }
    if (!pcf8574_set_inputs_mask(&dev, 0xFF)) { // all 8 pins as inputs
        printf("failed to configure inputs\n");
        return 1;
    }
    if (!pcf8574_is_connected(&dev)) {
        printf("pcf8574A probe failed\n");
        return 1;
    }

    uint8_t last = 0xFF;
    print_pressed(last);
    while (true) {
        uint8_t v = 0;
        if (pcf8574_read_byte(&dev, &v) && v != last) {
            sleep_ms(20); // debounce: let contacts settle, then re-read
            uint8_t confirm = 0;
            if (pcf8574_read_byte(&dev, &confirm) && confirm == v) {
                last = v;
                print_pressed(v);
            }
        }
        sleep_ms(10);
    }
}
