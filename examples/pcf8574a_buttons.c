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

// On-board LED sign-of-life (independent of USB): slow blink = running,
// fast blink = PCF8574A init/probe failed. Most RP2040 boards use GP25.
#ifndef DIAG_LED_PIN
#define DIAG_LED_PIN 25
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

// Scan 0x08-0x77 and print every responding address. If 0x38 is missing
// but e.g. 0x20 answers, the chip is the non-A variant (or A-pins strapped
// differently); if nothing answers, SDA/SCL/VCC/GND wiring is suspect.
static void i2c_scan(i2c_inst_t *i2c) {
    printf("i2c scan:");
    bool any = false;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        uint8_t b = 0;
        if (i2c_read_blocking(i2c, addr, &b, 1, false) == 1) {
            printf(" 0x%02X", addr);
            any = true;
        }
    }
    if (!any) printf(" (none)");
    printf("\n");
}

int main(void) {
    stdio_init_all();
    gpio_init(DIAG_LED_PIN);
    gpio_set_dir(DIAG_LED_PIN, GPIO_OUT);
    gpio_put(DIAG_LED_PIN, 1); // on from here = we reached main()
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4); gpio_pull_up(5);

    // Retry init forever with status output: a monitor opened at any time
    // sees what is happening instead of one easily-missed boot line.
    pcf8574_t dev;
    for (;;) {
        if (pcf8574_init(&dev, i2c0, PCF8574A_ADDR_A0_A1_A2_GND, 0xFF) &&
            pcf8574_set_inputs_mask(&dev, 0xFF)) { // all 8 pins as inputs
            break;
        }
        printf("pcf8574A not found at 0x38, retrying...\n");
        i2c_scan(i2c0);
        sleep_ms(2000);
    }
    printf("pcf8574A ready at 0x38\n");

    uint8_t last = 0xFF;
    print_pressed(last);
    uint32_t next_blink = 0;
    uint32_t next_heartbeat = 0;
    while (true) {
        uint8_t v = 0;
        if (!pcf8574_read_byte(&dev, &v)) {
            printf("read failed, re-probing...\n");
            i2c_scan(i2c0);
            sleep_ms(2000);
        } else if (v != last) {
            sleep_ms(20); // debounce: let contacts settle, then re-read
            uint8_t confirm = 0;
            if (pcf8574_read_byte(&dev, &confirm) && confirm == v) {
                last = v;
                print_pressed(v);
            }
        }
        uint32_t now = (uint32_t)to_ms_since_boot(get_absolute_time());
        if (now >= next_blink) {
            gpio_xor_mask(1u << DIAG_LED_PIN); // heartbeat: proves main loop runs
            next_blink = now + 500;
        }
        if (now >= next_heartbeat) {
            printf("alive: 0x%02X\n", last); // liveness even with no presses
            next_heartbeat = now + 2000;
        }
        sleep_ms(10);
    }
}
