idf_component_register(SRCS "src/wifi_control.c"
        INCLUDE_DIRS "include"
        PRIV_REQUIRES esp_wifi esp_event nvs_flash esp_netif)