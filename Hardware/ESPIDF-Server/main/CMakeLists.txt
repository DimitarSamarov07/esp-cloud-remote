#set(
 #       COMPONENT_REQUIRES "IRRemote"
#)
idf_component_register(SRCS "main.cpp"
        INCLUDE_DIRS "."
        REQUIRES arduino-esp32 wifi_service mqtt_service ble_service ir_service
        PRIV_REQUIRES nvs_flash)
