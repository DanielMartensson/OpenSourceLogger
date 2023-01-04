#include "PulseMeasurementsDialog.h"
#include <imgui.h>
#include "../../../../Hardware/USB/USBHandler.h"
#include "../../../../Constants.h"
#include "nlohmann/json.hpp"
#include <chrono>  
#include <fstream>
#include "../CalibrationDialog/CalibrationDialog.h"
#include "../../../../Hardware/Tools/TimeConverter.h"

inline void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray);
inline void createMeasurementPlotsForPulses(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector);
inline void updateVectorsForPlot(std::vector<float> lineChart[], int showSamplesInPlot, int dimensionOfVector, float calibratedArray[]);
inline void closeDownTheSystem();
inline void logDataPulseCount();

// Control values 
float controlPWM[PWM_LENGTH] = { 0 };
float controlDAC[DAC_LENGTH] = { 0 };

// Log file
bool startLogging = false;
std::ofstream logFile;
int sampleTime = 0;
int pulseCounterDAC[DAC_LENGTH] = { 0 };
int pulseCounterPWM[PWM_LENGTH] = { 0 };

void showPulseMeasureDialog(bool* collectMeasurements, char file_folder_path[]) {

	ImGui::SetNextWindowSize(ImVec2(850.0f, 850.0f));
	ImGui::Begin("Pulse measurements", collectMeasurements, ImGuiWindowFlags_NoResize);

	// Enable fields
	static bool enableADC[ADC_LENGTH];
	static bool enableDADC[DADC_LENGTH];
	static bool enableDAC[DAC_LENGTH];
	static bool enablePWM[PWM_LENGTH];
	static bool enableDI[DI_LENGTH];
	static bool enableIC[IC_LENGTH];
	static bool enableE[E_LENGTH];

	// Settings fields for sampling
	static int showSamplesInPlot = 0;
	static int alarmStopTime = 0;
	static int alarmStopTimeSum = 0;
	static long long timeDifferenceSamplingSum = 0;
	static long long timeDifferencePWMPulseSum[PWM_LENGTH] = { 0 };
	static long long timeDifferenceDACPulseSum[DAC_LENGTH] = { 0 };

	// Log file
	bool logFileExist = std::filesystem::exists(file_folder_path);
	static int pulseCounterMaxPWM[PWM_LENGTH] = { 0 };
	static int pulseCounterMaxDAC[DAC_LENGTH] = { 0 };


	// Settings fields for plot - These must be std::array due to json serialization/deserialization
	static std::array<float, ADC_LENGTH> adcMin;
	static std::array<float, ADC_LENGTH> adcMax;
	static std::array<float, PWM_LENGTH> pwmMin;
	static std::array<float, PWM_LENGTH> pwmMax;
	static std::array<float, DAC_LENGTH> dacMin;
	static std::array<float, DAC_LENGTH> dacMax;
	static std::array<float, DADC_LENGTH> dadcMin;
	static std::array<float, DADC_LENGTH> dadcMax;
	static std::array<float, DI_LENGTH> diMin;
	static std::array<float, DI_LENGTH> diMax;
	static std::array<float, IC_LENGTH> icMin;
	static std::array<float, IC_LENGTH> icMax;
	static std::array<float, E_LENGTH> eMin;
	static std::array<float, E_LENGTH> eMax;

	// Read only once
	static bool fieldsHasBeenLoaded = false;
	if (!fieldsHasBeenLoaded) {
		// For field settings
		std::ifstream plotSettingFields;
		plotSettingFields.open(PLOT_SETTING_FIELDS_JSON);
		if (plotSettingFields.is_open()) {
			// Read json
			std::string jsonLine;
			std::getline(plotSettingFields, jsonLine);
			nlohmann::json j = nlohmann::json::parse(jsonLine);

			// Sort it out
			adcMin = j["adcMin"].get<std::array<float, ADC_LENGTH>>();
			adcMax = j["adcMax"].get<std::array<float, ADC_LENGTH>>();
			pwmMin = j["pwmMin"].get<std::array<float, PWM_LENGTH>>();
			pwmMax = j["pwmMax"].get<std::array<float, PWM_LENGTH>>();
			dacMin = j["dacMin"].get<std::array<float, DAC_LENGTH>>();
			dacMax = j["dacMax"].get<std::array<float, DAC_LENGTH>>();
			dadcMin = j["dadcMin"].get<std::array<float, DADC_LENGTH>>();
			dadcMax = j["dadcMax"].get<std::array<float, DADC_LENGTH>>();
			diMin = j["diMin"].get<std::array<float, DI_LENGTH>>();
			diMax = j["diMax"].get<std::array<float, DI_LENGTH>>();
			icMin = j["icMin"].get<std::array<float, IC_LENGTH>>();
			icMax = j["icMax"].get<std::array<float, IC_LENGTH>>();
			eMin = j["eMin"].get<std::array<float, E_LENGTH>>();
			eMax = j["eMax"].get<std::array<float, E_LENGTH>>();
		}
		plotSettingFields.close();

		// Lock
		fieldsHasBeenLoaded = true;
	}

	// Time
	static std::chrono::steady_clock::time_point t1;
	static std::chrono::steady_clock::time_point t2;

	// Data plot fields
	static std::vector<float> ADC[ADC_LENGTH];
	static std::vector<float> DADC[DADC_LENGTH];
	static std::vector<float> DAC[DAC_LENGTH];
	static std::vector<float> PWM[PWM_LENGTH];
	static std::vector<float> DI[DI_LENGTH];
	static std::vector<float> IC[IC_LENGTH];
	static std::vector<float> E[E_LENGTH];

	// Check if we are connected to the USB
	if (isConnectedToUSB() && logFileExist) {
		// Create checkboxes
		ImGui::BeginChild("groupBoxEnable", ImVec2(0, ImGui::GetFontSize() * 15.0f), true);
		ImGui::Text("Enable measurements:");
		createCheckBoxes("ADC", enableADC, ADC_LENGTH);
		createCheckBoxes("DADC", enableDADC, DADC_LENGTH);
		createCheckBoxes("DAC", enableDAC, DAC_LENGTH);
		createCheckBoxes("PWM", enablePWM, PWM_LENGTH);
		createCheckBoxes("DI", enableDI, DI_LENGTH);
		createCheckBoxes("IC", enableIC, IC_LENGTH);
		createCheckBoxes("E", enableE, E_LENGTH);
		ImGui::EndChild();

		// Sample settings
		ImGui::BeginChild("groupBoxSettings", ImVec2(0, ImGui::GetFontSize() * 15.0f), true);
		ImGui::Text("Settings:");
		ImGui::SliderInt("Show samples for line plot", &showSamplesInPlot, 0, 2000);
		ImGui::InputInt("Alarm stop time [ms] for DI0 < 0.5", &alarmStopTime);

		// Buttons
		if (startLogging) {
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
		}
		if (ImGui::Button("Start logging")) {
			startLogging = true;

			// Open file
			logFile.open(file_folder_path);
			if (!logFile.is_open()) {
				logFileExist = false;
				closeDownTheSystem();
			}else {
				std::string header = "dateTime,sampletime,";
				for (int i = 0; i < PWM_LENGTH; i++) {
					header += "PWM" + std::to_string(i) +  std::string(",");
				}
				for (int i = 0; i < DAC_LENGTH - 1; i++) {
					header += "DAC" + std::to_string(i) + std::string(","); 
				}
				header += "DAC" + std::to_string(DAC_LENGTH - 1) + "\n";
				logFile << header;
			}
		}
		ImGui::PopStyleColor();
		if (!startLogging) {
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop logging")) {
			startLogging = false;
			// Save file
			logFile.close();
		}
		ImGui::PopStyleColor();

		// Input sample time
		ImGui::SameLine();
		int step = 1;
		ImGui::PushItemWidth(100);
		ImGui::InputScalar("Sample time for logging in milliseconds", ImGuiDataType_U16, &sampleTime, &step);
		ImGui::PopItemWidth();
		std::string filePath = std::string("Selected log file:") + std::string(file_folder_path);
		ImGui::Text(filePath.c_str());

		// Pulse time settings time
		char text[1000];
		static int pwmHigh[PWM_LENGTH] = { 0 };
		static int pwmLow[PWM_LENGTH] = { 0 };
		static int dacHigh[DAC_LENGTH] = { 0 };
		static int dacLow[DAC_LENGTH] = { 0 };
		for (int i = 0; i < PWM_LENGTH; i++) {
			if (enablePWM[i]) {
				sprintf_s(text, "PWM%i high time [ms]", i);
				if (ImGui::InputInt(text, &pwmHigh[i])) {
					if (pwmHigh[i] < 0) {
						pwmHigh[i] = 0;
					}
				}
				sprintf_s(text, "PWM%i low time [ms]", i);
				if (ImGui::InputInt(text, &pwmLow[i])) {
					if (pwmLow[i] < 0) {
						pwmLow[i] = 0;
					}
				}
				sprintf_s(text, "PWM%i initial pulse count", i);
				if (ImGui::InputInt(text, &pulseCounterPWM[i])) {
					if (pulseCounterPWM[i] < 0) {
						pulseCounterPWM[i] = 0;
					}
				}
				sprintf_s(text, "PWM%i max pulse count", i);
				if (ImGui::InputInt(text, &pulseCounterMaxPWM[i])) {
					if (pulseCounterMaxPWM[i] < 0) {
						pulseCounterMaxPWM[i] = 0;
					}
				}
				sprintf_s(text, "PWM%i pulse count: %i", i, pulseCounterPWM[i]);
				ImGui::Text(text);

			}else {
				// Disable
				pwmHigh[i] = 0;
				pwmLow[i] = 0;
			}
		}
		for (int i = 0; i < DAC_LENGTH; i++) {
			if (enableDAC[i]) {
				sprintf_s(text, "DAC%i high time [ms]", i);
				if (ImGui::InputInt(text, &dacHigh[i])) {
					if (dacHigh[i] < 0) {
						dacHigh[i] = 0;
					}
				}
				sprintf_s(text, "DAC%i low time [ms]", i);
				if (ImGui::InputInt(text, &dacLow[i])) {
					if (dacLow[i] < 0) {
						dacLow[i] = 0;
					}
				}
				sprintf_s(text, "DAC%i initial pulse count", i);
				if (ImGui::InputInt(text, &pulseCounterDAC[i])) {
					if (pulseCounterDAC[i] < 0) {
						pulseCounterDAC[i] = 0;
					}
				}
				sprintf_s(text, "DAC%i max pulse count", i);
				if (ImGui::InputInt(text, &pulseCounterMaxDAC[i])) {
					if (pulseCounterMaxDAC[i] < 0) {
						pulseCounterMaxDAC[i] = 0;
					}
				}
				sprintf_s(text, "DAC%i pulse count: %i", i, pulseCounterDAC[i]);
				ImGui::Text(text);
			}else {
				// Disable
				dacHigh[i] = 0;
				dacLow[i] = 0;
			}
		}
		ImGui::EndChild();

		// Create plots
		ImGui::Text("Plot lines");
		if (startLogging) {
			// Get the time difference
			t2 = std::chrono::steady_clock::now();
			long long timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			for (int i = 0; i < PWM_LENGTH; i++) {
				timeDifferencePWMPulseSum[i] += timeDifference;
			}
			for (int i = 0; i < DAC_LENGTH; i++) {
				timeDifferenceDACPulseSum[i] += timeDifference;
			}
			timeDifferenceSamplingSum += timeDifference;
			t1 = t2;

			// Implement values for the pulse control signals
			for (int i = 0; i < PWM_LENGTH; i++) {
				if (enablePWM[i] && pulseCounterPWM[i] < pulseCounterMaxPWM[i]) {
					if (pwmLow[i] >= timeDifferencePWMPulseSum[i]) {
						controlPWM[i] = getMinCalibrationPWM().at(i);
					}
					else if (pwmLow[i] < timeDifferencePWMPulseSum[i] && pwmLow[i] + pwmHigh[i] >= timeDifferencePWMPulseSum[i]) {
						controlPWM[i] = getMaxCalibrationPWM().at(i);
					}
					else {
						timeDifferencePWMPulseSum[i] = 0; // Reset
						pulseCounterPWM[i]++; // Count
					}
				}else {
					controlPWM[i] = getMinCalibrationPWM().at(i); // Stop
				}
			}
			for (int i = 0; i < DAC_LENGTH; i++) {
				if (enableDAC[i] && pulseCounterDAC[i] < pulseCounterMaxDAC[i]) {
					if (dacLow[i] >= timeDifferenceDACPulseSum[i]) {
						controlDAC[i] = getMinCalibrationDAC().at(i);
					}
					else if (dacLow[i] < timeDifferenceDACPulseSum[i] && dacLow[i] + dacHigh[i] >= timeDifferenceDACPulseSum[i]) {
						controlDAC[i] = getMaxCalibrationDAC().at(i);
					}
					else {
						timeDifferenceDACPulseSum[i] = 0; // Reset
						pulseCounterDAC[i]++; // Count
					}
				}else {
					controlDAC[i] = getMinCalibrationDAC().at(i); // Stop
				}
			}

			// Set the control signals
			setControlSignals(controlPWM, controlDAC);

			// Calibrate the measurements
			float calibratedADC[ADC_LENGTH];
			float calibratedDADC[DADC_LENGTH];
			float calibratedDI[DI_LENGTH];
			float calibratedIC[IC_LENGTH];
			float calibratedE[E_LENGTH];
			getCalibrateMeasurementsFromRawData(calibratedADC, calibratedDADC, calibratedDI, calibratedIC, calibratedE);

			// Update the sampling
			if (sampleTime <= timeDifferenceSamplingSum) {
				// Check if samples is above 0
				if (showSamplesInPlot > 0) {

					// Update vectors
					updateVectorsForPlot(ADC, showSamplesInPlot, ADC_LENGTH, calibratedADC);
					updateVectorsForPlot(DADC, showSamplesInPlot, DADC_LENGTH, calibratedDADC);
					updateVectorsForPlot(DAC, showSamplesInPlot, DAC_LENGTH, controlDAC);
					updateVectorsForPlot(PWM, showSamplesInPlot, PWM_LENGTH, controlPWM);
					updateVectorsForPlot(DI, showSamplesInPlot, DI_LENGTH, calibratedDI);
					updateVectorsForPlot(IC, showSamplesInPlot, IC_LENGTH, calibratedIC);
					updateVectorsForPlot(E, showSamplesInPlot, E_LENGTH, calibratedE);

					// Log values
					logDataPulseCount();
				}

				// Reset the time difference sum
				timeDifferenceSamplingSum = 0;
			}

			// Show plots
			ImGui::BeginChild("groupBoxPlots", ImVec2(0, 0), true);
			createMeasurementPlotsForPulses(ADC, "ADC", enableADC, ADC_LENGTH);
			createMeasurementPlotsForPulses(DADC, "DADC", enableDADC, DADC_LENGTH);
			createMeasurementPlotsForPulses(DAC, "DAC", enableDAC, DAC_LENGTH);
			createMeasurementPlotsForPulses(PWM, "PWM", enablePWM, PWM_LENGTH);
			createMeasurementPlotsForPulses(DI, "DI", enableDI, DI_LENGTH);
			createMeasurementPlotsForPulses(IC, "IC", enableIC, IC_LENGTH);
			createMeasurementPlotsForPulses(E, "E", enableE, E_LENGTH);
			ImGui::EndChild();

			// Break when DI0 is low for the time alarmStopTime. This is security alert.
			if (calibratedDI[0] < 0.5f) {
				alarmStopTimeSum += timeDifference;
				if (alarmStopTimeSum > alarmStopTime) {
					closeDownTheSystem();
				}				
			}else {
				alarmStopTimeSum = 0; // Reset
			}

			// Break when all pulses have reached its limits
			bool isAllDACPulsesHaveReachItsLimit = false;
			int pulseCounter = 0;
			for (int i = 0; i < DAC_LENGTH; i++) {
				if (pulseCounterDAC[i] >= pulseCounterMaxDAC[i]) {
					pulseCounter++;
				}
				isAllDACPulsesHaveReachItsLimit = pulseCounter >= DAC_LENGTH ? true : false;
			}
			bool isAllPWMPulsesHaveReachItsLimit = false;
			pulseCounter = 0;
			for (int i = 0; i < PWM_LENGTH; i++) {
				if (pulseCounterPWM[i] >= pulseCounterMaxPWM[i]) {
					pulseCounter++;
				}
				isAllPWMPulsesHaveReachItsLimit = pulseCounter >= PWM_LENGTH ? true : false;
			}
			if (isAllPWMPulsesHaveReachItsLimit && isAllDACPulsesHaveReachItsLimit) {
				closeDownTheSystem();
			}
		}
		else {
			// Clear all plots
			ADC->clear();
			DADC->clear();
			DAC->clear();
			PWM->clear();
			DI->clear();
			IC->clear();
			E->clear();
		}
	}
	else if (isConnectedToUSB() && !logFileExist) {
		ImGui::Text("You need to select a measurement file");
		if (ImGui::Button("Close")) {
			*collectMeasurements = false;
		}
	}
	else if (!isConnectedToUSB() && logFileExist) {
		ImGui::Text("You need to be connected to the USB");
		if (ImGui::Button("Close")) {
			*collectMeasurements = false;
		}
	}
	else {
		ImGui::Text("You need to be connected to the USB");
		ImGui::Text("You need to select a measurement file");
		if (ImGui::Button("Close")) {
			*collectMeasurements = false;
		}
	}
	ImGui::End();

	// When we close the window, then *collectMeasurements will be false
	if (*collectMeasurements == false) {
		// Save to json
		nlohmann::json j;
		j["adcMin"] = adcMin;
		j["adcMax"] = adcMax;
		j["pwmMin"] = pwmMin;
		j["pwmMax"] = pwmMax;
		j["dacMin"] = dacMin;
		j["dacMax"] = dacMax;
		j["dadcMin"] = dadcMin;
		j["dadcMax"] = dadcMax;
		j["diMin"] = diMin;
		j["diMax"] = diMax;
		j["icMin"] = icMin;
		j["icMax"] = icMax;
		j["eMin"] = eMin;
		j["eMax"] = eMax;
		std::string json = j.dump();

		// Write
		std::ofstream plotSettingFields;
		plotSettingFields.open(PLOT_SETTING_FIELDS_JSON);
		if (plotSettingFields.is_open()) {
			plotSettingFields << json;
		}
		plotSettingFields.close();

		// Lock
		fieldsHasBeenLoaded = false;
	}
}

inline void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray) {
	for (int i = 0; i < lengthOfArray; i++) {
		std::string text = std::string(pheferialName) + std::to_string(i);
		ImGui::Checkbox(text.data(), &enable[i]);
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

inline void createMeasurementPlotsForPulses(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector) {
	for (int i = 0; i < dimensionOfVector; i++) {
		// Show plot if enabled
		if (enable[i]) {
			// Create the plot - Variable size
			std::string text = std::string(pheferialName) + std::to_string(i) + " value:" + std::to_string(lineChart[i].back());
			float minPlotSize = *min_element(lineChart[i].begin(), lineChart[i].end());
			float maxPlotSize = *max_element(lineChart[i].begin(), lineChart[i].end());
			ImGui::PlotLines(text.data(), lineChart[i].data(), lineChart[i].size(), 0, "", minPlotSize, maxPlotSize, ImVec2(0, 100.0f));
		}
	}
}

inline void updateVectorsForPlot(std::vector<float> lineChart[], int showSamplesInPlot, int dimensionOfVector, float calibratedArray[]) {
	for (int i = 0; i < dimensionOfVector; i++) {
		// Compute the difference
		int difference = lineChart[i].size() - showSamplesInPlot;

		// If we have to much lineChart data compared to samples, then delete some
		if (difference >= 0) {
			for (int j = 0; j <= difference; j++) {
				// Delete one element
				lineChart[i].erase(lineChart[i].begin());

				// Update the difference
				difference = lineChart[i].size() - showSamplesInPlot;
			}
		}

		// Add a new sample
		lineChart[i].push_back(calibratedArray[i]);
	}
}

inline void logDataPulseCount() {
	std::string dataNewLine = currentISO8601Time() + "," + std::to_string(sampleTime) + ",";
	for (int i = 0; i < PWM_LENGTH; i++) {
		dataNewLine += std::to_string(pulseCounterPWM[i]) + std::string(",");
	}
	for (int i = 0; i < DAC_LENGTH - 1; i++) {
		dataNewLine += std::to_string(pulseCounterDAC[i]) + std::string(",");
	}
	dataNewLine += std::to_string(pulseCounterDAC[DAC_LENGTH - 1]) + "\n";
	logFile << dataNewLine;
}

inline void closeDownTheSystem() {
	startLogging = false;
	logDataPulseCount(); // Last value before closing
	logFile.close();

	// Important to close down the control signals
	memset(controlPWM, 0, sizeof(float) * PWM_LENGTH);
	memset(controlDAC, 0, sizeof(float) * DAC_LENGTH);
	setControlSignals(controlPWM, controlDAC);
}

