#include <soc/gpio_num.h>
extern "C"{
void mqtt_init();
void mqtt_first_init(const char* username, const char* password);
void wifi_init_sta();
void configure_gpio(gpio_num_t pin);
void flashLED();
bool is_setup_done();
esp_err_t save_setup_flag(bool isSetupRan);
void load_credentials_nvs(char* username_out, size_t username_size, char* password_out, size_t password_size);

}