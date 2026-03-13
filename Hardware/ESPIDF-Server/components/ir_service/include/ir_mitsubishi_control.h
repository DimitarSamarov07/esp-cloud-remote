#define AC_CONTROL_H


#ifdef __cplusplus
#include <string>

void startAcConnection();
void sendTurnSignalMitsubishi( bool state,  float temp,  std::string_view mode,  std::string_view fanSpeed,  bool swing);


#endif

