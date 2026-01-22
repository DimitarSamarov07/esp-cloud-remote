#set(
 #       COMPONENT_REQUIRES "IRRemote"
#)
idf_component_register(SRCS "ble_service/ble_communication.cpp"  "main.cpp" "ir_service/ir_communication.c"
        INCLUDE_DIRS "."
        REQUIRES arduino-esp32 wifi_service mqtt_service
        PRIV_REQUIRES nvs_flash)
