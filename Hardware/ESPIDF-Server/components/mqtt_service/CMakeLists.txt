idf_component_register(
        SRCS
        "src/mqtt_service.c"
        "src/mqtt_events.c"
        "src/mqtt_subscriptions.c"
        "src/mqtt_handlers.c"
        "src/mqtt_credentials.c"
        "src/mqtt_registration.c"
        INCLUDE_DIRS
        "include"
        REQUIRES
        mqtt
        esp_http_client
        json
        nvs_flash
)
target_include_directories(${COMPONENT_LIB} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")