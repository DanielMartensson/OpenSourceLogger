#include "OpenSourceLoggerProtocol.h"
#include "../../../../Constants.h"
#include <thread>

/* J1939 struct and USB port */
J1939* j1939_;
boost::asio::serial_port* port_;

/* Message types for STM32 PLC */
typedef enum {
	SEND_CAN_BUS_MESSAGE_TYPE,
	READ_CAN_BUS_MESSAGE_TYPE,
	SEND_MEASUREMENTS_MESSAGE_TYPE,
	READ_CONTROL_SIGNALS_MESSAGE_TYPE,
	READ_SET_ANALOG_INPUT_GAIN_MESSAGE_TYPE,
	READ_SET_PWM_PRESCALER_MESSAGE_TYPE,
	SEND_BACK_PWM_PRESCALERS_MESSAGE_TYPE,
	SEND_BACK_ANALOG_GAINS_MESSAGE_TYPE,
	READ_SET_DATE_TIME_MESSAGE_TYPE,
	SEND_BACK_DATE_TIME_MESSAGE_TYPE,
	READ_SET_ALARM_A_MESSAGE_TYPE,
	SEND_BACK_ALARM_A_MESSAGE_TYPE,
	READ_SET_ALARM_B_MESSAGE_TYPE,
	SEND_BACK_ALARM_B_MESSAGE_TYPE
}MESSAGE_TYPES;

/* For the CAN read callback function */
uint8_t RX_CAN_BUS_MESSAGE[CAN_BUS_MESSAGE] = { 0 };
uint8_t TX_CAN_BUS_MESSAGE[CAN_BUS_MESSAGE] = { 0 };
bool isNewRxMessage_ = false;
bool isNewTxMessage_ = false;

/* Handler for transmitting via USB */
void handler(const boost::system::error_code& error, std::size_t bytes_transferred) {
	//printf("Transmitting USB: error code = %li, bytes_transferred = %li\n", error.value(), bytes_transferred);
}

/* SAE J1939 callback function for sending a message */
void Callback_Function_Send(uint32_t ID, uint8_t DLC, uint8_t data[]) {
	uint8_t txData[1 + CAN_BUS_MESSAGE] = { 0 };
	txData[0] = READ_CAN_BUS_MESSAGE_TYPE;
	txData[1] = CAN_ID_EXT;
	TX_CAN_BUS_MESSAGE[0] = CAN_ID_EXT;
	txData[2] = ID >> 24;
	TX_CAN_BUS_MESSAGE[1] = ID >> 24;
	txData[3] = ID >> 16;
	TX_CAN_BUS_MESSAGE[2] = ID >> 16;
	txData[4] = ID >> 8;
	TX_CAN_BUS_MESSAGE[3] = ID >> 8;
	txData[5] = ID;
	TX_CAN_BUS_MESSAGE[4] = ID;
	txData[6] = DLC;
	TX_CAN_BUS_MESSAGE[5] = DLC;
	for (uint8_t i = 0; i < DLC; i++) {
		txData[7 + i] = data[i];
		TX_CAN_BUS_MESSAGE[6 + i] = data[i];
	}
	isNewTxMessage_ = true;

	// Send CAN-bus data
	port_->async_write_some(boost::asio::buffer(txData, sizeof(txData)), handler);
}

/* SAE J1939 callback function for reading a message */
void Callback_Function_Read(uint32_t* ID, uint8_t data[], bool* is_new_message) {
	/* Check if it's a message for SAE J1939 */
	if (RX_CAN_BUS_MESSAGE[0] == CAN_ID_STD) {
		*is_new_message = false;
	}
	else {
		*ID = (RX_CAN_BUS_MESSAGE[1] << 24) | (RX_CAN_BUS_MESSAGE[2] << 16) | (RX_CAN_BUS_MESSAGE[3] << 8) | RX_CAN_BUS_MESSAGE[4];
		uint8_t DLC = RX_CAN_BUS_MESSAGE[5];
		for (int i = 0; i < DLC; i++) {
			data[i] = RX_CAN_BUS_MESSAGE[6 + i];
		}
		isNewRxMessage_ = true;
		*is_new_message = true;
	}
}

// Thread for USB reading
uint8_t data[1024];
void readDataViaUSBThread();
std::thread usbReader(&readDataViaUSBThread);
bool readUSBThreadStart_ = false;
bool USBThreadActive_ = true;

// Fields for the measurements
uint8_t digitalInput[DI_LENGTH] = { 0 };
uint16_t analogSingleInput[ADC_LENGTH] = { 0 };
uint16_t analogDifferentialInput[DADC_LENGTH] = { 0 };
uint16_t inputCapture[IC_LENGTH] = { 0 };
uint16_t encoderInput[E_LENGTH] = { 0 };

// Fields for the settings
uint8_t analogGain_[ANALOG_GAIN] = { 0 };
uint16_t pwmPrescaler_[PWM_PRESCALER] = { 0 };

// Fields for the date time
uint8_t year_ = 0;
uint8_t month_ = 0;
uint8_t date_ = 0;
uint8_t hour_ = 0;
uint8_t minute_ = 0;

// Fields for the alarm A
uint8_t dateAlarmA_ = 1;
uint8_t hourAlarmA_ = 0;
uint8_t minuteAlarmA_ = 0;
uint8_t enabledAlarmA_ = 0;
uint8_t activatedAlarmA_ = 0;

// Fields for the alarm B
uint8_t weekDayAlarmB_ = 1;
uint8_t hourAlarmB_ = 0;
uint8_t minuteAlarmB_ = 0;
uint8_t enabledAlarmB_ = 0;
uint8_t activatedAlarmB_ = 0;

// Functions fot the measurements
uint32_t readAnalogGainsFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readPWMPrescalersFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readCanBusMessageFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readMeasurementsFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readDateTimeFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readAlarmAFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readAlarmBFromSTM32PLC(uint8_t data[], uint32_t byteIndex);


void readDataViaUSBThread() {
	while (USBThreadActive_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (readUSBThreadStart_) {
			// This try-catch statement is used because if we close the USB port, then the thread stops
			try {
				uint32_t readBytes;
				uint32_t byteIndex;
				while (1) {
					// Blocking read
					readBytes = port_->read_some(boost::asio::buffer(data, sizeof(data)));

					// Check what type of data
					byteIndex = 0;
					while (byteIndex < readBytes) {
						uint8_t messageType = data[byteIndex++];
						switch (messageType) {
						case SEND_BACK_ANALOG_GAINS_MESSAGE_TYPE:
							byteIndex = readAnalogGainsFromSTM32PLC(data, byteIndex);
							break;
						case SEND_BACK_PWM_PRESCALERS_MESSAGE_TYPE:
							byteIndex = readPWMPrescalersFromSTM32PLC(data, byteIndex);
							break;
						case SEND_MEASUREMENTS_MESSAGE_TYPE:
							byteIndex = readMeasurementsFromSTM32PLC(data, byteIndex);
							break;
						case SEND_CAN_BUS_MESSAGE_TYPE:
							byteIndex = readCanBusMessageFromSTM32PLC(data, byteIndex);
							break;
						case SEND_BACK_DATE_TIME_MESSAGE_TYPE:
							byteIndex = readDateTimeFromSTM32PLC(data, byteIndex);
							break;
						case SEND_BACK_ALARM_A_MESSAGE_TYPE:
							byteIndex = readAlarmAFromSTM32PLC(data, byteIndex);
							break;
						case SEND_BACK_ALARM_B_MESSAGE_TYPE:
							byteIndex = readAlarmBFromSTM32PLC(data, byteIndex);
							break;
						default:
							/* Something bad happen - End this by doing this */
							byteIndex = readBytes;
							break;
						}
					}

				}
			}
			catch (...) {}
		}
	}

	// Terminate the thread
	usbReader.detach();
}

void setReadUSBThreadStart(bool readUSBThreadStart) {
	readUSBThreadStart_ = readUSBThreadStart;
}

void setUSBThreadActive(bool USBThreadActive) {
	USBThreadActive_ = USBThreadActive;
}

void activateJ1939CallBackAndSetJ1939AndSetUSBPort(J1939* j1939, boost::asio::serial_port* port) {
	/* Set the stcture address */
	j1939_ = j1939;
	port_ = port;

	/* These functions will be called */
	CAN_Set_Callback_Functions(Callback_Function_Send, Callback_Function_Read);
}

uint32_t readAnalogGainsFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	analogGain_[0] = data[byteIndex++];
	analogGain_[1] = data[byteIndex++];
	analogGain_[2] = data[byteIndex++];
	return byteIndex;
}

uint32_t readPWMPrescalersFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	pwmPrescaler_[0] = (data[byteIndex] << 8) | data[byteIndex + 1];
	byteIndex += 2;
	pwmPrescaler_[1] = (data[byteIndex] << 8) | data[byteIndex + 1];
	byteIndex += 2;
	return byteIndex;
}

uint32_t readMeasurementsFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	/* Digital inputs */
	for (int i = 0; i < DI_LENGTH; i++)
		digitalInput[i] = data[byteIndex++];

	/* Analog single inputs */
	for (int i = 0; i < ADC_LENGTH; i++) {
		analogSingleInput[i] = (data[byteIndex] << 8) | data[byteIndex + 1];
		byteIndex += 2;
	}

	/* Analog differential inputs */
	for (int i = 0; i < DADC_LENGTH; i++) {
		analogDifferentialInput[i] = (data[byteIndex] << 8) | data[byteIndex + 1];
		byteIndex += 2;
	}

	/* Inputs capture */
	for (int i = 0; i < IC_LENGTH; i++) {
		uint16_t period = (data[byteIndex] << 8) | data[byteIndex + 1];
		inputCapture[i] = 1.0f / (((float)period) * 0.0001f); /* 0.0001 because the input capture clock is at 10 kHz */
		byteIndex += 2;
	}

	/* Encoder inputs */
	for (int i = 0; i < E_LENGTH; i++) {
		encoderInput[i] = (data[byteIndex] << 8) | data[byteIndex];
		byteIndex += 2;
	}

	return byteIndex;
}

uint32_t readCanBusMessageFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	RX_CAN_BUS_MESSAGE[0] = data[byteIndex++]; /* IDE */
	RX_CAN_BUS_MESSAGE[1] = data[byteIndex++]; /* ID MSB */
	RX_CAN_BUS_MESSAGE[2] = data[byteIndex++];
	RX_CAN_BUS_MESSAGE[3] = data[byteIndex++];
	RX_CAN_BUS_MESSAGE[4] = data[byteIndex++]; /* ID LSB */
	RX_CAN_BUS_MESSAGE[5] = data[byteIndex++]; /* DLC */
	for (int i = 0; i < 8; i++) {
		if (i < RX_CAN_BUS_MESSAGE[5]) {
			RX_CAN_BUS_MESSAGE[6 + i] = data[byteIndex++]; /* Data */
		} else {
			byteIndex++;
		}
	}

	/*
	 * Listen for CAN bus data - Only one function will proceed to read message.
	 * This will call the Callback_Function_Read read above
	 */
	Open_SAE_J1939_Listen_For_Messages(j1939_);

	/* TODO: Read the CANopen message her as well */

	return byteIndex;
}

uint32_t readDateTimeFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	/* Get date time */
	year_ = data[byteIndex++];
	month_ = data[byteIndex++];
	date_ = data[byteIndex++];
	data[byteIndex++]; // uint8_t weekDay =Not being used
	hour_ = data[byteIndex++];
	minute_ = data[byteIndex++];
	return byteIndex;
}

uint32_t readAlarmAFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	/* Get alarm A */
	dateAlarmA_ = data[byteIndex++];
	hourAlarmA_ = data[byteIndex++];
	minuteAlarmA_ = data[byteIndex++];
	enabledAlarmA_ = data[byteIndex++];
	activatedAlarmA_ = data[byteIndex++];
	return byteIndex;
}

uint32_t readAlarmBFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
	/* Get Alarm B */
	weekDayAlarmB_ = data[byteIndex++];
	hourAlarmB_ = data[byteIndex++];
	minuteAlarmB_ = data[byteIndex++];
	enabledAlarmB_ = data[byteIndex++];
	activatedAlarmB_ = data[byteIndex++];
	return byteIndex;
}

void getRawMeasurements(uint16_t adcRaw[], uint16_t dadcRaw[], uint8_t diRaw[], uint16_t icRaw[], uint16_t eRaw[]) {
	memcpy(adcRaw, analogSingleInput, sizeof(uint16_t) * ADC_LENGTH);
	memcpy(dadcRaw, analogDifferentialInput, sizeof(uint16_t) * DADC_LENGTH);
	memcpy(diRaw, digitalInput, sizeof(uint8_t) * DI_LENGTH);
	memcpy(icRaw, inputCapture, sizeof(uint16_t) * IC_LENGTH);
	memcpy(eRaw, encoderInput, sizeof(uint16_t) * E_LENGTH);
}

void sendControlSignals(uint16_t pwmRaw[], uint16_t dacRaw[]) {
	uint8_t data[1 + PWM_LENGTH * 2 + DAC_LENGTH * 2];
	int index = 0;
	data[index++] = READ_CONTROL_SIGNALS_MESSAGE_TYPE;
	for (int i = 0; i < PWM_LENGTH; i++) {
		data[index++] = pwmRaw[i] >> 8;
		data[index++] = pwmRaw[i];
	}
	for (int i = 0; i < DAC_LENGTH; i++) {
		data[index++] = dacRaw[i] >> 8;
		data[index++] = dacRaw[i];
	}

	// Send control data
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void setAnalogInputGainToSTM32PLC(uint8_t peripheral, uint8_t configurationIndex, uint8_t gain) {
	uint8_t data[4];
	data[0] = READ_SET_ANALOG_INPUT_GAIN_MESSAGE_TYPE;
	data[1] = peripheral;
	data[2] = configurationIndex;
	data[3] = gain;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void setPWMPrescalerToSTM32PLC(uint8_t pwmPeripheral, uint16_t prescaler) {
	uint8_t data[4];
	data[0] = READ_SET_PWM_PRESCALER_MESSAGE_TYPE;
	data[1] = pwmPeripheral;
	data[2] = prescaler >> 8;
	data[3] = prescaler;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void setDateTimeToSTM32PLC(uint8_t year, uint8_t month, uint8_t date, uint8_t weekDay, uint8_t hour, uint8_t minute) {
	uint8_t data[7];
	data[0] = READ_SET_DATE_TIME_MESSAGE_TYPE;
	data[1] = year;
	data[2] = month;
	data[3] = date;
	data[4] = weekDay;
	data[5] = hour;
	data[6] = minute;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void setAlarmAToSTM32PLC(uint8_t date, uint8_t hour, uint8_t minute, uint8_t enable) {
	data[0] = READ_SET_ALARM_A_MESSAGE_TYPE;
	data[1] = date;
	data[2] = hour;
	data[3] = minute;
	data[4] = enable;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void setAlarmBToSTM32PLC(uint8_t weekDay, uint8_t hour, uint8_t minute, uint8_t enable) {
	uint8_t data[5];
	data[0] = READ_SET_ALARM_B_MESSAGE_TYPE;
	data[1] = weekDay;
	data[2] = hour;
	data[3] = minute;
	data[4] = enable;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void askAnalogInputGainsFromSTM32(uint8_t peripheral) {
	/* Ask STM32 */
	uint8_t data[2];
	data[0] = SEND_BACK_ANALOG_GAINS_MESSAGE_TYPE;
	data[1] = peripheral;
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void askPWMPrescalersFromSTM32() {
	/* Ask STM32 */
	uint8_t data[1] = { SEND_BACK_PWM_PRESCALERS_MESSAGE_TYPE };
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void askDateTimeFromSTM32PLC() {
	/* Ask STM32 */
	uint8_t data[1] = { SEND_BACK_DATE_TIME_MESSAGE_TYPE };
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void askAlarmAFromSTM32PLC() {
	/* Ask STM32 */
	uint8_t data[1] = { SEND_BACK_ALARM_A_MESSAGE_TYPE };
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void askAlarmBFromSTM32PLC() {
	/* Ask STM32 */
	uint8_t data[1] = { SEND_BACK_ALARM_B_MESSAGE_TYPE };
	port_->async_write_some(boost::asio::buffer(data, sizeof(data)), handler);
}

void getPwmPrescalers(uint16_t pwmPrescaler[]) {
	pwmPrescaler[0] = pwmPrescaler_[0];
	pwmPrescaler[1] = pwmPrescaler_[1];

}

void resetPwmPrescalers() {
	pwmPrescaler_[0] = 0;
	pwmPrescaler_[1] = 0;
}

void getAnalogGain(uint8_t analogGain[]) {
	analogGain[0] = analogGain_[0];
	analogGain[1] = analogGain_[1];
}

void resetAnalogGain() {
	pwmPrescaler_[0] = 0;
	pwmPrescaler_[1] = 0;
}

void getDateTime(uint8_t* year, uint8_t* month, uint8_t* date, uint8_t* hour, uint8_t* minute) {
	*year = year_;
	*month = month_;
	*date = date_;
	*hour = hour_;
	*minute = minute_;
}

void resetDateTime() {
	year_ = 0;
	month_ = 0;
	date_ = 0;
	hour_ = 0;
	minute_ = 0;
}

void getAlarmA(uint8_t* enabledAlarmA, uint8_t* activatedAlarmA, uint8_t* dateAlarmA, uint8_t* hourAlarmA, uint8_t* minuteAlarmA) {
	*enabledAlarmA = enabledAlarmA_;
	*activatedAlarmA = activatedAlarmA_;
	*dateAlarmA = dateAlarmA_;
	*hourAlarmA = hourAlarmA_;
	*minuteAlarmA = minuteAlarmA_;
}

void resetAlarmA() {
	enabledAlarmA_ = 0;
	activatedAlarmA_ = 0;
	dateAlarmA_ = 1;
	hourAlarmA_ = 0;
	minuteAlarmA_ = 0;
}

void getAlarmB(uint8_t* enabledAlarmB, uint8_t* activatedAlarmB, uint8_t* weekDayAlarmB, uint8_t* hourAlarmB, uint8_t* minuteAlarmB) {
	*enabledAlarmB = enabledAlarmB_;
	*activatedAlarmB = activatedAlarmB_;
	*weekDayAlarmB = weekDayAlarmB_;
	*hourAlarmB = hourAlarmB_;
	*minuteAlarmB = minuteAlarmB_;
}

void resetAlarmB() {
	enabledAlarmB_ = 0;
	activatedAlarmB_ = 0;
	weekDayAlarmB_ = 1;
	hourAlarmB_ = 0;
	minuteAlarmB_ = 0;
}

uint8_t* getRxCanBusMessage(bool* isNewMessage) {
	*isNewMessage = isNewRxMessage_;
	isNewRxMessage_ = false;
	return RX_CAN_BUS_MESSAGE;
}

uint8_t* getTxCanBusMessage(bool* isNewMessage) {
	*isNewMessage = isNewTxMessage_;
	isNewTxMessage_ = false;
	return TX_CAN_BUS_MESSAGE;
}