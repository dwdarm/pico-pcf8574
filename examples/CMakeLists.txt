add_executable(output_blink output_blink.c)
target_link_libraries(output_blink PRIVATE pcf8574 pico_stdlib hardware_i2c)
pico_add_extra_outputs(output_blink)
pico_enable_stdio_usb(output_blink 1)

add_executable(input_buttons input_buttons.c)
target_link_libraries(input_buttons PRIVATE pcf8574 pico_stdlib hardware_i2c)
pico_add_extra_outputs(input_buttons)
pico_enable_stdio_usb(input_buttons 1)

add_executable(pcf8574a_buttons pcf8574a_buttons.c)
target_link_libraries(pcf8574a_buttons PRIVATE pcf8574 pico_stdlib hardware_i2c)
pico_add_extra_outputs(pcf8574a_buttons)
pico_enable_stdio_usb(pcf8574a_buttons 1)

# Fallback build on I2C1 / GP6+GP7: move SDA->GP6 (pad 17), SCL->GP7 (pad 16)
# and flash this if the default build sees nothing (tests for damaged GP4/5).
add_executable(pcf8574a_buttons_i2c1 pcf8574a_buttons.c)
target_compile_definitions(pcf8574a_buttons_i2c1 PRIVATE PCF_I2C_INST=i2c1 PCF_SDA_PIN=6 PCF_SCL_PIN=7)
target_link_libraries(pcf8574a_buttons_i2c1 PRIVATE pcf8574 pico_stdlib hardware_i2c)
pico_add_extra_outputs(pcf8574a_buttons_i2c1)
pico_enable_stdio_usb(pcf8574a_buttons_i2c1 1)
