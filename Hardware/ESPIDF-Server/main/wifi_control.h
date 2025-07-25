#include <esp_err.h>
#include <soc/gpio_num.h>

extern "C"{
    void wifi_init_sta();
    void wifi_init_setup();
    extern int IS_WIFI_CONNECTED;
}
