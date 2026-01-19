#include <esp_err.h>
#include <soc/gpio_num.h>

extern "C" {
void mqtt_init();

void mqtt_first_init(const char *password);

bool is_setup_done();

void load_credentials_nvs(char *username_out, size_t username_size, char *password_out, size_t password_size);
}
