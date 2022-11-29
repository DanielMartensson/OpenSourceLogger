
#ifndef CalibrationDialog
#define CalibrationDialog

#include "../../../Constants.h"
#include <array>

void showCalibrationDialog(bool* calibrateMeasurements);
void getCalibrateMeasurementsFromRawData(float calibratedADC[], float calibratedDADC[], float calibratedDI[], float calibratedIC[], float calibratedE[]);
void setControlSignals(float sliderPWM[], float sliderDAC[]);
std::array<float, PWM_LENGTH> getMinCalibrationPWM();
std::array<float, PWM_LENGTH> getMaxCalibrationPWM();
std::array<float, DAC_LENGTH> getMinCalibrationDAC();
std::array<float, DAC_LENGTH> getMaxCalibrationDAC();
#endif // !CalibrationDialog

