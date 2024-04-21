add_executable(miscale2
        miscale2.cpp
        tls_common.c)

# pull in common dependencies
target_link_libraries(miscale2
        pico_stdlib
        pico_btstack_ble
        pico_btstack_cyw43
        pico_lwip_mbedtls
        pico_mbedtls
        pico_cyw43_arch_lwip_poll
        )

target_compile_definitions(miscale2 PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )

target_include_directories(miscale2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/lib/btstack
        ${PICO_SDK_PATH}/lib/lwip
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(miscale2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(miscale2)


# enable usb output, disable uart output
pico_enable_stdio_usb(miscale2 1)
pico_enable_stdio_uart(miscale2 0)