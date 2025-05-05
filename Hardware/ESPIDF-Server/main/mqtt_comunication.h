#include <soc/gpio_num.h>
extern "C"{
void mqtt_init();
void mqtt_first_init();
void wifi_init_sta();
void configure_gpio(gpio_num_t pin);
void flashLED();
}