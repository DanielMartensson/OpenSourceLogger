#include "CalibrationDialog.h"
#include "imgui.h"
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
#include "../../../Constants.h"

#define CALIBRATION_FIELDS_JSON "Windows/Dialogs/CalibrationDialog/calibrationFields.json"

int createTable(const char collapsingHeaderText[], const char tableName[], const char pheferialName[], float minArray[], float maxArray[], int sizeOfArray, const int popIndexBegin);

std::array<float, ADC_LENGTH> adcMin;
std::array<float, ADC_LENGTH> adcMax;
std::array<float, PWM_LENGTH> pwmMin;
std::array<float, PWM_LENGTH> pwmMax;
std::array<float, DAC_LENGTH> dacMin;
std::array<float, DAC_LENGTH> dacMax;
std::array<float, DADC_LENGTH> dadcMin;
std::array<float, DADC_LENGTH> dadcMax;
std::array<float, DI_LENGTH> diMin;
std::array<float, DI_LENGTH> diMax;
std::array<float, IC_LENGTH> icMin;
std::array<float, IC_LENGTH> icMax;
std::array<float, E_LENGTH> eMin;
std::array<float, E_LENGTH> eMax;

void showCalibrationDialog(bool* calibrateMeasurements) {
	// Read only once
	static bool fieldsHasBeenLoaded = false;
	if (!fieldsHasBeenLoaded) {
		std::ifstream calibrationFields;
		calibrationFields.open(CALIBRATION_FIELDS_JSON);
		if (calibrationFields.is_open()) {
			// Read json
			std::string jsonLine;
			std::getline(calibrationFields, jsonLine);
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
		calibrationFields.close();

		// Lock
		fieldsHasBeenLoaded = true;
	}

	// Show calibration dialog
	ImGui::Begin("Calibrate measurement", calibrateMeasurements);
	int popIndex = 0;
	popIndex = createTable("Analog to Digital Converter(ADC)", "tableADC", "ADC", adcMin.data(), adcMax.data(), adcMax.size(), popIndex);
	popIndex = createTable("Pulse Width Modulation(PWM)", "tablePWM", "PWM", pwmMin.data(), pwmMax.data(), pwmMax.size(), popIndex);
	popIndex = createTable("Digital to Analog Converter(DAC)", "tableDAC", "DAC", dacMin.data(), dacMax.data(), dacMax.size(), popIndex);
	popIndex = createTable("Differential Analog to Digital Converter(DADC)", "tableDADC", "DADC", dadcMin.data(), dadcMax.data(), dadcMin.size(), popIndex);
	popIndex = createTable("Digital Input(DI)", "tableDI", "DI", diMin.data(), diMax.data(), diMax.size(), popIndex);
	popIndex = createTable("Input Capture(IC)", "tableIC", "IC", icMin.data(), icMax.data(), icMax.size(), popIndex);
	popIndex = createTable("Encoder(E)", "tableE", "E", eMin.data(), eMax.data(), eMax.size(), popIndex);
	ImGui::End();

	// When we close the window, then *calibrateMeasurements will be false
	if (*calibrateMeasurements == false) {
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
		std::ofstream calibrationFields;
		calibrationFields.open(CALIBRATION_FIELDS_JSON);
		if (calibrationFields.is_open()) {
			calibrationFields << json;
		}
		calibrationFields.close();

		// Lock
		fieldsHasBeenLoaded = false;
	}
}

int createTable(const char collapsingHeaderText[], const char tableName[], const char pheferialName[], float minArray[], float maxArray[], int sizeOfArray, const int popIndexBegin) {
	if (ImGui::CollapsingHeader(collapsingHeaderText)) {
		if (ImGui::BeginTable(tableName, 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
			ImGui::TableSetupColumn(pheferialName);
			ImGui::TableSetupColumn("Min");
			ImGui::TableSetupColumn("Max");
			ImGui::TableHeadersRow();
			char text[20];
			int popIndex = popIndexBegin;
			for (int i = 0; i < sizeOfArray; i++) {
				// Index column
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				sprintf_s(text, "%i", i);
				ImGui::Text(text);

				// Min column
				ImGui::TableSetColumnIndex(1);
				ImGui::PushID(popIndex++);
				ImGui::InputFloat("", &minArray[i]);
				ImGui::PopID();

				// Max column
				ImGui::TableSetColumnIndex(2);
				ImGui::PushID(popIndex++);
				ImGui::InputFloat("", &maxArray[i]);
				ImGui::PopID();
			}
			ImGui::EndTable();
			return popIndex;
		}
	}
	return 0;
}