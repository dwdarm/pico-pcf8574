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

// I2C wiring: override with -DPCF_SDA_PIN=.. -DPCF_SCL_PIN=.. -DPCF_I2C=i2c1
// to match your board. Defaults are I2C0 on GP4/GP5.
#ifndef PCF_I2C_INST
#define PCF_I2C_INST i2c0
#endif
#ifndef PCF_SDA_PIN
#define PCF_SDA_PIN 4
#endif
#ifndef PCF_SCL_PIN
#define PCF_SCL_PIN 5
#endif

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
    i2c_init(PCF_I2C_INST, 100 * 1000);
    gpio_set_function(PCF_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PCF_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PCF_SDA_PIN); gpio_pull_up(PCF_SCL_PIN);

    pcf8574_t dev;
    for (;;) {
        if (pcf8574_init(&dev, PCF_I2C_INST, PCF8574A_ADDR_A0_A1_A2_GND, 0xFF) &&
            pcf8574_set_inputs_mask(&dev, 0xFF)) { // all 8 pins as inputs
            break;
        }
        printf("pcf8574A not found at 0x38, check wiring, retrying...\n");
        sleep_ms(2000);
    }
    printf("pcf8574A ready at 0x38\n");

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
