// tests/mocks/hardware/i2c.h — host-only shadow of pico-sdk <hardware/i2c.h>
#ifndef MOCK_HARDWARE_I2C_H
#define MOCK_HARDWARE_I2C_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct i2c_inst_t { int id; } i2c_inst_t;

#define PICO_ERROR_GENERIC -1

// Controllable fake device state, defined in test_pcf8574_host.c
extern uint8_t mock_device_byte;
extern bool mock_fail_next;
extern uint8_t mock_last_written;
extern uint8_t mock_last_addr;

int i2c_write_blocking(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop);
int i2c_read_blocking(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop);
#endif
