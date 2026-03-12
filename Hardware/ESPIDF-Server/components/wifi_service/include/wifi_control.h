#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H

#ifdef __cplusplus
#include <string>

extern "C" {
#endif


void wifi_init_setup();

void wifi_init_sta(char *ssid, const char *password);

void change_wifi(char *new_ssid, const char *new_password);

int checkWIFI();

extern int IS_WIFI_CONNECTED;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
std::string perform_wifi_scan();
#endif

#endif // WIFI_CONTROL_H
