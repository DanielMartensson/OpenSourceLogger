
#include <stdint.h>

#ifndef CollectMeasurementDialog
#define CollectMeasurementDialog

void showCollectMeasurementsDialog(bool* collectMeasurements, char file_folder_path[]);
void getRawMeasurements(uint16_t adcRaw[], uint16_t dadcRaw[], uint8_t diRaw[], uint16_t icRaw[], uint16_t eRaw[]);
#endif // !CollectMeasurementDialog

