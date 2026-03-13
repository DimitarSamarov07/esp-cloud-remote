idf_component_register(SRCS "src/ir_remote.cpp"
        INCLUDE_DIRS "include"
        PRIV_REQUIRES esp_wifi esp_event nvs_flash ir_remote_wrapper)