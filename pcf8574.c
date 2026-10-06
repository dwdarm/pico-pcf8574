#include "pcf8574.h"
#include <stddef.h>

bool pcf8574_init(pcf8574_t *dev, i2c_inst_t *i2c, uint8_t addr, uint8_t initial) {
    if (!dev || !i2c) return false;
    if (!pcf8574_addr_valid(addr)) return false;
    dev->i2c = i2c;
    dev->addr = addr;
    dev->out_state = initial;
    int rc = i2c_write_blocking(i2c, addr, &initial, 1, false);
    if (rc != 1) return false;
    return true;
}

bool pcf8574_write_byte(pcf8574_t *dev, uint8_t val) {
    if (!dev || !dev->i2c) return false;
    int rc = i2c_write_blocking(dev->i2c, dev->addr, &val, 1, false);
    if (rc != 1) return false;
    dev->out_state = val;
    return true;
}

bool pcf8574_read_byte(pcf8574_t *dev, uint8_t *val) {
    if (!dev || !dev->i2c || !val) return false;
    uint8_t b = 0;
    int rc = i2c_read_blocking(dev->i2c, dev->addr, &b, 1, false);
    if (rc != 1) return false;
    *val = b;
    return true;
}

bool pcf8574_write_pin(pcf8574_t *dev, uint8_t pin, bool level) {
    if (!dev || !dev->i2c) return false;
    if (pin > 7) return false;
    uint8_t next = level ? (dev->out_state | (uint8_t)(1u << pin))
                         : (dev->out_state & (uint8_t)~(1u << pin));
    int rc = i2c_write_blocking(dev->i2c, dev->addr, &next, 1, false);
    if (rc != 1) return false;
    dev->out_state = next;
    return true;
}

bool pcf8574_read_pin(pcf8574_t *dev, uint8_t pin, bool *level) {
    if (!dev || !dev->i2c || !level) return false;
    if (pin > 7) return false;
    uint8_t b = 0;
    int rc = i2c_read_blocking(dev->i2c, dev->addr, &b, 1, false);
    if (rc != 1) return false;
    *level = (b & (uint8_t)(1u << pin)) != 0;
    return true;
}

bool pcf8574_toggle_pin(pcf8574_t *dev, uint8_t pin) {
    if (!dev || !dev->i2c) return false;
    if (pin > 7) return false;
    uint8_t next = dev->out_state ^ (uint8_t)(1u << pin);
    int rc = i2c_write_blocking(dev->i2c, dev->addr, &next, 1, false);
    if (rc != 1) return false;
    dev->out_state = next;
    return true;
}

bool pcf8574_set_inputs_mask(pcf8574_t *dev, uint8_t mask) {
    if (!dev || !dev->i2c) return false;
    uint8_t next = dev->out_state | mask;
    int rc = i2c_write_blocking(dev->i2c, dev->addr, &next, 1, false);
    if (rc != 1) return false;
    dev->out_state = next;
    return true;
}

bool pcf8574_is_connected(pcf8574_t *dev) {
    if (!dev || !dev->i2c) return false;
    uint8_t b = 0;
    int rc = i2c_read_blocking(dev->i2c, dev->addr, &b, 1, false);
    return rc == 1;
}
