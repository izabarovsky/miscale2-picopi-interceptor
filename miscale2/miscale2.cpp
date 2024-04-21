#include <cinttypes>
#include <cstdio>
#include <string>
#include <btstack.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "tls_common.h"

#define TLS_CLIENT_SERVER        "api.telegram.org"
#define TLS_CLIENT_TIMEOUT_SECS  15

#define MISCALE_MAC "mac-addr"

static btstack_packet_callback_registration_t hci_event_callback_registration;

/* LISTING_START(GAPLEAdvSetup): Setting up GAP LE client for receiving advertisements */
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static void gap_le_advertisements_setup(void) {
    // Active scanning, 100% (scan interval = scan window)
    gap_set_scan_parameters(1, 48, 48);
    gap_start_scan();

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
}

static const char *flags[] = {
        "LE Limited Discoverable Mode",
        "LE General Discoverable Mode",
        "BR/EDR Not Supported",
        "Simultaneous LE and BR/EDR to Same Device Capable (Controller)",
        "Simultaneous LE and BR/EDR to Same Device Capable (Host)",
        "Reserved",
        "Reserved",
        "Reserved"
};

static void parse_scale_data(const uint8_t *adv_data, uint8_t adv_size);

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    bd_addr_t address;
    uint8_t event_type;
    uint8_t length;
    const uint8_t *data;

    switch (hci_event_packet_get_type(packet)) {
        case GAP_EVENT_ADVERTISING_REPORT:
            gap_event_advertising_report_get_address(packet, address);
            event_type = gap_event_advertising_report_get_advertising_event_type(packet);
            length = gap_event_advertising_report_get_data_length(packet);
            data = gap_event_advertising_report_get_data(packet);
            if (strcmp(bd_addr_to_str(address), MISCALE_MAC) == 0 && event_type == 0) {
                parse_scale_data(data, length);
            }
            break;
        default:
            break;
    }
}

typedef struct scale_data {
    const char *unit;
    bool stable;
    bool released;
    int value;
} scale_data_t;

int btstack_main() {
    gap_le_advertisements_setup();
    // turn on!
    hci_power_control(HCI_POWER_ON);
    return 0;
}

int previous;

int apiCall(std::string msg) {
    std::string part1 = "POST /{bot_token}/sendMessage?chat_id={chat_id}=";
    std::string part2 = msg + " ";
    std::string part3 = "HTTP/1.1\r\n";
    std::string part4 = "Host: api.telegram.org\r\nConnection: close\r\n\r\n";
    std::string request = part1 + part2 + part3 + part4;

    const int length = request.length();

    // declaring character array (+1 for null terminator)
    char* request_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(request_array, request.c_str());
    bool pass = run_tls_client_test(NULL, 0, TLS_CLIENT_SERVER, request_array, TLS_CLIENT_TIMEOUT_SECS);
    printf("Api call %s\n", pass ? "success" : "failed");
    return 0;
}

static void parse_scale_data(const uint8_t *adv_data, uint8_t adv_size) {
    ad_context_t context;
    scale_data_t scaleData;
    for (ad_iterator_init(&context, adv_size, (uint8_t *) adv_data);
         ad_iterator_has_more(&context); ad_iterator_next(&context)) {
        uint8_t data_type = ad_iterator_get_data_type(&context);
        const uint8_t *data = ad_iterator_get_data(&context);
        if (data_type == BLUETOOTH_DATA_TYPE_SERVICE_DATA) {
            uint8_t ctrlByte = data[2];
            uint8_t unitValue = ctrlByte & 0b00000011;
            uint16_t value = little_endian_read_16(data, 3);
            if (unitValue == 2) {
                scaleData.unit = "kg";
                value = value / 2;
            } else if (unitValue == 3) {
                scaleData.unit = "lb";
            } else {
                scaleData.unit = "non";
            }
            scaleData.stable = ctrlByte == 34;
            scaleData.released = ctrlByte & (1 << 7);
            scaleData.value = value;
            if(scaleData.stable) {
                if(previous != scaleData.value) {
                    int temp = scaleData.value;
                    previous = scaleData.value;
                    std::string result = std::to_string(temp/1000);
                    temp %= 1000;
                    result += std::to_string(temp/100);
                    temp %= 100;
                    result += '.';
                    result += std::to_string(temp);
                    result += scaleData.unit;
                    printf("Weight %s\n", result.c_str());
                    apiCall(result);
                }
            } else {
                previous = 0;
            }
        }
    }
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to wifi connect\n");
        return -1;
    }
    apiCall("ScaleSpy started");

    btstack_main();
    btstack_run_loop_execute();
}

