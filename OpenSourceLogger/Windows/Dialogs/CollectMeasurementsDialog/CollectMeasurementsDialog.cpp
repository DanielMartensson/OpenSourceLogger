#include "CollectMeasurementsDialog.h"
#include <imgui.h>
#include "../../../Hardware/USB/USBHandler.h"
#include "../../../Constants.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <fstream>

#define PLOT_SETTING_FIELDS_JSON "Windows/Dialogs/CollectMeasurementsDialog/plotSettingFields.json"

void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray);
int createPlots(std::vector<float> vector[], const char pheferialName[], bool enable[], float minValue[], float maxValue[], int dimensionOfVector, int popIndexBegin);
void updateVectors(std::vector<float> vector[], int showSamplesInPlot, int dimensionOfVector, float inputValue);

void showCollectMeasurementsDialog(bool* collectMeasurements, char file_folder_path[]) {

	ImGui::Begin("Collect measurements", collectMeasurements);

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

	// Settings fields for plot
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
	if (!isConnectedToUSB() && logFileExist) {
		ImGui::BeginChild("groupBoxEnable", ImVec2(0, ImGui::GetFontSize() * 15.0f), true);

		// Create checkboxes
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
			if (logFile.is_open()) {
				// Write the first header for the log file when it just has been open
				std::string header = "time_stamp,";
				for (int i = 0; i < ADC_LENGTH; i++) {
					header += std::string("adc") + std::to_string(i) + std::string(", "); // Analog to digital converter
				}
				for (int i = 0; i < PWM_LENGTH; i++) {
					header += std::string("pwm") + std::to_string(i) + std::string(", "); // Pulse widt modulation
				}
				for (int i = 0; i < DAC_LENGTH; i++) {
					header += std::string("dac") + std::to_string(i) + std::string(", "); // Digital to analog converter
				}
				for (int i = 0; i < DADC_LENGTH; i++) {
					header += std::string("dadc") + std::to_string(i) + std::string(", "); // Differential ADC
				}
				for (int i = 0; i < DI_LENGTH; i++) {
					header += std::string("di") + std::to_string(i) + std::string(", "); // Digital input
				}
				for (int i = 0; i < IC_LENGTH; i++) {
					header += std::string("ic") + std::to_string(i) + std::string(", "); // Input capture
				}
				for (int i = 0; i < E_LENGTH - 1; i++) {
					header += std::string("e") + std::to_string(i) + std::string(", "); // Encoder
				}
				header += "e2\n";
				logFile << header;
			}else {
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
		ImGui::InputScalar("Sample time for logging", ImGuiDataType_U16, &sampleTime, &step);
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

			// TODO: Skicka PWM och DAC till STM32 PLC

			// Update the sampling
			if (sampleTime <= timeDifferenceSum) {
				// Check if samples is above 0
				if (showSamplesInPlot > 0) {

					// Get the measurements data
					uint16_t adcRaw[ADC_LENGTH];
					uint16_t dadcRaw[DADC_LENGTH];
					uint8_t diRaw[DI_LENGTH];
					uint16_t icRaw[IC_LENGTH];
					uint16_t eRaw[E_LENGTH];
					getRawMeasurements(adcRaw, dadcRaw, diRaw, icRaw, eRaw);

					// Update vectors
					updateVectors(ADC, showSamplesInPlot, ADC_LENGTH, showSamplesInPlot);
					updateVectors(DADC, showSamplesInPlot, DADC_LENGTH, showSamplesInPlot);
					updateVectors(DAC, showSamplesInPlot, DAC_LENGTH, showSamplesInPlot);
					updateVectors(PWM, showSamplesInPlot, PWM_LENGTH, showSamplesInPlot);
					updateVectors(DI, showSamplesInPlot, DI_LENGTH, showSamplesInPlot);
					updateVectors(IC, showSamplesInPlot, IC_LENGTH, showSamplesInPlot);
					updateVectors(E, showSamplesInPlot, E_LENGTH, showSamplesInPlot);
				}

				// Reset the time difference sum
				timeDifferenceSum = 0;
			}

			// Show plots
			ImGui::BeginChild("groupBoxPlots", ImVec2(0, 0), true);
			int popIndex = 0;
			popIndex = createPlots(ADC, "ADC", enableADC, adcMin.data(), adcMax.data(), ADC_LENGTH, popIndex);
			popIndex = createPlots(DADC, "DADC", enableDADC, dadcMin.data(), dadcMax.data(), DADC_LENGTH, popIndex);
			popIndex = createPlots(DAC, "DAC", enableDAC, dacMin.data(), dacMax.data(), DAC_LENGTH, popIndex);
			popIndex = createPlots(PWM, "PWM", enablePWM, pwmMin.data(), pwmMax.data(), PWM_LENGTH, popIndex);
			popIndex = createPlots(DI, "DI", enableDI, diMin.data(), diMax.data(), DI_LENGTH, popIndex);
			popIndex = createPlots(IC, "IC", enableIC, icMin.data(), icMax.data(), IC_LENGTH, popIndex);
			popIndex = createPlots(E, "E", enableE, eMin.data(), eMax.data(), E_LENGTH, popIndex);
			ImGui::EndChild();

			// TODO: 			
			logFile << std::to_string(showSamplesInPlot) + "\n";
	
		}else {
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
	else if (isConnectedToUSB() && logFileExist) {
		ImGui::Text("You need to select a measurement file");
		if (ImGui::Button("Close")) {
			*collectMeasurements = false;
		}
	}else if (!isConnectedToUSB() && logFileExist) {
		ImGui::Text("You need to be connected to the USB");
		if (ImGui::Button("Close")) {
			*collectMeasurements = false;
		}
	}else {
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

void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray) {
	for (int i = 0; i < lengthOfArray; i++) {
		std::string text = std::string(pheferialName) + std::to_string(i);
		ImGui::Checkbox(text.data(), &enable[i]);
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

int createPlots(std::vector<float> vector[], const char pheferialName[], bool enable[], float minValue[], float maxValue[], int dimensionOfVector, int popIndexBegin) {
	int popIndex = popIndexBegin;
	for (int i = 0; i < dimensionOfVector; i++) {
		// Show plot if enabled
		if (enable[i]) {

			// Create the plot
			std::string text = std::string(pheferialName) + std::to_string(i);
			ImGui::PlotLines(text.data(), vector[i].data(), vector[i].size(), 0, "", minValue[i], maxValue[i], ImVec2(0, 100.0f));
			
			// Create table
			ImGui::SameLine();
			ImGui::BeginTable("Min & Max", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_NoHostExtendX);
			ImGui::TableSetupColumn("Min");
			ImGui::TableSetupColumn("Max");
			ImGui::TableHeadersRow();
			ImGui::TableNextRow();

			// Fill the cells for min
			ImGui::TableSetColumnIndex(0);
			ImGui::PushID(popIndex++);
			ImGui::InputFloat("", &minValue[i]);
			ImGui::PopID();

			// Fill the cells for max
			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(popIndex++);
			ImGui::InputFloat("", &maxValue[i]);
			ImGui::PopID();
			ImGui::EndTable();
			
		}
	}

	// This is for the input fields
	return popIndex;
}

void updateVectors(std::vector<float> vector[], int showSamplesInPlot, int dimensionOfVector, float inputValue) {
	for (int i = 0; i < dimensionOfVector; i++) {
		// Compute the difference
		int difference = vector[i].size() - showSamplesInPlot;

		// If we have to much vector data compared to samples, then delete some
		if (difference >= 0) {
			for (int j = 0; j <= difference; j++) {
				// Delete one element
				vector[i].erase(vector[i].begin());

				// Update the difference
				difference = vector[i].size() - showSamplesInPlot;
			}
		}
		
		// Add a new sample
		vector[i].push_back(inputValue);
	}
}