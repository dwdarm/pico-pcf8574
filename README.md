# pico-pcf8574

C driver for PCF8574 / PCF8574A I2C 8-bit GPIO expander on RP2040 (pico-sdk).

## Wiring

| PCF8574 | Pico |
|---|---|
| SDA | GP4 (I2C0 SDA) |
| SCL | GP5 (I2C0 SCL) |
| A0-A2 | GND/VCC per address |
| INT | GP22 (pull-up) + GND common |

Addresses: PCF8574 `0x20-0x27`, PCF8574A `0x38-0x3F`. 100 kHz default, 400 kHz supported.

## Usage

```c
i2c_init(i2c0, 100*1000);
gpio_set_function(4, GPIO_FUNC_I2C);
gpio_set_function(5, GPIO_FUNC_I2C);
gpio_pull_up(4); gpio_pull_up(5);
pcf8574_t dev;
pcf8574_init(&dev, i2c0, 0x20, 0xFF);
pcf8574_set_inputs_mask(&dev, 0xF0);
pcf8574_write_pin(&dev, 0, false);
uint8_t v; pcf8574_read_byte(&dev, &v);
```

Inputs must be written `1` (quasi-bidirectional). Driver keeps `1`s in cache for input pins.

## Build

```
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -B build -DPICO_SDK_PATH=$PICO_SDK_PATH
cmake --build build
```

## Host tests (no hardware)

```
gcc -Wall -Wextra -I tests/mocks -I . tests/test_pcf8574_host.c pcf8574.c -o /tmp/test_pcf && /tmp/test_pcf
```
