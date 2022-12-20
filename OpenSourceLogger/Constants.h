
#ifndef Constants
#define Constants

// For the measurements
#define ADC_LENGTH 12
#define DADC_LENGTH 5
#define DAC_LENGTH 3
#define DI_LENGTH 10
#define E_LENGTH 3
#define PWM_LENGTH 8
#define IC_LENGTH 4
#define ALL_MEASUREMENT_LENGTH ADC_LENGTH + DADC_LENGTH + DAC_LENGTH + DI_LENGTH + E_LENGTH + PWM_LENGTH + IC_LENGTH

// For the settings
#define PWM_PRESCALER 2
#define ANALOG_GAIN 3

// For the CAN Bus
#define CAN_BUS_MESSAGE 14

// For the colors
#define COLOR_GREEN (ImVec4)ImColor(255, 0, 0)
#define COLOR_RED (ImVec4)ImColor(0, 255, 0)

// Files
#define CONNECTION_FIELDS_JSON "connectionFields.json"
#define PLOT_SETTING_FIELDS_JSON "plotSettingFields.json"
#define CALIBRATION_FIELDS_JSON "calibrationFields.json"

#endif // !Constants

