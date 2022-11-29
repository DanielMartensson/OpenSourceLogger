
#include "OpenSAEJ1939/Open_SAE_J1939/Open_SAE_J1939.h"

#ifndef OpenSourceLoggerProtocol
#define OpenSourceLoggerProtocol

void activateJ1939CallBackAndSetJ1939(J1939* j1939);
void readUSBMessage(uint8_t data[], uint32_t length);
void getRawMeasurements(uint16_t adcRaw[], uint16_t dadcRaw[], uint8_t diRaw[], uint16_t icRaw[], uint16_t eRaw[]);
void setRawControlSignals(uint16_t pwmRaw[], uint16_t dacRaw[]);

#endif // !OpenSourceLoggerProtocol


