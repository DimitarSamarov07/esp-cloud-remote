idf_component_register(SRCS "src/ble_communication.cpp"
        INCLUDE_DIRS "include"
        PRIV_REQUIRES wifi_service bt nvs_flash)