set(
        COMPONENT_REQUIRES "IRRemote"
)
idf_component_register(SRCS "ble_communication.c" "mqtt_communication.c" "main.cpp" "ir_communication.c"
        INCLUDE_DIRS "."
        REQUIRES arduino-esp32 mqtt
        PRIV_REQUIRES nvs_flash)
