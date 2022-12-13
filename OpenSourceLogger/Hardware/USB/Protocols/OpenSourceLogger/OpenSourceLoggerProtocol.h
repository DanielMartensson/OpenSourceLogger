
#include "OpenSAEJ1939/Open_SAE_J1939/Open_SAE_J1939.h"
#include <boost/asio.hpp>

#ifndef OpenSourceLoggerProtocol
#define OpenSourceLoggerProtocol

void setReadUSBThreadStart(bool readUSBThreadStart);
void setUSBThreadActive(bool USBThreadActive);
void activateJ1939CallBackAndSetJ1939AndSetUSBPort(J1939* j1939, boost::asio::serial_port* port);
void getRawMeasurements(uint16_t adcRaw[], uint16_t dadcRaw[], uint8_t diRaw[], uint16_t icRaw[], uint16_t eRaw[]);
void sendControlSignals(uint16_t pwmRaw[], uint16_t dacRaw[]);

void setAnalogInputGainToSTM32PLC(uint8_t peripheral, uint8_t configurationIndex, uint8_t gain);
void setPWMPrescalerToSTM32PLC(uint8_t pwmPeripheral, uint16_t prescaler);
void setDateTimeToSTM32PLC(uint8_t year, uint8_t month, uint8_t date, uint8_t weekDay, uint8_t hour, uint8_t minute);
void setAlarmAToSTM32PLC(uint8_t date, uint8_t hour, uint8_t minute, uint8_t enable);
void setAlarmBToSTM32PLC(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t enable);
void askAnalogInputGainsFromSTM32(uint8_t peripheral);
void askPWMPrescalersFromSTM32();
void askDateTimeFromSTM32PLC();
void askAlarmAFromSTM32PLC();
void askAlarmBFromSTM32PLC();
void getPwmPrescalers(uint16_t pwmPrescaler[]);
void resetPwmPrescalers();
void getAnalogGain(uint8_t analogGain[]);
void resetAnalogGain();
void getDateTime(uint8_t* year, uint8_t* month, uint8_t* date, uint8_t* hour, uint8_t* minute);
void resetDateTime();
void getAlarmA(uint8_t* enabledAlarmA, uint8_t* activatedAlarmA, uint8_t* dateAlarmA, uint8_t* hourAlarmA, uint8_t* minuteAlarmA);
void resetAlarmA();
void getAlarmB(uint8_t* enabledAlarmB, uint8_t* activatedAlarmB, uint8_t* weekDayAlarmB, uint8_t* hourAlarmB, uint8_t* minuteAlarmB);
void resetAlarmB();


#endif // !OpenSourceLoggerProtocol


