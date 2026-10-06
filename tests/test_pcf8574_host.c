// Minimal core test — compile with: gcc -I tests/mocks -I . tests/test_pcf8574_host.c pcf8574.c -o /tmp/test_pcf
#include <assert.h>
#include <stdio.h>
#include "hardware/i2c.h"
#include "../pcf8574.h"

uint8_t mock_device_byte = 0xFF;
bool mock_fail_next = false;
uint8_t mock_last_written = 0;
uint8_t mock_last_addr = 0;

int i2c_write_blocking(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop) {
    (void)i2c; (void)nostop;
    if (mock_fail_next) { mock_fail_next = false; return PICO_ERROR_GENERIC; }
    mock_last_addr = addr;
    if (len) { mock_device_byte = src[0]; mock_last_written = src[0]; }
    return (int)len;
}
int i2c_read_blocking(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop) {
    (void)i2c; (void)addr; (void)nostop;
    if (mock_fail_next) { mock_fail_next = false; return PICO_ERROR_GENERIC; }
    if (len) dst[0] = mock_device_byte;
    return (int)len;
}

int main(void) {
    i2c_inst_t i2c0 = { .id = 0 };
    pcf8574_t dev;
    // init rejects bad address without touching bus
    assert(pcf8574_init(&dev, &i2c0, 0x19, 0xFF) == false);
    assert(pcf8574_init(NULL, &i2c0, 0x20, 0xFF) == false);
    // init accepts 0x20 and 0x38 variants, writes initial
    assert(pcf8574_init(&dev, &i2c0, 0x20, 0xFF) == true);
    assert(dev.addr == 0x20 && dev.out_state == 0xFF);
    assert(mock_last_written == 0xFF && mock_last_addr == 0x20);
    assert(pcf8574_init(&dev, &i2c0, 0x3F, 0x00) == true);
    assert(dev.out_state == 0x00);
    // write_byte updates cache only on ACK
    assert(pcf8574_write_byte(&dev, 0xA5) == true);
    assert(dev.out_state == 0xA5 && mock_device_byte == 0xA5);
    mock_fail_next = true;
    assert(pcf8574_write_byte(&dev, 0x5A) == false);
    assert(dev.out_state == 0xA5); // unchanged on NACK
    // read_byte
    mock_device_byte = 0x3C;
    uint8_t v = 0; uint8_t sentinel = 0xAA;
    assert(pcf8574_read_byte(&dev, &v) == true && v == 0x3C);
    mock_fail_next = true; v = sentinel;
    assert(pcf8574_read_byte(&dev, &v) == false && v == sentinel);
    assert(pcf8574_read_byte(&dev, NULL) == false);
    assert(pcf8574_read_byte(NULL, &v) == false);
    // --- pin API ---
    assert(pcf8574_init(&dev, &i2c0, 0x20, 0xFF) == true);
    assert(pcf8574_write_pin(&dev, 0, false) == true);
    assert(dev.out_state == 0xFE && mock_device_byte == 0xFE);
    assert(pcf8574_write_pin(&dev, 0, true) == true);
    assert(dev.out_state == 0xFF);
    assert(pcf8574_write_pin(&dev, 8, true) == false); // pin > 7 rejected
    assert(pcf8574_write_pin(NULL, 0, true) == false);
    // RMW preserves other outputs: clear P3 while P0 stays low
    assert(pcf8574_write_byte(&dev, 0xFE) == true); // P0=0, rest=1
    assert(pcf8574_write_pin(&dev, 3, false) == true);
    assert(dev.out_state == 0xF6);
    // read_pin masks fresh bus value
    mock_device_byte = 0xF6;
    bool lvl = true;
    assert(pcf8574_read_pin(&dev, 0, &lvl) == true && lvl == false);
    assert(pcf8574_read_pin(&dev, 1, &lvl) == true && lvl == true);
    assert(pcf8574_read_pin(&dev, 8, &lvl) == false);
    assert(pcf8574_read_pin(&dev, 0, NULL) == false);
    // toggle flips cache bit without extra read
    assert(pcf8574_write_byte(&dev, 0xFF) == true);
    assert(pcf8574_toggle_pin(&dev, 2) == true);
    assert(dev.out_state == 0xFB && mock_device_byte == 0xFB);
    assert(pcf8574_toggle_pin(&dev, 2) == true);
    assert(dev.out_state == 0xFF);
    assert(pcf8574_toggle_pin(&dev, 9) == false);
    // inputs mask: 1s = inputs
    assert(pcf8574_set_inputs_mask(&dev, 0xF0) == true);
    assert((dev.out_state & 0xF0) == 0xF0);
    // is_connected probes bus
    assert(pcf8574_is_connected(&dev) == true);
    mock_fail_next = true;
    assert(pcf8574_is_connected(&dev) == false);
    printf("CORE PASS\n");
    return 0;
}
