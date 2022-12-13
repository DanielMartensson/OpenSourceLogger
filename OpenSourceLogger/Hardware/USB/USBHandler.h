
#include <string>
#include <vector>
#include "Protocols/OpenSourceLogger/OpenSourceLoggerProtocol.h"

#ifndef USBHandler
#define USBHandler

bool openUSBConnection(std::string portName, int baudrateIndex, J1939* J1939);
bool isConnectedToUSB();
bool closeUSBConnection();
void closeUSBThread();
void getUSBPortNames(std::vector<std::string>& portNames);

#endif // !USBHandler

