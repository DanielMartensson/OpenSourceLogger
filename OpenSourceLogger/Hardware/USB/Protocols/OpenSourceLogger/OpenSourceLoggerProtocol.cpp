#include "OpenSourceLoggerProtocol.h"
#include "../../../../Constants.h"

/* J1939 struct */
J1939* j1939_;

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
static uint8_t TX_CAN_BUS_MESSAGE[CAN_BUS_MESSAGE] = { 0 };
static uint8_t RX_CAN_BUS_MESSAGE[CAN_BUS_MESSAGE] = { 0 };

/* IDE in STM32 */
#define CAN_ID_STD                  (0x00000000U)  /*!< Standard Id */
#define CAN_ID_EXT                  (0x00000004U)  /*!< Extended Id */

/* SAE J1939 callback function for sending a message */
void Callback_Function_Send(uint32_t ID, uint8_t DLC, uint8_t data[]) {
    TX_CAN_BUS_MESSAGE[0] = CAN_ID_EXT;
    TX_CAN_BUS_MESSAGE[1] = ID >> 24;
    TX_CAN_BUS_MESSAGE[2] = ID >> 16;
    TX_CAN_BUS_MESSAGE[3] = ID >> 8;
    TX_CAN_BUS_MESSAGE[4] = ID;
    TX_CAN_BUS_MESSAGE[5] = DLC;
    for (uint8_t i = 0; i < DLC; i++)
        TX_CAN_BUS_MESSAGE[6 + i] = data[i];
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
        for (int i = 0; i < DLC; i++)
            data[i] = RX_CAN_BUS_MESSAGE[6 + i];
        *is_new_message = true;
    }
}

// Fields for the measurements
uint8_t analogGain[ANALOG_GAIN];
uint16_t pwmPrescaler[PWM_PRESCALER];
uint8_t digitalInput[DI_LENGTH];
uint16_t analogSingleInput[ADC_LENGTH];
uint16_t analogDifferentialInput[DADC_LENGTH];
uint16_t inputCapture[IC_LENGTH];
uint16_t encoderInput[E_LENGTH];

// Functions fot the measurements
uint32_t readAnalogGainsFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readPWMPrescalersFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readCanBusMessageFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readMeasurementsFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readDateTimeFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readAlarmAFromSTM32PLC(uint8_t data[], uint32_t byteIndex);
uint32_t readAlarmBFromSTM32PLC(uint8_t data[], uint32_t byteIndex);

void activateJ1939CallBackAndSetJ1939(J1939* j1939) {
    /* Set the stcture address */
    j1939_ = j1939;

    /* This functions will be called */
    CAN_Set_Callback_Functions(Callback_Function_Send, Callback_Function_Read);
}

void readUSBMessage(uint8_t data[], uint32_t length) {
    /* Check what type of data */
    uint32_t byteIndex = 0;
    while (byteIndex < length) {
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
            byteIndex = length;
            break;
        }
    }
}

uint32_t readAnalogGainsFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
    analogGain[0] = data[byteIndex++];
    analogGain[1] = data[byteIndex++];
    analogGain[2] = data[byteIndex++];
    return byteIndex;
}

uint32_t readPWMPrescalersFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
    pwmPrescaler[0] = (data[byteIndex] << 8) | data[byteIndex + 1];
    byteIndex += 2;
    pwmPrescaler[1] = (data[byteIndex] << 8) | data[byteIndex + 1];
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
    for (int i = 0; i < RX_CAN_BUS_MESSAGE[5]; i++)
        RX_CAN_BUS_MESSAGE[6 + i] = data[byteIndex++]; /* Data */

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
    uint8_t year = data[byteIndex++];
    uint8_t month = data[byteIndex++];
    uint8_t date = data[byteIndex++];
    uint8_t weekDay = data[byteIndex++];
    uint8_t hour = data[byteIndex++];
    uint8_t minute = data[byteIndex++];

    /* Update */

    return byteIndex;
}

uint32_t readAlarmAFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
    /* Get alarm A */
    uint8_t date = data[byteIndex++];
    uint8_t hour = data[byteIndex++];
    uint8_t minute = data[byteIndex++];
    uint8_t enabled = data[byteIndex++];
    uint8_t activated = data[byteIndex++];
    
  
    return byteIndex;
}

uint32_t readAlarmBFromSTM32PLC(uint8_t data[], uint32_t byteIndex) {
    /* Get Alarm B */
    uint8_t weekDay = data[byteIndex++];
    uint8_t hour = data[byteIndex++];
    uint8_t minute = data[byteIndex++];
    uint8_t enabled = data[byteIndex++];
    uint8_t activated = data[byteIndex++];

    return byteIndex;
}