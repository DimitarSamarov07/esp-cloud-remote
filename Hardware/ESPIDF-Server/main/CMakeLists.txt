#set(
 #       COMPONENT_REQUIRES "IRRemote"
#)
idf_component_register(SRCS "main.cpp" "ir_service/ir_communication.c"
        INCLUDE_DIRS "."
        REQUIRES arduino-esp32 wifi_service mqtt_service ble_service ir_remote_wrapper
        PRIV_REQUIRES nvs_flash)
