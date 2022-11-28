
#include "OpenSAEJ1939/Open_SAE_J1939/Open_SAE_J1939.h"

#ifndef OpenSourceLoggerProtocol
#define OpenSourceLoggerProtocol

void activateJ1939CallBackAndSetJ1939(J1939* j1939);
void readUSBMessage(uint8_t data[], uint32_t length);

#endif // !OpenSourceLoggerProtocol


