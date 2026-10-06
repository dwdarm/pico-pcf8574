// pcf8574.h — PCF8574/PCF8574A I2C GPIO expander driver for RP2040 (pico-sdk)
#ifndef PCF8574_H
#define PCF8574_H
#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

#ifndef PCF8574_I2C_TIMEOUT_US
#define PCF8574_I2C_TIMEOUT_US 10000u
#endif

#define PCF8574_ADDR_MIN 0x20u
#define PCF8574_ADDR_MAX 0x27u
#define PCF8574A_ADDR_MIN 0x38u
#define PCF8574A_ADDR_MAX 0x3Fu

typedef struct {
    i2c_inst_t *i2c;
    uint8_t addr;
    uint8_t out_state;
} pcf8574_t;

static inline bool pcf8574_addr_valid(uint8_t addr) {
    return ((addr >= PCF8574_ADDR_MIN && addr <= PCF8574_ADDR_MAX) ||
            (addr >= PCF8574A_ADDR_MIN && addr <= PCF8574A_ADDR_MAX));
}

bool pcf8574_init(pcf8574_t *dev, i2c_inst_t *i2c, uint8_t addr, uint8_t initial);
bool pcf8574_write_byte(pcf8574_t *dev, uint8_t val);
bool pcf8574_read_byte(pcf8574_t *dev, uint8_t *val);
bool pcf8574_write_pin(pcf8574_t *dev, uint8_t pin, bool level);
bool pcf8574_read_pin(pcf8574_t *dev, uint8_t pin, bool *level);
bool pcf8574_toggle_pin(pcf8574_t *dev, uint8_t pin);
bool pcf8574_set_inputs_mask(pcf8574_t *dev, uint8_t mask);
bool pcf8574_is_connected(pcf8574_t *dev);
#endif
