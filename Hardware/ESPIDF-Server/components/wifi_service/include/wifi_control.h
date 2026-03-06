// ... existing code ...

    void wifi_init_setup();
    void wifi_init_sta(char* ssid, const char* password);
    void change_wifi(char* new_ssid, const char* new_password);
    std::string perform_wifi_scan();


    extern bool is_user_initiated_disconnect;
    extern int IS_WIFI_CONNECTED;
