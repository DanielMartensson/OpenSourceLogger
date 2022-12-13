
#ifndef CalibrationDialog
#define CalibrationDialog

#include "../../../Constants.h"
#include <array>

void showCalibrationDialog(bool* calibrateMeasurements);
void getCalibrateMeasurementsFromRawData(float calibratedADC[], float calibratedDADC[], float calibratedDI[], float calibratedIC[], float calibratedE[]);
void setControlSignals(float sliderPWM[], float sliderDAC[]);
void loadMinMaxCalibration();
std::array<float, PWM_LENGTH> getMinCalibrationPWM();
std::array<float, PWM_LENGTH> getMaxCalibrationPWM();
std::array<float, DAC_LENGTH> getMinCalibrationDAC();
std::array<float, DAC_LENGTH> getMaxCalibrationDAC();
std::array<float, ADC_LENGTH> getMinCalibrationADC();
std::array<float, ADC_LENGTH> getMaxCalibrationADC();
std::array<float, DADC_LENGTH> getMinCalibrationDADC();
std::array<float, DADC_LENGTH> getMaxCalibrationDADC();
std::array<float, IC_LENGTH> getMinCalibrationIC();
std::array<float, IC_LENGTH> getMaxCalibrationIC();
std::array<float, E_LENGTH> getMinCalibrationE();
std::array<float, E_LENGTH> getMaxCalibrationE();
std::array<float, DI_LENGTH> getMinCalibrationDI();
std::array<float, DI_LENGTH> getMaxCalibrationDI();
#endif // !CalibrationDialog

