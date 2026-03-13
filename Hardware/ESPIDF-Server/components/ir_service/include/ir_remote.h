#define WIFI_CONTROL_H

#ifdef __cplusplus
#include <string>

void startAcConnection();
void sendTurnSignal(const std::string& input, const float temp );

#endif
