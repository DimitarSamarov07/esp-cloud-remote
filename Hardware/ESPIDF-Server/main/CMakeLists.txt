#set(
 #       COMPONENT_REQUIRES "IRRemote"
#)
idf_component_register(SRCS "ble_serice/ble_communication.cpp" "mqtt_service/mqtt_communication.c" "main.cpp" "ir_service/ir_communication.c" "wifi_service/wifi_control.c"
        INCLUDE_DIRS "."
        REQUIRES arduino-esp32 mqtt json
        PRIV_REQUIRES nvs_flash)
