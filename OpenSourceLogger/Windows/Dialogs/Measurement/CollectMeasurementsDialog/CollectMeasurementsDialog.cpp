#include "CollectMeasurementsDialog.h"
#include <imgui.h>
#include "../../../../Hardware/USB/USBHandler.h"
#include "../../../../Constants.h"
#include "nlohmann/json.hpp"
#include <chrono>  
#include <fstream>
#include "../CalibrationDialog/CalibrationDialog.h"
#include "../../../../Hardware/Tools/TimeConverter.h"

inline void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray);
inline void createMeasurementPlots(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector, bool isControlOutput = false, float minValue[] = nullptr, float maxValue[] = nullptr, float slider[] = nullptr);
inline void updateVectorsForPlot(std::vector<float> lineChart[], int showSamplesInPlot, int dimensionOfVector, float calibratedArray[]);

void showCollectMeasurementsDialog(bool* collectMeasurements, char file_folder_path[]) {

	ImGui::SetNextWindowSize(ImVec2(850.0f, 850.0f));
	ImGui::Begin("Collect measurements", collectMeasurements, ImGuiWindowFlags_NoResize);

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
	static int sampleTime = 0;
	static bool startLogging = false;
	static long long timeDifferenceSum = 0;

	// Log file
	static std::ofstream logFile;
	bool logFileExist = std::filesystem::exists(file_folder_path);

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

	// Control values 
	static float controlPWM[PWM_LENGTH] = { 0 };
	static float controlDAC[DAC_LENGTH] = { 0 };

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
				startLogging = false;
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
		ImGui::EndChild();

		// Create plots
		ImGui::Text("Plot lines");
		if (startLogging) {
			// Get the time difference
			t2 = std::chrono::steady_clock::now();
			timeDifferenceSum += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			t1 = t2;

			// Update the sampling
			if (sampleTime <= timeDifferenceSum) {
				// Check if samples is above 0
				if (showSamplesInPlot > 0) {

					// Set the control signals
					setControlSignals(controlPWM, controlDAC);

					// Calibrate the measurements
					float calibratedADC[ADC_LENGTH];
					float calibratedDADC[DADC_LENGTH];
					float calibratedDI[DI_LENGTH];
					float calibratedIC[IC_LENGTH];
					float calibratedE[E_LENGTH];
					getCalibrateMeasurementsFromRawData(calibratedADC, calibratedDADC, calibratedDI, calibratedIC, calibratedE);

					// Update vectors
					updateVectorsForPlot(ADC, showSamplesInPlot, ADC_LENGTH, calibratedADC);
					updateVectorsForPlot(DADC, showSamplesInPlot, DADC_LENGTH, calibratedDADC);
					updateVectorsForPlot(DAC, showSamplesInPlot, DAC_LENGTH, controlDAC);
					updateVectorsForPlot(PWM, showSamplesInPlot, PWM_LENGTH, controlPWM);
					updateVectorsForPlot(DI, showSamplesInPlot, DI_LENGTH, calibratedDI);
					updateVectorsForPlot(IC, showSamplesInPlot, IC_LENGTH, calibratedIC);
					updateVectorsForPlot(E, showSamplesInPlot, E_LENGTH, calibratedE);

					// Log values
					std::string dataNewLine = currentISO8601Time() + "," + std::to_string(sampleTime) + ",";
					for (int i = 0; i < ADC_LENGTH; i++) {
						dataNewLine += std::to_string(ADC[i].back()) + std::string(", "); // Analog to digital converter
					}
					for (int i = 0; i < PWM_LENGTH; i++) {
						dataNewLine += std::to_string(PWM[i].back()) + std::string(", "); // Pulse widt modulation
					}
					for (int i = 0; i < DAC_LENGTH; i++) {
						dataNewLine += std::to_string(DAC[i].back()) + std::string(", "); // Digital to analog converter
					}
					for (int i = 0; i < DADC_LENGTH; i++) {
						dataNewLine += std::to_string(DADC[i].back()) + std::string(", "); // Differential ADC
					}
					for (int i = 0; i < DI_LENGTH; i++) {
						dataNewLine += std::to_string(DI[i].back()) + std::string(", "); // Digital input
					}
					for (int i = 0; i < IC_LENGTH; i++) {
						dataNewLine += std::to_string(IC[i].back()) + std::string(", "); // Input capture
					}
					for (int i = 0; i < E_LENGTH - 1; i++) {
						dataNewLine += std::to_string(E[i].back()) + std::string(", "); // Encoder
					}
					dataNewLine += std::to_string(E[E_LENGTH - 1].back()) + "\n";
					logFile << dataNewLine;
				}

				// Reset the time difference sum
				timeDifferenceSum = 0;
			}

			// Show plots
			ImGui::BeginChild("groupBoxPlots", ImVec2(0, 0), true);
			createMeasurementPlots(ADC, "ADC", enableADC, ADC_LENGTH);
			createMeasurementPlots(DADC, "DADC", enableDADC, DADC_LENGTH);
			createMeasurementPlots(DAC, "DAC", enableDAC, DAC_LENGTH, true, getMinCalibrationDAC().data(), getMaxCalibrationDAC().data(), controlDAC);
			createMeasurementPlots(PWM, "PWM", enablePWM, PWM_LENGTH, true, getMinCalibrationPWM().data(), getMaxCalibrationPWM().data(), controlPWM);
			createMeasurementPlots(DI, "DI", enableDI, DI_LENGTH);
			createMeasurementPlots(IC, "IC", enableIC, IC_LENGTH);
			createMeasurementPlots(E, "E", enableE, E_LENGTH);
			ImGui::EndChild();

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

inline void createMeasurementPlots(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector, bool isControlOutput, float minValue[], float maxValue[], float slider[]) {
	for (int i = 0; i < dimensionOfVector; i++) {
		// Show plot if enabled
		if (enable[i]) {
			// Create the plot - Variable size
			std::string text = std::string(pheferialName) + std::to_string(i) + " value:" + std::to_string(lineChart[i].back());
			float minPlotSize = *min_element(lineChart[i].begin(), lineChart[i].end());
			float maxPlotSize = *max_element(lineChart[i].begin(), lineChart[i].end());
			ImGui::PlotLines(text.data(), lineChart[i].data(), lineChart[i].size(), 0, "", minPlotSize, maxPlotSize, ImVec2(0, 100.0f));

			// Slider for control at a new line
			if (isControlOutput) {
				text = "Control output for: " + std::string(pheferialName) + std::to_string(i);
				ImGui::SliderFloat(text.c_str(), &slider[i], minValue[i], maxValue[i]);
			}
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

