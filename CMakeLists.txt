cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)
# setup board type (values - pico|pico_w)
set(PICO_BOARD pico_w)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(picopi-ble C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

set(WIFI_SSID "your ssid")
set(WIFI_PASSWORD "pssd")

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
add_subdirectory(miscale2)

if (NOT TARGET pico_btstack_base)
    message("Skipping Pico W Bluetooth examples as support is not available")
endif()

if (NOT PICO_EXTRAS_PATH)
    message("Skipping some Pico W BTstack examples that require pico-extras")
endif()

if ("${WIFI_SSID}" STREQUAL "")
    message("Skipping some Pico W examples as WIFI_SSID is not defined")
endif()

if (PICO_CYW43_SUPPORTED) # set by PICO_BOARD=pico_w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping Pico W examples as support is not available")
    else()

        if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
            set(WIFI_SSID $ENV{WIFI_SSID})
            message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
        endif()

        if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
            set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
            message("Using WIFI_PASSWORD from environment")
        endif()

        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

        message("Using WiFi SSID '${WIFI_SSID}'")

        if (NOT TARGET pico_btstack_base)
            message("Skipping Pico W Bluetooth examples as support is not available")
        endif()
    endif()
endif()

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)
set(BTSTACK_EXAMPLE_PATH ${BTSTACK_ROOT}/example)
set(BTSTACK_3RD_PARTY_PATH ${BTSTACK_ROOT}/3rd-party)
set(BT_EXAMPLE_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}")

# Adds common stuff for all the examples
add_library(picow_bt_example_common INTERFACE)
target_sources(picow_bt_example_common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/picow_bt_example_common.c
        )
target_link_libraries(picow_bt_example_common
        INTERFACE pico_stdlib
        INTERFACE  pico_btstack_cyw43
        # Pico W wireless libraries
        #
        #   Pulls in libraries for hardware driver (`pico_cyw43_driver`) and TCP/IP
        #   stack (`pico_lwip`).
        #
        INTERFACE pico_cyw43_arch_lwip_threadsafe_background
        # TLS library
        #
        #   Required for HTTPS (= TCP + TLS + HTTP).
        #
        #   A port of the Mbed-TLS library is included in lwIP, and therefore in
        #   the Pico SDK. __N.b. this is not a full MbedTLS distribution__, but
        #   rather simply provides an lwIP compatible interface to Mbed-TLS.
        #
        INTERFACE pico_lwip_mbedtls
        #
        #   A full Mbed-TLS distributiom is however included in the Pico SDK
        #   (currently as a submodule). This is currently (Pico SDK 1.5.0) not well
        #   documented however.
        #
        INTERFACE pico_mbedtls                                 # mbedtls distributed with pico-sdk
        )
target_include_directories(picow_bt_example_common INTERFACE
        ${BT_EXAMPLE_COMMON_DIR}/config # Use our own config
        ${BTSTACK_EXAMPLE_PATH}/
        )
target_compile_definitions(picow_bt_example_common
        INTERFACE PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
        )