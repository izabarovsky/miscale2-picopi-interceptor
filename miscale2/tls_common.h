#include "pico/stdlib.h"
extern "C" {

bool run_tls_client_test(const uint8_t *cert, size_t cert_len, const char *server, const char *request, int timeout);

}